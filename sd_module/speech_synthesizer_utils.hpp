#pragma once

#include <string>
#include <speechapi_cxx.h>

using namespace Microsoft::CognitiveServices::Speech;
using namespace Microsoft::CognitiveServices::Speech::Audio;

std::unordered_map<std::string, std::string> getVoicesDirs();

std::shared_ptr<SpeechSynthesizer> createSpeechSynthesizer(const std::string& voiceName, const std::string& voicePath);