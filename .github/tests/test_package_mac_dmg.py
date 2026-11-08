"""Exercise recovery without attaching real disk images."""
import importlib.util
from pathlib import Path
import plistlib
import subprocess
import tempfile
import unittest
from unittest.mock import patch

spec = importlib.util.spec_from_file_location(
    "package_mac_dmg", Path(__file__).parents[1] / "workflows/package-mac-dmg.py")
dmg = importlib.util.module_from_spec(spec)
spec.loader.exec_module(dmg)


class PackagingTests(unittest.TestCase):
    def test_image_selection_does_not_match_neighboring_directory(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            ours = {"image-path": str(root / "rw.123.output.dmg")}
            other = {"image-path": str(root) + "-other/rw.456.output.dmg"}
            data = plistlib.dumps({"images": [ours, other, {"image-path": "/other.dmg"}]})
            with patch.object(dmg.subprocess, "run", return_value=
                              subprocess.CompletedProcess([], 0, stdout=data)):
                self.assertEqual(dmg.attached_images(root), [ours])

    def test_busy_detach_forces_only_owned_whole_disk(self):
        image = {"image-path": "/private/ours/rw.dmg", "system-entities": [
            {"dev-entry": "/dev/disk42"},
            {"dev-entry": "/dev/disk42s1", "mount-point": "/Volumes/ours"}]}
        with patch.object(dmg, "attached_images", side_effect=[[image], []]), \
             patch.object(dmg, "diagnostic") as diagnostic, \
             patch.object(dmg.subprocess, "run", side_effect=[
                 subprocess.CompletedProcess([], 16), subprocess.CompletedProcess([], 0)]) as run:
            dmg.cleanup(Path("/private/ours"))
            self.assertEqual(run.call_args_list[0].args[0], ["hdiutil", "detach", "/dev/disk42"])
            self.assertEqual(run.call_args_list[1].args[0], ["hdiutil", "detach", "-force", "/dev/disk42"])
            diagnostic.assert_called_once_with(["lsof", "+f", "--", "/Volumes/ours"])

    def exercise_package(self, results, fail_cleanup=False):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            workdir = root / "work"
            workdir.mkdir()
            archive = root / "app.dmg"
            calls = []

            def create(output, source, temporary, log):
                result = results[len(calls)]
                calls.append(result)
                log.write_text("Resource busy" if result == 16 else "other error")
                output.write_text("image")
                return result

            with patch.object(dmg.tempfile, "mkdtemp", return_value=str(workdir)), \
                 patch.object(dmg, "create_image", side_effect=create), \
                 patch.object(dmg, "cleanup", side_effect=RuntimeError("still attached")
                              if fail_cleanup else None), \
                 patch.object(dmg, "diagnostic"), patch.object(dmg.time, "sleep"), \
                 patch.object(dmg.subprocess, "run") as run:
                if results[-1] == 0 and not fail_cleanup:
                    dmg.package(archive, root)
                    self.assertTrue(archive.exists())
                    run.assert_called_once_with(["hdiutil", "verify", str(workdir / "app.dmg")],
                                                check=True, timeout=120)
                else:
                    with self.assertRaises(RuntimeError):
                        dmg.package(archive, root)
                    self.assertFalse(archive.exists())
                self.assertEqual(len(calls), 1 if fail_cleanup else len(results))
                self.assertEqual(workdir.exists(), fail_cleanup)

    def test_timeout_stops_process_group_and_prints_log(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            with patch.object(dmg.subprocess, "Popen") as popen, \
                 patch.object(dmg.os, "killpg") as kill:
                process = popen.return_value
                process.pid = 1234
                process.poll.return_value = None
                process.wait.side_effect = [subprocess.TimeoutExpired("create-dmg", 600), 0]
                with self.assertRaises(subprocess.TimeoutExpired):
                    dmg.create_image(root / "app.dmg", root, root, root / "attempt.log")
                kill.assert_called_once_with(1234, dmg.signal.SIGTERM)
                self.assertEqual(process.wait.call_count, 2)

    def test_busy_failure_then_success(self):
        self.exercise_package([16, 0])

    def test_busy_failures_are_bounded(self):
        self.exercise_package([16, 16, 16])

    def test_other_errors_are_not_retried(self):
        self.exercise_package([1])

    def test_failed_cleanup_preserves_image_and_stops_retry(self):
        self.exercise_package([16], fail_cleanup=True)


if __name__ == "__main__":
    unittest.main()
