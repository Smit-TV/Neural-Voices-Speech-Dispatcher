#include <atomic>
#include <iostream>
#include <chrono>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <speechapi_cxx.h>
#include <unordered_map>
#include <queue>

#include "ssml_preprocessor.hpp"
#include "config.hpp"
#include "split_string.hpp"

extern "C" {
#include <speech-dispatcher/spd_module_main.h>
#include <speech-dispatcher/speechd_types.h>
}


using namespace Microsoft::CognitiveServices::Speech;
using namespace Microsoft::CognitiveServices::Speech::Audio;


std::unordered_map<std::string, std::shared_ptr<VoiceInfo>> getAvailableVoices() {
    auto config = EmbeddedSpeechConfig::FromPath("/usr/local/neuralvoices/");
    auto synth = SpeechSynthesizer::FromConfig(config, nullptr);
    auto result = synth->GetVoicesAsync().get();
    std::unordered_map<std::string, std::shared_ptr<VoiceInfo>> voices;

    if (result->Reason != ResultReason::VoicesListRetrieved) {
        return voices;
    }

    for (const auto& v : result->Voices) {
        voices[v->Name] = v;
        voices[v->ShortName] = v;
    }

    return voices;
}

static std::atomic<bool> isThreadRunning = false;;
static std::unordered_map<std::string, std::shared_ptr<VoiceInfo>> availableVoices = getAvailableVoices();
static std::unordered_map<std::string, std::shared_ptr<SpeechSynthesizer>> voices;
static std::unordered_map<std::string, std::string> config;
static std::mutex config_mtx;
static std::atomic<bool> isSpeaking = false;
static std::mutex mtx;
static std::condition_variable cv;
static bool hasWork = false;
static std::atomic<int> stop_requested = 0;
static std::shared_ptr<SpeechSynthesizer> voice;
static std::mutex voice_mtx;
static std::queue<std::string> messageQueue;
static std::mutex messageQueue_mtx;

int getValueFromConfig(const std::string& key) {
    if (config.find(key) == config.end()) {
        return 0;
    }

    return std::stoi(config[key]);
}

int getRate() {
    return getValueFromConfig("rate") * 2;
}

int getVolume() {
    return getValueFromConfig("volume");
}

int getPitch() {
    return getValueFromConfig("pitch");
}

void speaking_complete() {
            std::lock_guard<std::mutex> lock(mtx);
    isSpeaking = false;

    if (stop_requested == 1) {
        // to avoid end event when speech is stopped
        module_report_event_stop();
        return;
    }
    module_report_event_end();
}

