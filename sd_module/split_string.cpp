#include <vector>
#include <string>
#include <unicode/uchar.h>
#include <unicode/utypes.h>
#include <unicode/utypes.h>
#include <unicode/uscript.h>
#include <unicode/unistr.h>
#include "split_string.hpp"

#include <iostream>

Alphabet getAlphabet(UChar32 c) {
    UErrorCode err = U_ZERO_ERROR;
    UScriptCode script = uscript_getScript(c, &err);
        if (script == USCRIPT_CYRILLIC) {
        return Alphabet::CYRILLIC;
    } else if (script == USCRIPT_LATIN) {
        return Alphabet::LATIN;
    }
    return Alphabet::NONE;
}

std::vector<StringPart> splitString(const std::string& str) {
    std::vector<StringPart> v;
    icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(str);

    StringPart currentPart;
    currentPart.alphabet = Alphabet::NONE;
    icu::UnicodeString newStr;
    icu::UnicodeString cyrillicStr;

    for (int32_t i = 0; i < ustr.length(); ) {
        UChar32 c = ustr.char32At(i);
        i += U16_LENGTH(c);
        Alphabet alphabet = getAlphabet(c);
        if (alphabet != currentPart.alphabet && alphabet != Alphabet::NONE) {
                    if (currentPart.alphabet == Alphabet::LATIN) {
            newStr.toUTF8String(currentPart.str);
            newStr = "";
                    } else {
                        cyrillicStr.toUTF8String(currentPart.str);
                        cyrillicStr = "";
                    }
            v.push_back(currentPart);
            currentPart = StringPart{};
            currentPart.alphabet = alphabet;
        }
        if (currentPart.alphabet == Alphabet::LATIN) {
        newStr.append(c);
        } else {
            cyrillicStr.append(c);
        }
    }

            if (currentPart.alphabet == Alphabet::LATIN) {

    newStr.toUTF8String(currentPart.str);
            } else {
                cyrillicStr.toUTF8String(currentPart.str);
            }
    v.push_back(currentPart);

    return v;
}