# /// script
# dependencies = [
#   "lxml",
#   "pyyaml",
# ]
# ///

import json
import yaml
import argparse
import sys
from lxml import html

def validate_relative(rule):
    if isinstance(rule, str):
        if not rule.startswith("."):
            raise ValueError(f"XSEE Context Leak: XPath '{rule}' must be relative (start with ./ or .// or .)")
    elif isinstance(rule, list):
        for item in rule:
            validate_relative(item)
    elif isinstance(rule, dict):
        for val in rule.values():
            validate_relative(val)

def extract_leaf(tree, xpath):
    results = tree.xpath(xpath)
    if not results:
        return None
    
    # If it's an attribute or string result
    if isinstance(results[0], str):
        return results[0].strip()
    
    # If it's an element, get text and normalize whitespace
    text = results[0].text_content()
    return " ".join(text.split()).strip()

def scrape_by_schema(content, schema):
    # If content is already an lxml element (passed from iterator), use it
    # Otherwise, parse the raw HTML string
    tree = content if hasattr(content, 'xpath') else html.fromstring(content)
    
    # 1. Leaf Pattern
    if isinstance(schema, str):
        return extract_leaf(tree, schema)

    # 3. Iterator Pattern (2-tuple list)
    if isinstance(schema, list):
        if len(schema) != 2:
            return [] # Or raise error based on preference
        
        selector_xpath, extractor = schema
        validate_relative(extractor)
        
        containers = tree.xpath(selector_xpath)
        if not containers:
            return []
            
        return [scrape_by_schema(c, extractor) for c in containers]

    # 2. Group Pattern
    if isinstance(schema, dict):
        result = {}
        for key, sub_rule in schema.items():
            result[key] = scrape_by_schema(tree, sub_rule)
        return result

    return None

def main():
    parser = argparse.ArgumentParser(description="XPath Structured Extraction Engine")
    parser.add_index = False # Not a standard argparse arg, just for clarity
    parser.add_argument("html_file", help="Path to the input HTML file")
    parser.add_argument("--yaml", required=True, help="Path to the XSEE YAML schema")
    
    args = parser.parse_args()

    try:
        with open(args.html_file, 'r', encoding='utf-8') as f:
            html_content = f.read()
        with open(args.yaml, 'r', encoding='utf-8') as f:
            schema = yaml.safe_load(f)

        data = scrape_by_schema(html_content, schema)
        
        # Output structured JSON to stdout
        print(json.dumps(data, indent=2))

    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    main()