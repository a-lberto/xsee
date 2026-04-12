#ifndef ENGINE_H
#define ENGINE_H

#include "HtmlDocument.h"
#include <yaml-cpp/yaml.h>
#include <nlohmann/json.hpp>
#include <string>
#include <fstream>
#include <sstream>

using json = nlohmann::ordered_json;

class Engine {
public:
    // --- Utilities ---
    static std::string readFile(const std::string& path) {
        std::ifstream in(path, std::ios::binary);
        if (!in.is_open()) throw std::runtime_error("File not found: " + path);
        std::stringstream buffer;
        buffer << in.rdbuf();
        return buffer.str();
    }

    static std::string normalizeSpace(const std::string& str) {
        std::string result;
        bool in_space = false;
        for (unsigned char c : str) {
            if (std::isspace(c)) {
                if (!in_space && !result.empty()) {
                    result += ' ';
                    in_space = true;
                }
            } else {
                result += (char)c;
                in_space = false;
            }
        }
        if (!result.empty() && std::isspace((unsigned char)result.back())) result.pop_back();
        return result;
    }

    // --- Core Functionality ---
    static json execute(const HtmlDocument& doc, const YAML::Node& schema) {
        return process(doc, doc.getRoot(), schema);
    }

private:
    static void validateRelative(const YAML::Node& node) {
        if (node.IsScalar()) {
            if (node.as<std::string>().find(".") != 0) 
                throw std::runtime_error("Context Leak: XPath must be relative (.)");
        } else if (node.IsSequence()) {
            for (const auto& item : node) validateRelative(item);
        } else if (node.IsMap()) {
            for (const auto& it : node) validateRelative(it.second);
        }
    }

    static json process(const HtmlDocument& doc, HtmlDocument::HtmlElement element, const YAML::Node& schema) {
        if (schema.IsScalar()) {
            std::string raw = doc.getRawXPathContent(element, schema.as<std::string>());
            std::string clean = normalizeSpace(raw);
            return clean.empty() ? json(nullptr) : json(clean);
        }

        if (schema.IsSequence()) {
            if (schema.size() != 2) return json::array();
            
            std::string selector = schema[0].as<std::string>();
            YAML::Node extractor = schema[1];
            validateRelative(extractor);

            auto targetElements = doc.queryElements(element, selector);
            json list = json::array();
            for (auto& el : targetElements) {
                list.push_back(process(doc, el, extractor));
            }
            return list;
        }

        if (schema.IsMap()) {
            json obj = json::object();
            for (auto it = schema.begin(); it != schema.end(); ++it) {
                obj[it->first.as<std::string>()] = process(doc, element, it->second);
            }
            return obj;
        }
        return nullptr;
    }
};

#endif