/*****************************************************************************
 * Project: RooFit                                                           *
 * Package: RooFitCore                                                       *
 * @(#)root/roofitcore:$Id$
 * Authors:                                                                  *
 *   WV, Wouter Verkerke, UC Santa Barbara, verkerke@slac.stanford.edu       *
 *   DK, David Kirkby,    UC Irvine,         dkirkby@uci.edu                 *
 *                                                                           *
 * Copyright (c) 2000-2005, Regents of the University of California          *
 *                          and Stanford University. All rights reserved.    *
 *                                                                           *
 * Redistribution and use in source and binary forms,                        *
 * with or without modification, are permitted according to the terms        *
 * listed in LICENSE (http://roofit.sourceforge.net/license.txt)             *
 *****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
//
// BEGIN_HTML
// RooVectorDataStore is the abstract base class for data collection that
// use a TTree as internal storage mechanism
// END_HTML
//

#include "RooFit.h"
#include "RooMsgService.h"
#include "RooVectorDataStore.h"
#include "RooTreeDataStore.h"

#include "Riostream.h"
#include "TTree.h"
#include "TChain.h"
#include "TDirectory.h"
#include "TROOT.h"
#include "RooFormulaVar.h"
#include "RooRealVar.h"
#include "RooCategory.h"
#include "RooHistError.h"

#include <iomanip>
using namespace std ;

ClassImp(RooVectorDataStore)
;





//_____________________________________________________________________________
RooVectorDataStore::RooVectorDataStore() :
  _wgtVar(0),
  _nReal(0),
  _nRealF(0),
  _nCat(0),
  _nEntries(0),	 
  _firstReal(0),
  _firstRealF(0),
  _firstCat(0),
  _sumWeight(0),
  _extWgtArray(0),
  _extWgtErrLoArray(0),
  _extWgtErrHiArray(0),
  _extSumW2Array(0),
  _curWgt(1),
  _curWgtErrLo(0),
  _curWgtErrHi(0),
  _curWgtErr(0),
  _cache(0)
{
}



//_____________________________________________________________________________
RooVectorDataStore::RooVectorDataStore(const char* name, const char* title, const RooArgSet& vars, const char* wgtVarName) :
  RooAbsDataStore(name,title,varsNoWeight(vars,wgtVarName)),
  _varsww(vars),
  _wgtVar(weightVar(vars,wgtVarName)),
  _nReal(0),
  _nRealF(0),
  _nCat(0),
  _nEntries(0),	   
  _firstReal(0),
  _firstRealF(0),
  _firstCat(0),
  _sumWeight(0),
  _extWgtArray(0),
  _extWgtErrLoArray(0),
  _extWgtErrHiArray(0),
  _extSumW2Array(0),
  _curWgt(1),
  _curWgtErrLo(0),
  _curWgtErrHi(0),
  _curWgtErr(0),
  _cache(0)
{
  TIterator* iter = _varsww.createIterator() ;
  RooAbsArg* arg ;
  while((arg=(RooAbsArg*)iter->Next())) {
    arg->attachToVStore(*this) ;
  }
  delete iter ;
}





//_____________________________________________________________________________
RooArgSet RooVectorDataStore::varsNoWeight(const RooArgSet& allVars, const char* wgtName) 
{
  // Utility function for constructors
  // Return RooArgSet that is copy of allVars minus variable matching wgtName if specified

  RooArgSet ret(allVars) ;
  if(wgtName) {
    RooAbsArg* wgt = allVars.find(wgtName) ;
    if (wgt) {
      ret.remove(*wgt,kTRUE,kTRUE) ;
    }
  }
  return ret ;
}



//_____________________________________________________________________________
RooRealVar* RooVectorDataStore::weightVar(const RooArgSet& allVars, const char* wgtName) 
{
  // Utility function for constructors
  // Return pointer to weight variable if it is defined

  if(wgtName) {
    RooRealVar* wgt = dynamic_cast<RooRealVar*>(allVars.find(wgtName)) ;
    return wgt ;
  } 
  return 0 ;
}




//_____________________________________________________________________________
RooVectorDataStore::RooVectorDataStore(const RooVectorDataStore& other, const char* newname) :
  RooAbsDataStore(other,newname), 
  _varsww(other._varsww),
  _wgtVar(other._wgtVar),
  _nReal(0),
  _nRealF(0),
  _nCat(0),
  _nEntries(other._nEntries),	 
  _sumWeight(other._sumWeight),
  _extWgtArray(other._extWgtArray),
  _extWgtErrLoArray(other._extWgtErrLoArray),
  _extWgtErrHiArray(other._extWgtErrHiArray),
  _extSumW2Array(other._extSumW2Array),
  _curWgt(other._curWgt),
  _curWgtErrLo(other._curWgtErrLo),
  _curWgtErrHi(other._curWgtErrHi),
  _curWgtErr(other._curWgtErr),
  _cache(0)
{
  // Regular copy ctor

  vector<RealVector*>::const_iterator oiter = other._realStoreList.begin() ;
  for (; oiter!=other._realStoreList.end() ; ++oiter) {
    _realStoreList.push_back(new RealVector(**oiter,(RooAbsReal*)_varsww.find((*oiter)->_real->GetName()))) ;
    _nReal++ ;
  }

  vector<RealFullVector*>::const_iterator fiter = other._realfStoreList.begin() ;
  for (; fiter!=other._realfStoreList.end() ; ++fiter) {
    _realfStoreList.push_back(new RealFullVector(**fiter,(RooAbsReal*)_varsww.find((*fiter)->_real->GetName()))) ;
    _nRealF++ ;
  }

  vector<CatVector*>::const_iterator citer = other._catStoreList.begin() ;
  for (; citer!=other._catStoreList.end() ; ++citer) {
    _catStoreList.push_back(new CatVector(**citer,(RooAbsCategory*)_varsww.find((*citer)->_cat->GetName()))) ;
    _nCat++ ;
 }

  _firstReal = _realStoreList.size()>0 ? &_realStoreList.front() : 0 ;
  _firstRealF = _realfStoreList.size()>0 ? &_realfStoreList.front() : 0 ;
  _firstCat = _catStoreList.size()>0 ? &_catStoreList.front() : 0 ;
}


//_____________________________________________________________________________
RooVectorDataStore::RooVectorDataStore(const RooTreeDataStore& other, const RooArgSet& vars, const char* newname) :
  RooAbsDataStore(other,varsNoWeight(vars,other._wgtVar?other._wgtVar->GetName():0),newname),
  _varsww(vars),
  _wgtVar(weightVar(vars,other._wgtVar?other._wgtVar->GetName():0)),
  _nReal(0),
  _nRealF(0),
  _nCat(0),
  _nEntries(0),	   
  _firstReal(0),
  _firstRealF(0),
  _firstCat(0),
  _sumWeight(0),
  _extWgtArray(0),
  _extWgtErrLoArray(0),
  _extWgtErrHiArray(0),
  _extSumW2Array(0),
  _curWgt(1),
  _curWgtErrLo(0),
  _curWgtErrHi(0),
  _curWgtErr(0),
  _cache(0)
{
  TIterator* iter = _varsww.createIterator() ;
  RooAbsArg* arg ;
  while((arg=(RooAbsArg*)iter->Next())) {
    arg->attachToVStore(*this) ;
  }
  delete iter ;

  // now copy contents of tree storage here
  for (Int_t i=0 ; i<other.numEntries() ; i++) {
    other.get(i) ;
    _varsww = other._varsww ;
    fill() ;
  }
  
}


//_____________________________________________________________________________
RooVectorDataStore::RooVectorDataStore(const RooVectorDataStore& other, const RooArgSet& vars, const char* newname) :
  RooAbsDataStore(other,varsNoWeight(vars,other._wgtVar?other._wgtVar->GetName():0),newname),
  _varsww(vars),
  _wgtVar(other._wgtVar?weightVar(vars,other._wgtVar->GetName()):0),
  _nReal(0),	 
  _nRealF(0),
  _nCat(0),
  _nEntries(other._nEntries),	 
  _sumWeight(other._sumWeight),
  _extWgtArray(other._extWgtArray),
  _extWgtErrLoArray(other._extWgtErrLoArray),
  _extWgtErrHiArray(other._extWgtErrHiArray),
  _extSumW2Array(other._extSumW2Array),
  _curWgt(other._curWgt),
  _curWgtErrLo(other._curWgtErrLo),
  _curWgtErrHi(other._curWgtErrHi),
  _curWgtErr(other._curWgtErr),
  _cache(0)
{
  // Clone ctor, must connect internal storage to given new external set of vars
  vector<RealVector*>::const_iterator oiter = other._realStoreList.begin() ;
  for (; oiter!=other._realStoreList.end() ; ++oiter) {
    RooAbsReal* real = (RooAbsReal*) vars.find((*oiter)->bufArg()->GetName()) ;
    if (real) {
      // Clone vector
      _realStoreList.push_back(new RealVector(**oiter,real)) ;
      // Adjust buffer pointer
      real->attachToVStore(*this) ;
      _nReal++ ;
    }
  }
  
  vector<RealFullVector*>::const_iterator fiter = other._realfStoreList.begin() ;
  for (; fiter!=other._realfStoreList.end() ; ++fiter) {
    RooAbsReal* real = (RooAbsReal*) vars.find((*fiter)->bufArg()->GetName()) ;
    if (real) {
      // Clone vector
      _realfStoreList.push_back(new RealFullVector(**fiter,real)) ;
      // Adjust buffer pointer
      real->attachToVStore(*this) ;
      _nRealF++ ;
    }
  }

  vector<CatVector*>::const_iterator citer = other._catStoreList.begin() ;
  for (; citer!=other._catStoreList.end() ; ++citer) {
    RooAbsCategory* cat = (RooAbsCategory*) vars.find((*citer)->bufArg()->GetName()) ;
    if (cat) {
      // Clone vector
      _catStoreList.push_back(new CatVector(**citer,cat)) ;
      // Adjust buffer pointer
      cat->attachToVStore(*this) ;
      _nCat++ ;
    }
  }

  _firstReal = _realStoreList.size()>0 ? &_realStoreList.front() : 0 ;
  _firstRealF = _realfStoreList.size()>0 ? &_realfStoreList.front() : 0 ;
  _firstCat = _catStoreList.size()>0 ? &_catStoreList.front() : 0 ;

}





//_____________________________________________________________________________
RooVectorDataStore::RooVectorDataStore(const char *name, const char *title, RooAbsDataStore& tds, 
			 const RooArgSet& vars, const RooFormulaVar* cutVar, const char* cutRange,
			 Int_t nStart, Int_t nStop, Bool_t /*copyCache*/, const char* wgtVarName) :

  RooAbsDataStore(name,title,varsNoWeight(vars,wgtVarName)),
  _varsww(vars),
  _wgtVar(weightVar(vars,wgtVarName)),
  _nReal(0),
  _nRealF(0),
  _nCat(0),
  _nEntries(0),	   
  _firstReal(0),
  _firstRealF(0),
  _firstCat(0),
  _sumWeight(0),
  _extWgtArray(0),
  _extWgtErrLoArray(0),
  _extWgtErrHiArray(0),
  _extSumW2Array(0),
  _curWgt(1),
  _curWgtErrLo(0),
  _curWgtErrHi(0),
  _curWgtErr(0),
  _cache(0)
{
  TIterator* iter = _varsww.createIterator() ;
  RooAbsArg* arg ;
  while((arg=(RooAbsArg*)iter->Next())) {
    arg->attachToVStore(*this) ;
  }
  delete iter ;

  // Deep clone cutVar and attach clone to this dataset
  RooFormulaVar* cloneVar = 0;
  if (cutVar) {    
    cloneVar = (RooFormulaVar*) cutVar->cloneTree() ;
    cloneVar->attachDataStore(tds) ;
  }

  RooVectorDataStore* vds = dynamic_cast<RooVectorDataStore*>(&tds) ;
  if (vds && vds->_cache) {    
    _cache = new RooVectorDataStore(*vds->_cache) ;
  }
  
  loadValues(&tds,cloneVar,cutRange,nStart,nStop);

  if (cloneVar) delete cloneVar ;
}






