#pragma once


#include <unistd.h>
#include <speechapi_cxx.h>
#include <pthread.h>

extern "C" {
    #include <speech-dispatcher/spd_module_main.h>
    #include <speech-dispatcher/spd_audio_plugin.h>
    #include <speech-dispatcher/speechd_types.h>
}

using namespace Microsoft::CognitiveServices::Speech::Audio;

class SpeechSynthesisCallback : public PushAudioOutputStreamCallback
{
public:
        AudioTrack track;
        AudioFormat format = SPD_AUDIO_LE;

    void Close() override {}
    int Write(uint8_t* dataBuffer, uint32_t size) override {
        module_audio_set_server();
    track.bits = 16;
    track.num_channels = 1;      
    track.sample_rate = 24000;   
    track.samples = reinterpret_cast<signed short*>(dataBuffer);
    track.num_samples = size / sizeof(signed short);
			module_tts_output_server(&track, format);
        return size;
    }
};