# Workflow dependencies

Actions use full stable version tags in the workflows. Their exact commits and
repository identities are recorded in `.github/workflows/actions.lock`, generated
by GitHub's [actions-lock extension](https://github.com/github/gh-actions-lock).
`mknejp/delete-release-assets` publishes no tags or releases, so its maintained
`v1` branch is retained and pinned to a commit in the lockfile.

After adding a workflow or changing an action reference, run from the repository
root with an authenticated GitHub CLI:

```sh
gh extension install github/gh-actions-lock --pin v0.4.1
gh actions-lock
gh actions-lock --verify
```

If the extension is already installed, use `gh extension upgrade gh-actions-lock`
to update it. Commit the workflow changes and generated lockfile together; never
edit the lockfile by hand. Review changed commits and investigate any changes to
`owner_id` or `repo_id`. Use `gh actions-lock --relock` to advance a branch pin
only after reviewing its upstream changes. `gh actions-lock --verify-local`
provides an offline coverage check.

The `actions-lock` job in `check.yml` verifies coverage and upstream provenance
on pushes and pull requests. Dependabot checks action versions weekly and updates
the matching lockfile entries for onboarded workflows.

## Token permissions

All workflows explicitly scope `GITHUB_TOKEN`; unspecified permissions are
disabled. The check workflow grants only `contents: read`. Other workflows default
to `permissions: {}` and grant permissions to individual jobs:

| Jobs | Permissions | Purpose |
| --- | --- | --- |
| Checks, including lockfile verification | `contents: read` | Checkout and dependency verification |
| Release builds | `contents: write` | Checkout, push tags, and manage releases and assets |
| CodeQL analysis | `contents: read`, `actions: read`, `security-events: write` | Checkout, workflow metadata, and code-scanning results |
| Stale issues and PRs | `issues: write`, `pull-requests: write`, `actions: write` | Label and close issues/PRs, and replace the saved state cache |

The stale action does not delete branches, so it does not need `contents: write`.

## CodeQL coverage

The CodeQL workflow scans C/C++ source and GitHub Actions workflows in separate
matrix jobs on pushes and pull requests. Both use the `security-extended` query
suite. C/C++ uses a manual build with the existing Qt setup; Actions uses
`build-mode: none` and skips the application setup and build steps. Results use
separate language categories in code scanning.

## Compatibility notes

The November 2026 update was checked against the upstream release notes:

- [Checkout v7](https://github.com/actions/checkout/tree/v7.0.1) restricts fork
  checkouts on `pull_request_target` and `workflow_run`; these workflows use
  `push` and `pull_request`. Its v6 credential storage change still supports
  authenticated `git push`, which the release jobs run on the host.
- [Stale v11](https://github.com/actions/stale/tree/v11.0.0) changes its internal
  module format without changing inputs, outputs, or behavior.
- [Release v3](https://github.com/softprops/action-gh-release/releases/tag/v3.0.0)
  moves to Node 24, already required by checkout v5 and stale v10 and supported
  by the GitHub-hosted runners used here.
- [CodeQL v4.39.3](https://github.com/github/codeql-action/blob/v4.39.3/CHANGELOG.md)
  supports the existing C++ analysis inputs. The workflow uses its default
  CodeQL bundle and checkout path, so the newer minimum bundle requirement does
  not require configuration changes.
- [MSYS2 v2.33.0](https://github.com/msys2/setup-msys2/releases/tag/v2.33.0)
  runs the Windows checks and releases in UCRT64 with GCC and Qt 6 packages
  from the UCRT64 repository. Release packaging uses `MINGW_PREFIX` to find
  runtime DLLs. The workflows do not rely on the removed default Perl installation.

### Source suppressions

The C/C++ job also runs `codeql/cpp-queries:AlertSuppression.ql`. Place a
`// codeql[rule-id]` comment with a justification immediately before the flagged
line to mark an intentional finding. Comments apply only to the next line;
keep the rule enabled for all other code.

The `advanced-security/dismiss-alerts` action, pinned through `actions.lock`, consumes the local
SARIF and dismisses matching alerts as "won't fix" only on pushes to the
repository's default branch. Pull requests and other branches generate the
metadata but do not change dismissals, which are repository-wide. Removing a
comment allows the action to reopen an alert it previously dismissed; manual
dismissals are preserved. The action's SHA and repository identity are also
recorded in `actions.lock`.

The four output-file permission suppressions intentionally retain the caller's
umask for ordinary disc images, sector maps, EEPROM dumps, and strategy exports.

Ten ordinary file-path findings are also suppressed: these local CLI operations
intentionally accept caller-selected input and output paths, including absolute
paths and parent-directory components. Calls with both intentional file creation
and path selection list both rule IDs in the same suppression comment.

The shared device-opening path (`cpp/path-injection`, alert #20) remains
unsuppressed. `qscand` accepts a device path from network clients and forwards it
to `qscan`; it currently checks its length, not authorization for the requested
device. The daemon defaults to listening on all interfaces. Any future device
allowlist should be enforced at the daemon request boundary, preserving arbitrary
device selection for local CLI callers. A `/dev/` prefix alone is not device
authorization.

## macOS disk-image recovery

`make-mac-app.sh` calls `package-mac-dmg.py` to run `create-dmg` in a private
temporary directory. It uses `--skip-jenkins` to avoid Finder's cosmetic
AppleScript step: Launch Services (`lsd`) has been observed holding the bundled
executable open on the temporary volume, blocking normal and forced detach.
Finder window and icon positioning therefore use defaults. The app contents
are unchanged. It makes at most three attempts, retrying only failures
whose logs contain "Resource busy", with five- and ten-second delays. Each
attempt has a ten-minute timeout; a timed-out or interrupted process group is
stopped before cleanup. Other failures stop immediately.

Failed attempts print the packaging log, disk-image state, free space, and
`lsof` output for the owned mounted volumes. Per-attempt logs remain beside the
archive in `<archive>.logs`. Cleanup identifies owned images using
`hdiutil info -plist` and their resolved paths, tries normal detach, then force
detach for those images only. It never kills Finder or other runner services.
If cleanup cannot confirm all owned images are detached, temporary files remain
for investigation and packaging fails. A successful image passes `hdiutil verify`
before being moved to its release filename. Existing release outputs are not
overwritten.

Recovery tests run in the lint job and can be run locally with
`python3 -m unittest discover -s .github/tests -v`. They mock macOS commands;
the release jobs retain the real mount, architecture, and launch checks.
