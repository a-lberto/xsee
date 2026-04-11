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

std::string readFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary); // Open in binary mode for UTF-8 safety
    if (!in.is_open()) {
        throw std::runtime_error("Could not open file: " + path);
    }
    std::stringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

std::string normalizeSpace(const std::string& str) {
    std::string result;
    bool in_space = false;
    for (unsigned char c : str) { // Use unsigned char for UTF-8 safety
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

void validateRelative(const YAML::Node& node) {
    if (node.IsScalar()) {
        std::string val = node.as<std::string>();
        if (val.find(".") != 0) {
            throw std::runtime_error("XSE Context Leak: XPath '" + val + "' must be relative (start with .)");
        }
    } else if (node.IsSequence()) {
        for (const auto& item : node) validateRelative(item);
    } else if (node.IsMap()) {
        for (const auto& it : node) validateRelative(it.second);
    }
}

json scrapeLeaf(xmlNodePtr node, const std::string& xpath_str) {
    xmlXPathContextPtr context = xmlXPathNewContext(node->doc);
    context->node = node;
    xmlXPathObjectPtr result = xmlXPathEvalExpression((const xmlChar*)xpath_str.c_str(), context);

    if (!result || xmlXPathNodeSetIsEmpty(result->nodesetval)) {
        xmlXPathFreeObject(result);
        xmlXPathFreeContext(context);
        return nullptr;
    }

    xmlChar* raw = xmlNodeGetContent(result->nodesetval->nodeTab[0]);
    std::string text = normalizeSpace((char*)raw);
    xmlFree(raw);
    
    xmlXPathFreeObject(result);
    xmlXPathFreeContext(context);
    return text;
}

json xsee(xmlNodePtr node, const YAML::Node& schema) {
    if (schema.IsScalar()) {
        return scrapeLeaf(node, schema.as<std::string>());
    }

    if (schema.IsSequence()) {
        if (schema.size() != 2) return json::array();
        
        std::string selector = schema[0].as<std::string>();
        YAML::Node extractor = schema[1];
        validateRelative(extractor);

        xmlXPathContextPtr context = xmlXPathNewContext(node->doc);
        context->node = node;
        xmlXPathObjectPtr result = xmlXPathEvalExpression((const xmlChar*)selector.c_str(), context);

        json list = json::array();
        if (result && !xmlXPathNodeSetIsEmpty(result->nodesetval)) {
            for (int i = 0; i < result->nodesetval->nodeNr; ++i) {
                list.push_back(xsee(result->nodesetval->nodeTab[i], extractor));
            }
        }
        xmlXPathFreeObject(result);
        xmlXPathFreeContext(context);
        return list;
    }

    if (schema.IsMap()) {
        json obj = json::object();
        for (auto it = schema.begin(); it != schema.end(); ++it) {
            obj[it->first.as<std::string>()] = xsee(node, it->second);
        }
        return obj;
    }

    return nullptr;
}


int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <html_file> --yaml <yaml_file>" << std::endl;
        return 1;
    }

    std::string html_path = argv[1];
    std::string yaml_path;

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
        std::string html_content = readFile(html_path);
        
        htmlDocPtr doc = htmlReadMemory(
            html_content.c_str(), 
            (int)html_content.length(), 
            html_path.c_str(), 
            "UTF-8", 
            HTML_PARSE_RECOVER | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING
        );

        if (!doc) {
            throw std::runtime_error("Failed to parse HTML document.");
        }
        
        YAML::Node schema = YAML::LoadFile(yaml_path);
        json output = xsee(xmlDocGetRootElement(doc), schema);
        
        std::cout << output.dump(2) << std::endl;

        xmlFreeDoc(doc);
        xmlCleanupParser();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}