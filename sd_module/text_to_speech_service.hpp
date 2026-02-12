#pragma once

#include <atomic>
#include <functional>
#include <vector>
#include <string>
#include <unordered_map>
#include <speechapi_cxx.h>
#include "speech_synthesizer_utils.hpp"
#include "split_string.hpp"

using namespace Microsoft::CognitiveServices::Speech;
using namespace Microsoft::CognitiveServices::Speech::Audio;

constexpr const int LANGUAGE_MODE_SINGLE = 0;
constexpr const int LANGUAGE_MODE_MULTILINGUAL = 1;

class TextToSpeechService {
    public:
    std::unordered_map<std::string, std::string> settings;
    int languageMode = LANGUAGE_MODE_SINGLE;
    std::shared_ptr<SpeechSynthesizer> defaultTTSEngine;
    std::shared_ptr<SpeechSynthesizer> cyrillicTTSEngine;
    std::shared_ptr<SpeechSynthesizer> latinTTSEngine;
    std::vector<StringPart> textParts;
    std::vector<std::function<void(const SpeechSynthesisEventArgs&)>> speechSynthesisCompletedCallbacks;
    std::atomic<bool> isSpeaking = false;
    std::atomic<bool> stopRequested = false;
    std::shared_ptr<SpeechSynthesisRequest> defaultTTSEngineRequest;

    void init();
    void stop();
    void speak(const std::string& text);
    void stopCallbacks();

        int getRate();
    int getPitch();
    int getVolume();

    private:
    std::unordered_map<std::string, std::string> voicesDirs = getVoicesDirs();
};