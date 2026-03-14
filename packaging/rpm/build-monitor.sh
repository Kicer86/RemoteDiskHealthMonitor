#!/bin/sh
#
# Build rdhm-monitor RPM package.
#
# Run from the project root directory:
#   ./packaging/rpm/build-monitor.sh
#
# Prerequisites: rpm-build, cmake, gcc-c++, qt6-qtbase-devel,
#                qt6-qtdeclarative-devel, avahi-devel
#
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
SPEC="$SCRIPT_DIR/rdhm-monitor.spec"

VERSION=$(grep '^Version:' "$SPEC" | awk '{print $2}')
PKG_NAME="rdhm-monitor-${VERSION}"

# Set up rpmbuild tree
RPMBUILD_DIR=$(mktemp -d)
trap 'rm -rf "$RPMBUILD_DIR"' EXIT

mkdir -p "$RPMBUILD_DIR"/{BUILD,RPMS,SOURCES,SPECS,SRPMS}

# Create source tarball
tar czf "$RPMBUILD_DIR/SOURCES/${PKG_NAME}.tar.gz" \
    --transform "s,^,${PKG_NAME}/," \
    -C "$PROJECT_ROOT" \
    --exclude='.git' \
    --exclude='build' \
    .

cp "$SPEC" "$RPMBUILD_DIR/SPECS/"

rpmbuild -bb \
    --define "_topdir $RPMBUILD_DIR" \
    "$RPMBUILD_DIR/SPECS/rdhm-monitor.spec"

echo ""
echo "RPM package(s) built in: $RPMBUILD_DIR/RPMS/"
find "$RPMBUILD_DIR/RPMS/" -name '*.rpm' -print
