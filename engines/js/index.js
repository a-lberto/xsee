#!/usr/bin/env node
const { JSDOM } = require("jsdom");
const yaml = require("js-yaml");
const fs = require("fs");
const path = require("path");

function xseeEngine(html, schema) {
  const dom = new JSDOM(html);
  const doc = dom.window.document;

  // Enforces context isolation rule
  const validateRelative = (rule) => {
    if (typeof rule === "string") {
      if (!rule.startsWith(".")) {
        throw new Error(`XSEE Context Leak: XPath "${rule}" must be relative (start with ./ or .// or .)`);
      }
    } else if (Array.isArray(rule)) {
      validateRelative(rule[0]);
      validateRelative(rule[1]);
    } else if (typeof rule === "object" && rule !== null) {
      for (const val of Object.values(rule)) {
        validateRelative(val);
      }
    }
  };

  // Helper to evaluate XPath and get text/attribute
 const extractLeaf = (context, xpath) => {
  // Use 9 (FIRST_ORDERED_NODE_TYPE) to get the first match specifically
  const result = doc.evaluate(xpath, context, null, 9, null); 

  // Handle Boolean, Number, String results (though rare in basic XSEE)
  if (result.resultType === 1) return result.numberValue;
  if (result.resultType === 2) return result.stringValue.trim();
  if (result.resultType === 3) return result.booleanValue;

  const node = result.singleNodeValue; // Use singleNodeValue for type 9
  
  // XSEE Requirement: normalize-space() equivalent
  return node ? node.textContent.replace(/\s+/g, ' ').trim() : null;
};

  const processNode = (context, rule) => {
    // 1. Leaf Pattern
    if (typeof rule === "string") {
      return extractLeaf(context, rule);
    }

    // 3. Iterator Pattern (2-tuple Array)
    if (Array.isArray(rule)) {
      const [selector, extractor] = rule;
      
      // Enforce relative path rule for extractors to prevent context leak
      validateRelative(extractor); 

      const nodes = [];
      const query = doc.evaluate(selector, context, null, 4, null); // 4 is UNORDERED_NODE_ITERATOR_TYPE
      
      let node = query.iterateNext();
      while (node) {
        nodes.push(node);
        node = query.iterateNext();
      }

      if (nodes.length === 0) return []; // Return [] if selector finds nothing

      // Process and filter out nulls if the extractor finds nothing
      return nodes
        .map(itemNode => processNode(itemNode, extractor))
        .filter(res => res !== null); 
    }

    // 2. Group Pattern
    if (typeof rule === "object" && rule !== null) {
      const groupResult = {};
      for (const [key, subRule] of Object.entries(rule)) {
        groupResult[key] = processNode(context, subRule);
      }
      return groupResult;
    }

    return null;
  };

  return processNode(doc, schema);
}

// --- CLI Parsing & Execution ---
function main() {
  const args = process.argv.slice(2);
  let htmlPath = null;
  let yamlPath = null;

  // Simple argument parser
  for (let i = 0; i < args.length; i++) {
    if (args[i] === "--yaml" && i + 1 < args.length) {
      yamlPath = args[i + 1];
      i++; // skip the value
    } else if (!htmlPath) {
      // First non-flag argument is treated as the input HTML
      htmlPath = args[i];
    }
  }

  // Validation
  if (!htmlPath || !yamlPath) {
    console.error("Usage: node drivers/js/index.js <input.html> --yaml <schema.xsee.yaml>");
    process.exit(1);
  }

  try {
    // Resolve paths relative to the current working directory (project root)
    const absoluteHtmlPath = path.resolve(process.cwd(), htmlPath);
    const absoluteYamlPath = path.resolve(process.cwd(), yamlPath);

    if (!fs.existsSync(absoluteHtmlPath)) throw new Error(`HTML file not found: ${absoluteHtmlPath}`);
    if (!fs.existsSync(absoluteYamlPath)) throw new Error(`YAML file not found: ${absoluteYamlPath}`);

    const inputHtml = fs.readFileSync(absoluteHtmlPath, "utf-8");
    const xseeYaml = fs.readFileSync(absoluteYamlPath, "utf-8");

    const schema = yaml.load(xseeYaml);
    const output = xseeEngine(inputHtml, schema);

    console.log(JSON.stringify(output, null, 2));

  } catch (e) {
    console.error("Error:", e.message);
    process.exit(1);
  }
}

// Run the script
if (require.main === module) {
  main();
}