/*****************************************************************************
 * Project: BaBar detector at the SLAC PEP-II B-factory
 * Package: RooFitCore
 *    File: $Id: RooFitContext.cc,v 1.11 2001/06/18 21:04:21 verkerke Exp $
 * Authors:
 *   DK, David Kirkby, Stanford University, kirkby@hep.stanford.edu
 *   WV, Wouter Verkerke, UC Santa Barbara, verkerke@slac.stanford.edu
 * History:
 *   07-Mar-2001 WV Created initial version
 *
 * Copyright (C) 2001 University of California
 *****************************************************************************/

// -- CLASS DESCRIPTION --
// RooFitContext holds and combines a RooAbsPdf and a RooDataSet
// for unbinned maximum likelihood fitting. The PDF and DataSet 
// are both cloned and tied to each other.
//
// The context implements various optimization techniques that can
// only be made under the assumption that the dependent/parameter
// interpretation of all servers is fixed for the duration of the
// fit. (For example PDFs with exclusively constant parameters
// can be precalculated)
//
// This class also contains the interface to MINUIT to peform the
// actual fitting.


#include <fstream.h>
#include <iomanip.h>
#include "TStopwatch.h"
#include "TFitter.h"
#include "TMinuit.h"
#include "RooFitCore/RooFitContext.hh"
#include "RooFitCore/RooDataSet.hh"
#include "RooFitCore/RooAbsPdf.hh"
#include "RooFitCore/RooRealVar.hh"

ClassImp(RooFitContext)
;

static TVirtualFitter *_theFitter(0);


RooFitContext::RooFitContext(const RooDataSet* data, const RooAbsPdf* pdf) :
  TNamed(*pdf), _origLeafNodeList("origLeafNodeList")
{
  // Constructor

  if(0 == data) {
    cout << "RooFitContext: cannot create without valid dataset" << endl;
    return;
  }
  if(0 == pdf) {
    cout << "RooFitContext: cannot create without valid PDF" << endl;
    return;
  }

  // Clone data 
  _dataClone = new RooDataSet(*data) ;

  // Clone all PDF compents by copying all branch nodes
  RooArgSet tmp("PdfBranchNodeList") ;
  pdf->branchNodeServerList(&tmp) ;
  _pdfCompList = tmp.snapshot(kFALSE) ;

  // Find the top level PDF in the snapshot list
  _pdfClone = (RooAbsPdf*) _pdfCompList->FindObject(pdf->GetName()) ;
  
  // Attach PDF to data set
  _pdfClone->attachDataSet(*_dataClone) ;
  _pdfClone->resetErrorCounters() ;

  // Cache parameter list
  _paramList = _pdfClone->getParameters(_dataClone) ;
  _nPar      = _paramList->GetSize() ;  

  // Store the original leaf node list
  pdf->leafNodeServerList(&_origLeafNodeList) ;  
}



RooFitContext::~RooFitContext() 
{
  // Destructor

  delete _pdfCompList ;
  delete _dataClone ;
  delete _paramList ;
}


void RooFitContext::printToStream(ostream &os, PrintOption opt, TString indent) const
{
  // Print contents 
  os << "DataSet clone:" << endl ;
  _dataClone->printToStream(os,opt,indent) ;

  os << indent << "PDF clone:" << endl ;
  _pdfClone->printToStream(os,opt,indent) ;

  os << indent << "PDF component list:" << endl ;
  _pdfCompList->printToStream(os,opt,indent) ;

  os << indent << "Parameter list:" << endl ;
  _paramList->printToStream(os,opt,indent) ;

  return ;
}


Double_t RooFitContext::getPdfParamVal(Int_t index)
{
  // Access PDF parameter value by ordinal index (needed by MINUIT)
  return ((RooRealVar*)_paramList->At(index))->getVal() ;
}


Double_t RooFitContext::getPdfParamErr(Int_t index)
{
  // Access PDF parameter error by ordinal index (needed by MINUIT)
  return ((RooRealVar*)_paramList->At(index))->getError() ;  
}


void RooFitContext::setPdfParamVal(Int_t index, Double_t value)
{
  // Modify PDF parameter value by ordinal index (needed by MINUIT)
  RooRealVar* par = (RooRealVar*)_paramList->At(index) ;

  if (par->getVal()!=value) {
    par->setVal(value) ;  
  }
}



