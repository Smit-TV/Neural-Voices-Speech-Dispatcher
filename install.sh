#!/usr/bin/env bash

set -e

BINARY="build/neuralvoices-linux-x64"
TARGET_NAME="sd_neuralvoices"
CONFIG_FILE="neuralvoices.conf"

MODULE_DIRS=(
    /usr/lib/speech-dispatcher/speech-dispatcher-modules
    /usr/lib/speech-dispatcher-modules
    /usr/lib64/speech-dispatcher-modules
)

CONFIG_DIRS=(
    "$HOME/.config/speech-dispatcher/modules"
    /etc/speech-dispatcher/modules
)

MODULE_DIR=""
CONFIG_DIR=""

for dir in "${MODULE_DIRS[@]}"; do
    if [[ -d "$dir" ]]; then
        MODULE_DIR="$dir"
        break
    fi
done

if [[ -z "$MODULE_DIR" ]]; then
    echo "Error: Module directory was not found."
    exit 1
fi

echo "Module directory located at: $MODULE_DIR"

for dir in "${CONFIG_DIRS[@]}"; do
    if [[ -d "$dir" ]]; then
        CONFIG_DIR="$dir"
        break
    fi
done

if [[ -z "$CONFIG_DIR" ]]; then
    CONFIG_DIR="$HOME/.config/speech-dispatcher/modules"
    mkdir -p "$CONFIG_DIR"
    echo "Configuration directory created at: $CONFIG_DIR"
else
    echo "Configuration directory located at: $CONFIG_DIR"
fi

MODULE_PATH="$MODULE_DIR/$TARGET_NAME"
CONFIG_PATH="$CONFIG_DIR/$CONFIG_FILE"

if [[ -f "$MODULE_PATH" ]]; then
    echo "Existing module detected. Removing previous version."
    sudo rm "$MODULE_PATH"
fi

sudo cp "$BINARY" "$MODULE_PATH"
sudo chmod 555 "$MODULE_PATH"

if [[ -f "$CONFIG_PATH" ]]; then
    echo "Existing configuration detected. Overwriting."
fi

if [[ "$CONFIG_DIR" == "$HOME"* ]]; then
    cp "$CONFIG_FILE" "$CONFIG_PATH"
else
    sudo cp "$CONFIG_FILE" "$CONFIG_PATH"
fi

echo "Installation completed successfully."