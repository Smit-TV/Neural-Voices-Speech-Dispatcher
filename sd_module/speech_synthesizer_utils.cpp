#include <speechapi_cxx.h>
#include <string>
#include <fstream>
#include <unordered_map>
#include "speech_synthesizer_utils.hpp"

using namespace Microsoft::CognitiveServices::Speech;
using namespace Microsoft::CognitiveServices::Speech::Audio;

std::unordered_map<std::string, std::string> getVoicesDirs() {
    auto synth = createSpeechSynthesizer("", "/usr/local/neuralvoices/");
    std::unordered_map<std::string, std::string> voices;
    auto result = synth->GetVoicesAsync().get();

    if (result->Reason != ResultReason::VoicesListRetrieved) {
        return voices;
    }

    for (const auto& voice : result->Voices) {
        voices[voice->ShortName] = voice->VoicePath;
        voices[voice->Name] = voice->VoicePath;
    }
    return voices;
}

std::string getVoiceLicense(const std::string& voicePath) {
    std::ifstream licenseFile(voicePath + "/model.key");
    if (!licenseFile) {
        return "";
    }

    std::string line;
    std::string license;

    while (std::getline(licenseFile, line)) {
        if (!license.empty()) {
            license = license + "\n" + line;
            continue;
        }

        license = line;
    }

    return license;
}

std::shared_ptr<SpeechSynthesizer> createSpeechSynthesizer(const std::string& voiceName, const std::string& voicePath) {
    auto config = EmbeddedSpeechConfig::FromPath("/usr/local/neuralvoices/");
    config->SetSpeechSynthesisVoice(voiceName, getVoiceLicense(voicePath));
    config->SetSpeechSynthesisOutputFormat(SpeechSynthesisOutputFormat::Riff24Khz16BitMonoPcm);
    return SpeechSynthesizer::FromConfig(config, AudioConfig::FromDefaultSpeakerOutput());
}