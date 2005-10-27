#include "TROOT.h"
#include "TFitterMinuit.h"
#include "TMath.h"
#include "TF1.h"
#include "TH1.h"
#include "TGraph.h"

#include "TChi2FCN.h"
#include "TChi2ExtendedFCN.h"
#include "TBinLikelihoodFCN.h"


#include "Minuit/MnMigrad.h"
#include "Minuit/MnMinos.h"
#include "Minuit/MnHesse.h"
#include "Minuit/MinuitParameter.h"
#include "Minuit/MnPrint.h"
#include "Minuit/FunctionMinimum.h"
#include "Minuit/VariableMetricMinimizer.h"
#include "Minuit/SimplexMinimizer.h"
#include "Minuit/CombinedMinimizer.h"
#include "Minuit/ScanMinimizer.h"


#ifndef ROOT_TMethodCall
#include "TMethodCall.h"
#endif
//for the G__p2f2funcname symbol
#include "Api.h"

//#define DEBUG 1

//#define OLDFCN_INTERFACE
#ifdef OLDFCN_INTERFACE
extern void H1FitChisquare(Int_t &npar, Double_t *gin, Double_t &f, Double_t *u, Int_t flag);
extern void H1FitLikelihood(Int_t &npar, Double_t *gin, Double_t &f, Double_t *u, Int_t flag);
extern void Graph2DFitChisquare(Int_t &npar, Double_t *gin, Double_t &f, Double_t *u, Int_t flag);
extern void MultiGraphFitChisquare(Int_t &npar, Double_t *gin, Double_t &f, Double_t *u, Int_t flag);
#endif



ClassImp(TFitterMinuit);

TFitterMinuit* gMinuit2 = 0;

TFitterMinuit::TFitterMinuit() : fErrorDef(0.) , fEDMVal(0.), fGradient(false),
		     fState(MnUserParameterState()), theMinosErrors(std::vector<MinosError>()), fMinimizer(0), fMinuitFCN(0), fDebug(1), fStrategy(1), fMinTolerance(0) {
  Initialize();
}


// needed this additional contructor ? 
TFitterMinuit::TFitterMinuit(Int_t /* maxpar */) : fErrorDef(0.) , fEDMVal(0.), fGradient(false), fState(MnUserParameterState()), theMinosErrors(std::vector<MinosError>()), fMinimizer(0), fMinuitFCN(0), fDebug(1), fStrategy(1), fMinTolerance(0) { 
  Initialize();
}

void TFitterMinuit::Initialize() {
   SetName("Minuit2");
   gMinuit2 = this;
   gROOT->GetListOfSpecials()->Add(gMinuit2);
 
}

// create the minimizer engine and register the plugin in ROOT 
void TFitterMinuit::CreateMinimizer(EMinimizerType type) { 
#ifdef DEBUG
  std::cout<<"TFitterMinuit:create minimizer of type "<< type << std::endl;
#endif
  if (fMinimizer) delete fMinimizer;

  switch (type) { 
  case kMigrad: 
    SetMinimizer( new VariableMetricMinimizer() );
    return;
  case kSimplex: 
    SetMinimizer( new SimplexMinimizer() );
    return;
  case kCombined: 
    SetMinimizer( new CombinedMinimizer() );
    return;
  case kScan: 
    SetMinimizer( new ScanMinimizer() );
    return;
  case kFumili: 
    std::cout << "TFitterMinuit::Error - Fumili Minimizer must be created from TFitterFumili " << std::endl;
    SetMinimizer(0);
    return;
  default: 
    //migrad minimizer
     SetMinimizer( new VariableMetricMinimizer() );
  }
}
 


 // destructor - deletes the minimizer and minuit fcn
TFitterMinuit::~TFitterMinuit() { 
  //std::cout << "delete minimizer and FCN" << std::endl;
  if (fMinuitFCN) delete fMinuitFCN; 
  if (fMinimizer) delete fMinimizer; 
}


Double_t TFitterMinuit::Chisquare(Int_t npar, Double_t *params) const {
  // do chisquare calculations in case of likelihood fits 
  TChi2FCN chi2( *this ); 
  std::vector<double> p(npar); 
  for (int i = 0; i < npar; ++i) 
    p[i] = params[i];
  return chi2(p);
}

