/*
 *  XercesStream.cxx
 *  Test
 *
 *  Created by Matthias Wiesmann on 12.01.05.
 *  Copyright 2005 CERN. All rights reserved.
 *
 */

#include <sstream>
#include <iostream>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/StdOutFormatTarget.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/parsers/AbstractDOMParser.hpp>

#include "ers/XercesStream.h"
#include "ers/XercesString.h"
#include "ers/Precondition.h"
#include "ers/InvalidReferenceIssue.h"
#include "ers/ParseIssue.h"
#include "ers/NotImplemented.h"

/** Stuff needed to initialise the DOM implementation. 
  * Probably a list off character sets 
  */

static const XMLCh gLS[] = { chLatin_L, chLatin_S, chNull };

/** Constructor - builds a stream that will write into a file
  * \param file the file data will be written into 
  */

ers::XercesStream::XercesStream(const System::File &file) : Stream() ,DOMErrorHandler() {
    m_file_path = file.full_name(); 
} // XercesStream

/** Destructor 
  */

ers::XercesStream::~XercesStream() {} //~XercesStream

// ----------------------
// Error Handling methods
// ----------------------

/** Extracts an error message out of a node
  * \param pointer to the node to information about
  * \return a string containg the name and the data for the node separated by a colon (:) 
  */

std::string ers::XercesStream::error_msg(const DOMNode *node) {
    const XMLCh* data = node->getNodeValue(); 
    const XMLCh* name = node->getNodeName() ; 
    std::ostringstream element_stream ; 
    element_stream <<  to_string(name) << ":" << to_string(data)  ; 
    return element_stream.str() ; 
} // error_msg

/** Converts a DOM error into an Issue (ParseIssue). 
  * The following conversions are done: 
  * \li The severity is translated (warning becomes a warning and both error and fatal become errors).
  * \li The error message is copied without changes 
  * \li The line number is inserted into the \c ParseIssue using method \c offending_line_number() 
  * \li The name of the file is inserted into the \c ParseIssue using method \c file_name()
  * \li The problematic node is inserted into the \c ParseIssue using method \c offending_line()
  * \param domError the DOM error
  * \return An Issue object with all the relevant fields set up 
  */

ers::Issue ers::XercesStream::to_issue(const DOMError& domError){
    const short x_severity = domError.getSeverity();
    ers::ers_severity severity = ers_error ;
    switch (x_severity) {
	case DOMError::DOM_SEVERITY_WARNING: severity = ers_warning ; break ;
	case DOMError::DOM_SEVERITY_ERROR: severity = ers_error ; break ; 
	case DOMError::DOM_SEVERITY_FATAL_ERROR: severity = ers_error ; break ; 
	default: 
	    ERS_NOT_IMPLEMENTED(); 
	    break ; 
    } // switch 
    std::string message = to_string(domError.getMessage()); 
    ParseIssue issue(ERS_HERE,severity,message);
    DOMLocator *location = domError.getLocation(); 
    const int line_number = location->getLineNumber() ;
    issue.offending_line_number(line_number); 
    std::string uri = to_string(location->getURI());
    issue.file_name(uri); 
    DOMNode *node = location->getErrorNode(); 
    std::string node_name = error_msg(node); 
    issue.offending_line(node_name);
    return issue ; 
} // 

/** This method is called when a node cannot be parsed.
  * It throws an Issue with the relevant fields set up 
  * \param node the node that cannot be parsed
  */

void ers::XercesStream::cannot_parse(const DOMNode *node) const {
    throw ERS_PARSE_ERROR("Cannot parse element '%s'",error_msg(node).c_str()); 
} // cannot_parse

/** DOM Error handling method - this method is called when the DOM parser encounters an error.
  * If the error is a warning, it is send to the appropriate stream. 
  * If it is an error, the appropriate Issue is build and thrown as an exception 
  * \param domError the actual error encoutered 
  * \return either \c true or an exception is thrown 
  */
  
