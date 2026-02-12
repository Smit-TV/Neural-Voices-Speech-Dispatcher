#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include "ssml_preprocessor.hpp"
#include "text_to_speech_service.hpp"

TextToSpeechService tts;

extern "C" {
#include <spd_module_main.h>
#include <speechd_types.h>
}

static int stop_requested = 0;

/* ===================== CONFIG ===================== */

extern "C" int module_config(const char *configfile)
{
    std::fprintf(stderr, "opening %s\n", configfile);
    return 0;
}

extern "C" int module_init(char **msg)
{
    std::fprintf(stderr, "initializing\n");

    tts.init();

    *msg = ::strdup("ok!");
    return 0;
}

/* ===================== VOICES ===================== */

extern "C" SPDVoice **module_list_voices(void)
{
    // 1 voice + NULL terminator
    SPDVoice **ret = static_cast<SPDVoice **>(
        std::malloc(2 * sizeof(SPDVoice *))
    );

    if (!ret)
        return nullptr;

    ret[0] = static_cast<SPDVoice *>(std::malloc(sizeof(SPDVoice)));
    if (!ret[0]) {
        std::free(ret);
        return nullptr;
    }

    ret[0]->name     = ::strdup("foo");
    ret[0]->language = ::strdup("eo");
    ret[0]->variant  = nullptr;

    ret[1] = nullptr;
    return ret;
}

/* ===================== PARAMETERS ===================== */

extern "C" int module_set(const char *var, const char *val)
{
    std::fprintf(stderr, "got var '%s' to be set to '%s'\n", var, val);

    tts.settings[std::string(var)] = std::string(val);

    return 0;
}

/* ===================== AUDIO ===================== */

extern "C" int module_audio_set(const char *var, const char *val)
{
    tts.settings[std::string(var)] = std::string(val);
    return 0;
}

extern "C" int module_audio_init(char **)
{
    return 0;
}

/* ===================== DEBUG ===================== */

extern "C" int module_loglevel_set(const char *, const char *)
{
    return 0;
}

extern "C" int module_debug(int, const char *)
{
    return 0;
}

/* ===================== MAIN LOOP ===================== */

extern "C" int module_loop(void)
{
    std::fprintf(stderr, "main loop\n");

    int ret = module_process(STDIN_FILENO, 1);
    if (ret != 0)
        std::fprintf(stderr, "Broken pipe, exiting...\n");

    return ret;
}

/* ===================== SPEAK ===================== */

extern "C" void module_speak_sync(const char *data, size_t, SPDMessageType msgType)
{
    stop_requested = 0;

    module_speak_ok();

    std::fprintf(stderr, "speaking '%s'\n", data);

    module_report_event_begin();

    tts.speak(data);
}

/* ===================== CONTROL ===================== */

extern "C" size_t module_pause(void)
{
    std::fprintf(stderr, "pausing\n");
    stop_requested = 1;
    tts.stop();

    module_report_event_stop();
    return 0;
}

extern "C" int module_stop(void)
{
    std::fprintf(stderr, "stopping\n");
    stop_requested = 1;
    tts.stop();

    module_report_event_stop();
    return 0;
}

extern "C" int module_close(void)
{
    std::fprintf(stderr, "closing\n");
    return 0;
}
