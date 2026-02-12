#include <string>
#include "ssml_preprocessor.hpp"
#include <pugixml.hpp>
#include <string>
#include <iostream>

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

std::string extract_text_from_ssml(const std::string& ssml) {
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


std::string getProcentage(int value) {
    return std::to_string(value) + std::string("%' ");
}

std::string processSSML(const std::string& text, int rate, int pitch, int volume) {
    std::string ssmlHead = "<speak version='1.0' xml:lang='ru-RU' xmlns='http://www.w3.org/2001/10/synthesis' xmlns:mstts='http://www.w3.org/2001/mstts'><prosody rate='";
    std::string rateProcentage = getProcentage(rate);
    ssmlHead = ssmlHead + rateProcentage + std::string(" pitch='") + getProcentage(pitch) + std::string(" volume='") + getProcentage(volume);

    return ssmlHead + std::string(">") + text + std::string("</prosody></speak>");
}