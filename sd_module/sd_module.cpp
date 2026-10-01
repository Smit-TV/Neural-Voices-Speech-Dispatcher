#include <atomic>
#include <algorithm>
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
#include "utils.hpp"
#include "speech_synthesis_callback.hpp"
#include "log.hpp"

extern "C"
{
#include <speech-dispatcher/spd_module_main.h>
#include <speech-dispatcher/speechd_types.h>
}

using namespace Microsoft::CognitiveServices::Speech;
using namespace Microsoft::CognitiveServices::Speech::Audio;

std::map<std::string, std::shared_ptr<VoiceInfo>> getAvailableVoices()
{
    auto config = EmbeddedSpeechConfig::FromPath("/usr/local/neuralvoices/");
    auto synth = SpeechSynthesizer::FromConfig(config, nullptr);
    auto result = synth->GetVoicesAsync().get();
    std::map<std::string, std::shared_ptr<VoiceInfo>> voices;

    if (result->Reason != ResultReason::VoicesListRetrieved)
    {
        return voices;
    }

    for (const auto &v : result->Voices)
    {
        voices[v->Name] = v;
        voices[v->ShortName] = v;
    }

    return voices;
}

static std::atomic<bool> isThreadRunning = false;
static std::atomic<bool> useDefaultPitch = false;
static std::map<std::string, std::shared_ptr<VoiceInfo>> availableVoices = getAvailableVoices();
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

int getValueFromConfig(const std::string &key)
{
    if (config.find(key) == config.end())
    {
        return 0;
    }

    return std::stoi(config[key]);
}

int getRate()
{
    return getValueFromConfig("rate") * 2;
}

int getVolume()
{
    return getValueFromConfig("volume");
}

int getPitch()
{
    return getValueFromConfig("pitch");
}

void speaking_complete()
{
    std::lock_guard<std::mutex> lock(mtx);
    isSpeaking = false;

    if (stop_requested == 1)
    {
        // to avoid end event when speech is stopped
        module_report_event_stop();
        return;
    }
    module_report_event_end();
}

void speak_thread()
{
    while (isThreadRunning)
    {
        std::string text;
        {
            std::unique_lock<std::mutex> lock(messageQueue_mtx);
            cv.wait(lock, []
                    { return !messageQueue.empty() || !isThreadRunning; });
            if (!isThreadRunning)
            {
                break;
            }

            text = messageQueue.front();
            messageQueue.pop();
            lock.unlock();
        }
        module_report_event_begin();
        isSpeaking = true;
        stop_requested = 0;

        bool useUnicodeLanguageDetection;
        {
            std::lock_guard<std::mutex> lock(config_mtx);

            useUnicodeLanguageDetection = config["use-unicode-language-detection"] == "yes" ? true : false;
        }

        if (useUnicodeLanguageDetection)
        {
            auto parts = splitString(text);
            bool isComplete = false;

            for (const auto &part : parts)
            {
                if (stop_requested == 1)
                {
                    break;
                }

                std::string voiceName;
                {
                    std::lock_guard<std::mutex> lock(config_mtx);

                    voiceName = getVoiceName(&config, part.alphabet);

                    if (voiceName.empty())
                    {
                        voiceName = config["synthesis_voice"];
                    }
                }

                module_set("voice", voiceName.c_str());
                std::shared_ptr<SpeechSynthesizer> mVoice;
                {
                    std::lock_guard<std::mutex> lock(voice_mtx);
                    mVoice = voice;
                }

                if (mVoice == nullptr)
                {
                    continue;
                }

                auto text = part.str;

                if (text.empty())
                {
                    continue;
                }

                auto ssml = createSpeakSSML(text,
                                            getRate(),
                                            getVolume(),
                                            getPitch(),
                                            useDefaultPitch);
                auto result = mVoice->SpeakSsml(ssml);
                if (result->Reason != ResultReason::SynthesizingAudioCompleted)
                {
                    auto cancellationDetails = SpeechSynthesisCancellationDetails::FromResult(result);
                    std::cerr << cancellationDetails->ErrorDetails << std::endl;
                    if (result->Reason == ResultReason::Canceled)
                    {
                        speaking_complete();
                    }
                    else
                    {
                        module_speak_error();
                    }
                    isComplete = true;
                }
            }
            if (!isComplete)
                speaking_complete();
        }
        else
        {
            std::shared_ptr<SpeechSynthesizer> mVoice;
            {
                std::lock_guard<std::mutex> lock(voice_mtx);
                mVoice = voice;
            }
            if (mVoice == nullptr)
            {
                speaking_complete();
                continue;
            }
            auto ssml = createSpeakSSML(text,
                                        getRate(),
                                        getVolume(),
                                        getPitch(),
                                        useDefaultPitch);
            auto result = mVoice->SpeakSsml(ssml);
            bool isComplete = false;
            if (result->Reason != ResultReason::SynthesizingAudioCompleted)
            {
                auto cancellationDetails = SpeechSynthesisCancellationDetails::FromResult(result);
                std::cerr << cancellationDetails->ErrorDetails << std::endl;

                if (result->Reason == ResultReason::Canceled)
                {
                    speaking_complete();
                }
                else
                {
                    module_speak_error();
                }
                isComplete = true;
            }
            if (!isComplete)
            {
                speaking_complete();
            }
        }
    }
}

