// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: BrimstoneDoc.cpp 650 2009-11-05 22:24:55Z dglane001 $
#include "stdafx.h"

#include "Brimstone.h"

#include "BrimstoneDoc.h"

#ifdef USE_RTOPT
#include <Prescription.h>
#endif

#include <BeamDoseCalc.h>

#include "SeriesDicomImporter.h"
#include "PlanSetupDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBrimstoneDoc

IMPLEMENT_DYNCREATE(CBrimstoneDoc, CDocument)

BEGIN_MESSAGE_MAP(CBrimstoneDoc, CDocument)
	//{{AFX_MSG_MAP(CBrimstoneDoc)
	ON_COMMAND(ID_GENBEAMLETS, OnGenbeamlets)
	ON_COMMAND(ID_FILE_IMPORT_DCM, OnFileImportDcm)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBrimstoneDoc construction/destruction

/////////////////////////////////////////////////////////////////////////////
CBrimstoneDoc::CBrimstoneDoc()
{
	// ensure delete of document on close
	m_bAutoDelete = TRUE;
}

/////////////////////////////////////////////////////////////////////////////
CBrimstoneDoc::~CBrimstoneDoc()
{
}

/////////////////////////////////////////////////////////////////////////////
void CBrimstoneDoc::DeleteContents() 
{
	// create series * plan
	m_pSeries = dH::Series::New();
	m_pPlan = dH::Plan::New();
	m_pPlan->SetSeries(m_pSeries);

#ifdef USE_RTOPT
	m_pOptimizer.reset(new dH::PlanOptimizer(m_pPlan));
#endif

	CDocument::DeleteContents();
}

/////////////////////////////////////////////////////////////////////////////
// CBrimstoneDoc serialization

