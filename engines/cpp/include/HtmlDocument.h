#ifndef HTML_DOCUMENT_H
#define HTML_DOCUMENT_H

#include <string>
#include <vector>

class HtmlDocument {
public:
    struct HtmlElement { void* ptr; };

    explicit HtmlDocument(const std::string& path);
    ~HtmlDocument();

    HtmlElement getRoot() const;
    
    // Returns raw content; normalization happens in the Engine
    std::string getRawXPathContent(HtmlElement element, const std::string& xpath) const;
    std::vector<HtmlElement> queryElements(HtmlElement element, const std::string& xpath) const;

private:
    void* doc_ptr; 
};

#endif