//_____________________________________________________________________________
RooVectorDataStore::~RooVectorDataStore()
{
  // Destructor
  vector<RealVector*>::const_iterator iter = _realStoreList.begin(), iend = _realStoreList.end() ;
  for ( ; iter!=iend ; ++iter) {
    delete *iter ;
  }

  vector<RealFullVector*>::const_iterator iter3 = _realfStoreList.begin(), iend3 = _realfStoreList.end() ;
  for ( ; iter3!=iend3 ; ++iter3) {
    delete *iter3 ;
  }

  vector<CatVector*>::const_iterator iter2 = _catStoreList.begin(), iend2 = _catStoreList.end() ;
  for ( ; iter2!=iend2 ; ++iter2) {
    delete *iter2 ;
  }

  if (_cache) delete _cache ;
}




//_____________________________________________________________________________
Bool_t RooVectorDataStore::valid() const 
{
  // Return true if currently loaded coordinate is considered valid within
  // the current range definitions of all observables
  return kTRUE ;
}




//_____________________________________________________________________________
Int_t RooVectorDataStore::fill()
{
  // Interface function to TTree::Fill
  vector<RealVector*>::iterator iter = _realStoreList.begin() ;
  for ( ; iter!=_realStoreList.end() ; ++iter) {
    (*iter)->fill() ;
  }
  vector<RealFullVector*>::iterator iter3 = _realfStoreList.begin() ;
  for ( ; iter3!=_realfStoreList.end() ; ++iter3) {
    (*iter3)->fill() ;
  }
  vector<CatVector*>::iterator iter2 = _catStoreList.begin() ;
  for ( ; iter2!=_catStoreList.end() ; ++iter2) {
    (*iter2)->fill() ;
  }
  _sumWeight += _wgtVar ? _wgtVar->getVal() : 1. ;
  _nEntries++ ;  

  return 0 ;
}
 


