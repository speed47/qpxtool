"""Device-policy unit tests and opt-in tests of the actual daemon protocol."""
import os
from pathlib import Path
import socket
import subprocess
import sys
import tempfile
import time
import unittest

ROOT = Path(__file__).resolve().parents[2]


@unittest.skipUnless(sys.platform.startswith("linux"), "Linux default device names")
class PolicyTests(unittest.TestCase):
    def test_device_policy(self):
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "policy-test"
            subprocess.run([os.environ.get("CXX", "g++"), "-std=c++17", "-Wall", "-Wextra", "-Werror",
                            "-I" + str(ROOT / "console/qscand"),
                            str(ROOT / ".github/tests/qscand_device_policy_test.cpp"),
                            str(ROOT / "console/qscand/device_policy.cpp"),
                            "-Wl,--wrap=realpath,--wrap=stat", "-o", str(executable)], check=True)
            subprocess.run([str(executable)], check=True)


@unittest.skipUnless(os.environ.get("QSCAND_TEST_BINARY"), "Set QSCAND_TEST_BINARY to test the built daemon")
class DaemonTests(unittest.TestCase):
    def setUp(self):
        self.binary = str(Path(os.environ["QSCAND_TEST_BINARY"]).resolve())
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.directory = Path(self.temporary.name)
        self.invocations = self.directory / "invocations"
        # A fake qscan records argv without touching any real drive.
        fake = self.directory / "qscan"
        fake.write_text('#!/bin/sh\nprintf "%s\\n" "$@" >> "$QSCAND_TEST_INVOCATIONS"\n')
        fake.chmod(0o755)
        self.env = dict(os.environ, PATH=str(self.directory) + os.pathsep + os.environ["PATH"],
                        QSCAND_TEST_INVOCATIONS=str(self.invocations))

    def start(self, *options):
        with socket.socket() as probe:
            probe.bind(("127.0.0.1", 0))
            port = probe.getsockname()[1]
        self.process = subprocess.Popen([self.binary, "-n", "-i", "127.0.0.1", "-p", str(port), *options],
                                        env=self.env, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
        def stop():
            if self.process.poll() is None:
                self.process.terminate()
                try:
                    self.process.wait(timeout=3)
                except subprocess.TimeoutExpired:
                    self.process.kill()
                    self.process.wait()
            self.process.stderr.close()
        self.addCleanup(stop)
        for _ in range(100):
            if self.process.poll() is not None:
                self.fail(self.process.stderr.read().decode())
            try:
                self.connection = socket.create_connection(("127.0.0.1", port), timeout=1)
                break
            except ConnectionRefusedError:
                time.sleep(0.02)
        else:
            self.fail("daemon did not start")
        self.addCleanup(self.connection.close)
        self.read_prompt()

    def read_prompt(self):
        data = b""
        while b"qscand $ " not in data:
            part = self.connection.recv(4096)
            self.assertTrue(part, "daemon disconnected")
            data += part
        return data

    def command(self, command):
        self.connection.sendall(command.encode() + b"\n")
        return self.read_prompt()

    def test_default_denies_paths_and_does_not_spawn(self):
        self.start()
        for path in ["/etc/passwd", "/dev/null", "/dev/sda", "/dev/sr0/../sda", "/dev/sr0x"]:
            self.assertIn(b"not permitted", self.command("set dev=" + path))
            for command in ("dinfo", "minfo", "run"):
                self.assertIn(b"not permitted", self.command(command))
        self.assertFalse(self.invocations.exists())

    def test_exact_list_replaces_defaults_and_denial_clears_selection(self):
        self.start("--allow-device", "/custom/optical device", "--allow-device", "/dev/sg7")
        for path in ["/custom/optical device", "/dev/sg7"]:
            self.assertNotIn(b"not permitted", self.command("set dev=" + path))
            self.command("dinfo")
        self.assertEqual(self.invocations.read_text().splitlines(),
                         ["-d", "/custom/optical device", "-Ip", "-d", "/dev/sg7", "-Ip"])
        self.assertIn(b"not permitted", self.command("set dev=/dev/sr0"))
        self.assertIn(b"not permitted", self.command("dinfo"))
        self.assertEqual(len(self.invocations.read_text().splitlines()), 6)

    def test_embedded_nul_clears_selection(self):
        self.start("--allow-device", "/dev/sr0")
        self.command("set dev=/dev/sr0")
        self.assertIn(b"embedded NUL", self.command("set dev=/dev/sr0\0ignored"))
        self.assertIn(b"not permitted", self.command("dinfo"))
        self.assertFalse(self.invocations.exists())

    def test_unrestricted_override(self):
        self.start("--allow-all-devices")
        self.assertIn(b"WARNING: --allow-all-devices", self.process.stderr.readline())
        self.command("set dev=/legacy/path")
        self.command("dinfo")
        self.assertIn("/legacy/path", self.invocations.read_text())

    def test_invalid_options_fail_before_listening(self):
        for options in [("--allow-device", ""), ("--allow-device", "/dev/sr0", "--allow-all-devices"),
                        ("--allow-device",), ("--allow-all-devices=oops",), ("--unknown",)]:
            result = subprocess.run([self.binary, *options], env=self.env, capture_output=True, timeout=3)
            self.assertNotEqual(result.returncode, 0, options)


if __name__ == "__main__":
    unittest.main()
