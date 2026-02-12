#ifndef SSML_PREPROCESSOR_HPP
#define SSML_PREPROCESSOR_HPP

#include <string>

std::string extract_text_from_ssml(const std::string& ssml);
std::string processSSML(const std::string& text, int rate, int pitch, int volume);

#endif // SSML_PREPROCESSOR_HPP