//_____________________________________________________________________________
const RooArgSet* RooVectorDataStore::get(Int_t index) const 
{
  // Load the n-th data point (n='index') in memory
  // and return a pointer to the internal RooArgSet
  // holding its coordinates.

  if (index>=_nEntries) return 0 ;
    
  for (Int_t i=0 ; i<_nReal ; i++) {
    (*(_firstReal+i))->get(index) ;
  }

  if (_nRealF>0) {
    for (Int_t i=0 ; i<_nRealF ; i++) {
      (*(_firstRealF+i))->get(index) ;
    }
  }

  if (_nCat>0) {
    for (Int_t i=0 ; i<_nCat ; i++) {
      (*(_firstCat+i))->get(index) ;
    }
  }

  if (_doDirtyProp) {
    // Raise all dirty flags 
    _iterator->Reset() ;
    RooAbsArg* var = 0;
    while ((var=(RooAbsArg*)_iterator->Next())) {
      var->setValueDirty() ; // This triggers recalculation of all clients
    }     
  }
  
  // Update current weight cache
  if (_extWgtArray) {

    // If external array is specified use that  
    _curWgt = _extWgtArray[index] ;
    _curWgtErrLo = _extWgtErrLoArray[index] ;
    _curWgtErrHi = _extWgtErrHiArray[index] ;
    _curWgtErr   = sqrt(_extSumW2Array[index]) ;

  } else if (_wgtVar) {

    // Otherwise look for weight variable
    _curWgt = _wgtVar->getVal() ;
    _curWgtErrLo = _wgtVar->getAsymErrorLo() ;
    _curWgtErrHi = _wgtVar->getAsymErrorHi() ;
    _curWgtErr   = _wgtVar->hasAsymError() ? ((_wgtVar->getAsymErrorHi() - _wgtVar->getAsymErrorLo())/2)  : _wgtVar->getError() ;

  } else {

    // Otherwise return 1 
    _curWgt=1.0 ;
    _curWgtErrLo = 0 ;
    _curWgtErrHi = 0 ;
    _curWgtErr = 0 ;
    
  }

  if (_cache) {
    _cache->get(index) ;
  }

  return &_vars;
}