void TFitterMinuit::Clear(Option_t*) {
  //std::cout<<"clear "<<std::endl;
  
//   fErrorDef = 0.;
//   theNFcn = 0;
//   fEDMVal = 0.;
//   fGradient = false;

  State() = MnUserParameterState();
  theMinosErrors.clear();
}



FunctionMinimum TFitterMinuit::DoMinimization( int nfcn, double edmval)  { 
  assert(GetMinuitFCN() );
  assert(GetMinimizer() );
  // use always strategy 1 (2 is not yet fully tested)
  return GetMinimizer()->minimize(*GetMinuitFCN(), State(), MnStrategy(1), nfcn, edmval);
}

  

int  TFitterMinuit::Minimize( int nfcn, double edmval)  { 

  // min tolerance
  edmval = std::max(fMinTolerance, edmval);
  if (fDebug >=0) { 
    std::cout << "TFitterMinuit - Minimize with max iterations " << nfcn << " edmval " << edmval << std::endl;
  }
  FunctionMinimum min = DoMinimization(nfcn,edmval);
  fState = min.userState();
  return ExamineMinimum(min);
}

Int_t TFitterMinuit::ExecuteCommand(const char *command, Double_t *args, Int_t nargs){	

#ifdef DEBUG
  std::cout<<"Execute command= "<<command<<std::endl;
#endif

  // MIGRAD 
  if (strncmp(command,"MIG",3) == 0 || strncmp(command,"mig",3)  == 0) {
    int nfcn = 0; // default values for Minuit
    double edmval = 0.1;
//     nfcn = GetMaxIterations();
//     edmval = GetPrecision();
    if (nargs > 0) nfcn = int ( args[0] );
    if (nargs > 1) edmval = args[1];

    // create migrad minimizer
    CreateMinimizer(kMigrad);
    return  Minimize(nfcn, edmval);

//     if(fGradient) {
//       MnMigrad migrad(theFcn, State());
//       theMinimum = migrad(theNFcn, fEDMVal);
//       State() = theMinimum.userParameters();
//     } else {
//     std::cout<<"State(): "<<State()<<std::endl;
//     std::cout<<"State(): "<<State()<<std::endl;
//     }      


   
  } 
  //Minimize
  if (strncmp(command,"MINI",4) == 0 || strncmp(command,"mini",4)  == 0) {

    int nfcn = 0; // default values for Minuit
    double edmval = 0.1;
    if (nargs > 0) nfcn = int ( args[0] );
    if (nargs > 1) edmval = args[1];
    // create combined minimizer
    CreateMinimizer(kCombined);
    return Minimize(nfcn, edmval);
  }
  //Simplex
  if (strncmp(command,"SIM",3) == 0 || strncmp(command,"sim",3)  == 0) {

    int nfcn = 0; // default values for Minuit
    double edmval = 0.1;
    if (nargs > 0) nfcn = int ( args[0] );
    if (nargs > 1) edmval = args[1];
    // create combined minimizer
    CreateMinimizer(kSimplex);
    return Minimize(nfcn, edmval);
  }
  //SCan
  if (strncmp(command,"SCA",3) == 0 || strncmp(command,"sca",3)  == 0) {

    int nfcn = 0; // default values for Minuit
    double edmval = 0.1;
    if (nargs > 0) nfcn = int ( args[0] );
    if (nargs > 1) edmval = args[1];
    // create combined minimizer
    CreateMinimizer(kScan);
    return Minimize(nfcn, edmval);
  }
  // MINOS 
  else if (strncmp(command,"MINO",4) == 0 || strncmp(command,"mino",4)  == 0) {

    // recall minimize using default nfcn and edmval
    // should use maybe FunctionMinimum from previous call to migrad
    // need to keep a pointer to function minimum in the class
    FunctionMinimum min = DoMinimization();
    if (!min.isValid() ) { 
      std::cout << "TFitterMinuit::MINOS failed due to invalid function minimum" << std::endl;
      return -10;
    }
    MnMinos minos( *fMinuitFCN, min);
    for(unsigned int i = 0; i < State().variableParameters(); i++) {
      MinosError me = minos.minos(State().extOfInt(i));
      theMinosErrors.push_back(me);
    }
    return 0;
  } 
  //HESSE
  else if (strncmp(command,"HES",3) == 0 || strncmp(command,"hes",3)  == 0) {
    MnHesse hesse( GetStrategy() ); 
    // update the state
    const FCNBase * fcn = GetMinuitFCN();
    assert(fcn);
    fState = hesse(*fcn, State(),100000 );
    if (!fState.isValid() ) {
      std::cout << "TFitterMinuit::Hesse calculation failed " << std::endl;
      return -10;
    }
    return 0; 
  } 

  // FIX 
  else if (strncmp(command,"FIX",3) == 0 || strncmp(command,"fix",3)  == 0) {
    for(int i = 0; i < nargs; i++) {
      FixParameter(int(args[i]));
    }
    return 0;
  } 
  // SET LIMIT (uper and lower)
  else if (strncmp(command,"SET LIM",7) == 0 || strncmp(command,"set lim",7)  == 0) {
    assert(nargs >= 3);
    int ipar = int(args[0]);
    double low = args[1];
    double up = args[2];
    State().setLimits(ipar, low, up);
    return 0; 
  } 
  // SET PRINT
  else if (strncmp(command,"SET PRINT",9) == 0 || strncmp(command,"set print",9)  == 0) {
    fDebug = int(args[0]);
#ifdef DEBUG
    fDebug = 3;
#endif
    return 0; 
  } 
  // SET STRATEGY
  else if (strncmp(command,"SET STR",7) == 0 || strncmp(command,"set str",7)  == 0) {
    fStrategy = int(args[0]);
    return 0; 
  } 
  //SET GRAD (not impl.) 
  else if (strncmp(command,"SET GRA",7) == 0 || strncmp(command,"set gra",7)  == 0) {
//     not yet available
//     fGradient = true;
    return -1;
  } 
  else {
    // other commands passed 
    return 0;
  }
 

}


