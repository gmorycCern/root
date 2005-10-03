// @(#)root/gl:$Name:  $:$Id: TGLLogicalShape.h,v 1.4 2005/05/28 12:21:00 rdm Exp $
// Author:  Richard Maunder  25/05/2005

/*************************************************************************
 * Copyright (C) 1995-2004, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TGLLogicalShape
#define ROOT_TGLLogicalShape

#ifndef ROOT_TGLDrawable
#include "TGLDrawable.h"
#endif

class TContextMenu;

/*************************************************************************
 * TGLLogicalShape - TODO
 *
 *
 *
 *************************************************************************/
class TGLLogicalShape : public TGLDrawable { // Rename TGLLogicalObject?
private:
   // Fields
   mutable UInt_t fRef;       //! physical instance ref counting

protected:
   mutable Bool_t fRefStrong; //! Strong ref (delete on 0 ref)

   // TODO: Common UInt_t flags section (in TGLDrawable?) to avoid multiple bools

public:
   TGLLogicalShape(ULong_t ID);
   virtual ~TGLLogicalShape();

   virtual void Purge();

   virtual void InvokeContextMenu(TContextMenu & menu, UInt_t x, UInt_t y) const = 0;

   // Physical shape ref counting
   void   AddRef() const                 { ++fRef; }
   Bool_t SubRef() const;
   UInt_t Ref()    const                 { return fRef; }
   void   StrongRef(Bool_t strong) const { fRefStrong = strong; }

   ClassDef(TGLLogicalShape,0) // a logical (non-placed, local frame) drawable object
};

//______________________________________________________________________________
inline Bool_t TGLLogicalShape::SubRef() const
{ 
   if (--fRef == 0) {
      if (fRefStrong) {
         delete this;
      }
      return kTRUE;
   }
   return kFALSE;
}

#endif // ROOT_TGLLogicalShape
