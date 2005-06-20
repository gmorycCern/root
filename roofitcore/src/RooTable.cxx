/*****************************************************************************
 * Project: RooFit                                                           *
 * Package: RooFitCore                                                       *
 *    File: $Id: RooTable.cc,v 1.12 2005/06/16 09:31:32 wverkerke Exp $
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

// -- CLASS DESCRIPTION [PLOT] --
// RooTable is the abstract interface for table objects.
// Table objects are the category equivalent of RooPlot objects
// (which are used for real-valued objects)

#include "RooFitCore/RooFit.hh"

#include "RooFitCore/RooTable.hh"
#include "RooFitCore/RooTable.hh"

ClassImp(RooTable)


RooTable::RooTable(const char *name, const char *title) : TNamed(name,title)
{
}


RooTable::RooTable(const RooTable& other) : TNamed(other), RooPrintable(other)
{
}


RooTable::~RooTable()
{
}


void RooTable::printToStream(ostream& os, PrintOption /*opt*/, TString indent) const
{
  os << indent << "RooTable" << endl ;
}
