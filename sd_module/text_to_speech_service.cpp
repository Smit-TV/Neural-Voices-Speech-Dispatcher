#include <thread>
#include <chrono>
#include <string>
#include <cstdio>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include "text_to_speech_service.hpp"
#include "speech_synthesizer_utils.hpp"
#include "ssml_preprocessor.hpp"
#include "split_string.hpp"

extern "C" {
    #include <spd_module_main.h>
}

int TextToSpeechService::getRate() {
    return std::stoi(settings["rate"]);
}

int TextToSpeechService::getPitch() {
    return 0;
}

int TextToSpeechService::getVolume() {
    return 0;
}

void TextToSpeechService::stopCallbacks() {
            int i = 0;
        while (i < speechSynthesisCompletedCallbacks.size()) {
            auto& part = textParts[i];
            auto& callback = speechSynthesisCompletedCallbacks[i];
            auto tts = part.alphabet == Alphabet::CYRILLIC ? cyrillicTTSEngine : latinTTSEngine;
            tts->SynthesisCompleted -= callback;
            i++;
        }
        speechSynthesisCompletedCallbacks.clear();
}

void TextToSpeechService::stop() {
    stopRequested = true;
    if (languageMode == LANGUAGE_MODE_MULTILINGUAL) {
        cyrillicTTSEngine->StopSpeakingAsync().get();
        latinTTSEngine->StopSpeakingAsync().get();
        return;
    }
    defaultTTSEngine->StopSpeakingAsync().get();
}

void TextToSpeechService::init() {
    std::ifstream config("/etc/neuralvoices/neuralvoices.conf");

    if (!config) {
        std::cout << "300 INITILIZATION FAILED" << std::endl;
        return;
    }

    std::string line;

    while (std::getline(config, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        auto pos = line.find('=');

        if (pos == std::string::npos) {
            continue;
        }

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        std::cerr << "Key: " << key << " value: " << value << std::endl;
        settings[key] = value;
    }

    if (settings.find("voice") != settings.end()) {
        const auto& voice = settings["voice"];
        defaultTTSEngine = createSpeechSynthesizer(voice, voicesDirs[voice]);
        std::cerr << "Default voice detected" << std::endl;
    }

    if (settings.find("cyrillic-voice") != settings.end()) {
        const auto& voice = settings["cyrillic-voice"];
        cyrillicTTSEngine = createSpeechSynthesizer(voice, voicesDirs[voice]);
        std::cerr << "Cyrillic voice detected" << std::endl;
    }

    if (settings.find("latin-voice") != settings.end()) {
        const auto& voice = settings["latin-voice"];
        latinTTSEngine = createSpeechSynthesizer(voice, voicesDirs[voice]);
        std::cerr << "Latin voice detected" << std::endl;
    }

    if (latinTTSEngine == nullptr && cyrillicTTSEngine != nullptr) {
        latinTTSEngine = defaultTTSEngine;
        std::cerr << "Latin voice was set to default voice" << std::endl;
    }

    if (cyrillicTTSEngine == nullptr && latinTTSEngine != nullptr) {
        cyrillicTTSEngine = defaultTTSEngine;
        std::cerr << "Cyrillic voice was set to default voice" << std::endl;
    }

    if (latinTTSEngine != nullptr && cyrillicTTSEngine != nullptr) {
        languageMode = LANGUAGE_MODE_MULTILINGUAL;
        std::cerr << "Multilingual mode is enabled" << std::endl;
    }

    if (languageMode ==LANGUAGE_MODE_SINGLE) {
        defaultTTSEngine->SynthesisCompleted += [this](const SpeechSynthesisEventArgs& ea) {
            isSpeaking = false;
            module_report_event_end();
        };
        defaultTTSEngine->SynthesisStarted += [this](const SpeechSynthesisEventArgs& ea) {
            isSpeaking = true;
        };
        defaultTTSEngine->SynthesisCanceled += [this](const SpeechSynthesisEventArgs& ea) {
            isSpeaking = false;
        };

    }

    if (languageMode == LANGUAGE_MODE_MULTILINGUAL) {
        cyrillicTTSEngine->SynthesisCompleted += [this](const SpeechSynthesisEventArgs& ea) {
            isSpeaking = false;
        };
        cyrillicTTSEngine->SynthesisStarted += [this](const SpeechSynthesisEventArgs& ea) {
            isSpeaking = true;
        };
        cyrillicTTSEngine->SynthesisCanceled += [this](const SpeechSynthesisEventArgs& ea) {
            isSpeaking = false;
        };

        latinTTSEngine->SynthesisCompleted += [this](const SpeechSynthesisEventArgs& ea) {
            isSpeaking = false;
        };
        latinTTSEngine->SynthesisStarted += [this](const SpeechSynthesisEventArgs& ea) {
            isSpeaking = true;
        };
        latinTTSEngine->SynthesisCanceled += [this](const SpeechSynthesisEventArgs& ea) {
            isSpeaking = false;
        };

    }
    std::cerr.flush();
}

void speakInternal(std::shared_ptr<SpeechSynthesizer> ttsEngine, bool isSSML, const std::string& text) {
        auto result = (isSSML ? ttsEngine->StartSpeakingSsml(text) : ttsEngine->StartSpeakingText(text));

    if (result->Reason == ResultReason::Canceled) {
        auto cancellation = SpeechSynthesisCancellationDetails::FromResult(result);
        std::cerr << cancellation->ErrorDetails << std::endl;
    }
}

void speakMultilingual(TextToSpeechService *ttsService, const std::string& text) {
}

void TextToSpeechService::speak(const std::string& text) {
    auto data = extract_text_from_ssml(text);
    // if (languageMode != LANGUAGE_MODE_MULTILINGUAL) {
        data = processSSML(data, getRate(), getPitch(), getVolume());
        speakInternal(defaultTTSEngine, true, data);
        return;
    // }

    // speakMultilingual(this, data);
}

