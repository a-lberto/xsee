
<p align="center">
  <img src="docs/images/logo.png" alt="Eye-glasses with attached multiple stacking lenses" width="300">
</p>

<h1 align="center">xsee</h1>

<p align="center">
  XPaths in a YAML get you clean JSON from HTML.
</p>

---

Tired of writing yet another scraping script? Tired to recompile your scrapers after an XPath changes?

By using a simple strict YAML format as input, `xsee` allows you to turn an HTML page into a fine tuned JSON.

No release yet as it is still in very early development.

## Project structure

```text
xsee/
├── docs
├── engines                   # Different implementations
│   ├── cpp
│   ├── js
│   └── python
├── scripts                   # Small utility scripts
└── tests                     # Shared tests folder
    ├── example               # Regression test examples
    │   ├── expected.json     # Expected output
    │   ├── input.html        # Input to process
    │   └── xsee.yaml         # Minimal scraping config
    └── test.py               # Runs for every engine, every test
```

---

## Example

Imagine you have a messy HTML page

<details>
<summary><b>View Messy Source HTML</b></summary>

:[Input HTML](../tests/example/input.html)

</details>

Once you have found the XPaths that lead to your desired information and compiled the `xsee.yaml`

:[Config](../tests/example/xsee.yaml)

And run `xsee` with

```shell
curl http://yourfavoritewebsite/ > input.html
xsee input.html --yaml xsee.yaml
```

You directly get this as output

<details>
<summary><b>View Structured JSON Output</b></summary>

:[Expected Output](../tests/example/expected.json)

</details>

---

## Rules of thumb

XSEE replaces procedural scraping scripts with a **structural contract**, treating the DOM as a queryable data source.

XSEE is text first, and explicitly does no data processing other than extracting raw information for the DOM. Processing is left to be done to other tools of your choice.

XSEE uses XPath 1.0 for best portability. The implementation of XSEE applies a `normalize-space()` to the string extracted.

## The patterns

XSEE uses three simple patterns to map DOM elements to data:

1. **Leaf**: `key: "xpath"` e.g. `title: "//h1"`, `url : "//a/@href"`
   
   Extracts **only** the `textContent` of the first matching node or its first XPath attribute selector (e.g., `@src`, `@href`, `@content`). Returns `null` if not found.  
   
2. **Group**: `key: {group}` e.g. `meta: { author: "//span", date: "//time" }`

   Used to group related data.
   
3. **Iterator**: `key: ["selector", "extractor"]` 
   e.g. `related_articles: [ "//li", ".//a/@href" ]`

   This is the only allowed type of list (2-tuple). Iterates over all objects in the DOM found by the **first** XPath _selector_ and applies the **second** _extractor_ to each element.
   
   The extractor can be either a single `"xpath"` or a `{group}`.
   Extraction XPaths must use the `./` or `.//` relative prefix to remain relative to the parent and prevent context leak (the engine must enforce this by throwing an error).
   
   If selector finds no results, returns `[]`, if extractor finds no results, it returns `[]`. Leafs inside groups are handled normally as `null` when leaf is not found.


## References

- [XPath Spec](https://www.w3.org/TR/1999/REC-xpath-19991116/)
- [YAML Spec](https://yaml.org/spec/)
- [JSON Spec](https://www.json.org/json-en.html)
