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
import io
from lxml import html

# Fix for Windows/certain terminals to ensure utf-8 output piping
if sys.stdout.encoding != 'utf-8':
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')

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
    
    if isinstance(results[0], str):
        return results[0].strip()
    
    # .text_content() is better for extracting all nested text
    text = results[0].text_content()
    return " ".join(text.split()).strip()

def xsee(content, schema):
    # Check if content is raw bytes or a string (HTML)
    if isinstance(content, (str, bytes)):
        try:
            # Force lxml to treat the input as UTF-8
            parser = html.HTMLParser(encoding='utf-8')
            if isinstance(content, str):
                content = content.encode('utf-8')
            tree = html.fromstring(content, parser=parser)
        except Exception:
            return None
    else:
        tree = content
    
    if isinstance(schema, str):
        return extract_leaf(tree, schema)

    if isinstance(schema, list):
        if len(schema) != 2:
            return []
        
        selector_xpath, extractor = schema
        validate_relative(extractor)
        
        containers = tree.xpath(selector_xpath)
        if not containers:
            return []
            
        return [xsee(c, extractor) for c in containers]

    if isinstance(schema, dict):
        result = {}
        for key, sub_rule in schema.items():
            result[key] = xsee(tree, sub_rule)
        return result

    return None

def main():
    parser = argparse.ArgumentParser(description="XPath Structured Extraction Engine")
    parser.add_argument("html_file", help="Path to the input HTML file")
    parser.add_argument("--yaml", required=True, help="Path to the XSEE YAML schema")
    
    args = parser.parse_args()

    try:
        # Read HTML as raw bytes to prevent premature/incorrect decoding
        with open(args.html_file, 'rb') as f:
            html_bytes = f.read()
            
        with open(args.yaml, 'r', encoding='utf-8') as f:
            schema = yaml.safe_load(f)

        data = xsee(html_bytes, schema)
        
        print(json.dumps(data, indent=2, ensure_ascii=False))

    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    main()