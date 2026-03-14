#!/bin/sh
#
# Build rdhm-monitor .deb package.
#
# Run from the project root directory:
#   ./packaging/deb/rdhm-monitor/build.sh
#
# Prerequisites: debhelper, cmake, g++, dpkg-dev, qt6-base-dev,
#                qt6-declarative-dev, libavahi-client-dev
#
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"

BUILD_DIR=$(mktemp -d)
PKG_DIR="$BUILD_DIR/rdhm-monitor-0.2.0"

trap 'rm -rf "$BUILD_DIR"' EXIT

cp -a "$PROJECT_ROOT" "$PKG_DIR"
cp -a "$SCRIPT_DIR/debian" "$PKG_DIR/debian"

cd "$PKG_DIR"
dpkg-buildpackage -b -us -uc

echo ""
echo "Package built. .deb file(s) in: $BUILD_DIR"
echo "Copy the .deb from there before this script exits, or pass --no-cleanup."
ls -1 "$BUILD_DIR"/*.deb 2>/dev/null || true
