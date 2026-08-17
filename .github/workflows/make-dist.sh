#!/bin/bash
set -e
set -x

version="$1"

case "$MSYSTEM" in
    UCRT64|MINGW64) os=win64;;
    MINGW32) os=win32;;
    *)       os=unknown;;
esac

if [ ! -e "gui/release/qpxtool.exe" ] && [ -e "gui/debug/qpxtool.exe" ]; then
    suffix=debug
    flavor=debug
else
    suffix=""
    flavor=release
fi

archive=qpxtool-v$version-$os-portable$suffix.zip
echo "Archive name is $archive"
[ -n "$GITHUB_OUTPUT" ] && echo "archive=$archive" >> $GITHUB_OUTPUT

mkdir dist
# ntldd prints Windows paths. Match the active environment rather than a
# hard-coded mingw directory, excluding Windows system DLLs.
for i in $(ntldd -R gui/$flavor/qpxtool.exe | tr '\\' / |
    awk -v prefix="$MINGW_PREFIX/bin/" 'index(tolower($3), tolower(prefix)) {n = split($3, parts, "/"); print parts[n]}'); do
    cp -va "$MINGW_PREFIX/bin/$i" dist/
done
cp -va gui/$flavor/qpxtool.exe console/*/*.exe dist/
find lib/lib -name "*.dll" | xargs cp -vat dist/
cp -va COPYING THIRD_PARTY_NOTICES dist/
mkdir -p dist/share/qt6/plugins/platforms
cp -va $MINGW_PREFIX/share/qt6/plugins/platforms/* dist/share/qt6/plugins/platforms/
mkdir -p dist/plugins
cp -va plugins/lib/libqscan_*.dll dist/plugins/

if command -v zip >/dev/null; then
    mv dist ${archive/.zip/}
    zip -9r $archive ${archive/.zip/}
    mv ${archive/.zip/} dist
fi
echo "dist done ($archive)"
