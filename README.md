# XPath Structured Extraction

> XSE is to HTML what SQL is to Databases: A declarative way to query and shape unstructured web data into structured objects.

XSE replaces procedural scraping scripts with a **structural contract**, treating the DOM as a queryable data source.

---

## The rules

An XSE map is composed of three primitive structures used to extract text from the DOM:

1. Leaf (String): A key-value pair where the value is a valid XPath targeting a single value.
2. Scope (Object): A nested object used to group related data. To ensure encapsulation, all internal XPaths must be relative to the parent node.
3. Iterator (Array): A pair defined as `["xpath", {template}]`. It iterates over every match of the XPath, applying the template to each instance.

---

## Example

Imagine you have a messy HTML page

<details>
<summary><b>View Messy Source HTML</b></summary>

```html
<header class="site-header-v2">
  <div class="banner-ad">Buy Crypto Now!</div>
  <h1>Tech Gadget Emporium</h1> 
</header>

<main id="content-7721">
  <section class="grid-layout">
    <div class="product card-style-prime">
      <div class="img-wrapper">
        <img src="kb.jpg" />
        <span class="tooltip">Bestseller</span>
      </div>
      <div class="details">
        <h2 class="title">Mechanical Keyboard</h2> 
        <div class="price-container">
          <span class="p">$120</span> 
          <span class="old-price">$150</span>
        </div>
        <ul class="tag-cloud">
          <li>peripherals</li>
          <li>gaming</li>
          <li>usb-c</li>
        </ul>
      </div>
      <script>trackImpression('prod_01');</script>
    </div>

    <div class="spacer-ads">Some Garbled Mess</div>

    <div class="product card-style-prime">
      <h2 class="title">Wireless Mouse</h2>
      <span class="p">$60</span>
      <ul class="tag-cloud">
        <li>ergonomic</li>
        <li>battery-powered</li>
      </ul>
    </div>
  </section>
</main>
```
</details>

Once you have found the XPaths that lead to your desired information and compiled the `store.xse.yaml`
```yaml
store_name: "//h1"
catalog:
  - "//div[@class='product']"    # Iterator: Find all product containers
  - name: ".//h2"                # Relative Leaf: Extract title
    price: ".//span[@class='p']" # Relative Leaf: Extract price
    tags:                        # Nested Iterator:
      - ".//li"                  # Find all list items
      - "."                      # Extract current node text
```

You directly get this as output

<details>
<summary><b>View Structured JSON Output</b></summary>

```json
{
  "store_name": "Tech Gadget Emporium",
  "catalog": [
    {
      "name": "Mechanical Keyboard",
      "price": "$120",
      "tags": ["peripherals", "gaming", "usb-c"]
    },
    {
      "name": "Wireless Mouse",
      "price": "$60",
      "tags": ["ergonomic", "battery-powered"]
    }
  ]
}
```

</details>

You can also prepare the XSE as a `store.xse.json`

```json
{
  "store_name": "//h1",
  "catalog": [
    "//div[@class='product']",
    {
      "name": ".//h2",
      "price": ".//span[@class='p']",
      "tags": [
        ".//li",
        "."
      ]
    }
  ]
}
```

but, as you can see, YAML is less verbose.