/////////////////////////////////////////////////////////////////////////////
void CBrimstoneDoc::Serialize(CArchive& ar)
{
	CObArray arrWorkspace;
	if (ar.IsStoring())
	{
#ifdef USE_MFC_SERIALIZATION
		arrWorkspace.Add(m_pSeries.get());
		arrWorkspace.Add(m_pPlan.get());
#endif
	}

	arrWorkspace.Serialize(ar);

	if (ar.IsLoading())
	{
#ifdef USE_MFC_SERIALIZATION
		m_pSeries.reset();
		m_pPlan.reset();
		for (int nAt = 0; nAt < arrWorkspace.GetSize(); nAt++)
		{
			if (arrWorkspace[nAt]->IsKindOf(RUNTIME_CLASS(CSeries)))
			{
				m_pSeries.reset((CSeries *) arrWorkspace[nAt]);
			}
			else if (arrWorkspace[nAt]->IsKindOf(RUNTIME_CLASS(CPlan)))
			{
				m_pPlan.reset((CPlan *) arrWorkspace[nAt]);
			}
		}

		// link plan to series
		m_pPlan->SetSeries(m_pSeries.get());

		// test if the beamlets were saved from the single-plane version
		VolumeReal::Pointer pTestBeamlet = m_pPlan->GetBeamAt(0)->GetBeamlet(0);
		if (pTestBeamlet->GetBufferedRegion().GetSize()[2] == 1)
		{
			// fix beam isocenters
			VolumeReal *pDose = m_pPlan->GetDoseMatrix();
			for (int nAt = 0; nAt < m_pPlan->GetBeamCount(); nAt++)
			{
				CBeam *pBeam = m_pPlan->GetBeamAt(nAt);

				// this is just used to set the z-coordinate of the iso
				itk::Vector<REAL> vIso;
				vIso[0] = pDose->GetOrigin()[0];
				vIso[1] = pDose->GetOrigin()[1];
				vIso[2] = pDose->GetOrigin()[2];
				pBeam->SetIsocenter(vIso);

				// now replicate the beamlet slices
				int nBeamletCount = pBeam->GetBeamletCount() / 2;
				for (int nAtShift = -nBeamletCount; nAtShift <= nBeamletCount; nAtShift++)
				{
					VolumeReal::Pointer pBeamlet = pBeam->GetBeamlet(nAtShift);
					VolumeReal::Pointer pNewBeamlet = VolumeReal::New();
					ConformTo<VOXEL_REAL, 3>(pBeamlet, pNewBeamlet);

					Size<3> sz = pNewBeamlet->GetBufferedRegion().GetSize();
					sz[2] = pDose->GetBufferedRegion().GetSize()[2];
					pNewBeamlet->SetRegions(sz);
					pNewBeamlet->Allocate();

					VOXEL_REAL *pSrc = pBeamlet->GetBufferPointer();
					int nCount = sz[0] * sz[1];
					for (int nZ = 0; nZ < sz[2]; nZ++)
					{
						VOXEL_REAL *pDst = &pNewBeamlet->GetBufferPointer()[nZ * nCount];
						CopyValues<VOXEL_REAL>(pDst, pSrc, nCount);
					}

					// now copy the re-formatted beamlet to the original beamlet
					CopyImage<VOXEL_REAL, 3>(pNewBeamlet, pBeamlet);
				}
			}
		}

#ifdef USE_RTOPT
		// create an empty prescription, if none available
		m_pOptimizer.reset(new dH::PlanOptimizer(m_pPlan.get()));
#endif
#endif
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBrimstoneDoc diagnostics

#ifdef _DEBUG
void CBrimstoneDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CBrimstoneDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
BOOL CBrimstoneDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
void CBrimstoneDoc::OnGenbeamlets() 
{
	// DON'T delete existing plan, as this will require resetting the PlanarView
	/// m_pPlan.reset(new CPlan());

	// set up a new optimizer
#ifdef USE_RTOPT
	m_pOptimizer.reset(new dH::PlanOptimizer(m_pPlan));
#endif

	CPlanSetupDlg dlgSetup(m_pOptimizer->GetPyramid(), AfxGetMainWnd());
	if (dlgSetup.DoModal() == IDOK)
	{

	}
	VolumeReal *pVR = m_pPlan->GetBeamAt(0)->GetBeamlet(0);
	VOXEL_REAL max;
	VoxelMax<VOXEL_REAL>(&max, pVR->GetBufferPointer(),
		pVR->GetBufferedRegion().GetSize()[0],
		pVR->GetBufferedRegion().GetSize());

	CVectorN<> vWeights;
	vWeights.SetDim(1); // 39);
	vWeights[0 /*19*/] = 0.99; // 70.0;
	for (int nAtBeam = 0; nAtBeam < m_pPlan->GetBeamCount(); nAtBeam++)
		m_pPlan->GetBeamAt(nAtBeam)->SetIntensityMap(vWeights);
	m_pPlan->GetDoseMatrix();

	// this is used to prime CBrimstoneView's optimizer thread
	SendInitialUpdate();
}

/////////////////////////////////////////////////////////////////////////////
void CBrimstoneDoc::OnFileImportDcm() 
{
	// remove existing data
	if (!SaveModified())
		return;

	CFileDialog dlg(TRUE, NULL, NULL, OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR);
	CString strFilenames;
	dlg.m_ofn.lpstrFile = strFilenames.GetBuffer(16384);
	dlg.m_ofn.nMaxFile = 16384;
	if (dlg.DoModal() == IDOK)
	{
		// shut it down
		DeleteContents();

		// update views for new document objects
		//SendInitialUpdate();

		// construct the importer
		CSeriesDicomImporter dcmImp(m_pSeries, &dlg);

		// process files
		int nCount = 0;
		do
		{
			nCount = dcmImp.ProcessNext();
			TRACE("Processing %i\n", nCount);
		} while (nCount > 0);

#define PATCH_HOLE
#ifdef PATCH_HOLE
		// now patch the hole
		VolumeReal::IndexType idx;
		for (idx[2] = 0; idx[2] < m_pSeries->GetDensity()->GetBufferedRegion().GetSize()[2]; idx[2]++)
		{
			for (idx[1] = 97; idx[1] < 107; idx[1]++)
			{
				for (idx[0] = 109; idx[0] < 126; idx[0]++)
				{
					VOXEL_REAL value = 25.0 - 50.0 * (VOXEL_REAL) rand() / (VOXEL_REAL) RAND_MAX;
					m_pSeries->GetDensity()->SetPixel(idx, value);
					// (*m_pSeries->m_pDens)[nZ][nY][nX] = 25.0 - 50.0 * (VOXEL_REAL) rand() / (VOXEL_REAL) RAND_MAX;
				}
			}
		}
#endif

		dH::Structure *pStruct = this->m_pSeries->GetStructureAt(0);
		CString strName(pStruct->GetName().c_str());
		//pStruct->CalcRegion();

		// TODO: update the viewer with the correct plane isocenter position

		// set path name
		// SetPathName("");

		SetModifiedFlag(TRUE);

		// update views for new document objects
		SendInitialUpdate();
	}
}