/// study the function minimum 
int  TFitterMinuit::ExamineMinimum(const FunctionMinimum & min) {  


  // debug ( print all the states) 
  if (fDebug >= 3) { 

    const std::vector<MinimumState>& iterationStates = min.states();
    std::cout << "Number of iterations " << iterationStates.size() << std::endl;
    for (unsigned int i = 0; i <  iterationStates.size(); ++i) {
      //std::cout << iterationStates[i] << std::endl;                                                                       
      const MinimumState & st =  iterationStates[i];
      std::cout << "----------> Iteration " << i << std::endl;
      std::cout << "            FVAL = " << st.fval() << " Edm = " << st.edm() << " Nfcn = " << st.nfcn() << std::endl;
      std::cout << "            Error matrix change = " << st.error().dcovar() << std::endl;
      std::cout << "            Internal parameters : ";
      for (int j = 0; j < st.size() ; ++j) std::cout << " p" << j << " = " << st.vec()(j);
      std::cout << std::endl;
    }
  }
  // print result 
  if (min.isValid() ) {
    if (fDebug >=1 ) { 
      std::cout << "Minimum Found" << std::endl; 
      std::cout << "FVAL  = " << State().fval() << std::endl;
      std::cout << "Edm   = " << State().edm() << std::endl;
      std::cout << "Nfcn  = " << State().nfcn() << std::endl;
      std::vector<double> par = State().params();
      std::vector<double> err = State().errors();
      for (unsigned int i = 0; i < State().params().size(); ++i) 
	std::cout << State().parameter(i).name() << "\t  = " << par[i] << "\t  +/-  " << err[i] << std::endl; 
    }
    return 0;
  }
  else { 
    if (fDebug >= 1)  std::cout << "TFitterMinuit::Minimization DID not converge !" << std::endl; 
    if (min.hasMadePosDefCovar() ) { 
      if (fDebug >= 1) std::cout << "      Covar was made pos def" << std::endl;
      return -11; 
    }
    if (min.hesseFailed() ) { 
      if (fDebug >= 1) std::cout << "      Hesse is not valid" << std::endl;
      return -12; 
    }
    if (min.isAboveMaxEdm() ) { 
      if (fDebug >= 1) std::cout << "      Edm is above max" << std::endl;
      return -13; 
    }
    if (min.hasReachedCallLimit() ) { 
      if (fDebug >= 1) std::cout << "      Reached call limit" << std::endl;
      return -14; 
    }
    return -10; 
  }
  return 0;
}

void TFitterMinuit::FixParameter(Int_t ipar) {
//   std::cout<<"FixParameter"<<std::endl;
  State().fix(ipar);
}

Double_t* TFitterMinuit::GetCovarianceMatrix() const {
  return (Double_t*)(&(State().covariance().data().front()));
}

