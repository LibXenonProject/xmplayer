#!/bin/sh

test "$1" && extra="-$1"

# releases extract the version number from the VERSION file
version=$(git log -1 --format="%h")


NEW_REVISION="#define VERSION \"${version}${extra}\""
OLD_REVISION=$(head -n 1 version.h 2> /dev/null)
TITLE='#define MP_TITLE "%s "VERSION" (C) 2000-2012 MPlayer Team\n"'

# Update version.h only on revision changes to avoid spurious rebuilds
if test "$NEW_REVISION" != "$OLD_REVISION"; then
    cat <<EOF > version.h
$NEW_REVISION
$TITLE
EOF
fi