//_____________________________________________________________________________
Double_t RooVectorDataStore::weight(Int_t index) const 
{
  // Return the weight of the n-th data point (n='index') in memory
  get(index) ;
  return weight() ;
}



//_____________________________________________________________________________
Double_t RooVectorDataStore::weight() const 
{
  // Return the weight of the n-th data point (n='index') in memory
  return _curWgt ;
}


//_____________________________________________________________________________
Double_t RooVectorDataStore::weightError(RooAbsData::ErrorType etype) const 
{
  if (_extWgtArray) {

    // We have a weight array, use that info

    // Return symmetric error on current bin calculated either from Poisson statistics or from SumOfWeights
    Double_t lo,hi ;
    weightError(lo,hi,etype) ;
    return (lo+hi)/2 ;

   } else if (_wgtVar) {

    // We have a a weight variable, use that info
    if (_wgtVar->hasAsymError()) {
      return ( _wgtVar->getAsymErrorHi() - _wgtVar->getAsymErrorLo() ) / 2 ;
    } else {
      return _wgtVar->getError() ;    
    }

  } else {

    // We have no weights
    return 0 ;

  }
}



//_____________________________________________________________________________
void RooVectorDataStore::weightError(Double_t& lo, Double_t& hi, RooAbsData::ErrorType etype) const
{
  if (_extWgtArray) {
    
    // We have a weight array, use that info
    switch (etype) {
      
    case RooAbsData::Auto:
      throw string(Form("RooDataHist::weightError(%s) weight type Auto not allowed here",GetName())) ;
      break ;
      
    case RooAbsData::Poisson:
      // Weight may be preset or precalculated    
      if (_curWgtErrLo>=0) {
	lo = _curWgtErrLo ;
	hi = _curWgtErrHi ;
	return ;
      }
      
      // Otherwise Calculate poisson errors
      Double_t ym,yp ;  
      RooHistError::instance().getPoissonInterval(Int_t(weight()+0.5),ym,yp,1) ;
      lo = weight()-ym ;
      hi = yp-weight() ;
      return ;
      
    case RooAbsData::SumW2:
      lo = _curWgtErr ;
      hi = _curWgtErr ;
      return ;
      
    case RooAbsData::None:
      lo = 0 ;
      hi = 0 ;
      return ;
    }    
    
  } else if (_wgtVar) {

    // We have a a weight variable, use that info
    if (_wgtVar->hasAsymError()) {
      hi = _wgtVar->getAsymErrorHi() ;
      lo = _wgtVar->getAsymErrorLo() ;
    } else {
      hi = _wgtVar->getError() ;
      lo = _wgtVar->getError() ;
    }  

  } else {

    // We are unweighted
    lo=0 ;
    hi=0 ;

  }
}



