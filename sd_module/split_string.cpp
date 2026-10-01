#include <vector>
#include <string>
#include <unicode/uchar.h>
#include <unicode/utypes.h>
#include <unicode/utypes.h>
#include <unicode/uscript.h>
#include <unicode/unistr.h>
#include "split_string.hpp"

#include <iostream>
#include <unordered_map>

std::string getVoiceName(const std::unordered_map<std::string, std::string>* config, Alphabet alphabet)
{
    std::string key;

    switch (alphabet)
    {
        case Alphabet::LATIN:
            key = "latin-voice";
            break;

        case Alphabet::CYRILLIC:
            key = "cyrillic-voice";
            break;

        case Alphabet::GREEK:
            key = "greek-voice";
            break;

        case Alphabet::ARABIC:
            key = "arabic-voice";
            break;

        case Alphabet::HEBREW:
            key = "hebrew-voice";
            break;

        case Alphabet::ARMENIAN:
            key = "armenian-voice";
            break;

        case Alphabet::GEORGIAN:
            key = "georgian-voice";
            break;

        case Alphabet::DEVANAGARI:
            key = "devanagari-voice";
            break;

        case Alphabet::BENGALI:
            key = "bengali-voice";
            break;

            case Alphabet::GURMUKHI:
            key = "gurmukhi-voice";
            break;

            case Alphabet::GUJARATI:
            key = "gujarati-voice";
            break;

            case Alphabet::ORIYA:
            key = "oriya-voice";
            break;

        case Alphabet::TAMIL:
            key = "tamil-voice";
            break;

        case Alphabet::TELUGU:
            key = "telugu-voice";
            break;

        case Alphabet::KANNADA:
            key = "kannada-voice";
            break;

        case Alphabet::MALAYALAM:
            key = "malayalam-voice";
            break;

            case Alphabet::SINHALA:
            key = "sinhala-voice";
            break;

        case Alphabet::THAI:
            key = "thai-voice";
            break;

        case Alphabet::LAO:
            key = "lao-voice";
            break;

        case Alphabet::KHMER:
            key = "khmer-voice";
            break;

        case Alphabet::MYANMAR:
            key = "myanmar-voice";
            break;

        case Alphabet::HANGUL:
            key = "hangul-voice";
            break;

        case Alphabet::HAN:
            key = "han-voice";
            break;

        case Alphabet::HIRAGANA:
            key = "hiragana-voice";
            break;

        case Alphabet::KATAKANA:
            key = "katakana-voice";
            break;

            case Alphabet::ETHIOPIC:
            key = "ethiopic-voice";
            break;

            case Alphabet::SYRIAC:
            key = "syriac-voice";
            break;

            case Alphabet::THAANA:
            key = "thaana-voice";
            break;

            case Alphabet::MONGOLIAN:
            key = "mongolian-voice";
            break;

            case Alphabet::TIBETAN:
            key = "tibetan-voice";
            break;

        default:
            return "";
    }

    if (config == nullptr) {
        return "";
    }

    auto it = config->find(key);

    if (it == config->end()) {
        return "";
    }

    return it->second;
}

Alphabet getAlphabet(UChar32 c) {
    UErrorCode err = U_ZERO_ERROR;
    UScriptCode script = uscript_getScript(c, &err);

    switch (script) {
        case USCRIPT_LATIN:
            return Alphabet::LATIN;

        case USCRIPT_CYRILLIC:
            return Alphabet::CYRILLIC;

        case USCRIPT_GREEK:
            return Alphabet::GREEK;

        case USCRIPT_ARABIC:
            return Alphabet::ARABIC;

        case USCRIPT_HEBREW:
            return Alphabet::HEBREW;

        case USCRIPT_ARMENIAN:
            return Alphabet::ARMENIAN;

        case USCRIPT_GEORGIAN:
            return Alphabet::GEORGIAN;

        case USCRIPT_DEVANAGARI:
            return Alphabet::DEVANAGARI;

        case USCRIPT_BENGALI:
            return Alphabet::BENGALI;

        case USCRIPT_GURMUKHI:
            return Alphabet::GURMUKHI;

        case USCRIPT_GUJARATI:
            return Alphabet::GUJARATI;

        case USCRIPT_ORIYA:
            return Alphabet::ORIYA;

        case USCRIPT_TAMIL:
            return Alphabet::TAMIL;

        case USCRIPT_TELUGU:
            return Alphabet::TELUGU;

        case USCRIPT_KANNADA:
            return Alphabet::KANNADA;

        case USCRIPT_MALAYALAM:
            return Alphabet::MALAYALAM;

        case USCRIPT_SINHALA:
            return Alphabet::SINHALA;

        case USCRIPT_THAI:
            return Alphabet::THAI;

        case USCRIPT_LAO:
            return Alphabet::LAO;

        case USCRIPT_KHMER:
            return Alphabet::KHMER;

        case USCRIPT_MYANMAR:
            return Alphabet::MYANMAR;

        case USCRIPT_HANGUL:
            return Alphabet::HANGUL;

        case USCRIPT_HAN:
            return Alphabet::HAN;

        case USCRIPT_HIRAGANA:
            return Alphabet::HIRAGANA;

        case USCRIPT_KATAKANA:
            return Alphabet::KATAKANA;

        case USCRIPT_ETHIOPIC:
            return Alphabet::ETHIOPIC;

        case USCRIPT_SYRIAC:
            return Alphabet::SYRIAC;

        case USCRIPT_THAANA:
            return Alphabet::THAANA;

        case USCRIPT_MONGOLIAN:
            return Alphabet::MONGOLIAN;

        case USCRIPT_TIBETAN:
            return Alphabet::TIBETAN;

        default:
            return Alphabet::NONE;
    }
}

std::vector<StringPart> splitString(const std::string& str) {
    std::vector<StringPart> v;
    icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(str);

    StringPart currentPart;
    currentPart.alphabet = Alphabet::NONE;
    icu::UnicodeString newStr;

    for (int32_t i = 0; i < ustr.length(); ) {
        UChar32 c = ustr.char32At(i);
        i += U16_LENGTH(c);
        Alphabet alphabet = getAlphabet(c);
        if (alphabet != currentPart.alphabet && alphabet != Alphabet::NONE) {
            newStr.toUTF8String(currentPart.str);
            newStr = "";
            v.push_back(currentPart);
            currentPart = StringPart{};
            currentPart.alphabet = alphabet;
        }
        newStr.append(c);
    }

    newStr.toUTF8String(currentPart.str);
    v.push_back(currentPart);

    return v;
}