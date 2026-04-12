#include "HtmlDocument.h"
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <stdexcept>

HtmlDocument::HtmlDocument(const std::string& path) {
    doc_ptr = htmlReadFile(
        path.c_str(), 
        "UTF-8", 
        HTML_PARSE_RECOVER | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING
    );
    
    if (!doc_ptr) throw std::runtime_error("Failed to parse HTML document");
}

HtmlDocument::~HtmlDocument() {
    if (doc_ptr) xmlFreeDoc((xmlDocPtr)doc_ptr);
    xmlCleanupParser();
}

HtmlDocument::HtmlElement HtmlDocument::getRoot() const {
    return { (void*)xmlDocGetRootElement((xmlDocPtr)doc_ptr) };
}

std::string HtmlDocument::getRawXPathContent(HtmlElement element, const std::string& xpath) const {
    xmlXPathContextPtr context = xmlXPathNewContext((xmlDocPtr)doc_ptr);
    context->node = (xmlNodePtr)element.ptr;
    xmlXPathObjectPtr result = xmlXPathEvalExpression((const xmlChar*)xpath.c_str(), context);

    std::string text;
    if (result && !xmlXPathNodeSetIsEmpty(result->nodesetval)) {
        xmlChar* raw = xmlNodeGetContent(result->nodesetval->nodeTab[0]);
        if (raw) {
            text = (char*)raw;
            xmlFree(raw);
        }
    }

    xmlXPathFreeObject(result);
    xmlXPathFreeContext(context);
    return text;
}

std::vector<HtmlDocument::HtmlElement> HtmlDocument::queryElements(HtmlElement element, const std::string& xpath) const {
    std::vector<HtmlElement> elements;
    xmlXPathContextPtr context = xmlXPathNewContext((xmlDocPtr)doc_ptr);
    context->node = (xmlNodePtr)element.ptr;
    xmlXPathObjectPtr result = xmlXPathEvalExpression((const xmlChar*)xpath.c_str(), context);

    if (result && !xmlXPathNodeSetIsEmpty(result->nodesetval)) {
        for (int i = 0; i < result->nodesetval->nodeNr; ++i) {
            elements.push_back({ (void*)result->nodesetval->nodeTab[i] });
        }
    }

    xmlXPathFreeObject(result);
    xmlXPathFreeContext(context);
    return elements;
}