// @(#)root/tmva $Id: TNeuronInputSqSum.h,v 1.7 2007/02/02 19:16:05 brun Exp $
// Author: Matt Jachowski 

/**********************************************************************************
 * Project: TMVA - a Root-integrated toolkit for multivariate data analysis       *
 * Package: TMVA                                                                  *
 * Class  : TMVA::TNeuronInputSqSum                                               *
 * Web    : http://tmva.sourceforge.net                                           *
 *                                                                                *
 * Description:                                                                   *
 *       TNeuron input calculator -- calculates the square                        *
 *       of the weighted sum of inputs.                                           *
 *                                                                                *
 * Authors (alphabetical):                                                        *
 *      Matt Jachowski  <jachowski@stanford.edu> - Stanford University, USA       *
 *                                                                                *
 * Copyright (c) 2005:                                                            *
 *      CERN, Switzerland                                                         *
 *                                                                                *
 * Redistribution and use in source and binary forms, with or without             *
 * modification, are permitted according to the terms listed in LICENSE           *
 * (http://tmva.sourceforge.net/LICENSE)                                          *
 **********************************************************************************/
 

#ifndef ROOT_TMVA_TNeuronInputSqSum
#define ROOT_TMVA_TNeuronInputSqSum

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TNeuronInputSqSum                                                    //
//                                                                      //
// TNeuron input calculator -- calculates the squared weighted sum of   //
// inputs                                                               //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TObject.h"
#include "TString.h"

#ifndef ROOT_TMVA_TNeuronInput
#include "TMVA/TNeuronInput.h"
#endif
#ifndef ROOT_TMVA_TNeuron
#include "TMVA/TNeuron.h"
#endif

namespace TMVA {
  
   class TNeuronInputSqSum : public TNeuronInput {
    
   public:

      TNeuronInputSqSum() {}
      virtual ~TNeuronInputSqSum() {}

      // calculate the input value for the neuron
      Double_t GetInput(TNeuron* neuron) {
         if (neuron->IsInputNeuron()) return 0;
         Double_t result = 0;
         for (Int_t i=0; i < neuron->NumPreLinks(); i++) {
            Double_t val = neuron->PreLinkAt(i)->GetWeightedValue();
            result += val*val;
         }
         return result;
      }

      // name of the class
      TString GetName() { return "Sum of weighted activations squared"; }

      ClassDef(TNeuronInputSqSum,0) // Calculates square of  weighted sum of neuron inputs
   };

} // namespace TMVA

#endif
