#include <string>
#include "ssml_preprocessor.hpp"
#include <pugixml.hpp>
#include <string>
#include <iostream>
#include <sstream>

void collect_text(const pugi::xml_node& node, std::string& out) {
    for (auto child : node.children()) {
        if (child.type() == pugi::node_pcdata ||
            child.type() == pugi::node_cdata) {
            out += child.value();
            out += ' ';
        } else {
            collect_text(child, out);
        }
    }
}

std::string extractTextFromSSML(const std::string& ssml) {
    try {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_string(ssml.c_str());

    if (!result) {
        throw std::runtime_error("Invalid SSML/XML");
    }

    std::string text;
    collect_text(doc, text);
    return text;
} catch (std::runtime_error& e) {
    return ssml;
} 
}

std::string createSpeakSSML(const std::string& text, int rate, int volume, int pitch)
{
    pugi::xml_document doc;

    pugi::xml_node speak = doc.append_child("speak");
    speak.append_attribute("version") = "1.0";
    speak.append_attribute("xml:lang") = "ru-RU";
    speak.append_attribute("xmlns") = "http://www.w3.org/2001/10/synthesis";
    speak.append_attribute("xmlns:mstts") = "http://www.w3.org/2001/mstts";

    pugi::xml_node prosody = speak.append_child("prosody");

    std::string rate_str = std::to_string(rate) + "%";
    std::string volume_str = std::to_string(volume) + "%";
    std::string pitch_str = std::to_string(pitch) + "%";

    prosody.append_attribute("rate") = rate_str.c_str();
    prosody.append_attribute("volume") = volume_str.c_str();
    prosody.append_attribute("pitch") = pitch_str.c_str();

    prosody.text().set(text.c_str());

    std::ostringstream oss;
    doc.save(oss, "", pugi::format_raw);

    return oss.str();
}