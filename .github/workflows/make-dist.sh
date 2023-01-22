#!/bin/bash
set -e
set -x

github_ref="$1"

case "$MSYSTEM" in
    MINGW64) os=win64;;
    MINGW32) os=win32;;
    *)       os=unknown;;
esac

if [ ! -e "gui/release/qpxtool.exe" ] && [ -e "gui/debug/qpxtool.exe" ]; then
    flavor=debug
else
    flavor=release
fi

archive=qpxtool-$(echo "$github_ref" | grep -Eo '[^/]+$')-$os-portable-$flavor.zip
echo "Archive name is $archive"
[ -n "$GITHUB_OUTPUT" ] && echo "archive=$archive" >> $GITHUB_OUTPUT

mkdir dist
for i in $(ntldd -R gui/$flavor/qpxtool.exe  | awk '/mingw/ {print $3}' | tr \\\\ / | grep -Eo '[^/]+$'); do cp -va $MINGW_PREFIX/bin/$i dist/; done
cp -va gui/$flavor/qpxtool.exe console/*/*.exe dist/
find lib/lib -name "*.dll" | xargs cp -vat dist/
mkdir -p dist/share/qt5/plugins/platforms
cp -va $MINGW_PREFIX/share/qt5/plugins/platforms/* dist/share/qt5/plugins/platforms/
mkdir -p dist/plugins
cp -va plugins/lib/libqscan_*.dll dist/plugins/

if command -v zip >/dev/null; then
    mv dist ${archive/.zip/}
    zip -9r $archive ${archive/.zip/}
    mv ${archive/.zip/} dist
fi
echo "dist done ($archive)"
