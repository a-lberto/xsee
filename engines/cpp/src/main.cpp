#include "html.h"
#include "Engine.h"

#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <html_file> --yaml <yaml_file>" << std::endl;
        return 1;
    }

    try {
        std::string html_path = argv[1];
        std::string yaml_path;

        for (int i = 1; i < argc; ++i) {
            if (std::string(argv[i]) == "--yaml" && i + 1 < argc) {
                yaml_path = argv[i + 1];
            }
        }

        if (yaml_path.empty()) throw std::runtime_error("Missing --yaml argument");

        // Load HTML into the Document wrapper
        html::Document doc(html_path);

        // Load the Schema
        YAML::Node schema = YAML::LoadFile(yaml_path);

        // Execute the Engine
        json output = Engine::run(doc, schema);

        // Output the result
        std::cout << output.dump(2) << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Critical Engine Failure: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}