#ifndef HTML_DOCUMENT_H
#define HTML_DOCUMENT_H

#include <string>
#include <vector>

namespace html {

struct Element {
    void* ptr;
};

class Document {
public:
    explicit Document(const std::string& path);
    ~Document();

    // Prevent copying to avoid double-free of doc_ptr
    Document(const Document&) = delete;
    Document& operator=(const Document&) = delete;

    Element getRoot() const;
    
    std::string getRawXPathContent(Element element, const std::string& xpath) const;
    std::vector<Element> queryElements(Element element, const std::string& xpath) const;

private:
    void* doc_ptr; 
};

} // namespace html

#endif