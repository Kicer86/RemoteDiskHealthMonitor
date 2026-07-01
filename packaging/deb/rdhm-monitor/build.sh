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
OUTPUT_DIR="${1:-$PROJECT_ROOT}"

BUILD_DIR=$(mktemp -d)
PKG_DIR="$BUILD_DIR/rdhm-monitor-0.2.1"

trap 'rm -rf "$BUILD_DIR"' EXIT

cp -a "$PROJECT_ROOT" "$PKG_DIR"
cp -a "$SCRIPT_DIR/debian" "$PKG_DIR/debian"

cd "$PKG_DIR"
dpkg-buildpackage -b -us -uc

cp "$BUILD_DIR"/*.deb "$OUTPUT_DIR/"
echo ""
echo "Package(s) copied to: $OUTPUT_DIR"
ls -1 "$OUTPUT_DIR"/rdhm-monitor*.deb 2>/dev/null || true
