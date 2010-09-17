// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: VOITerm.cpp 640 2009-06-13 05:06:50Z dglane001 $
#include "stdafx.h"
#include <VOITerm.h>

#include <UtilMacros.h>
#include <Structure.h>

namespace dH
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
VOITerm::VOITerm(Structure *pStructure, REAL weight)
	: m_pVOI(NULL)
		, m_pHistogram(NULL)
{
	SetVOI(pStructure);
	SetHistogram(new CHistogramWithGradient());

	SetWeight(weight);

}	// VOITerm::VOITerm


///////////////////////////////////////////////////////////////////////////////
VOITerm::~VOITerm()
{
}	// VOITerm::~VOITerm

///////////////////////////////////////////////////////////////////////////////
void
	VOITerm::UpdateFrom(const VOITerm * otherTerm)
{
	SetWeight(otherTerm->GetWeight());
}

}	// namespace dH
