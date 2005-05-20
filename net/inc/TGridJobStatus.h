// @(#)root/net:$Name:  $:$Id: TGridJobStatus.h,v 1.1 2005/05/12 13:19:39 rdm Exp $
// Author: Jan Fiete Grosse-Oetringhaus   06/10/2004

/*************************************************************************
 * Copyright (C) 1995-2004, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TGridJobStatus
#define ROOT_TGridJobStatus

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TGridJobStatus                                                       //
//                                                                      //
// Abstract base class containing the status of a Grid job.             //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TObject
#include "TObject.h"
#endif


class TGridJobStatus : public TObject {

public:
   // Subset of Grid job states for common GetStatus function
   enum EGridJobStatus { kUNKNOWN, kWAITING, kRUNNING, kABORTED, kFAIL, kDONE };

  TGridJobStatus() { }
  virtual ~TGridJobStatus() { }

  // These functions reduces the possible job states to the subset given above
  // in EGridJobStatus, for detailed status information query the specific
  // implementation
  virtual EGridJobStatus GetStatus() const = 0;

  ClassDef(TGridJobStatus,1)  // ABC defining status of a Grid job
};

#endif
