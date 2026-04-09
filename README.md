# XPath Structured Extraction Engine

<p align="center">
  <img src="docs/images/logo.png" alt="Eye-glasses with attached multiple stacking lenses" width="600">
</p>

> XSEE is to HTML what SQL is to Databases: A declarative way to query and shape unstructured (or deliberately obfuscated) web data into structured objects.

XSEE replaces procedural scraping scripts with a **structural contract**, treating the DOM as a queryable data source.

XSEE is text first, and explicitly does no data processing other than extracting raw information for the DOM. Processing is left to be done to other tools of your choice.

XSEE uses XPath 1.0 for best portability. The implementation of XSEE applies a `normalize-space()` to the string extracted.

---

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

Once you have found the XPaths that lead to your desired information and compiled the `example.xsee.yaml`

```yaml
store_name: "//h1"
catalog:
  - "//div[contains(@class, 'product')]"
  - name: ".//h2"
    price: ".//span[@class='p']"
    tags: [ ".//li", "." ]
```

And run your engine of choice with

```shell
curl http://yourfavoritewebsite/ > input.html
xsee input.html --yaml xsee.yaml
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
