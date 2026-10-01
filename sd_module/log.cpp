#include "log.hpp"
#include <speechapi_cxx.h>
#include <string>
#include <iostream>

using namespace Microsoft::CognitiveServices::Speech::Diagnostics::Logging;

bool isLoggingEnabled = false;
bool isMicrosoftLoggingEnabled = false;

void enableLogging(bool enable) {
    isLoggingEnabled = enable;
}

void enableMicrosoftLogging(int level, const std::string& path) {
    isMicrosoftLoggingEnabled = level;
    Level logLevel;

    switch (level)
    {
        // VERBOSE
    case 1:
        logLevel = Level::Verbose;
        break;

        // INFO
        case 2:
    logLevel = Level::Info;
    break;

    // Warning 
    case 3:
    logLevel = Level::Warning;
    break;

    // ERROR
    case 4:
    logLevel = Level::Error;
    break;

    default:
    MemoryLogger::Stop();
    FileLogger::Stop();
        return;
    }

    MemoryLogger::SetLevel(logLevel);
    EventLogger::SetLevel(logLevel);
    FileLogger::SetLevel(logLevel);

    MemoryLogger::Start();
    FileLogger::Start(path, true);
}

void log(std::string text) {
    if (!isLoggingEnabled) {
        return;
    }

    std::cerr << text << std::endl;
}