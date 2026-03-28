#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <yaml-cpp/yaml.h>
#include <nlohmann/json.hpp>

using json = nlohmann::ordered_json;

// --- Utilities ---

std::string read_file(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        throw std::runtime_error("Could not open file: " + path);
    }
    std::stringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

std::string normalize_space(const std::string& str) {
    std::string result;
    bool in_space = false;
    for (char c : str) {
        if (std::isspace(c)) {
            if (!in_space && !result.empty()) {
                result += ' ';
                in_space = true;
            }
        } else {
            result += c;
            in_space = false;
        }
    }
    if (!result.empty() && std::isspace(result.back())) result.pop_back();
    return result;
}

void validate_relative(const YAML::Node& node) {
    if (node.IsScalar()) {
        std::string val = node.as<std::string>();
        if (val.find(".") != 0) {
            throw std::runtime_error("XSE Context Leak: XPath '" + val + "' must be relative (start with .)");
        }
    } else if (node.IsSequence()) {
        for (const auto& item : node) validate_relative(item);
    } else if (node.IsMap()) {
        for (const auto& it : node) validate_relative(it.second);
    }
}

// --- Core Engine ---

json scrape_leaf(xmlNodePtr node, const std::string& xpath_str) {
    xmlXPathContextPtr context = xmlXPathNewContext(node->doc);
    context->node = node;
    xmlXPathObjectPtr result = xmlXPathEvalExpression((const xmlChar*)xpath_str.c_str(), context);

    if (!result || xmlXPathNodeSetIsEmpty(result->nodesetval)) {
        xmlXPathFreeObject(result);
        xmlXPathFreeContext(context);
        return nullptr;
    }

    xmlChar* raw = xmlNodeGetContent(result->nodesetval->nodeTab[0]);
    std::string text = normalize_space((char*)raw);
    xmlFree(raw);
    
    xmlXPathFreeObject(result);
    xmlXPathFreeContext(context);
    return text;
}

json scrape_by_schema(xmlNodePtr node, const YAML::Node& schema) {
    if (schema.IsScalar()) {
        return scrape_leaf(node, schema.as<std::string>());
    }

    if (schema.IsSequence()) {
        if (schema.size() != 2) return json::array();
        
        std::string selector = schema[0].as<std::string>();
        YAML::Node extractor = schema[1];
        validate_relative(extractor);

        xmlXPathContextPtr context = xmlXPathNewContext(node->doc);
        context->node = node;
        xmlXPathObjectPtr result = xmlXPathEvalExpression((const xmlChar*)selector.c_str(), context);

        json list = json::array();
        if (result && !xmlXPathNodeSetIsEmpty(result->nodesetval)) {
            for (int i = 0; i < result->nodesetval->nodeNr; ++i) {
                list.push_back(scrape_by_schema(result->nodesetval->nodeTab[i], extractor));
            }
        }
        xmlXPathFreeObject(result);
        xmlXPathFreeContext(context);
        return list;
    }

    if (schema.IsMap()) {
        json obj = json::object();
        for (auto it = schema.begin(); it != schema.end(); ++it) {
            obj[it->first.as<std::string>()] = scrape_by_schema(node, it->second);
        }
        return obj;
    }

    return nullptr;
}

// --- Main with Argument Parsing ---

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <html_file> --yaml <yaml_file>" << std::endl;
        return 1;
    }

    std::string html_path = argv[1];
    std::string yaml_path;

    // Simple check for --yaml flag
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--yaml" && i + 1 < argc) {
            yaml_path = argv[i + 1];
        }
    }

    if (yaml_path.empty()) {
        std::cerr << "Error: --yaml argument is required." << std::endl;
        return 1;
    }

    try {
        std::string html_content = read_file(html_path);
        htmlDocPtr doc = htmlReadMemory(html_content.c_str(), html_content.length(), NULL, NULL, HTML_PARSE_RECOVER | HTML_PARSE_NOERROR);
        
        YAML::Node schema = YAML::LoadFile(yaml_path);
        json output = scrape_by_schema(xmlDocGetRootElement(doc), schema);
        
        std::cout << output.dump(2) << std::endl;

        xmlFreeDoc(doc);
        xmlCleanupParser();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}