//_____________________________________________________________________________
void RooVectorDataStore::loadValues(const RooAbsDataStore *ads, const RooFormulaVar* select, const char* rangeName, Int_t nStart, Int_t nStop) 
{
  //   throw(std::string("RooVectorDataSore::loadValues() NOT IMPLEMENTED")) ;
  
  // Load values from dataset 't' into this data collection, optionally
  // selecting events using 'select' RooFormulaVar
  //

  // Redirect formula servers to source data row
  RooFormulaVar* selectClone(0) ;
  if (select) {
    selectClone = (RooFormulaVar*) select->cloneTree() ;
    selectClone->recursiveRedirectServers(*ads->get()) ;
    selectClone->setOperMode(RooAbsArg::ADirty,kTRUE) ;
  }

  // Force DS internal initialization
  ads->get(0) ;

  // Loop over events in source tree   
  RooAbsArg* arg = 0;
  TIterator* destIter = _varsww.createIterator() ;
  Int_t nevent = nStop < ads->numEntries() ? nStop : ads->numEntries() ;
  Bool_t allValid ;

  Bool_t isTDS = dynamic_cast<const RooTreeDataStore*>(ads) ;
  Bool_t isVDS = dynamic_cast<const RooVectorDataStore*>(ads) ;
  for(Int_t i=nStart; i < nevent ; ++i) {
    ads->get(i) ;
    
    // Does this event pass the cuts?
    if (selectClone && selectClone->getVal()==0) {
      continue ; 
    }


    if (isTDS) {
      _varsww.assignValueOnly(((RooTreeDataStore*)ads)->_varsww) ;
    } else if (isVDS) {
      _varsww.assignValueOnly(((RooVectorDataStore*)ads)->_varsww) ;
    } else {
      _varsww.assignValueOnly(*ads->get()) ;
    }

    destIter->Reset() ;
    // Check that all copied values are valid
    allValid=kTRUE ;
    while((arg=(RooAbsArg*)destIter->Next())) {
      if (!arg->isValid() || (rangeName && !arg->inRange(rangeName))) {
	allValid=kFALSE ;
	break ;
      }
    }
    if (!allValid) {
      continue ;
    }
    
    //_cachedVars = ((RooTreeDataStore*)ads)->_cachedVars ;
    fill() ;
   }

  delete destIter ;  
  delete selectClone ;
  
  SetTitle(ads->GetTitle());
}