Double_t TFitterMinuit::GetCovarianceMatrixElement(Int_t i, Int_t j) const {

  std::cout<<"GetCovarianceMatrix not implemented"<<std::endl;
  return State().covariance()(i,j);
}


Int_t TFitterMinuit::GetErrors(Int_t ipar,Double_t &eplus, Double_t &eminus, Double_t &eparab, Double_t &globcc) const {
//   std::cout<<"GetError"<<std::endl;
  if(theMinosErrors.empty()) {
    eplus = 0.;
    eminus = 0.; 
  } else {
    unsigned int nintern = State().intOfExt(ipar);
    eplus = theMinosErrors[nintern].upper();
    eminus = theMinosErrors[nintern].lower();
  }
  eparab = State().error(ipar);
  globcc = State().globalCC().globalCC()[ipar];
  return 0;
}

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// here is example of deficit of new Minuit interface
//////////////////////////////////////////////
Int_t TFitterMinuit::GetNumberTotalParameters() const {


  return State().parameters().parameters().size();
}
Int_t TFitterMinuit::GetNumberFreeParameters() const {

  return State().parameters().variableParameters();
}


Double_t TFitterMinuit::GetParError(Int_t ipar) const {
//   std::cout<<"GetParError"<<std::endl;
  return State().error(ipar);
}

Double_t TFitterMinuit::GetParameter(Int_t ipar) const {
//   std::cout<<"GetParameter"<<std::endl;
  return State().value(ipar);
}

Int_t TFitterMinuit::GetParameter(Int_t ipar,char *name,Double_t &value,Double_t &verr,Double_t &vlow, Double_t &vhigh) const {
//   std::cout<<"GetParameter(Int_t ipar,char"<<std::endl;
  const MinuitParameter& mp = State().parameter(ipar);
//   std::cout<<"i= "<<ipar<<" verr= "<<mp.error()<<std::endl;
  name = const_cast<char*>(mp.name());
  value = mp.value();
  verr = mp.error();
  vlow = mp.lowerLimit();
  vhigh = mp.upperLimit();
  return 0;
}

Int_t TFitterMinuit::GetStats(Double_t &amin, Double_t &edm, Double_t &errdef, Int_t &nvpar, Int_t &nparx) const {
//   std::cout<<"GetStats"<<std::endl;
  amin = State().fval();
  edm = State().edm();
  errdef = fErrorDef;
  nvpar = State().variableParameters();
  nparx = State().parameters().parameters().size();
  return 0;
}

Double_t TFitterMinuit::GetSumLog(Int_t) {
//   std::cout<<"GetSumLog"<<std::endl;
  return 0.;
}

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
Bool_t TFitterMinuit::IsFixed(Int_t ipar) const {
  return State().parameter(ipar).isFixed();
}


void TFitterMinuit::PrintResults(Int_t level, Double_t) const {
//   std::cout<<"PrintResults"<<std::endl;
//   std::cout<<State().parameters()<<std::endl;
  std::cout<<State()<<std::endl;
  if (level > 3) { 
    for(std::vector<MinosError>::const_iterator ime = theMinosErrors.begin();
	ime != theMinosErrors.end(); ime++) {
      std::cout<<*ime<<std::endl;
    }
  }
}

void TFitterMinuit::ReleaseParameter(Int_t ipar) {
//   std::cout<<"ReleaseParameter"<<std::endl;
  State().release(ipar);
}



