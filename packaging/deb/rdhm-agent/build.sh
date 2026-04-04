#!/bin/sh
#
# Build rdhm-agent .deb package.
#
# Run from the project root directory:
#   ./packaging/deb/rdhm-agent/build.sh
#
# Prerequisites: debhelper, cmake, g++, dpkg-dev
#
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"
OUTPUT_DIR="${1:-$PROJECT_ROOT}"

# Create a temporary build directory with the expected source tree
BUILD_DIR=$(mktemp -d)
PKG_DIR="$BUILD_DIR/rdhm-agent-0.2.0"

trap 'rm -rf "$BUILD_DIR"' EXIT

cp -a "$PROJECT_ROOT" "$PKG_DIR"
cp -a "$SCRIPT_DIR/debian" "$PKG_DIR/debian"

cd "$PKG_DIR"
dpkg-buildpackage -b -us -uc

cp "$BUILD_DIR"/*.deb "$OUTPUT_DIR/"
echo ""
echo "Package(s) copied to: $OUTPUT_DIR"
ls -1 "$OUTPUT_DIR"/rdhm-agent*.deb 2>/dev/null || true