bool ers::XercesStream::handleError(const DOMError& domError) {
    Issue issue = to_issue(domError); 
    dispatch(&issue,true); 
    return true  ; 
} //

/** Extracts the text part of a node 
  * \param node the node to extract text from 
  * \return a string with the text part, an empty string if no text is available 
  */

std::string ers::XercesStream::get_text(const DOMNode *node) {
    const XMLCh* data = node->getNodeValue(); 
    if (data) return to_string(data); 
    return "" ; 
} // get_text

/** Extracts the text for an element.
  * This is the text of the first child of the element. 
  * I.e this method assumes that the element does not contain further elements 
  * \param element the element to extract the text from 
  * \return a string containing the text
  */

std::string ers::XercesStream::get_text(const DOMElement *element) {
    const DOMNode* child = element->getFirstChild() ; 
    return get_text(child); 
}  // get_text

/** Parses an element (i.e a tag like \<key\>)
  * \li If the element is a key tag, its content is taken as a key name and put into parameter \c key
  * \li If the element is a string tag, its content is taken as a value, and it is put with parameter \c key
  * Into the \c values. 
  * \param the element to parse
  * \param key the last key 
  * \values the table containing all key and values
  */

void ers::XercesStream::parse(const DOMElement *element, std::string &key, string_map_type &values) const {
    ERS_PRE_CHECK_PTR(element); 
    const XMLCh *x_tag = element->getTagName() ;  
    std::string tag = to_string(x_tag); 
    if (tag == ers::Core::XML_KEY_TAG) {
	key = get_text(element); 
	return ; 
    } // Key 
    if (tag == ers::Core::XML_STRING_VALUE_TAG) {
	values[key] = get_text(element);
	return ;
    } // String value
    throw ERS_PARSE_ERROR("Unknown tag type: %s",tag); 
} // parse

/** Parses a node 
  * If the node is an element method \c parse is called with the converted parameter. 
  * Text and comment nodes are ignored. 
  */

void ers::XercesStream::parse(const DOMNode *node, std::string &key, string_map_type &values) const {
    ERS_PRE_CHECK_PTR(node); 
    if (node->getNodeType()==DOMNode::ELEMENT_NODE) {
	const DOMElement *element = dynamic_cast <const DOMElement *> (node) ;
	parse(element,key,values); 
	return ; 
    } // Element node
    if (node->getNodeType()==DOMNode::TEXT_NODE) {
	return ; 
    } // TEXT node
    if (node->getNodeType()==DOMNode::COMMENT_NODE) {
#ifdef XERCES_STREAM_LOG_COMMENT
	std::string text = get_text(node) ; 
	ERS_DEBUG_3(text.c_str()) ; 
#endif
	return ; 
    } // TEXT node
    cannot_parse(node); 
} // parse

/** Read an Issue out of a root element. 
  * This method assumes that the issue is of the right type (i.e the root \<issue\> tag). 
  * \param issue_element_ptr pointer to the root element 
  * \return an issue if it could be read, or a null pointer if there was no issue in the element 
  */

ers::Issue *ers::XercesStream::receive(const DOMElement *issue_element_ptr) const {
    string_map_type values ;
    std::string key ;
    DOMNode *child = issue_element_ptr->getFirstChild() ; 
    while(child!=0) {
	parse(child,key,values); 
	child=child->getNextSibling() ; 
    } //
    std::string class_name = values[Issue::CLASS_KEY] ; 
    Issue *issue_ptr = ers::IssueFactory::instance()->build(class_name,&values); 
    return issue_ptr ;
} // receive

/** Reads an Issue out of DOM document 
  * \param document_ptr pointer to the root document where the issue should be 
  * \return an issue if it could be read, or a null pointer if there was no issue in the document 
  */

