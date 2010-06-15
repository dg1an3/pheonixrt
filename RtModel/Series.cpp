// Copyright (C) 2nd Messenger Systems
// $Id: Series.cpp 640 2009-06-13 05:06:50Z dglane001 $
#include "stdafx.h"

#include <UtilMacros.h>

#include <Series.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSeries

///////////////////////////////////////////////////////////////////////////////
CSeries::CSeries()
{
	SetDensity(VolumeReal::New());

}	// CSeries::CSeries

///////////////////////////////////////////////////////////////////////////////
CSeries::~CSeries()
{
}	// CSeries::~CSeries

///////////////////////////////////////////////////////////////////////////////
int CSeries::GetStructureCount() const
{
	return (int) m_arrStructures.size/*GetSize*/();

}	// CSeries::GetStructureCount

///////////////////////////////////////////////////////////////////////////////
dH::Structure *CSeries::GetStructureAt(int nAt)
{
	return (dH::Structure *) m_arrStructures.at(nAt);

}	// CSeries::GetStructureAt

/////////////////////////////////////////////////////////////////////////////
dH::Structure * 
	CSeries::GetStructureFromName(const CString &strName)
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
	CSeries::AddStructure(dH::Structure *pStruct)
{
	pStruct->SetSeries(this);
	m_arrStructures.push_back(pStruct);
}


/////////////////////////////////////////////////////////////////////////////
// CSeries diagnostics

#ifdef _DEBUG
void CSeries::AssertValid() const
{
	CModelObject::AssertValid();

	// m_arrStructures.AssertValid();
}

void CSeries::Dump(CDumpContext& dc) const
{
	CModelObject::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSeries serialization

#ifdef USE_MFC_SERIALIZATION
IMPLEMENT_SERIAL(CSeries, CModelObject, 1)

///////////////////////////////////////////////////////////////////////////////
void CSeries::Serialize(CArchive& ar)
{
	CModelObject::Serialize(ar);

	// use temp CVolume for serialization
	SerializeVolume<VOXEL_REAL>(ar, GetDensity());

	//m_arrStructures.Serialize(ar);
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

}	// CSeries::Serialize
#endif

/////////////////////////////////////////////////////////////////////////////
// CSeries commands
