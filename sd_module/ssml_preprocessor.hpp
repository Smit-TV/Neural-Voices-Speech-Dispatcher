#ifndef SSML_PREPROCESSOR_HPP
#define SSML_PREPROCESSOR_HPP

#include <string>

std::string extractTextFromSSML(const std::string& ssml);
std::string createSpeakSSML(const std::string& text, int rate, int volume, int pitch);

#endif // SSML_PREPROCESSOR_HPP