void speak_thread() {
    while (isThreadRunning) {
        std::string text;
        {
        std::unique_lock<std::mutex> lock(messageQueue_mtx);
        cv.wait(lock, [] { return !messageQueue.empty() || !isThreadRunning; });
        if (!isThreadRunning) {
            break;
        }

        text = messageQueue.front();
        messageQueue.pop();
        lock.unlock();
    }
                module_report_event_begin();
                isSpeaking = true;

        if (config["latin-voice"] != "" && config["cyrillic-voice"] != "") {
            auto parts = splitString(text);

            for (const auto& part : parts) {
                if (stop_requested == 1) {
                    break;
                }
                if (part.alphabet == Alphabet::LATIN) {
                    module_set("voice", config["latin-voice"].c_str());
                } else {
                    module_set("voice", config["cyrillic-voice"].c_str());
                }
                std::shared_ptr<SpeechSynthesizer> mVoice;
                    {
                                    std::lock_guard<std::mutex> lock(voice_mtx);
                                    mVoice = voice;
                    }

                if (mVoice == nullptr) {
                    continue;
                }

                auto text = part.str;

                if (text.empty()) {
                    continue;
                }

        auto ssml = createSpeakSSML(text,
            getRate(),
            getVolume(),
            getPitch()
);
        auto result = mVoice->SpeakSsmlAsync(ssml).get();
        if (result->Reason == ResultReason::SynthesizingAudioCompleted) {
            continue;
        } else if (result->Reason == ResultReason::Canceled) {
            module_speak_error();
            auto cancellationDetails = SpeechSynthesisCancellationDetails::FromResult(result);
            std::cerr << cancellationDetails->ErrorDetails << std::endl;
            stop_requested = 1;
            speaking_complete();
        }
            }
            speaking_complete();

        } else {
            std::shared_ptr<SpeechSynthesizer> mVoice;
            {
                            std::lock_guard<std::mutex> lock(voice_mtx);
            mVoice = voice;
            }
            if (mVoice == nullptr) {
                stop_requested = 1;
                speaking_complete();
                continue;
            }
        auto ssml = createSpeakSSML(text,
            getRate(),
            getVolume(),
            getPitch()
);
        auto result = mVoice->SpeakSsmlAsync(ssml).get();
        if (result->Reason == ResultReason::SynthesizingAudioCompleted) {
            speaking_complete();
        } else if (result->Reason == ResultReason::Canceled) {
            module_speak_error();
            auto cancellationDetails = SpeechSynthesisCancellationDetails::FromResult(result);
            std::cerr << cancellationDetails->ErrorDetails << std::endl;
            stop_requested = 1;
            speaking_complete();
        }
    }
    }
}

static std::thread synth_thread;


extern "C" int module_config(const char *configPath)
{
    if (configPath != nullptr) {
    auto module_cfg = readConfig(configPath);
    config.insert(module_cfg.begin(), module_cfg.end());
    for (const auto [key, value] : config) {
        module_set(key.c_str(), value.c_str());
    }
    }
    return 0;
}

extern "C" int module_init(char **msg)
{
    std::cerr << "Initializing" << std::endl;

    isThreadRunning = true;
    synth_thread = std::thread(speak_thread);

    if (msg)
        *msg = ::strdup("ok!");

    return 0;
}

extern "C" SPDVoice **module_list_voices(void) {
    size_t count = availableVoices.size();

    SPDVoice **ret = static_cast<SPDVoice **>(std::calloc(count + 1, sizeof(SPDVoice *)));
    if (!ret) return nullptr;

    size_t i = 0;
    for (const auto &pair : availableVoices) {
        const auto &v = pair.second;

        ret[i] = static_cast<SPDVoice *>(std::calloc(1, sizeof(SPDVoice)));
        if (!ret[i]) {
            for (size_t j = 0; j < i; ++j) {
                std::free(ret[j]->name);
                std::free(ret[j]->language);
                if (ret[j]->variant) std::free(ret[j]->variant);
                std::free(ret[j]);
            }
            std::free(ret);
            return nullptr;
        }

        ret[i]->name     = ::strdup(v->Name.c_str());
        ret[i]->language = ::strdup(v->Locale.c_str());
        // ret[i]->variant  = v.Variant.empty() ? nullptr : ::strdup(v.Variant.c_str());

        ++i;
    }

    ret[count] = nullptr; 
    return ret;
}

std::shared_ptr<SpeechSynthesizer> createSynthesizer(const std::string& voice) {
    std::cerr << "Creating synthesizer for " << voice << std::endl;
    if (availableVoices.find(voice) == availableVoices.end()) {
        std::cerr << "Voice not found" << std::endl;
        return nullptr;
    }
    std::string path = availableVoices[voice]->VoicePath;
    std::cerr << "Voice data path: " << path << std::endl;
    auto config = EmbeddedSpeechConfig::FromPath(path);
    config->SetSpeechSynthesisVoice(voice, getVoiceLicense(path));
    config->SetSpeechSynthesisOutputFormat(SpeechSynthesisOutputFormat::Riff24Khz16BitMonoPcm);
    return SpeechSynthesizer::FromConfig(config, AudioConfig::FromDefaultSpeakerOutput());
}

