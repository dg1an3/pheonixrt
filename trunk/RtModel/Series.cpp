// Copyright (C) 2nd Messenger Systems
// $Id: Series.cpp 640 2009-06-13 05:06:50Z dglane001 $
#include "stdafx.h"

#include <UtilMacros.h>

#include <Series.h>

namespace dH 
{

///////////////////////////////////////////////////////////////////////////////
Series::Series()
{
	SetDensity(VolumeReal::New());
}


///////////////////////////////////////////////////////////////////////////////
VolumeReal *
	Series::GetDensity()
{ 
	return m_pDensity; 
}		

///////////////////////////////////////////////////////////////////////////////
void 
	Series::SetDensity(VolumeReal *pValue)
{ 
	m_pDensity = pValue; 
}

///////////////////////////////////////////////////////////////////////////////
int 
	Series::GetStructureCount() const
{
	return (int) m_arrStructures.size();
}

///////////////////////////////////////////////////////////////////////////////
Structure *
	Series::GetStructureAt(int nAt)
{
	return (Structure *) m_arrStructures.at(nAt);
}

/////////////////////////////////////////////////////////////////////////////
Structure * 
	Series::GetStructureFromName(const std::string &strName)
{
	for (int nAt = 0; nAt < GetStructureCount(); nAt++)
	{
		if (GetStructureAt(nAt)->GetName().compare(strName) == 0)
		{
			return GetStructureAt(nAt);
		}
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
void 
	Series::AddStructure(Structure *pStruct)
{
	pStruct->SetSeries(this);
	m_arrStructures.push_back(pStruct);
}

}	// namespace dH