//_____________________________________________________________________________
Bool_t RooVectorDataStore::changeObservableName(const char* /*from*/, const char* /*to*/) 
{
  return kFALSE ;
}

  

//_____________________________________________________________________________
RooAbsArg* RooVectorDataStore::addColumn(RooAbsArg& newVar, Bool_t /*adjustRange*/)
{
  // Add a new column to the data set which holds the pre-calculated values
  // of 'newVar'. This operation is only meaningful if 'newVar' is a derived
  // value.
  //
  // The return value points to the added element holding 'newVar's value
  // in the data collection. The element is always the corresponding fundamental
  // type of 'newVar' (e.g. a RooRealVar if 'newVar' is a RooFormulaVar)
  //
  // Note: This function is explicitly NOT intended as a speed optimization
  //       opportunity for the user. Components of complex PDFs that can be
  //       precalculated with the dataset are automatically identified as such
  //       and will be precalculated when fitting to a dataset
  // 
  //       By forcibly precalculating functions with non-trivial Jacobians,
  //       or functions of multiple variables occurring in the data set,
  //       using addColumn(), you may alter the outcome of the fit. 
  //
  //       Only in cases where such a modification of fit behaviour is intentional, 
  //       this function should be used. 

  // Create a fundamental object of the right type to hold newVar values
  RooAbsArg* valHolder= newVar.createFundamental();
  // Sanity check that the holder really is fundamental
  if(!valHolder->isFundamental()) {
    coutE(InputArguments) << GetName() << "::addColumn: holder argument is not fundamental: \""
	 << valHolder->GetName() << "\"" << endl;
    return 0;
  }

  // Clone variable and attach to cloned tree 
  RooAbsArg* newVarClone = newVar.cloneTree() ;
  newVarClone->recursiveRedirectServers(_vars,kFALSE) ;

  // Attach value place holder to this tree
  valHolder->attachToVStore(*this) ;
  _vars.add(*valHolder) ;
  _varsww.add(*valHolder) ;

  // Fill values of of placeholder
  RealVector* rv(0) ;
  CatVector* cv(0) ;
  if (dynamic_cast<RooAbsReal*>(valHolder)) {
    rv = addReal((RooAbsReal*)valHolder); 
    rv->resize(numEntries()) ;
  } else if (dynamic_cast<RooAbsCategory*>((RooAbsCategory*)valHolder)) {
    cv = addCategory((RooAbsCategory*)valHolder) ;
    cv->resize(numEntries()) ;
  } 

  for (int i=0 ; i<numEntries() ; i++) {
    get(i) ;

    newVarClone->syncCache(&_vars) ;
    valHolder->copyCache(newVarClone) ;
    if (rv) rv->write(i) ;
    if (cv) cv->write(i) ;
  }

  delete newVarClone ;  
  return valHolder ;

}



