#!/bin/sh

VERSION=$(git describe --tags 2> /dev/null)
[ -z "${VERSION}" ] && { echo "ERROR: Cannot determine version."; exit 1; }

FILES=$(git ls-files | grep -v .gitignore)
[ -z "${FILES}" ] && { echo "ERROR: No files found."; exit 1; }

DIRNAME="$(basename $(pwd))"
ZIPNAME="${DIRNAME}-${VERSION}"

mkdir release

TDIR=/tmp/$DIRNAME
[ -d "${TDIR}" ] && { echo "ERROR: Directory '${TDIR}' exists. Please remove."; exit 1; }

mkdir -p ${TDIR}
git ls-files |
grep -v .gitignore |
cpio -pmud ${TDIR}/
(cd /tmp && zip -r $ZIPNAME.zip $DIRNAME)
mv /tmp/$ZIPNAME.zip ./release
rm -rf ${TDIR}
