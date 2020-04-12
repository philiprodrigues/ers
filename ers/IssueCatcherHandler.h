#ifndef ERS_ISSUE_CATCHER_HANDLER_H
#define ERS_ISSUE_CATCHER_HANDLER_H

/*
 *  IssueCatcherHandler.h
 *  ers
 *
 *  Created by Serguei Kolos on 21.11.05.
 *  Copyright 2005 CERN. All rights reserved.
 *
 */

/** \file IssueCatcherHandler.h This file declares utility class which is used to uninstall IssueCatcher.
  * \author Serguei Kolos
  * \brief ers header and documentation file
  */
namespace ers
{
    class LocalStream;
    
    class IssueCatcherHandler
    {
	friend class LocalStream;
        
      public:	
	~IssueCatcherHandler();
        
      private:
	IssueCatcherHandler() = default;

	IssueCatcherHandler (const IssueCatcherHandler &) = delete;
	IssueCatcherHandler & operator = (const IssueCatcherHandler &) = delete;
    };
}

#endif