void TFitterMinuit::SetFitMethod(const char *name) {
#ifdef DEBUG
  std::cout<<"SetFitMethod to "<< name << std::endl;
#endif
  if (!strcmp(name,"H1FitChisquare")) {
    fErrorDef = 1.;
    // old way of passing
#ifdef OLDFCN_INTERFACE 
    SetFCN(H1FitChisquare);
#else 
    // call function (because overloaded by derived class)
    CreateChi2FCN();
#endif
    return;
  }
  if (!strcmp(name,"GraphFitChisquare")) {
    fErrorDef = 1.;
#ifdef OLDFCN_INTERFACE 
    SetFCN(GraphFitChisquare);
#else 
    // use for graph extended chi2 to include error in X coordinate
    if (!GetFitOption().W1) 
      CreateChi2ExtendedFCN( );
     else
      CreateChi2FCN( );
#endif
    return;
  }
  if (!strcmp(name, "Graph2DFitChisquare")) {
    fErrorDef = 1.;
#ifdef OLDFCN_INTERFACE 
    SetFCN(Graph2DFitChisquare);
#else 
    // use for graph extended chi2 to include error in X and Y coordinates
//     if (!GetFitOption().W1) {
//       CreateChi2ExtendedFCN( );
//      else
      CreateChi2FCN( );
#endif
    return;
  }
  if (!strcmp(name, "MultiGraphFitChisquare")) {
    fErrorDef = 1.;
#ifdef OLDFCN_INTERFACE 
   SetFCN(MultiGraphFitChisquare);
#else 
   CreateChi2FCN();
#endif
   return;
  }
  if (!strcmp(name, "H1FitLikelihood")) {
    // old way of passing
#ifdef OLDFCN_INTERFACE 
    fErrorDef = 1.;
    SetFCN(H1FitLikelihood);
#else 
    fErrorDef = 0.5;
    CreateBinLikelihoodFCN();    
#endif  
    return;
  }

  std::cout << "TFitterMinuit::fit method " << name << " is not  supported !" << std::endl; 
  
  assert(fMinuitFCN);
  


// 
//   } else {
//     SetFCN(H1FitChisquare);
//   }
    // TFitterMinuit manages the data


}

Int_t TFitterMinuit::SetParameter(Int_t,const char *parname,Double_t value,Double_t verr,Double_t vlow, Double_t vhigh) {
#ifdef DEBUG
   std::cout<<"SetParameter";
   std::cout<<" i= "<<parname<<" value = " << value << " verr= "<<verr<<std::endl;
#endif
   if(vlow < vhigh) { 
    State().add(parname, value, verr, vlow, vhigh);
   }
  else
    State().add(parname, value, verr);
  return 0;
}

// override setFCN to use the Adapter
void TFitterMinuit::SetFCN(void (*fcn)(Int_t &, Double_t *, Double_t &f, Double_t *, Int_t))
{
//*-*-*-*-*-*-*To set the address of the minimization function*-*-*-*-*-*-*-*
//*-*          ===============================================
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
   fFCN = fcn;
   if (fMinuitFCN) delete fMinuitFCN;
   fMinuitFCN = new TFcnAdapter(fFCN);
}

// need for interactive environment


// global functions needed by interpreter 


//______________________________________________________________________________
void InteractiveFCNm2(Int_t &npar, Double_t *gin, Double_t &f, Double_t *u, Int_t flag)
{
//*-*-*-*-*-*-*Static function called when SetFCN is called in interactive mode
//*-*          ===============================================

   TMethodCall *m  = gMinuit2->GetMethodCall();
   if (!m) return;

   Long_t args[5];
   args[0] = (Long_t)&npar;
   args[1] = (Long_t)gin;
   args[2] = (Long_t)&f;
   args[3] = (Long_t)u;
   args[4] = (Long_t)flag;
   m->SetParamPtrs(args);
   Double_t result;
   m->Execute(result);
}


//______________________________________________________________________________
void TFitterMinuit::SetFCN(void *fcn)
{
//*-*-*-*-*-*-*To set the address of the minimization function*-*-*-*-*-*-*-*
//*-*          ===============================================
//     this function is called by CINT instead of the function above
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   if (!fcn) return;

   char *funcname = G__p2f2funcname(fcn);
   if (funcname) {
      fMethodCall = new TMethodCall();
      fMethodCall->InitWithPrototype(funcname,"Int_t&,Double_t*,Double_t&,Double_t*,Int_t");
    }
   fFCN = InteractiveFCNm2;
   gMinuit2 = this; //required by InteractiveFCNm

   if (fMinuitFCN) delete fMinuitFCN;
   fMinuitFCN = new TFcnAdapter(fFCN);
}

void TFitterMinuit::SetMinuitFCN(  FCNBase * f) { 
  // class takes the ownership of the passed pointer
  // so needs to delete previous one 
  if (fMinuitFCN) delete fMinuitFCN;
  fMinuitFCN = f; 
}

void TFitterMinuit::CreateChi2FCN() { 
  SetMinuitFCN(new TChi2FCN( *this ) );
}


void TFitterMinuit::CreateChi2ExtendedFCN() { 
  SetMinuitFCN(new TChi2ExtendedFCN( *this ) );
}

void TFitterMinuit::CreateBinLikelihoodFCN() { 
  SetMinuitFCN(new TBinLikelihoodFCN( *this ) );
}
