#!/usr/bin/env bash

rm /usr/lib/speech-dispatcher/speech-dispatcher-modules/sd_neuralvoices 2> /dev/null
cp neuralvoices-linux-x64 /usr/lib/speech-dispatcher/speech-dispatcher-modules/sd_neuralvoices
chmod 555 /usr/lib/speech-dispatcher/speech-dispatcher-modules/sd_neuralvoices