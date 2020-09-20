#! /bin/bash
set -e
set -x

github_ref="$1"

if [ -n "$GITHUB_EVENT_PATH" ] && [ -f "$GITHUB_EVENT_PATH" ]; then
    if command -v jq >/dev/null; then
        upload_url=$(jq -r '.release.upload_url' < $GITHUB_EVENT_PATH)
        echo "Upload URL is $upload_url"
        echo "::set-output name=upload_url::$upload_url"
    fi
else
    echo "This should only be run from GitHub Actions"
    exit 1
fi

case "$MSYSTEM" in
    MINGW64) os=win64; exe=.exe;;
    MINGW32) os=win32; exe=.exe;;
    *)       os=linux64; exe='';;
esac

if [ ! -e "gui/release/qpxtool.exe" ] && [ -e "gui/debug/qpxtool.exe" ]; then
	flavor=debug
else
	flavor=release
fi

archive=qpxtool-$(echo "$github_ref" | grep -Eo '[^/]+$')-$os-$flavor.zip
echo "Archive name is $archive"
echo "::set-output name=archive::$archive"

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
