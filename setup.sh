#!/usr/bin/env bash

 ./build.sh && ./install.sh && sudo mkdir /usr/local/neuralvoices && systemctl --user restart speech-dispatcher