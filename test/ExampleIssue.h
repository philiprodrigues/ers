/*
 *  ExampleIssue.h
 *  Test
 *
 *  Created by Matthias Wiesmann on 24.01.05.
 *  Copyright 2005 CERN. All rights reserved.
 *
 */

#include "ers/Issue.h"


/** This class is an example of an user issue. 
  * It contains a integer member field
  * \brief Example issue
  * \author Matthias Wiesmann
  * \version 1.0 
  */

class ExampleIssue : public ers::Issue {
    
protected:
    ExampleIssue(const ers::Context& context, ers::severity_t severity) ;
public:
    static const char* const CLASS_NAME ; 
    static const char* const PROCRASTINATION_LEVEL_KEY ; 
    ExampleIssue();
    ExampleIssue(const ers::Context& context, ers::severity_t severity, int procrastination_level) ;
    virtual const char *get_class_name() const throw() ;
    int procrastination_level() const ; 
} ; // ExempleIssue

#define EXAMPLE_ERROR(level) ExampleIssue(ERS_HERE,ers_error,level)


