#include <string>
#include <unicode/unistr.h>

#include "utils.hpp"


std::string toLower(const std::string& s) {
    icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(s);
    ustr.toLower();
    std::string result;
    ustr.toUTF8String(result);
    return result;
}