void RooFitContext::setPdfParamErr(Int_t index, Double_t value)
{
  // Modify PDF parameter error by ordinal index (needed by MINUIT)
  ((RooRealVar*)_paramList->At(index))->setError(value) ;    
}



Bool_t RooFitContext::optimize(Bool_t doPdf, Bool_t doData) 
{
  // PDF/Dataset optimizer entry point

  // Find PDF nodes that can be cached in the data set
  RooArgSet cacheList("cacheList") ;

  if (doPdf) {
    findCacheableBranches(_pdfClone,_dataClone,cacheList) ;
  
    // Add cached branches from the data set
    _dataClone->cacheArgs(cacheList) ;
  }


  if (doData) {
    // Find unused/unnecessary branches from the data set
    RooArgSet pruneList("pruneList") ;
    findUnusedDataVariables(_pdfClone,_dataClone,pruneList) ;

    if (doPdf)
      findRedundantCacheServers(_pdfClone,_dataClone,cacheList,pruneList) ;
    
    if (pruneList.GetSize()!=0) {
      // Created trimmed list of data variables
      RooArgSet newVarList(*_dataClone->get()) ;
      TIterator* iter = pruneList.MakeIterator() ;
      RooAbsArg* arg ;
      while (arg = (RooAbsArg*) iter->Next()) {
	cout << "RooFitContext::optimizePDF: dropping variable " 
	     << arg->GetName() << " from context data set" << endl ;
	newVarList.remove(*arg) ;      
      }      
      delete iter ;
      
      // Create trimmed data set
      RooDataSet *trimData = new RooDataSet("trimData","Reduced data set for fit context",
					    _dataClone,newVarList,kTRUE) ;
      
      // Unattach PDF clone from previous dataset
      _pdfClone->recursiveRedirectServers(_origLeafNodeList,kFALSE);
      
      // Reattach PDF clone to newly trimmed dataset
      _pdfClone->attachDataSet(*trimData) ;
      
      // Substitute new data for old data 
      delete _dataClone ;
      _dataClone = trimData ;
    }
  }
  return kFALSE ;
}



Bool_t RooFitContext::findCacheableBranches(RooAbsPdf* pdf, RooDataSet* dset, 
					    RooArgSet& cacheList) 
{
  // Find branch PDFs with all-constant parameters, and add them
  // to the dataset cache list

  TIterator* sIter = pdf->serverIterator() ;
  RooAbsPdf* server ;

  while(server=(RooAbsPdf*)sIter->Next()) {
    if (server->isDerived() && server->IsA()->InheritsFrom(RooAbsPdf::Class())) {
      // Check if this branch node is eligible for precalculation
      Bool_t canOpt(kTRUE) ;

      RooArgSet* branchParamList = server->getParameters(dset) ;
      TIterator* pIter = branchParamList->MakeIterator() ;
      RooAbsArg* param ;
      while(param = (RooAbsArg*)pIter->Next()) {
	if (!param->isConstant()) canOpt=kFALSE ;
      }
      delete branchParamList ;

      if (canOpt) {
	cout << "RooFitContext::optimizePDF: component PDF " << server->GetName() 
	     << " of PDF " << pdf->GetName() << " will be cached" << endl ;

	// Add to cache list
	cacheList.add(*server) ;

      } else {
	// Recurse if we cannot optimize at this level
	findCacheableBranches(server,dset,cacheList) ;
      }
    }
  }
  delete sIter ;
  return kFALSE ;
}



void RooFitContext::findUnusedDataVariables(RooAbsPdf* pdf,RooDataSet* dset,RooArgSet& pruneList) 
{
  TIterator* vIter = dset->get()->MakeIterator() ;
  RooAbsArg* arg ;
  while (arg=(RooAbsArg*) vIter->Next()) {
    if (!pdf->dependsOn(*arg)) pruneList.add(*arg) ;
  }
  delete vIter ;
}


void RooFitContext::findRedundantCacheServers(RooAbsPdf* pdf,RooDataSet* dset,RooArgSet& cacheList, RooArgSet& pruneList) 
{
  TIterator* vIter = dset->get()->MakeIterator() ;
  RooAbsArg *var ;
  while (var=(RooAbsArg*) vIter->Next()) {
    if (allClientsCached(var,cacheList)) pruneList.add(*var) ;
  }
  delete vIter ;
}



