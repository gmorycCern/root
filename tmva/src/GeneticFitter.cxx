// @(#)root/tmva $Id: GeneticFitter.cxx,v 1.20 2007/06/08 06:45:53 speckmayer Exp $ 
// Author: Peter Speckmayer

/**********************************************************************************
 * Project: TMVA - a Root-integrated toolkit for multivariate data analysis       *
 * Package: TMVA                                                                  *
 * Class  : GeneticFitter                                                         *
 * Web    : http://tmva.sourceforge.net                                           *
 *                                                                                *
 * Description:                                                                   *
 *      Implementation                                                            *
 *                                                                                *
 * Authors (alphabetical):                                                        *
 *      Peter Speckmayer <speckmay@mail.cern.ch> - CERN, Switzerland              *
 *                                                                                *
 * Copyright (c) 2005:                                                            *
 *      CERN, Switzerland                                                         * 
 *      MPI-K Heidelberg, Germany                                                 * 
 *                                                                                *
 * Redistribution and use in source and binary forms, with or without             *
 * modification, are permitted according to the terms listed in LICENSE           *
 * (http://tmva.sourceforge.net/LICENSE)                                          *
 **********************************************************************************/

//_______________________________________________________________________
//                                                                      
// Fitter using a Genetic Algorithm
//_______________________________________________________________________

#include "TMVA/GeneticFitter.h"
#include "TMVA/GeneticAlgorithm.h"
#include "TMVA/Interval.h"
#include "TMVA/Timer.h"

ClassImp(TMVA::GeneticFitter)

//_______________________________________________________________________
TMVA::GeneticFitter::GeneticFitter( IFitterTarget& target, 
                                    const TString& name, 
                                    const std::vector<TMVA::Interval*>& ranges, 
                                    const TString& theOption ) 
   : FitterBase( target, name, ranges, theOption )
{
   // constructor

   // default parameters settings for Genetic Algorithm
   DeclareOptions();
   ParseOptions();
}            

//_______________________________________________________________________
void TMVA::GeneticFitter::DeclareOptions() 
{
   // declare GA options

   DeclareOptionRef( fPopSize=300,    "PopSize",   "Population size for GA" );  
   DeclareOptionRef( fNsteps=40,      "Steps",     "Number of steps for convergence" );  
   DeclareOptionRef( fCycles=3,       "Cycles",    "Independent cycles of GA fitting" );  
   DeclareOptionRef( fSC_steps=10,    "SC_steps",  "Spread control, steps" );  
   DeclareOptionRef( fSC_rate=5,      "SC_rate",   "Spread control, rate: factor is changed depending on the rate" );  
   DeclareOptionRef( fSC_factor=0.95, "SC_factor", "Spread control, factor" );  
   DeclareOptionRef( fConvCrit=0.001, "ConvCrit",   "Convergence criteria" );  

   DeclareOptionRef( fSaveBestFromGeneration=1, "SaveBestGen", 
                     "Saves the best n results from each generation. They are included in the last cycle" );  
   DeclareOptionRef( fSaveBestFromCycle=10,     "SaveBestCycle", 
                     "Saves the best n results from each cycle. They are included in the last cycle" );  
   
   DeclareOptionRef( fTrim=kFALSE, "Trim", 
                     "Trim the population to PopSize after assessing the fitness of each individual" );  
   DeclareOptionRef( fSeed=100, "Seed", "Set seed of random generator (0 gives random seeds)" );  
}

//_______________________________________________________________________
void TMVA::GeneticFitter::SetParameters(  Int_t cycles,
                                          Int_t nsteps,
                                          Int_t popSize,
                                          Int_t SC_steps,
                                          Int_t SC_rate,
                                          Double_t SC_factor,
                                          Double_t convCrit)
{
   // set GA configuration parameters
   fNsteps    = nsteps;
   fCycles    = cycles;
   fPopSize   = popSize;
   fSC_steps  = SC_steps;
   fSC_rate   = SC_rate;
   fSC_factor = SC_factor;
   fConvCrit  = convCrit;
}

//_______________________________________________________________________
Double_t TMVA::GeneticFitter::Run( std::vector<Double_t>& pars )
{
   // Execute fitting
   fLogger << kINFO << "<GeneticFitter> Optimisation, please be patient "
           << "... (note: inaccurate progress timing for GA)" << Endl;

   GeneticAlgorithm gstore( GetFitterTarget(), fPopSize, fRanges ); 

   // timing of GA
   Timer timer( 100*(fCycles), GetName() ); 
   timer.DrawProgressBar( 0 );

   Double_t steps = (Double_t)fNsteps;
   Double_t progress = 0.;

   for (Int_t cycle = 0; cycle < fCycles; cycle++) {
      // ---- perform series of fits to achieve best convergence
         
      // "m_ga_spread" times the number of variables
      GeneticAlgorithm ga( GetFitterTarget(), fPopSize, fRanges, fSeed ); 

      if ( pars.size() == fRanges.size() ){
          ga.GetGeneticPopulation().GiveHint( pars, 0.0 );
      }
      if (cycle==fCycles-1) {
         ga.GetGeneticPopulation().AddPopulation( gstore.GetGeneticPopulation() );
      }
      
      ga.CalculateFitness();
      ga.GetGeneticPopulation().TrimPopulation();

      Double_t n=0.;
      do {
         ga.Init();
         ga.CalculateFitness();
         if ( fTrim ) ga.GetGeneticPopulation().TrimPopulation();
         ga.SpreadControl( fSC_steps, fSC_rate, fSC_factor );

         // monitor progrss
         if (ga.fConvCounter > n) n = Double_t(ga.fConvCounter);
         progress = 100*((Double_t)cycle) + 100*(n/Double_t(steps)); 

         timer.DrawProgressBar( (Int_t)progress );
            
         for ( Int_t i = 0; i<fSaveBestFromGeneration && i<fPopSize; i++ ) {
            gstore.GetGeneticPopulation().GiveHint( ga.GetGeneticPopulation().GetGenes(i)->GetFactors(), 0.0 );
         }
      } while (!ga.HasConverged( fNsteps, fConvCrit ));                

      timer.DrawProgressBar( 100*(cycle+1) );
      
      for ( Int_t i = 0; i<fSaveBestFromCycle && i<fPopSize; i++ ) {
         gstore.GetGeneticPopulation().GiveHint( ga.GetGeneticPopulation().GetGenes(i)->GetFactors(), 0.0 );
      }

   }

   // get elapsed time   
   fLogger << kINFO << "Elapsed time: " << timer.GetElapsedTime() 
           << "                            " << Endl;  

   Double_t fitness = gstore.CalculateFitness();
   pars.swap( gstore.GetGeneticPopulation().GetGenes(0)->GetFactors() );

   return fitness;
}
