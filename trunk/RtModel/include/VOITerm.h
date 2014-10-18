// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: VOITerm.h 640 2009-06-13 05:06:50Z dglane001 $
#include <HistogramGradient.h>

#pragma once

namespace dH
{

class Structure;
class Prescription;

///////////////////////////////////////////////////////////////////////////////
// class VOITerm
// 
// base class for objective function terms
///////////////////////////////////////////////////////////////////////////////
class VOITerm : public itk::DataObject
{
public:
	// constructor / destructor
	VOITerm(Structure *pStructure = NULL, REAL weight = 1.0);
	virtual ~VOITerm();

	// copy update
	virtual void UpdateFrom(const VOITerm *otherTerm);

	// accessors for structure and histogram
	DECLARE_ATTRIBUTE_PTR(VOI, Structure);
	DECLARE_ATTRIBUTE_PTR(Histogram, CHistogramWithGradient);

	// weight accessors
	DECLARE_ATTRIBUTE(Weight, REAL);

	// over-ride for real terms
	/// TODO: change CArray to std::vector
	virtual REAL Eval(CVectorN<> *pvGrad, const CArray<BOOL, BOOL>& arrInclude) = 0;

	// helper to create pyramid - constructs a copy, except with nScale + 1
	virtual VOITerm *Clone() = 0;

//protected:
//	friend class dH::Prescription;
};	// class VOITerm

}	// namespace dH