static std::thread synth_thread;

extern "C" int module_config(const char *configPath)
{
    if (configPath != nullptr)
    {
        auto module_cfg = readConfig(configPath);
        config.insert(module_cfg.begin(), module_cfg.end());
        for (const auto [key, value] : config)
        {
            module_set(key.c_str(), value.c_str());
        }

        auto it = config.find("microsoft-log-level");
        if (it != config.end()) {
            auto pathIt = config.find("path-to-log-file");
            if (pathIt != config.end()) {
            enableMicrosoftLogging(
                std::stoi(it->second),
                pathIt->second
            );
        }
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

extern "C" SPDVoice **module_list_voices(void)
{
    size_t count = availableVoices.size();

    SPDVoice **ret = static_cast<SPDVoice **>(std::calloc(count + 1, sizeof(SPDVoice *)));
    if (!ret)
        return nullptr;

    size_t i = 0;
    for (const auto &pair : availableVoices)
    {
        const auto &v = pair.second;

        ret[i] = static_cast<SPDVoice *>(std::calloc(1, sizeof(SPDVoice)));
        if (!ret[i])
        {
            for (size_t j = 0; j < i; ++j)
            {
                std::free(ret[j]->name);
                std::free(ret[j]->language);
                if (ret[j]->variant)
                    std::free(ret[j]->variant);
                std::free(ret[j]);
            }
            std::free(ret);
            return nullptr;
        }

        ret[i]->name = ::strdup(v->Name.c_str());
        ret[i]->language = ::strdup(v->Locale.c_str());
        // ret[i]->variant  = v.Variant.empty() ? nullptr : ::strdup(v.Variant.c_str());

        ++i;
    }

    ret[count] = nullptr;
    return ret;
}

std::shared_ptr<SpeechSynthesizer> createSynthesizer(const std::string &voice)
{
    log("Creating synthesizer for " + voice);
    if (availableVoices.find(voice) == availableVoices.end())
    {
        log("Voice not found");
        return nullptr;
    }
    std::string path = availableVoices[voice]->VoicePath;
    log("Voice data path: " + path);
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

    {
        std::lock_guard<std::mutex> lock(config_mtx);

        config[var] = val;
    }

    if (var == "voice" || var == "synthesis_voice")
    {
        if (availableVoices.size() == 0)
        {
            log("No voices available");
            return -1;
        }
        if (val.empty() || val == "NULL" || val == "null" || availableVoices.find(val) == availableVoices.end())
        {
            // Speech dispatcher uses null as voice name when we should select a default voice
            // When orca uses  default voice it'll send incorrect voice name for eg synthesis_voice=Default voice neuralvoices

            std::shared_ptr<VoiceInfo> defaultVoice;
            log("Defining default voice");
            for (auto &[key, v] : availableVoices)
            {
                if (toLower(v->Locale) != toLower(config["language"]))
                {
                    defaultVoice = v;
                    continue;
                }
                defaultVoice = v;
                break;
            }
            val = defaultVoice->Name;
            log(val + " will be used as default voice");
        }

        {
            std::lock_guard<std::mutex> lock(voice_mtx);

            if (voices.find(val) == voices.end())
            {
                // We should create synthesizer before using
                voices[val] = createSynthesizer(val);
            }
            voice = voices[val];
        }
        log(val + " will be used by default voice for synthesis");
    }
    else if (var == "use-default-pitch")
    {
        useDefaultPitch = val == "yes" ? true : false;
    }
    else if (var == "enable-logging")
    {
        enableLogging(val == "yes" ? true : false);
    }

    log("Set " + std::string(param) + " = " + value);

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

    if (!data)
    {
        return -1;
    }

    {
        std::lock_guard<std::mutex> lock(voice_mtx);
        if (voice == nullptr)
        {
            std::cerr << "Voice is not set" << std::endl;
            return -1;
        }
    }

    auto text = extractTextFromSSML(data);

    log("Speaking " + text);

    {
        std::lock_guard<std::mutex> lock(messageQueue_mtx);
        messageQueue.push(text);
    }
    cv.notify_one();

    return 1;
}

extern "C" size_t module_pause()
{
    log("Pausing");

    stop_requested = 1;

    module_stop();

    return -1;
}

extern "C" int module_stop()
{
    log("Stopping");

    isSpeaking = false;
    stop_requested = 1;
    {
        std::lock_guard<std::mutex> lock(voice_mtx);
        if (voice == nullptr)
        {
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
    if (synth_thread.joinable())
    {
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
