#!/usr/bin/env bash

set -e

echo "Directory neural-voices-installer"
mkdir -p ./neural-voices-installer

echo "Creating app image"
./create_app_image.sh

echo "Moving binary to the directory"
mv ./sd_neuralvoices-x86_64.AppImage ./neural-voices-installer/sd_neuralvoices

echo "Copying config to the directory"
cp neuralvoices.conf ./neural-voices-installer/

echo "Done"

echo "Creating a zip archive"

cd neural-voices-installer

zip neural-voices-linux-with-installer.zip sd_neuralvoices neuralvoices.conf README.ru.md README.md install.sh

echo "Done"

cd -