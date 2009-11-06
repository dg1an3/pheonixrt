// Copyright (C) 2nd Messenger Systems
// $Id: Plan.cpp 640 2009-06-13 05:06:50Z dglane001 $
#include "stdafx.h"

#include "Plan.h"

#include <EnergyDepKernel.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


///////////////////////////////////////////////////////////////////////////////
CPlan::CPlan()
	: m_pSeries(NULL)
	, m_DoseResolution(4.0) // 
		// 2.0)
{
	m_pKernel = new CEnergyDepKernel(6.0); // 
		// 15.0);

	m_pMassDensity = VolumeReal::New();

	m_pDose = VolumeReal::New();
	m_pBeamDoseRot = VolumeReal::New();
	m_pTempBuffer = VolumeReal::New();

}	// CPlan::CPlan

///////////////////////////////////////////////////////////////////////////////
CPlan::~CPlan()
{
	// delete the beams
	for (int nAt = 0; nAt < m_arrBeams.GetSize(); nAt++)
	{
		delete m_arrBeams[nAt];
	}

	// delete the histograms
	POSITION pos = m_mapHistograms.GetStartPosition();
	while (NULL != pos)
	{
		CString strName;
		CHistogram *pHistogram = NULL;
		m_mapHistograms.GetNextAssoc(pos, strName, pHistogram); 

		delete pHistogram;
	}

	// delete the kernel
	delete m_pKernel;

}	// CPlan::~CPlan

///////////////////////////////////////////////////////////////////////////////
void 
	CPlan::SetSeries(CSeries *pSeries)
{
	// store the series pointer
	m_pSeries = pSeries;

	// trigger calc of dose matrix basis
	SetDoseResolution(GetDoseResolution());

}	// CPlan::SetSeries

///////////////////////////////////////////////////////////////////////////////
int 
	CPlan::GetBeamCount() const
{
	return (int) m_arrBeams.GetSize();

}	// CPlan::GetBeamCount

///////////////////////////////////////////////////////////////////////////////
int 
	CPlan::GetTotalBeamletCount()
{
	int nBeamlets = 0;

	for (int nAtBeam = 0; nAtBeam < GetBeamCount(); nAtBeam++)
	{
		nBeamlets += GetBeamAt(nAtBeam)->GetBeamletCount();
	}

	return nBeamlets;

}	// CPlan::GetTotalBeamletCount

///////////////////////////////////////////////////////////////////////////////
CBeam * 
	CPlan::GetBeamAt(int nAt)
{
	return m_arrBeams.GetAt(nAt);

}	// CPlan::GetBeamAt

///////////////////////////////////////////////////////////////////////////////
int 
	CPlan::AddBeam(CBeam *pBeam)
{
	int nIndex = (int) m_arrBeams.Add(pBeam);
	pBeam->SetPlan(this);

	// a change has occurred, so fire
	GetChangeEvent().Fire();

	return nIndex;

}	// CPlan::AddBeam

///////////////////////////////////////////////////////////////////////////////
VolumeReal * 
	CPlan::GetDoseMatrix()
{
	// total the dose for all beams
	if (GetBeamCount() > 0)
	{
		// clear the dose matrix
		m_pDose->FillBuffer(0.0);

		for (int nAt = 0; nAt < GetBeamCount(); nAt++)
		{
			ConformTo<VOXEL_REAL,3>(m_pDose, m_pBeamDoseRot);
			m_pBeamDoseRot->FillBuffer(0.0); 

			VolumeReal *pBeamDose = GetBeamAt(nAt)->GetDoseMatrix();
			// Resample(pBeamDose, m_pBeamDoseRot, TRUE);
			Resample3D(pBeamDose, m_pBeamDoseRot, TRUE);

			// add this beam's dose matrix to the total
			ConformTo<VOXEL_REAL,3>(m_pDose, m_pTempBuffer);
			// Accumulate<VOXEL_REAL>(m_pBeamDoseRot, 
			//	/* beam weight = */ 1.0, m_pDose, m_pTempBuffer);
			Accumulate3D<VOXEL_REAL>(m_pBeamDoseRot, 
				/* beam weight = */ 1.0, m_pDose, m_pTempBuffer);
		}
	}

	return m_pDose;

}	// CPlan::GetDoseMatrix

