# XPath Structured Extraction

> XSE is to HTML what SQL is to Databases: A structured, declarative way to query and shape information.

**XSE** eliminates procedural scraping code by treating the extraction map as a structural contract.

---

## The rules

Any valid XSE map must be composed exclusively of these structures, which will extract text from the DOM:

1. The **leaf** (string): A key-value pair where the value is a valid **XPath**.
2. The **scope** (object): A nested object used to group related data. To prevent context leaking, all XPaths nested inside a **scope** must be relative to the current node.
4. The **iterator** (pair): An ordered pair  defined as `["xpath", {object}]`, which iterates for any matching XPath the structured extraction specified in the object.

---

## Example (`store.xse.yaml`)

```yaml
store_name: "//h1"
catalog:
  - "//div[@class='product']"    # For each product <div class="product"></div>, find
  - name: ".//h2"                # product name in text inside <h2></h2>
    price: ".//span[@class='p']" # price in text inside <span class="p"></span>
    tags:                        # tags, which are as
      - ".//li"                  # <li></li> items
      - "."                      # the text inside
```
