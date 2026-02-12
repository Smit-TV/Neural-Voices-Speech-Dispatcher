#!/usr/bin/env bash

 ./build.sh && sudo ./install.sh && mkdir /etc/neuralvoices/ &&  sudo cp neuralvoices.conf /etc/neuralvoices/ && sudo mkdir /usr/local/neuralvoices && systemctl --user restart speech-dispatcher