///////////////////////////////////////////////////////////////////////////////
void 
	CPlan::UpdateAllHisto()
{
#ifdef USE_RTOPT
	// first recalc dose matrix
	GetDoseMatrix();

	// now iterate over histo's
	POSITION pos = m_mapHistograms.GetStartPosition();
	CString strName;
	CHistogram *pHisto;
	while (pos != NULL)
	{
		m_mapHistograms.GetNextAssoc(pos, strName, pHisto);
		pHisto->OnVolumeChange(NULL, NULL);
	}
#endif
}


///////////////////////////////////////////////////////////////////////////////
void
	CPlan::SetDoseResolution(const REAL& res)
	// sets shape for dose matrix
{
	m_DoseResolution = res;

	// accessor to planning volume
	const VolumeReal *pVolume = GetSeries()->GetDensity();
	const itk::Vector<REAL> vVolSpacing = pVolume->GetSpacing();

	// compute height / width / depth
	int nHeight = 
		Round<int>(pVolume->GetBufferedRegion().GetSize()[1] 
			* vVolSpacing[1] / m_DoseResolution);
	int nWidth = 
		Round<int>(pVolume->GetBufferedRegion().GetSize()[0] 
			* vVolSpacing[0] / m_DoseResolution);
	int nDepth = 
		Round<int>(pVolume->GetBufferedRegion().GetSize()[2] 
			* vVolSpacing[2] / m_DoseResolution);

	// set dimensions
	m_pDose->SetRegions(MakeSize(nWidth, nHeight, nDepth));
	m_pDose->Allocate();

	m_pDose->SetOrigin(pVolume->GetOrigin());
	m_pDose->SetDirection(pVolume->GetDirection());
	m_pDose->SetSpacing(
		MakeVector<3>(m_DoseResolution, m_DoseResolution, m_DoseResolution));

}	// CPlan::SetDoseResolution


///////////////////////////////////////////////////////////////////////////////
CHistogram *
	CPlan::GetHistogram(dH::Structure *pStructure, bool bCreate)
{
	CHistogram *pHisto = NULL;
#ifdef USE_RTOPT
	if (!m_mapHistograms.Lookup(pStructure->GetName(), pHisto))
	{
		if (bCreate)
		{
			pHisto = new CHistogram();

			pHisto->SetBinning((REAL) 0.0, (REAL) 0.01 /* 0.02 */, GBINS_BUFFER);
			pHisto->SetVolume(GetDoseMatrix());

			// resample region, if needed
			VolumeReal *pResampRegion = pStructure->GetConformRegion(GetDoseMatrix());
			pHisto->SetRegion(pResampRegion);

			// calculate slice number for the isocenter
			REAL sliceZ = GetBeamAt(0)->GetIsocenter()[2];
			int nSlice = Round<int>((sliceZ - pResampRegion->GetOrigin()[2]) / pResampRegion->GetSpacing()[2]);
			pHisto->SetSlice(nSlice);

			// add to map
			m_mapHistograms[pStructure->GetName()] = pHisto;
		}
	}

	if (pHisto != NULL)
	{
		// resample region, if needed (this is to always create the exclusion region
		VolumeReal *pResampRegion = pStructure->GetConformRegion(GetDoseMatrix());
		pHisto->SetRegion(pResampRegion);
	}
#endif

	return pHisto;

}	// CPlan::GetHistogram

///////////////////////////////////////////////////////////////////////////////
void 
	CPlan::RemoveHistogram(dH::Structure *pStructure)
{
#ifdef USE_RTOPT
	CHistogram *pHisto = NULL;
	if (m_mapHistograms.Lookup(pStructure->GetName(), pHisto))
	{
		m_mapHistograms.RemoveKey(pStructure->GetName());
		delete pHisto;
	}
#endif
}	// CPlan::RemoveHistogram