Bool_t RooFitContext::allClientsCached(RooAbsArg* var, RooArgSet& cacheList)
{
  Bool_t ret(kTRUE), anyClient(kFALSE) ;

  TIterator* cIter = var->valueClientIterator() ;    
  RooAbsArg* client ;
  while (client=(RooAbsArg*) cIter->Next()) {

    anyClient = kTRUE ;
    if (!cacheList.FindObject(client)) {
      // If client is not cached recurse
      ret = allClientsCached(client,cacheList) ;
    }
  }
  delete cIter ;

  return anyClient?ret:kFALSE ;
}



Int_t RooFitContext::fit(Option_t *options, Double_t *minVal) 
{
  // Setup and perform MINUIT fit of PDF to dataset

  // Parse our options string
  TString opts= options;
  opts.ToLower();
  Bool_t verbose       =!opts.Contains("q") ;
  Bool_t migradOnly    = opts.Contains("m") ;
  Bool_t estimateSteps = opts.Contains("s") ;
  Bool_t performHesse  = opts.Contains("h") ;
  Bool_t saveLog       = opts.Contains("l") ;
  Bool_t profileTimer  = opts.Contains("t") ;
         _extendedMode = opts.Contains("e") ;
  Bool_t doOptPdf      = opts.Contains("o") ;
  Bool_t doOptData     = opts.Contains("oo") ;

  // Check if an extended ML fit is possible
  if(_extendedMode) {
    if(!_pdfClone->canBeExtended()) {
      cout << _pdfClone->GetName() << "::fitTo: this PDF does not support extended "
           << "maximum likelihood fits" << endl;
      return -1;
    }
    if(verbose) {
      cout << _pdfClone->GetName() << "::fitTo: will use extended maximum likelihood" << endl;
    }
  }

  // Check if there are any unprotected multiple occurrences of dependents
  if (_pdfClone->checkDependents(_dataClone)) {
    cout << "RooFitContext::fit: Error in PDF dependents, abort" << endl ;
    return -1 ;
  }

  // Run the optimizer if requested
  if (doOptPdf||doOptData) optimize(doOptPdf,doOptData) ;

  // Create a log file if requested
  if(saveLog) {
    TString logname= fName;
    logname.Append(".log");
    _logfile= new ofstream(logname.Data());
    if(_logfile && _logfile->good()) {
      cout << fName << "::fitTo: saving fit log to " << logname << endl;
    } else {
      cout << fName << "::fitTo: unable to open logfile " << logname << endl;
      _logfile= 0;
      saveLog= kFALSE;
    }
  } else {
    _logfile = 0 ;
  }

  // Start a profiling timer if requested
  TStopwatch timer;
  if(profileTimer) timer.Start();

  // Initialize MINUIT
  Int_t nPar= _paramList->GetSize();
  Double_t params[100], arglist[100];

  if (_theFitter) delete _theFitter ;
  _theFitter = new TFitter(nPar) ;
  _theFitter->SetObjectFit(this) ;

  _theFitter->Clear();

  // Be quiet during the setup
  arglist[0] = -1;
//_theFitter->ExecuteCommand("SET PRINT",arglist,1);
//_theFitter->ExecuteCommand("SET NOWARNINGS",arglist,0);

  // Tell MINUIT to use our global glue function
  _theFitter->SetFCN(RooFitGlue);

  // Use +0.5 for 1-sigma errors
  arglist[0]= 0.5;
  _theFitter->ExecuteCommand("SET ERR",arglist,1);

  // Declare our parameters
  Int_t index(0), nFree(nPar);
  for(index= 0; index < nPar; index++) {
    RooRealVar *par= (RooRealVar*)_paramList->At(index);

    Double_t pstep(0) ;
    Double_t pmin= par->getFitMin();
    Double_t pmax= par->getFitMax();

    if(!par->isConstant()) {

      // Verify that floating parameter is indeed of type RooRealVar 
      if (!par->IsA()->InheritsFrom(RooRealVar::Class())) {
	cout << "RooFitContext::fit: Error, non-constant parameter " << par->GetName() 
	     << " is not of type RooRealVar" << endl ;
	return -1 ;
      }

      // Calculate step size
      pstep= par->getError();
      if(pstep <= 0) {
	pstep= 0.1*(pmax-pmin);
	if(!estimateSteps && verbose) {
	  cout << "*** WARNING: no initial error estimate available for "
	       << par->GetName() << ": using " << pstep << endl;
	}
      }
    } 

    _theFitter->SetParameter(index, par->GetName(), par->getVal(),
			     pstep, pmin, pmax);

    if(par->isConstant() && (pmax > pmin) && (pstep > 0)) {
      // Declare fixed parameters (not necessary if range is zero)
      _theFitter->FixParameter(index);
      nFree--;
    }
  }

  // Now be verbose if requested
  if(verbose) {
    arglist[0] = 1;
    _theFitter->ExecuteCommand("SET PRINT",arglist,1);
    _theFitter->ExecuteCommand("SET WARNINGS",arglist,1);
  }

  // Reset the *largest* negative log-likelihood value we have seen so far
  _maxNLL= 0;

  // Do the fit
  arglist[0]= 250*nFree; // maximum iterations
  arglist[1]= 1.0;       // tolerance
  Int_t status(0);
  if(estimateSteps) {
    // Use HESSE to get reasonable starting step sizes for MIGRAD
    status= _theFitter->ExecuteCommand("HESSE",arglist,1);
  }

  // Always use MIGRAD unless an earlier step failed
  if(status == 0) {
    status= _theFitter->ExecuteCommand("MIGRAD",arglist,2);
  }

  // If the fit suceeded, follow with a HESSE analysis if requested
  if(status == 0 && performHesse) {
    arglist[0]= 250*nFree; // maximum iterations
    status= _theFitter->ExecuteCommand("HESSE",arglist,1);
  }

  // If the fit suceeded, follow with a MINOS analysis if requested
  if(status == 0 && !migradOnly) {
    arglist[0]= 250*nFree; // maximum iterations
    status= _theFitter->ExecuteCommand("MINOS",arglist,1);
  }

  // Get the fit results
  Double_t val,err,vlo,vhi, eplus, eminus, eparab, globcc;
  char buffer[10240];
  for(index= 0; index < nPar; index++) {
    _theFitter->GetParameter(index, buffer, val, err, vlo, vhi);
    setPdfParamVal(index, val);
    _theFitter->GetErrors(index, eplus, eminus, eparab, globcc);
    if(eplus > 0 || eminus < 0) {
    // Use the average asymmetric error, if it is available
      setPdfParamErr(index, 0.5*(eplus-eminus));
    }
    else {
    // Otherwise, use the parabolic error
      setPdfParamErr(index, eparab);
    }
  }

  if(minVal != 0) { // Get the minimum function value if requested
    Double_t edm, errdef;
    Int_t nvpar, nparx;
    _theFitter->GetStats(*minVal, edm, errdef, nvpar, nparx);
  }

  // Print the time used, if requested
  if(profileTimer) {
    timer.Stop();
    cout << fName << "::fitTo: ";
    timer.Print();
  }

  // Close the log file now
  if(saveLog) {
    _logfile->close();
    delete _logfile;
    _logfile= 0;
  }

  return status;
}




void RooFitGlue(Int_t &np, Double_t *gin,
                Double_t &f, Double_t *par, Int_t flag)
{
  // Static function that interfaces minuit with RooFitContext

  // Retrieve fit context and its components
  RooFitContext* context = (RooFitContext*) _theFitter->GetObjectFit() ;
  RooAbsPdf* pdf   = context->pdf() ;
  RooDataSet* data = context->data() ;
  ofstream* logf   = context->logfile() ;
  Double_t& maxNLL = context->maxNLL() ;

  // Set the parameter values for this iteration
  Int_t nPar= context->getNPar();
  for(Int_t index= 0; index < nPar; index++) {
    if (logf) (*logf) << par[index] << " ";
    context->setPdfParamVal(index, par[index]);
  }

  // Calculate the negative log-likelihood for these parameters
  f= pdf->nLogLikelihood(data, context->extendedMode());
  if (f==0) {
    // if any event has a prob <=0 return a flat likelihood 
    // at the max value we have seen so far
    f = maxNLL ;
  } else if (f>maxNLL) {
    maxNLL = f ;
  }

  // Optional logging
  if (logf) *logf << setprecision(15) << f << endl;
}


