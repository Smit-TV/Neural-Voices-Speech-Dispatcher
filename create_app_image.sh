#!/usr/bin/env bash

set -e

NO_STRIP=1

echo "Creating AppImage..."

# Check build directory
if [[ ! -d "./build" ]]; then
    echo "Failed to create AppImage."
    echo "Reason: build directory was not found."
    exit 1
fi

# Check executable
if [[ ! -f "./build/neuralvoices-linux-x64" ]]; then
    echo "Failed to create AppImage."
    echo "Reason: ./build/neuralvoices-linux-x64 was not found."
    exit 1
fi

# Check linuxdeploy
if [[ ! -x "./linuxdeploy-x86_64.AppImage" ]]; then
    echo "Failed to create AppImage."
    echo "Reason: linuxdeploy-x86_64.AppImage was not found or is not executable."
    exit 1
fi

# Clean previous AppImage directory
rm -rf "./appImage"

# Create AppDir structure
mkdir -p "./appImage/usr/bin"
mkdir -p "./appImage/usr/lib"
mkdir -p "appImage/usr/share/icons/hicolor/256x256/apps/"

# Empty icon
touch appImage/usr/share/icons/hicolor/256x256/apps/sd_neuralvoices.png

# Copy Speech Dispatcher module
cp "./build/neuralvoices-linux-x64" \
   "./appImage/usr/bin/sd_neuralvoices"

# Copy required libraries
cp -r ./libs/runtimes/linux-x64/native/lib* \
   "./appImage/usr/lib/"


# Make sure the module is executable
chmod +x "./appImage/usr/bin/sd_neuralvoices"

echo "Running linuxdeploy..."


./linuxdeploy-x86_64.AppImage \
    --appdir "./appImage" \
    --executable "./appImage/usr/bin/sd_neuralvoices" \
   --output appimage \
   --create-desktop-file

echo "AppImage created successfully."