//_____________________________________________________________________________
RooArgSet* RooVectorDataStore::addColumns(const RooArgList& varList)
{
  // Utility function to add multiple columns in one call
  // See addColumn() for details

  TIterator* vIter = varList.createIterator() ;
  RooAbsArg* var ;

  checkInit() ;

  TList cloneSetList ;
  RooArgSet cloneSet ;
  RooArgSet* holderSet = new RooArgSet ;

  while((var=(RooAbsArg*)vIter->Next())) {
    // Create a fundamental object of the right type to hold newVar values
    RooAbsArg* valHolder= var->createFundamental();
    holderSet->add(*valHolder) ;

    // Sanity check that the holder really is fundamental
    if(!valHolder->isFundamental()) {
      coutE(InputArguments) << GetName() << "::addColumn: holder argument is not fundamental: \""
	   << valHolder->GetName() << "\"" << endl;
      return 0;
    }
    
    // Clone variable and attach to cloned tree 
    RooArgSet* newVarCloneList = (RooArgSet*) RooArgSet(*var).snapshot() ;  
    if (!newVarCloneList) {
      coutE(InputArguments) << "RooTreeDataStore::RooTreeData(" << GetName() 
			    << ") Couldn't deep-clone variable " << var->GetName() << ", abort." << endl ;
      return 0 ;
    }
    RooAbsArg* newVarClone = newVarCloneList->find(var->GetName()) ;   
    newVarClone->recursiveRedirectServers(_vars,kFALSE) ;
    newVarClone->recursiveRedirectServers(*holderSet,kFALSE) ;

    cloneSetList.Add(newVarCloneList) ;
    cloneSet.add(*newVarClone) ;

    // Attach value place holder to this tree
    valHolder->attachToVStore(*this) ;
    _vars.add(*valHolder) ;
  }
  delete vIter ;


  TIterator* cIter = cloneSet.createIterator() ;
  TIterator* hIter = holderSet->createIterator() ;
  RooAbsArg *cloneArg, *holder ;

  // Dimension storage area for new vectors
  while((holder = (RooAbsArg*)hIter->Next())) {
      if (dynamic_cast<RooAbsReal*>(holder)) {
	addReal((RooAbsReal*)holder)->resize(numEntries()) ;
      } else {
	addCategory((RooAbsCategory*)holder)->resize(numEntries()) ;
      }
    }

  // Fill values of of placeholder
  for (int i=0 ; i<numEntries() ; i++) {
    get(i) ;

    cIter->Reset() ;
    hIter->Reset() ;
    while((cloneArg=(RooAbsArg*)cIter->Next())) {
      holder = (RooAbsArg*)hIter->Next() ;

      cloneArg->syncCache(&_vars) ;
      holder->copyCache(cloneArg) ;

      if (dynamic_cast<RooAbsReal*>(holder)) {
	addReal((RooAbsReal*)holder)->write(i) ;
      } else {
	addCategory((RooAbsCategory*)holder)->write(i) ;
      }
    }
  }
  
  delete cIter ;
  delete hIter ;

  cloneSetList.Delete() ;
  return holderSet ;
}




//_____________________________________________________________________________
RooAbsDataStore* RooVectorDataStore::merge(const RooArgSet& allVars, list<RooAbsDataStore*> dstoreList)
{
  // Merge columns of supplied data set(s) with this data set.  All
  // data sets must have equal number of entries.  In case of
  // duplicate columns the column of the last dataset in the list
  // prevails
    
  RooVectorDataStore* mergedStore = new RooVectorDataStore("merged","merged",allVars) ;

  Int_t nevt = dstoreList.front()->numEntries() ;
  for (int i=0 ; i<nevt ; i++) {

    // Copy data from self
    mergedStore->_vars = *get(i) ;
      
    // Copy variables from merge sets
    for (list<RooAbsDataStore*>::iterator iter = dstoreList.begin() ; iter!=dstoreList.end() ; iter++) {
      const RooArgSet* partSet = (*iter)->get(i) ;
      mergedStore->_vars = *partSet ;
    }

    mergedStore->fill() ;
  }
  return mergedStore ;
}





//_____________________________________________________________________________
void RooVectorDataStore::append(RooAbsDataStore& other) 
{
  Int_t nevt = other.numEntries() ;
  for (int i=0 ; i<nevt ; i++) {  
    _vars = *other.get(i) ;
    if (_wgtVar) {
      _wgtVar->setVal(other.weight()) ;
    }
    
    fill() ;
  }
}



//_____________________________________________________________________________
Int_t RooVectorDataStore::numEntries() const 
{
  return _nEntries ;
}



//_____________________________________________________________________________
void RooVectorDataStore::reset() 
{
  _nEntries=0 ;
  _sumWeight=0 ;
  vector<RealVector*>::iterator iter = _realStoreList.begin() ;
  for ( ; iter!=_realStoreList.end() ; ++iter) {
    (*iter)->reset() ;
  }  
  vector<RealFullVector*>::iterator iter3 = _realfStoreList.begin() ;
  for ( ; iter3!=_realfStoreList.end() ; ++iter3) {
    (*iter3)->reset() ;
  }  
  vector<CatVector*>::iterator iter2 = _catStoreList.begin() ;
  for ( ; iter2!=_catStoreList.end() ; ++iter2) {
    (*iter2)->reset() ;
  }  

}