ers::Issue *ers::XercesStream::receive(const DOMDocument *document_ptr) const {
    ERS_PRE_CHECK_PTR(document_ptr);
    const DOMNode *issue_node = document_ptr->getFirstChild();
    if (! issue_node) return 0 ;     
    if (issue_node->getNodeType()!=DOMNode::ELEMENT_NODE) { cannot_parse(issue_node); }
    const DOMElement *issue_element = dynamic_cast <const DOMElement *> (issue_node) ;
    ERS_PRE_CHECK_PTR(issue_element);
    std::string issue_name = to_string(issue_element->getTagName()) ; 
    if (issue_name!=ers::Core::XML_ISSUE_TAG) {
    	throw ERS_PARSE_ERROR("Invalid root tag : %s (should be %s)",
	issue_name,ers::Core::XML_ISSUE_TAG); 
    } // not issue 
    return receive(issue_element);
} // receive

/** Reads an Issue in the Stream
  * \return an issue if it could be read, or a null pointer if there was no issue in the stream 
  */

ers::Issue *ers::XercesStream::receive() {
    DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(gLS);
    ERS_CHECK_PTR(impl);
    DOMBuilder        *parser = ((DOMImplementationLS*)impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS, 0);
    ERS_CHECK_PTR(parser);
    parser->setFeature(XMLUni::fgDOMNamespaces, false);
    parser->setFeature(XMLUni::fgXercesSchema, false);
    parser->setFeature(XMLUni::fgDOMValidation, false);
    parser->setErrorHandler((DOMErrorHandler *) this) ; 
    parser->setFeature(XMLUni::fgDOMDatatypeNormalization, true);
    parser->resetDocumentPool();
    DOMDocument *document = parser->parseURI(m_file_path.c_str()); 
    if (document) {
	return receive(document); 
    } else {
	return 0 ;
    } // else 
} // receive

/** Sends an issue into DOM Document 
  * \param document_ptr pointer to the target DOM document 
  * \param issue_ptr the issue to send 
  */

void ers::XercesStream::send(DOMDocument *document_ptr, const Issue *issue_ptr) {
    ERS_PRE_CHECK_PTR(document_ptr);
    ERS_PRE_CHECK_PTR(issue_ptr);
    DOMElement *root_element = document_ptr->getDocumentElement();
    const string_map_type *table = issue_ptr->get_value_table(); 
    for(string_map_type::const_iterator pos = table->begin();pos!=table->end();++pos) {
	DOMElement *key_element = document_ptr->createElement(to_unicode(ers::Core::XML_KEY_TAG)) ; 
	DOMText *key_text = document_ptr->createTextNode(to_unicode(pos->first.c_str())) ; 
	key_element->appendChild((DOMNode *)key_text);
	root_element->appendChild(key_element);
	DOMElement *value_element = document_ptr->createElement(to_unicode(ers::Core::XML_STRING_VALUE_TAG)) ; 
	DOMText *value_text = document_ptr->createTextNode(to_unicode(pos->second.c_str())) ;
	value_element->appendChild((DOMNode *) value_text);
	root_element->appendChild(value_element);
    } // for 
} // send

/** Sends an issue into the stream 
  * This method sets up the document and streams the Issue into it. 
  * \param issue_ptr pointer to the issue to send
  */

void ers::XercesStream::send(const Issue *issue_ptr) {
    ERS_PRE_CHECK_PTR(issue_ptr);
    DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(gLS);
    ERS_CHECK_PTR(impl);
    DOMDocument* document_ptr = impl->createDocument(0,to_unicode(ers::Core::XML_ISSUE_TAG),0);
    ERS_CHECK_PTR(document_ptr);
    document_ptr->setStandalone(true); 
    send(document_ptr,issue_ptr);  
    XMLFormatTarget *form_target = new LocalFileFormatTarget(m_file_path.c_str());
    ERS_CHECK_PTR(form_target);
    DOMWriter *writer_ptr = impl->createDOMWriter();
    ERS_CHECK_PTR(writer_ptr);
    writer_ptr->setFeature(to_unicode("format-pretty-print"),true) ; 
    writer_ptr->writeNode(form_target,*document_ptr);
    writer_ptr->release(); 
    document_ptr->release();
    delete form_target ; 
}// send


