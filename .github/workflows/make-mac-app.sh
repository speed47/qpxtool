#!/bin/bash
# Build a self-contained qpxtool.app bundle and wrap it in a .dmg.
#
# Usage: make-mac-app.sh <version> <arch> <staging-prefix>
#   <version>        e.g. 0.8.1-pl7 (no leading "v")
#   <arch>           x86_64 | arm64
#   <staging-prefix> directory where "make install DESTDIR=... prefix=/usr"
#                    placed the tree, i.e. it contains usr/bin, usr/lib, ...
#
# Qt frameworks (and other Homebrew dylibs) are bundled via macdeployqt; the
# qpxtool libraries and scan plugins are bundled and relinked by hand so the
# app runs on a machine that has neither qpxtool nor Homebrew installed.
set -e
set -x

version="$1"
arch="${2:-$(uname -m)}"
staging="${3:-dist}"

archive="qpxtool-v${version}-macos-${arch}.dmg"
[ -n "$GITHUB_OUTPUT" ] && echo "archive=$archive" >> "$GITHUB_OUTPUT"
echo "mac dmg is <$archive>"

BREW_PREFIX="$(brew --prefix)"
APP="qpxtool.app"
CONTENTS="$APP/Contents"

rm -rf "$APP" dist-mac
mkdir -p "$CONTENTS/MacOS" "$CONTENTS/Resources" "$CONTENTS/libs" \
         "$CONTENTS/Resources/usr/lib/qpxtool"

# --- main GUI binary -------------------------------------------------------
cp "$staging/usr/bin/qpxtool" "$CONTENTS/MacOS/qpxtool"
chmod +x "$CONTENTS/MacOS/qpxtool"

# Optional CLI helpers, bundled alongside the GUI.
for tool in qscan qscand cdvdcontrol readdvd pxfw f1tattoo; do
  [ -f "$staging/usr/bin/$tool" ] && cp "$staging/usr/bin/$tool" "$CONTENTS/MacOS/$tool"
done

# --- Info.plist ------------------------------------------------------------
cat > "$CONTENTS/Info.plist" << EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleName</key>            <string>QPxTool</string>
  <key>CFBundleDisplayName</key>     <string>QPxTool</string>
  <key>CFBundleIdentifier</key>      <string>org.qpxtool.QPxTool</string>
  <key>CFBundleVersion</key>         <string>${version}</string>
  <key>CFBundleShortVersionString</key><string>${version}</string>
  <key>CFBundleExecutable</key>      <string>qpxtool</string>
  <key>CFBundleIconFile</key>        <string>qpxtool.icns</string>
  <key>CFBundlePackageType</key>     <string>APPL</string>
  <key>NSHighResolutionCapable</key> <true/>
  <key>LSMinimumSystemVersion</key>  <string>11.0</string>
</dict>
</plist>
EOF

# --- application icon (best effort) ----------------------------------------
if [ -f gui/qpxtool.png ]; then
  iconset="$(mktemp -d)/qpxtool.iconset"
  mkdir -p "$iconset"
  for sz in 16 32 64 128 256 512; do
    sips -z $sz $sz       gui/qpxtool.png --out "$iconset/icon_${sz}x${sz}.png"     >/dev/null 2>&1 || true
    sips -z $((sz*2)) $((sz*2)) gui/qpxtool.png --out "$iconset/icon_${sz}x${sz}@2x.png" >/dev/null 2>&1 || true
  done
  iconutil -c icns "$iconset" -o "$CONTENTS/Resources/qpxtool.icns" 2>/dev/null || true
fi

# --- bundle Qt frameworks & other Homebrew deps via macdeployqt ------------
# macdeployqt rewrites the GUI binary's Qt references to @executable_path/../Frameworks.
macdeployqt "$APP" -verbose=1 -no-strip

# --- relink qpxtool's own libraries & plugins ------------------------------
# Any reference resolving to one of these prefixes is one we bundle ourselves.
staging_libdir="$(cd "$staging/usr/lib" && pwd -P)"

