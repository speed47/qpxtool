#!/usr/bin/env python3
"""Package with create-dmg, cleaning up only this invocation's disk images."""
import os
from pathlib import Path
import plistlib
import re
import shutil
import signal
import subprocess
import sys
import tempfile
import time


def attached_images(workdir):
    info = subprocess.run(["hdiutil", "info", "-plist"], check=True,
                          stdout=subprocess.PIPE, timeout=30)
    images = plistlib.loads(info.stdout).get("images", [])
    # Resolve /var vs /private/var, and compare path components, not prefixes.
    return [image for image in images
            if image.get("image-path") and
            workdir.resolve() in Path(image["image-path"]).resolve().parents]


def diagnostic(command):
    print("Diagnostic:", command, flush=True)
    try:
        subprocess.run(command, timeout=20, check=False)
    except (OSError, subprocess.TimeoutExpired) as error:
        print("Diagnostic unavailable:", error, flush=True)


def cleanup(workdir):
    for image in attached_images(workdir):
        print("Cleaning image:", image["image-path"], flush=True)
        entities = image.get("system-entities", [])
        for entity in entities:
            mount = entity.get("mount-point")
            if mount:
                diagnostic(["lsof", "+f", "--", mount])
        # The whole-disk entity precedes the mounted partition in hdiutil's plist.
        device = next((e.get("dev-entry") for e in entities
                       if re.fullmatch(r"/dev/disk\d+", e.get("dev-entry", ""))), None)
        if not device:
            raise RuntimeError("Cannot identify whole disk for " + image["image-path"])
        try:
            result = subprocess.run(["hdiutil", "detach", device], timeout=30)
            detached = result.returncode == 0
        except subprocess.TimeoutExpired:
            detached = False
        if not detached:
            print("Normal detach failed; forcing detach of", device, flush=True)
            subprocess.run(["hdiutil", "detach", "-force", device],
                           check=True, timeout=30)
    if attached_images(workdir):
        raise RuntimeError("Temporary image still attached; preserving " + str(workdir))


def create_image(output, source, workdir, log):
    # Finder cosmetics can trigger Launch Services (lsd) to hold the app binary
    # open on the temporary volume, preventing even forced detach on CI runners.
    command = ["create-dmg", "--skip-jenkins", "--hdiutil-retries", "5", str(output), str(source)]
    # A separate process group lets a timeout or cancellation stop children before cleanup.
    with log.open("wb") as stream:
        process = subprocess.Popen(command, cwd=workdir, stdout=stream,
                                   stderr=subprocess.STDOUT, start_new_session=True)
        try:
            return process.wait(timeout=600)
        finally:
            if process.poll() is None:
                os.killpg(process.pid, signal.SIGTERM)
                try:
                    process.wait(timeout=10)
                except subprocess.TimeoutExpired:
                    os.killpg(process.pid, signal.SIGKILL)
                    process.wait()
            print(log.read_text(errors="replace"), flush=True)


def package(archive, source):
    archive, source = archive.resolve(), source.resolve()
    if archive.exists():
        raise RuntimeError("Refusing to replace existing archive: " + str(archive))
    # Keep the bundle name stable inside the image; only its parent is temporary.
    workdir = Path(tempfile.mkdtemp(prefix="qpxtool-dmg-"))
    logs = archive.parent / (archive.name + ".logs")
    logs.mkdir(exist_ok=True)
    print("Temporary images:", workdir, "Logs:", logs, flush=True)
    try:
        for attempt in range(1, 4):
            print("DMG packaging attempt", attempt, "of 3", flush=True)
            output = workdir / archive.name
            log = logs / ("attempt-%d.log" % attempt)
            result = create_image(output, source, workdir, log)
            if result:
                diagnostic(["hdiutil", "info"])
                diagnostic(["df", "-h"])
            cleanup(workdir)
            if result == 0:
                subprocess.run(["hdiutil", "verify", str(output)], check=True, timeout=120)
                output.replace(archive)
                return
            if attempt == 3 or "resource busy" not in log.read_text(errors="replace").lower():
                raise RuntimeError("create-dmg failed with exit code %d; see %s" % (result, log))
            # Cleanup has confirmed no images from this directory remain attached.
            for image in workdir.glob("*.dmg"):
                image.unlink()
            time.sleep(attempt * 5)
    finally:
        # Also covers errors, timeouts, and cancellation. Never unlink a mounted image.
        cleanup(workdir)
        shutil.rmtree(workdir)


def interrupted(signum, frame):
    raise KeyboardInterrupt("Interrupted by signal %d" % signum)


if __name__ == "__main__":
    signal.signal(signal.SIGTERM, interrupted)
    try:
        package(Path(sys.argv[1]), Path(sys.argv[2]))
    except (OSError, RuntimeError, subprocess.SubprocessError, KeyboardInterrupt) as error:
        print("DMG packaging failed:", error, file=sys.stderr)
        sys.exit(1)
