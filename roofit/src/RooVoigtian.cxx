/*****************************************************************************
 * Project: BaBar detector at the SLAC PEP-II B-factory
 * Package: RooFitTools
 *    File: $Id: RooVoigtian.cc,v 1.1 2001/08/09 19:15:55 schieti Exp $
 * Authors:
 *   TS, Thomas Schietinger, SLAC, schieti@slac.stanford.edu
 * History:
 *   09-Aug-2001 TS Created initial version from RooGaussian
 *   27-Aug-2001 TS Port to RooFitModels/RooFitCore
 *
 * Copyright (C) 2001 Stanford Linear Accelerator Center
 *****************************************************************************/
#include "BaBar/BaBar.hh"

#include <iostream.h>
#include <math.h>

#include "RooFitModels/RooVoigtian.hh"
#include "RooFitCore/RooAbsReal.hh"
#include "RooFitCore/RooRealVar.hh"
#include "RooFitCore/RooComplex.hh"
#include "RooFitCore/RooMath.hh"

ClassImp(RooVoigtian)

RooVoigtian::RooVoigtian(const char *name, const char *title,
			 RooAbsReal& _x, RooAbsReal& _mean,
			 RooAbsReal& _width, RooAbsReal& _sigma) :
  RooAbsPdf(name,title),
  x("x","Dependent",this,_x),
  mean("mean","Mean",this,_mean),
  width("width","Breit-Wigner Width",this,_width),
  sigma("sigma","Gauss Width",this,_sigma),
  _doFast(kFALSE)
{
  _invRootPi= 1./sqrt(atan2(0.,-1.));
}


RooVoigtian::RooVoigtian(const RooVoigtian& other, const char* name) : 
  RooAbsPdf(other,name), x("x",this,other.x), mean("mean",this,other.mean),
  width("width",this,other.width),sigma("sigma",this,other.sigma),
  _doFast(other._doFast)
{
  _invRootPi= 1./sqrt(atan2(0.,-1.));
}


Double_t RooVoigtian::evaluate() const
{
  Double_t s = (sigma>0) ? sigma : -sigma ;
  Double_t w = (width>0) ? width : -width ;

  Double_t coef= -0.5/(s*s);
  Double_t arg = x - mean;

  // return constant for zero width and sigma
  if (s==0. && w==0.) return 1.;

  // Breit-Wigner for zero sigma
  if (s==0.) return (1./(arg*arg+0.25*w*w));

  // Gauss for zero width
  if (w==0.) return exp(coef*arg*arg);

  // actual Voigtian for non-trivial width and sigma
  Double_t c = 1./(sqrt(2.)*s);
  Double_t a = 0.5*c*w;
  Double_t u = c*arg;
  RooComplex z(u,a) ;
  RooComplex v(0.) ;

  if (_doFast) {
    v = RooMath::FastComplexErrFunc(z);
  } else {
    v = RooMath::ComplexErrFunc(z);
  }
  return c*_invRootPi*v.re();

}
