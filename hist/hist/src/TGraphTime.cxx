// @(#)root/hist:$Id$
// Author: Rene Brun   14/07/2009

/*************************************************************************
 * Copyright (C) 1995-2009, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TGraphTime.h"
#include "TROOT.h"
#include "TVirtualPad.h"
#include "TH1.h"
//#include "TText.h"
#include "TObjArray.h"
#include "TSystem.h"

ClassImp(TGraphTime)


//______________________________________________________________________________
//
// TGraphTime is used to draw a set of objects evolving with nsteps in time between tmin and tmax.
// each time step has a new list of objects. This list can be identical to
// the list of objects in the previous steps, but with different attributes.
//   see example of use in $ROOTSYS/tutorials/graphs/gtime.C

//______________________________________________________________________________
TGraphTime::TGraphTime(): TNamed()
{
   // Marker default constructor.

   fSleepTime = 0;
   fNsteps    = 0;
   fXmin      = 0;
   fXmax      = 1;
   fYmin      = 0;
   fYmax      = 1;
   fSteps     = 0;
   fFrame     = 0;
}


//______________________________________________________________________________
TGraphTime::TGraphTime(Int_t nsteps, Double_t xmin, Double_t ymin, Double_t xmax, Double_t ymax)
      :TNamed()
{
   // Create a TGraphTime with nsteps in range [xmin,xmax][ymin,ymax]

   if (nsteps <= 0) {
      Warning("TGraphTime", "Number of steps %d changed to 100",nsteps);
      nsteps = 100;
   }
   fSleepTime = 0;
   fNsteps    = nsteps;
   fXmin      = xmin;
   fXmax      = xmax;
   fYmin      = ymin;
   fYmax      = ymax;
   fSteps     = new TObjArray(nsteps+1);
   fFrame     = new TH1D("frame","",100,fXmin,fXmax);
   fFrame->SetMinimum(ymin);
   fFrame->SetMaximum(ymax);
   fFrame->SetStats(0);
}


//______________________________________________________________________________
TGraphTime::~TGraphTime()
{
   // GraphTime default destructor.
   
   if (!fSteps) return;
   fSteps->Delete();
   delete fSteps; fSteps=0;
}


//______________________________________________________________________________
TGraphTime::TGraphTime(const TGraphTime &gtime) : TNamed(gtime)
{
   // copy constructor.

   fSleepTime = gtime.fSleepTime;
   fNsteps    = gtime.fNsteps;
   fXmin      = gtime.fXmin;
   fXmax      = gtime.fXmax;
   fYmin      = gtime.fYmin;
   fYmax      = gtime.fYmax;
   fSteps     = new TObjArray(fNsteps+1);
   fFrame     = new TH1D("frame","",100,fXmin,fXmax);
   fFrame->SetMinimum(fYmin);
   fFrame->SetMaximum(fYmax);
   fFrame->SetStats(0);
}

//______________________________________________________________________________
Int_t TGraphTime::Add(const TObject *obj, Int_t slot)
{
   // Add one object to a time slot. 
   // TGraphTime becomes the owner of this object.
   
   if (!fSteps) {
      fNsteps = 100;
      fSteps = new TObjArray(fNsteps+1);
   }
   if (slot < 0 || slot >= fNsteps) return -1;
   TList *list = (TList*)fSteps->UncheckedAt(slot);
   if (!list) {
      list = new TList();
      fSteps->AddAt(list,slot);
   }
   list->Add((TObject*)obj);
   return slot;
}


//______________________________________________________________________________
void TGraphTime::Draw(Option_t *option)
{
   // Draw this TGraphTime with its current attributes.
   // if option "time" is specified the current time value is drawn at 
   //  the top right corner

   if (!gPad) {
      gROOT->MakeDefCanvas();
      gPad->SetFillColor(41);
      gPad->SetFrameFillColor(19);
      gPad->SetGrid();
   }
   if (fFrame) fFrame->Draw();
   Paint(option);

}

//______________________________________________________________________________
void TGraphTime::Paint(Option_t *option)
{
   // Paint this marker with its current attributes.
   
   TString opt = option;
   opt.ToLower();
   TObject *frame = gPad->GetPrimitive("frame");
   TList *list = 0;
   //TText *text = 0;
   //if (opt.Contains("time")) {
   //   text = new TText();
   //   text->SetNDC();
   //   text->SetTextSize(0.05);
   //   text->SetTextAlign(23);
   //}

   for (Int_t s=0;s<fNsteps;s++) {
      list = (TList*)fSteps->UncheckedAt(s);
      if (list) {
         gPad->GetListOfPrimitives()->Remove(frame);
         gPad->GetListOfPrimitives()->Clear();
         if (frame) gPad->GetListOfPrimitives()->Add(frame);
         list->Draw("");
         //if (text) text->DrawText(.94,.98,Form("%d",s));
         gPad->Update();
         if (fSleepTime > 0) gSystem->Sleep(fSleepTime);
      }
   }
   //delete text;
}