//_____________________________________________________________________________
void RooVectorDataStore::cacheArgs(const RooAbsArg* owner, RooArgSet& newVarSet, const RooArgSet* nset) 
{
  // Cache given RooAbsArgs with this tree: The tree is
  // given direct write access of the args internal cache
  // the args values is pre-calculated for all data points
  // in this data collection. Upon a get() call, the
  // internal cache of 'newVar' will be loaded with the
  // precalculated value and it's dirty flag will be cleared.


  TIterator* vIter = newVarSet.createIterator() ;
  RooAbsArg* var ;
  
  checkInit() ;
  
  TList cloneSetList ;
  RooArgSet cloneSet ;

  while((var=(RooAbsArg*)vIter->Next())) {

    // Clone variable and attach to cloned tree 
    RooArgSet* newVarCloneList = (RooArgSet*) RooArgSet(*var).snapshot() ;  
    RooAbsArg* newVarClone = newVarCloneList->find(var->GetName()) ;   
    newVarClone->recursiveRedirectServers(_vars,kFALSE) ;

    cloneSetList.Add(newVarCloneList) ;
    cloneSet.add(*newVarClone) ;

  }
  delete vIter ;

  _cacheOwner = (RooAbsArg*) owner ;
  RooVectorDataStore* newCache = new RooVectorDataStore("cache","cache",newVarSet) ;

  TIterator* cIter = cloneSet.createIterator() ;
  RooAbsArg *cloneArg ;

  RooAbsArg::setDirtyInhibit(kTRUE) ;

  // Now need to attach branch buffers of clones
  TIterator* it = cloneSet.createIterator() ;
  RooAbsArg* arg ;
  while ((arg=(RooAbsArg*)it->Next())) {
    arg->attachToVStore(*newCache) ;
  }
  delete it ;

  // Fill values of of placeholder
  for (int i=0 ; i<numEntries() ; i++) {
    get(i) ;

    cIter->Reset() ;
    while((cloneArg=(RooAbsArg*)cIter->Next())) {
      cloneArg->syncCache(nset) ;
    }
    newCache->fill() ;
  }
  
  delete cIter ;

  RooAbsArg::setDirtyInhibit(kFALSE) ;

  cloneSetList.Delete() ;    
  
  // Now need to attach branch buffers of original function objects 
  it = newVarSet.createIterator() ;
  while ((arg=(RooAbsArg*)it->Next())) {
    arg->attachToVStore(*newCache) ;
  }
  delete it ;

  _cache = newCache ;
  _cache->setDirtyProp(_doDirtyProp) ;
}


//_____________________________________________________________________________
void RooVectorDataStore::attachCache(const RooAbsArg* newOwner, const RooArgSet& cachedVarsIn) 
{
  // Initialize cache of dataset: attach variables of cache ArgSet
  // to the corresponding TTree branches

  // Only applicabel if a cache exists
  if (!_cache) return ;

  // Clone ctor, must connect internal storage to given new external set of vars
  vector<RealVector*>::const_iterator oiter = _cache->_realStoreList.begin() ;
  for (; oiter!=_cache->_realStoreList.end() ; ++oiter) {
    RooAbsReal* real = (RooAbsReal*) cachedVarsIn.find((*oiter)->bufArg()->GetName()) ;
    if (real) {
      // Adjust buffer pointer
      real->attachToVStore(*_cache) ;
    }
  }

  vector<RealFullVector*>::const_iterator fiter = _cache->_realfStoreList.begin() ;
  for (; fiter!=_cache->_realfStoreList.end() ; ++fiter) {
    RooAbsReal* real = (RooAbsReal*) cachedVarsIn.find((*fiter)->bufArg()->GetName()) ;
    if (real) {
      // Adjust buffer pointer
      real->attachToVStore(*_cache) ;
    }
  }

  vector<CatVector*>::const_iterator citer = _cache->_catStoreList.begin() ;
  for (; citer!=_cache->_catStoreList.end() ; ++citer) {
    RooAbsCategory* cat = (RooAbsCategory*) cachedVarsIn.find((*citer)->bufArg()->GetName()) ;
    if (cat) {
      // Adjust buffer pointer
      cat->attachToVStore(*_cache) ;
    }
  }

  _cacheOwner = (RooAbsArg*) newOwner ;
}




//_____________________________________________________________________________
void RooVectorDataStore::resetCache() 
{
  if (_cache) {
    delete _cache ;
    _cache = 0 ;
    _cacheOwner = 0 ;
  }
  return ;
}





//_____________________________________________________________________________
void RooVectorDataStore::setArgStatus(const RooArgSet& /*set*/, Bool_t /*active*/) 
{
  // Disabling of branches is (intentionally) not implemented in vector
  // data stores (as the doesn't result in a net saving of time)

  return ;
}




