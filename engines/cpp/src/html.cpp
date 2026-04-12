#include "html.h"
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <stdexcept>

namespace html {

Document::Document(const std::string& path) {
    doc_ptr = htmlReadFile(
        path.c_str(), 
        "UTF-8", 
        HTML_PARSE_RECOVER | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING
    );
    
    if (!doc_ptr) {
        throw std::runtime_error("Failed to parse HTML document: " + path);
    }
}

Document::~Document() {
    if (doc_ptr) {
        xmlFreeDoc((xmlDocPtr)doc_ptr);
    }
    // Note: xmlCleanupParser() is global. If you have multiple threads 
    // or docs, call this only when the application exits.
}

Element Document::getRoot() const {
    return { (void*)xmlDocGetRootElement((xmlDocPtr)doc_ptr) };
}

std::string Document::getRawXPathContent(Element element, const std::string& xpath) const {
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

std::vector<Element> Document::queryElements(Element element, const std::string& xpath) const {
    std::vector<Element> elements;
    xmlXPathContextPtr context = xmlXPathNewContext((xmlDocPtr)doc_ptr);
    context->node = (xmlNodePtr)element.ptr;
    
    xmlXPathObjectPtr result = xmlXPathEvalExpression((const xmlChar*)xpath.c_str(), context);

    if (result && !xmlXPathNodeSetIsEmpty(result->nodesetval)) {
        elements.reserve(result->nodesetval->nodeNr);
        for (int i = 0; i < result->nodesetval->nodeNr; ++i) {
            elements.push_back({ (void*)result->nodesetval->nodeTab[i] });
        }
    }

    xmlXPathFreeObject(result);
    xmlXPathFreeContext(context);
    return elements;
}

} // namespace html