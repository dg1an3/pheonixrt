// Copyright (C) 2nd Messenger Systems
// $Id: Series.cpp 640 2009-06-13 05:06:50Z dglane001 $
#include "stdafx.h"

#include <Series.h>

BeginNamespace(dH)

///////////////////////////////////////////////////////////////////////////////
Series::Series()
{
	SetDensity(VolumeReal::New());
}

///////////////////////////////////////////////////////////////////////////////
Series::~Series()
{
}	

///////////////////////////////////////////////////////////////////////////////
int 
	Series::GetStructureCount() const
{
	return m_arrStructures.size();
}

///////////////////////////////////////////////////////////////////////////////
dH::Structure *
	Series::GetStructureAt(int nAt)
{
	return m_arrStructures.at(nAt);
}	

/////////////////////////////////////////////////////////////////////////////
dH::Structure * 
	Series::GetStructureFromName(const CString &strName)
{
	for (int nAt = 0; nAt < GetStructureCount(); nAt++)
	{
		if (GetStructureAt(nAt)->GetName() == strName)
		{
			return GetStructureAt(nAt);
		}
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
void 
	Series::AddStructure(dH::Structure *pStruct)
{
	pStruct->SetSeries(this);
	m_arrStructures.push_back(pStruct);

	UpdateStructurePipelines();
}

/////////////////////////////////////////////////////////////////////////////
void 
	Series::UpdateStructurePipelines()
{
	for (int nAt = 0; nAt < GetStructureCount(); nAt++)
	{
		GetStructureAt(nAt)->UpdatePipeline();
	}
}

///////////////////////////////////////////////////////////////////////////////
void 
	Series::SerializeExt(CArchive& ar, int nSchema)
{
	// schema serialization
	nSchema = 1;
	SerializeValue(ar, nSchema);

	// use temp CVolume for serialization
	SerializeVolume<VOXEL_REAL>(ar, GetDensity());

	if (ar.IsLoading())
	{
		// delete existing structures
		m_arrStructures.clear();

		int nStructureCount = 0;
		SerializeValue(ar, nStructureCount);
		for (int nStruct = 0; nStruct < nStructureCount; nStruct++)
		{
			dH::Structure::Pointer pStruct = dH::Structure::New();
			pStruct->SerializeExt(ar, -1);
			AddStructure(pStruct);
		}
	}
	else if (ar.IsStoring())
	{
		int nStructureCount = GetStructureCount();
		SerializeValue(ar, nStructureCount);
		for (int nStruct = 0; nStruct < nStructureCount; nStruct++)
		{
			GetStructureAt(nStruct)->SerializeExt(ar, -1);
		}
	}
}

EndNamespace(dH)