extern "C" int module_set(const char *param, const char *value)
{
    if (!param || !value)
        return -1;

                std::string var(param);
        std::string val(value);

        std::system(("echo "  + var + "=" + val + " >> ~/null &").c_str());

        //std::system(("espeak " + var + "=" + val + "").c_str());

    {
                    std::lock_guard<std::mutex> lock(mtx);

        config[var] = val;
    }

        if (var == "voice" || var == "synthesis_voice") {
            if (availableVoices.size() == 0) {
                std::cerr << "No voices available" << std::endl;
                return -1;
            }
            if (val.empty() || val == "NULL" || val == "null" || availableVoices.find(val) == availableVoices.end()) {
                // Speech dispatcher uses null as voice name when we should select a default voice
                // When orca uses  default voice it'll send incorrect voice name for eg synthesis_voice=Default voice neuralvoices
                
                std::shared_ptr<VoiceInfo> defaultVoice;
                for (auto& [key, v] : availableVoices) {
                    if (strcmp(v->Locale.c_str(), config["language"].c_str()) == 0) {
                                            defaultVoice = v;
                        continue;
                    }
                    defaultVoice = v;
                    break;
                }
                    val = defaultVoice->Name;
            }

            if (voices.find(val) == voices.end()) {
                // We should create synthesizer before using
                voices[val] = createSynthesizer(val);
            }
            std::cerr << val << " will be used by default voice for synthesis" << std::endl;
                    {
            std::lock_guard<std::mutex> lock(voice_mtx);
            voice = voices[val];
                    }
        }

    std::fprintf(stderr, "set '%s' = '%s'\n", param, value);

    return 0;
}

extern "C" int module_audio_set(const char *var, const char *val)
{
    if (!var || !val)
        return -1;

        {
            std::lock_guard<std::mutex> lock(config_mtx);
            config[var] = val;
        }

    return 0;
}

extern "C" int module_audio_init(char **)
{
    return 0;
}

extern "C" int module_loglevel_set(const char *, const char *)
{
    return 0;
}

extern "C" int module_debug(int, const char *)
{
    return 0;
}

extern "C" int module_loop(void)
{
    std::fprintf(stderr, "main loop\n");

    int ret = module_process(STDIN_FILENO, 1);

    if (ret != 0)
        std::fprintf(stderr, "Broken pipe, exiting...\n");

    return ret;
}

extern "C" int module_speak(char *data, size_t, SPDMessageType)
{
                        // std::system("espeak 1 &");

    if (!data) {
        return -1;
    }

            {
            std::lock_guard<std::mutex> lock(voice_mtx);
    if (voice == nullptr) {
        std::cerr << "Voice is not set" << std::endl;
        return -1;
    }
}

    stop_requested = 0;

    {
            std::lock_guard<std::mutex> lock(messageQueue_mtx);
    messageQueue.push(extractTextFromSSML(data));
    }
    cv.notify_one();

    return 1;
}

extern "C" size_t module_pause()
{
    std::fprintf(stderr, "pausing\n");

    stop_requested = 1;

    module_stop();

    return -1;
}

extern "C" int module_stop()
{
    std::fprintf(stderr, "stopping\n");

    isSpeaking = false;
    stop_requested = 1;
        {
            std::lock_guard<std::mutex> lock(voice_mtx);
                if (voice == nullptr) {
        return -1;
    }

    voice->StopSpeakingAsync();
        }

    return 0;
}

extern "C" int module_close(void)
{
    std::fprintf(stderr, "closing\n");

    {
            std::lock_guard<std::mutex> lock(mtx);

    isThreadRunning = false;
    hasWork = true;
    }

    cv.notify_one();
    if (synth_thread.joinable()) {
        synth_thread.join();
    }
    voices.clear();
    availableVoices.clear();
            {
            std::lock_guard<std::mutex> lock(voice_mtx);
    voice.reset();
            }

    return 0;
}
