/*
 *  STLOutStream.cxx
 *  ers
 *
 *  Created by Matthias Wiesmann on 01.12.04.
 *  Copyright 2004 CERN. All rights reserved.
 *
 */

#include "ers/STLOutStream.h"
#include "ers/Precondition.h"

/** Builds a stream that writes into a file 
  * @param filename the name of the file
  */

ers::STLOutStream::STLOutStream(const char* filename) {
    ERS_PRECONDITION(filename!=0,"Null pointer for filename"); 
    _stream = new std::ofstream(filename) ; 
    delete_stream = true ; 
} // Stream_Stream

ers::STLOutStream::STLOutStream(std::ostream *s) {
    _stream = s ; 
     delete_stream = false ;
} // STLOutStream

ers::STLOutStream::STLOutStream() {
    _stream = &std::cout ;
    delete_stream = false ; 
} // Stream_Stream

ers::STLOutStream::~STLOutStream() {
    ERS_PRECONDITION(_stream,"Destructor called with null stream");
    if (delete_stream) {
	delete(_stream); 
    } // if 
    _stream = 0 ; 
} // ~Stream_Stream

void ers::STLOutStream::send(const Issue *i) {
    ERS_PRECONDITION(i!=0,"Null Issue pointer"); 
    const string_map_type *table = i->get_value_table(); 
    serialize_start(i); 
    bool first = true ; 
    for(string_map_type::const_iterator pos = table->begin();pos!=table->end();++pos) {
        if (first) {
            first = false ;
        } else {
            serialize_separator(i);
        } // first or not
        serialize(pos->first,pos->second);
    } // for
    serialize_end(i); 
} // send

ers::Issue *ers::STLOutStream::receive() {
    return 0 ;
} // receive

void ers::STLOutStream::serialize_separator(const ers::Issue *i) {
     ERS_PRECONDITION(i!=0,"Null Issue pointer");
} // serialize_separator

void ers::STLOutStream::serialize_start(const ers::Issue *i) {
     ERS_PRECONDITION(i!=0,"Null Issue pointer");
} // serialize_start

void ers::STLOutStream::serialize_end(const Issue *i) {
    ERS_PRECONDITION(i!=0,"Null Issue pointer");
} // serialize_end