# Rewrite every qpxtool/@rpath dependency of $1 to @executable_path/../libs,
# copying the referenced library into Contents/libs the first time we see it.
rewrite_deps() {
  local target="$1"
  local dep libname src
  for dep in $(otool -L "$target" | tail -n +2 | awk '{print $1}'); do
    case "$dep" in
      */libqpx*.dylib|"$staging_libdir"/*|@rpath/libqpx*)
        libname="$(basename "$dep")"
        if [ ! -f "$CONTENTS/libs/$libname" ]; then
          # Resolve the real file: the staging libdir holds the actual dylibs.
          src=""
          [ -f "$staging_libdir/$libname" ] && src="$staging_libdir/$libname"
          [ -z "$src" ] && [ -f "$dep" ] && src="$dep"
          if [ -n "$src" ]; then
            echo "Bundling lib: $libname from $src"
            cp "$src" "$CONTENTS/libs/$libname"
            chmod +w "$CONTENTS/libs/$libname"
            install_name_tool -id "@executable_path/../libs/$libname" "$CONTENTS/libs/$libname"
            # Recurse so transitive qpxtool deps get bundled & rewritten too.
            rewrite_deps "$CONTENTS/libs/$libname"
          else
            echo "WARNING: could not locate $libname (dep $dep of $target)"
            continue
          fi
        fi
        install_name_tool -change "$dep" "@executable_path/../libs/$libname" "$target"
        ;;
    esac
  done
}

rewrite_deps "$CONTENTS/MacOS/qpxtool"
for tool in qscan qscand cdvdcontrol readdvd pxfw f1tattoo; do
  [ -f "$CONTENTS/MacOS/$tool" ] && rewrite_deps "$CONTENTS/MacOS/$tool"
done

# Scan plugins go where the APPDIR-based loader looks ($APPDIR/usr/lib/qpxtool).
for plug in "$staging/usr/lib/qpxtool/"libqscan_*; do
  [ -f "$plug" ] || continue
  dst="$CONTENTS/Resources/usr/lib/qpxtool/$(basename "$plug")"
  cp "$plug" "$dst"
  chmod +w "$dst"
  rewrite_deps "$dst"
done

# --- locale & docs ---------------------------------------------------------
if [ -d "$staging/usr/share/qpxtool/locale" ]; then
  mkdir -p "$CONTENTS/Resources/share/qpxtool/locale"
  cp "$staging/usr/share/qpxtool/locale/"*.qm "$CONTENTS/Resources/share/qpxtool/locale/" 2>/dev/null || true
fi
mkdir -p "$CONTENTS/Resources/documentation"
cp COPYING README README.md ChangeLog AUTHORS "$CONTENTS/Resources/documentation/" 2>/dev/null || true

# --- launcher --------------------------------------------------------------
# Wrap the real binary so we can point the plugin loader at the bundled
# plugins via APPDIR (the same env var honoured by the AppImage build).
mv "$CONTENTS/MacOS/qpxtool" "$CONTENTS/MacOS/qpxtool.bin"
cat > "$CONTENTS/MacOS/qpxtool" << 'EOF'
#!/bin/bash
DIR="$(cd "$(dirname "$0")/../.." && pwd)"
# $APPDIR/usr/lib/qpxtool is searched by qpxtool's plugin loader.
export APPDIR="$DIR/Contents/Resources"
exec "$DIR/Contents/MacOS/qpxtool.bin" "$@"
EOF
chmod +x "$CONTENTS/MacOS/qpxtool" "$CONTENTS/MacOS/qpxtool.bin"

# --- ad-hoc re-sign --------------------------------------------------------
# install_name_tool invalidates signatures; on Apple Silicon unsigned code
# can't be loaded, so re-sign everything we touched (and finally the bundle).
find "$CONTENTS/libs" "$CONTENTS/Resources/usr/lib/qpxtool" -type f \
  \( -name '*.dylib' -o -name 'libqscan_*' \) -print0 2>/dev/null |
  while IFS= read -r -d '' f; do codesign --force --sign - "$f"; done
codesign --force --sign - "$CONTENTS/MacOS/qpxtool.bin"
codesign --force --deep --sign - "$APP" || true

# --- package as .dmg -------------------------------------------------------
mkdir -p dist-mac
mv "$APP" dist-mac/
create-dmg "$archive" dist-mac

echo "dist done ($archive)"
