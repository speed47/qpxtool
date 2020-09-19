#! /bin/bash
set -e
cd "$(dirname "$0")"
rm -rf dist
mkdir dist
for i in $(ntldd -R gui/*/qpxtool.exe  | awk '/mingw/ {print $3}' | tr \\\\ / | grep -Eo '[^/]+$'); do cp -va $MINGW_PREFIX/bin/$i dist/; done
cp -va gui/*/qpxtool.exe console/*/*.exe dist/
find lib/lib -name "*.dll" | xargs cp -vat dist
mkdir -p dist/share/qt5/plugins/platforms
cp -va $MINGW_PREFIX/share/qt5/plugins/platforms/* dist/share/qt5/plugins/platforms/
mkdir -p dist/plugins
cp -va plugins/lib/libqscan_*.dll dist/plugins/