///////////////////////////////////////////////////////////////////////////////
VolumeReal * 
	CPlan::GetMassDensity()
	// used to format the mass density array, conformant to dose matrix
{
	// fix mass density
	ConformTo<VOXEL_REAL,3>(GetSeries()->GetDensity(), m_pMassDensity);
	m_pMassDensity->FillBuffer(0.0); 

	// lookup values
	VOXEL_REAL *pCTVoxels = GetSeries()->GetDensity()->GetBufferPointer(); 
	VOXEL_REAL *pMDVoxels = m_pMassDensity->GetBufferPointer(); 
	int nVoxels = m_pMassDensity->GetBufferedRegion().GetNumberOfPixels();
	for (int nAtVoxel = 0; nAtVoxel < nVoxels; nAtVoxel++)
	{
		if (pCTVoxels[nAtVoxel] < 0.0)
		{
			pMDVoxels[nAtVoxel] =
				(VOXEL_REAL)(0.0 + 1.0 * (pCTVoxels[nAtVoxel] - -1024.0) / 1024.0);
		}
		else if (pCTVoxels[nAtVoxel] < 1024.0)
		{
			pMDVoxels[nAtVoxel] = 
				(VOXEL_REAL)(1.0 + 0.0/*0.5*/ * pCTVoxels[nAtVoxel] / 1024.0);
		}
		else
		{
			pMDVoxels[nAtVoxel] = 1.0/*1.5*/;
		}
	}

	return m_pMassDensity;

}	// CPlan::GetMassDensity

/////////////////////////////////////////////////////////////////////////////
// CPlan serialization

#define PLAN_SCHEMA 5
	// Schema 1: initial plan schema
	// Schema 2: + target DVH curves
	// Schema 3: + number of fields
	// Schema 4: + target DVH count
	// Schema 5: + DVHs

IMPLEMENT_SERIAL(CPlan, CModelObject, VERSIONABLE_SCHEMA | PLAN_SCHEMA)

///////////////////////////////////////////////////////////////////////////////
// CPlan::Serialize
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CPlan::Serialize(CArchive& ar)
{
	// schema for the plan object
	UINT nSchema = ar.IsLoading() ? ar.GetObjectSchema() : PLAN_SCHEMA;

	CModelObject::Serialize(ar);

	if (ar.IsLoading())
	{
		// now clear out any existing beams
		m_arrBeams.RemoveAll();
	}

	m_arrBeams.Serialize(ar);

	if (ar.IsLoading())
	{
		// set up as change listener for beams
		for (int nAt = 0; nAt < GetBeamCount(); nAt++)
		{
			GetBeamAt(nAt)->SetPlan(this);
		}
	}

	// DEPRACATED flag to serialize dose matrix
	BOOL bDoseValid = FALSE; 
	SERIALIZE_VALUE(ar, bDoseValid);

	// serialize the dose matrix
	SerializeVolume<VOXEL_REAL>(ar, m_pDose);

	// for the schema 2, serialize target DVHs
	if (nSchema >= 2)
	{
		if (ar.IsLoading())
		{
			int nCount = 0;
			if (nSchema >= 4)
			{
				ar >> nCount;
			}

			for (int nAt = 0; nAt < nCount; nAt++)
			{
				CString strStructureName;
				ar >> strStructureName;

				CMatrixNxM<> mTargetDVH;
				ar >> mTargetDVH;
			}
		}
		else 
		{
			if (nSchema >= 4)
			{
				// DEPRECATED
				// ar << m_mapTargetDVHs.GetCount();
				ar << 0;
			}
		}
	}

	if (nSchema >= 3)
	{
		int m_nFields = 0;
		SERIALIZE_VALUE(ar, m_nFields);
	}

	if (nSchema >= 5)
	{
		SERIALIZE_VALUE(ar, m_pSeries);
		ASSERT(m_pSeries != NULL);

		m_mapHistograms.Serialize(ar);

		if (ar.IsLoading())
		{
#ifdef USE_RTOPT
			// set up regions and volumes for histograms
			//		because serialization doesn't restore this
			POSITION pos = m_mapHistograms.GetStartPosition();
			while (NULL != pos)
			{
				CString strName;
				CHistogram *pHistogram = NULL;
				m_mapHistograms.GetNextAssoc(pos, strName, pHistogram); 

				// set volume
				pHistogram->SetVolume(GetDoseMatrix());

				// set region
				dH::Structure *pStruct = GetSeries()->GetStructureFromName(strName);
				VolumeReal *pConformRegion = pStruct->GetConformRegion(GetDoseMatrix());
				pHistogram->SetRegion(pConformRegion);
			}
#endif
		}
	}

}	// CPlan::Serialize



/////////////////////////////////////////////////////////////////////////////
// CPlan diagnostics

#ifdef _DEBUG
void CPlan::AssertValid() const
{
	CModelObject::AssertValid();
}

void CPlan::Dump(CDumpContext& dc) const
{
	CModelObject::Dump(dc);

	int nOrigDepth = dc.GetDepth();
	dc.SetDepth(nOrigDepth + 1);

	dc.SetDepth(nOrigDepth);
}
#endif //_DEBUG
