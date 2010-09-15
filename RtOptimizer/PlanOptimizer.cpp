// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: PlanOptimizer.cpp 647 2009-11-05 21:52:59Z dglane001 $
#include "StdAfx.h"
#include "PlanOptimizer.h"

#include <ConjGradOptimizer.h>


namespace dH
{

///////////////////////////////////////////////////////////////////////////////

// constants for access to the registry
const CString REG_KEY			= _T("Prescription");

///////////////////////////////////////////////////////////////////////////////
/// TODO: move this to utils
REAL 
	GetProfileRealAt(const CString& strKey, int nAt, REAL defaultValue)
{
	USES_CONVERSION;
	CString strKeyAt;
	strKeyAt.Format(strKey, nAt);
	return GetProfileReal(W2A(REG_KEY), W2A(strKeyAt), defaultValue);
}

///////////////////////////////////////////////////////////////////////////////

// other constants + registry keys

const CString GBINSIGMA_KEY		= _T("GBinSigma");
const REAL DEFAULT_GBINSIGMA	= 0.2;

const CString LEVELSIGMA_KEY	= _T("LevelSigma%i");
const REAL DEFAULT_LEVELSIGMA[] = {8.0, 3.2, 1.3, 0.5, 0.25}; // { 8.0, 4.0, 2.0, 1.0, 0.5 };

const CString CGTOL_KEY			= _T("CGTolerance%i");
const CString LINETOL_KEY		= _T("Tolerance%i");
const REAL DEFAULT_TOLERANCE	= 1e-3;



///////////////////////////////////////////////////////////////////////////////
PlanOptimizer::PlanOptimizer(CPlan *pPlan)
	: m_pPlan(pPlan)
{
	SetupPrescription();
}

///////////////////////////////////////////////////////////////////////////////
PlanOptimizer::~PlanOptimizer(void)
{
	for (int nAt = 0; nAt < m_arrPrescriptions.size(); nAt++)
	{
		delete m_arrPrescriptions[nAt].first;
		delete m_arrPrescriptions[nAt].second;
	}

	delete m_pPyramid;

	/// TODO: delete the plan???
}

///////////////////////////////////////////////////////////////////////////////
Prescription *
	PlanOptimizer::GetPrescription(int nLevel)
	// returns the given level of the pyramid
{
	return m_arrPrescriptions[nLevel].first;

}	// PlanOptimizer::GetPrescription

///////////////////////////////////////////////////////////////////////////////
DynamicCovarianceOptimizer *
	PlanOptimizer::GetOptimizer(int nLevel)
	// returns the given level of the pyramid
{
	return m_arrPrescriptions[nLevel].second;

}	// PlanOptimizer::GetOptimizer

///////////////////////////////////////////////////////////////////////////////
void 
	PlanOptimizer::AddStructureTerm(VOITerm *pST)
{
	// these need to be generated first, before the call to AddStructureTerm
	// GetPyramid()->CalcPencilSubBeamlets();

	for (int nLevel = 0; nLevel < PlanPyramid::MAX_SCALES; nLevel++)
	{
		GetPrescription(nLevel)->AddStructureTerm(pST);
		pST = pST->Clone();
	}

}	// PlanOptimizer::AddStructureTerm

///////////////////////////////////////////////////////////////////////////////
bool 
	PlanOptimizer::Optimize(CVectorN<>& vInit, OptimizerCallback *pFunc, void *pParam)
	// performs multi-level optimization
{
	// make sure pencil subbeamlets are properly generated
	GetPyramid()->CalcPencilSubBeamlets();

	// make sure prescription terms are synched
	for (int nLevel = 1; nLevel < m_arrPrescriptions.size(); nLevel++)
		GetPrescription(nLevel)->UpdateTerms(GetPrescription(0));

	// compute the starting point
	GetInitStateVector(vInit);

	for (int nLevel = m_arrPrescriptions.size()-1; nLevel >= 0; nLevel--)
	{
		dH::Prescription *pPresc = GetPrescription(nLevel);
		DynamicCovarianceOptimizer *pOpt = GetOptimizer(nLevel);

		// update the histogram regions
		pPresc->UpdateHistogramRegions();

		// set the callback
		pOpt->SetCallback(pFunc, pParam);

		// inverse transform the initial vector to form initial optimizer parameter
		pPresc->InvTransform(&vInit);

		// NOTE: this needs to be in the form of an initializer, 
		//	or else SetDim needs to be called for vRes before the call
		// CVectorN<> vRes = pOpt->Optimize(vInit);
		pOpt->minimize(vInit.GetVnlVector());
		CVectorN<> vRes = vInit;

		// check for problem with optimization
		if (pOpt->get_num_iterations() == -1)
		{
			return false;
		}

		// compute final state vector
		pPresc->Transform(&vRes);

		// if we are not at the last level,
		if (nLevel > 0)
		{
			// then inverse filter to the next level
			InvFilterStateVector(nLevel, vRes, vInit);
		}
		else
		{
			// set the final (for output)
			vInit = vRes;
		}
	}

	return true;

}	// PlanOptimizer::Optimize

///////////////////////////////////////////////////////////////////////////////
void 
	PlanOptimizer::GetInitStateVector(CVectorN<>&vInit)
{
	Prescription *pLevelMax = GetPrescription(PlanPyramid::MAX_SCALES-1);

	vInit.SetDim(GetPyramid()->GetPlan(PlanPyramid::MAX_SCALES-1)->GetTotalBeamletCount());
	for (int nAt = 0; nAt < vInit.GetDim(); nAt++)
	{
		if (pLevelMax->m_arrIncludeElement[nAt])
		{
			vInit[nAt] = R(0.001);
#ifdef ADD_RANDOM_INIT
			vInit[nAt] += R(0.01)*R(rand())/R(RAND_MAX);
#endif
		}
		else
		{
			vInit[nAt] = R(0.001) / R(GetPlan()->GetBeamCount());
			TRACE("Excluded element %n\n", nAt);
		}
	}

}	// PlanOptimizer::GetInitStateVector

///////////////////////////////////////////////////////////////////////////////
void 
	PlanOptimizer::GetStateVectorFromPlan(CVectorN<>& vState)
{
	//CMatrixNxM<> mBeamletWeights(GetPlan()->GetBeamCount(), 
	//	GetPlan()->GetBeamAt(0)->GetBeamletCount());
	for (int nAtBeam = 0; nAtBeam < GetPlan()->GetBeamCount(); nAtBeam++)
	{
		CBeam::IntensityMap *pIM = GetPlan()->GetBeamAt(nAtBeam)->GetIntensityMap();
		IntensityMapToStateVector(0, nAtBeam, pIM, vState);
		//for (int nAt = 0; nAt < pIM->GetBufferedRegion().GetSize()[0]; nAt++)
		//	mBeamletWeights[nAtBeam][nAt] = pIM->GetBufferPointer()[nAt];
	}
	//BeamletWeightsToStateVector(0, mBeamletWeights, vState);

}	// PlanOptimizer::GetStateVectorFromPlan

///////////////////////////////////////////////////////////////////////////////
void 
	PlanOptimizer::SetStateVectorToPlan(const CVectorN<>& vState)
{
	//CMatrixNxM<> mBeamletWeights;
	//StateVectorToBeamletWeights(0, vState, mBeamletWeights);

	for (int nAtBeam = 0; nAtBeam < GetPlan()->GetBeamCount(); nAtBeam++)
	{
		StateVectorToIntensityMap(0, nAtBeam, vState, 
			GetPlan()->GetBeamAt(nAtBeam)->GetIntensityMap());
		GetPlan()->GetBeamAt(nAtBeam)->OnIntensityMapChanged();
		//GetPlan()->GetBeamAt(nAtBeam)->SetIntensityMap(mBeamletWeights[nAtBeam]);
	}

	// now iterate through histograms, updating
	GetPlan()->UpdateAllHisto();

}	// PlanOptimizer::SetStateVectorToPlan

///////////////////////////////////////////////////////////////////////////////
void 
	PlanOptimizer::InvFilterStateVector(int nScale, const CVectorN<>& vIn, CVectorN<>& vOut)
	// transfers beamlet weights from level n-1 to level n
{
	ASSERT(&vIn != &vOut);
	// TODO: make a member so don't have to re-initialize
	//CMatrixNxM<> mBeamletWeights;
	//StateVectorToBeamletWeights(nScale, vIn, mBeamletWeights);

	//CMatrixNxM<> mFiltBeamletWeights(mBeamletWeights.GetCols(),	
	//	GetPyramid()->GetPlan(nScale-1)->GetBeamAt(0)->GetBeamletCount());

	CBeam::IntensityMap::Pointer intMapIn = CBeam::IntensityMap::New();
	ConformTo<VOXEL_REAL, 1>(GetPyramid()->GetPlan(nScale)->GetBeamAt(0)->GetIntensityMap(), intMapIn);
	//intMapIn->CopyInformation(GetPyramid()->GetPlan(nScale)->GetBeamAt(0)->GetIntensityMap());
	//intMapIn->SetBufferedRegion( GetPyramid()->GetPlan(nScale)->GetBeamAt(0)->GetIntensityMap()->GetBufferedRegion() );
	//intMapIn->SetRegions(GetPyramid()->GetPlan(nScale)->GetBeamAt(0)->GetIntensityMap()->GetB);
	//intMapIn->SetRegions(MakeSize(GetPyramid()->GetPlan(nScale)->GetBeamAt(0)->GetBeamletCount()));
	//intMapIn->Allocate();

	CBeam::IntensityMap::Pointer intMapOut = CBeam::IntensityMap::New();
	ConformTo<VOXEL_REAL, 1>(GetPyramid()->GetPlan(nScale-1)->GetBeamAt(0)->GetIntensityMap(), intMapOut);
	//intMapOut->CopyInformation(GetPyramid()->GetPlan(nScale-1)->GetBeamAt(0)->GetIntensityMap());
	//intMapOut->SetRegions(MakeSize(GetPyramid()->GetPlan(nScale-1)->GetBeamAt(0)->GetBeamletCount()));
	//intMapOut->Allocate();

	for (int nAtBeam = 0; nAtBeam < GetPlan()->GetBeamCount(); nAtBeam++)
	{
		StateVectorToIntensityMap(nScale, nAtBeam, vIn, intMapIn); 
		//for (int nAt = 0; nAt < intMapIn->GetBufferedRegion().GetSize()[0]; nAt++)
		//	intMapIn->GetBufferPointer()[nAt] = mBeamletWeights[nAtBeam][nAt];

		GetPyramid()->InvFiltIntensityMap(
			nScale,
			intMapIn, // mBeamletWeights[nAtBeam],
			intMapOut); // mFiltBeamletWeights[nAtBeam]);

		IntensityMapToStateVector(nScale-1, nAtBeam, intMapOut, vOut);
		//for (int nAt = 0; nAt < intMapOut->GetBufferedRegion().GetSize()[0]; nAt++)
		//	mFiltBeamletWeights[nAtBeam][nAt] = intMapOut->GetBufferPointer()[nAt];
	} 

	//BeamletWeightsToStateVector(nScale-1, mFiltBeamletWeights, vOut); 

}	// PlanOptimizer::InvFilterStateVector


///////////////////////////////////////////////////////////////////////////////
void
	PlanOptimizer::SetupPrescription()
	// returns the given level of the pyramid
{
	USES_CONVERSION;

	// create the pyramid
	SetPyramid(new dH::PlanPyramid(GetPlan()));

	// get main sigma parameter from registry
	const REAL GBinSigma = GetProfileReal(W2A(REG_KEY), W2A(GBINSIGMA_KEY), DEFAULT_GBINSIGMA);

	// form vector with levels of the presc object
	m_arrPrescriptions.clear();
	for (int nLevel = 0; nLevel < PlanPyramid::MAX_SCALES; nLevel++)
	{
		// create a new prescription object
		Prescription * pPresc = new dH::Prescription(GetPlan());
		pPresc->SetPlan(GetPyramid()->GetPlan(nLevel));		// TODO: why is this called here?

		/// TODO: set up slice
		///		or just in Presc::AddStructTerm

		// calculate the variance range for this level
		const REAL sigma = GetProfileRealAt(LEVELSIGMA_KEY, nLevel, DEFAULT_LEVELSIGMA[nLevel]);
		const REAL binVar = pow(GBinSigma / sigma, 2);
		const REAL varMin = binVar * 0.25;
		const REAL varMax = binVar;

		// set the variance range in the objective function
		// NOTE: this has to be done after the Optimizer->SetAdaptiveVariance, because
		//		it will over-ride some of those settings
		// pPresc->SetGBinVar(varMin, varMax);

		// construct the optimizer
		DynamicCovarianceOptimizer *pOptimizer = new DynamicCovarianceOptimizer(pPresc);

		// set the variance range for the optimizer
		pOptimizer->SetAdaptiveVariance(true, varMin, varMax);

		// set the tolerances
		//pOptimizer->SetLineToleranceEqual(false);

		// set the line tolerance
		const REAL cgTolerance = GetProfileRealAt(CGTOL_KEY, nLevel, DEFAULT_TOLERANCE);
		pOptimizer->set_x_tolerance/*SetTolerance*/(cgTolerance); 

		// set the CG tolerance
		const REAL lineTolerance = GetProfileRealAt(LINETOL_KEY, nLevel, DEFAULT_TOLERANCE);
		// pOptimizer->GetBrentOptimizer().set_x_tolerance(lineTolerance);
		pOptimizer->SetLineOptimizerTolerance(lineTolerance);

		// do not apply transform slope variance for lowest-res level
		if (nLevel == PlanPyramid::MAX_SCALES-1)
			pPresc->SetTransformSlopeVariance(false);

		// set the variance range in the objective function
		// NOTE: this has to be done after the Optimizer->SetAdaptiveVariance, because
		//		it will over-ride some of those settings
		pPresc->SetGBinVar(varMin, varMax);

		m_arrPrescriptions.push_back(std::pair<Prescription*, DynamicCovarianceOptimizer*>(pPresc, pOptimizer));
	}

}	// PlanOptimizer::SetupPrescription


///////////////////////////////////////////////////////////////////////////////
void 
	PlanOptimizer::StateVectorToIntensityMap(int nScale, int nBeam,
		const CVectorN<>& vState, 
		CBeam::IntensityMap *pIntensityMap) // CMatrixNxM<>& mBeamletWeights)
{
	CPlan *pPlan = GetPyramid()->GetPlan(nScale);
	int nBeamletCount = pPlan->GetBeamAt(nBeam/*0*/)->GetBeamletCount();
	int nBeamletOffset = nBeamletCount / 2;
	ConformTo<VOXEL_REAL>(pPlan->GetBeamAt(nBeam)->GetIntensityMap(), 
		pIntensityMap);		// TODO: make this an ASSERT, as caller should be responsible for this
	ASSERT(nBeamletCount == pIntensityMap->GetBufferedRegion().GetSize()[0]);

	for (int nAtElem = 0; nAtElem < vState.GetDim(); nAtElem++)
	{
		int nCurrBeam;
		int nCurrBeamlet;

		GetPrescription(nScale)->GetBeamletFromSVElem(nAtElem, &nCurrBeam, &nCurrBeamlet);
		if (nCurrBeam == nBeam)
		{
			pIntensityMap->GetBufferPointer()[nCurrBeamlet + nBeamletOffset] = vState[nAtElem];
		}
	}

}	// PlanOptimizer::StateVectorToBeamletWeights

///////////////////////////////////////////////////////////////////////////////
void 
	PlanOptimizer::IntensityMapToStateVector(int nScale, int nBeam,
			const CBeam::IntensityMap *pIntensityMap/*CMatrixNxM<>& mBeamletWeights */, CVectorN<>& vState)
{
	CPlan *pPlan = GetPyramid()->GetPlan(nScale);
	int nBeamletCount = pPlan->GetBeamAt(nBeam/*0*/)->GetBeamletCount();

	int nBeamletOffset = nBeamletCount / 2;

	if (vState.GetDim() != pPlan->GetTotalBeamletCount())
		vState.SetDim(pPlan->GetTotalBeamletCount());

	for (int nAtElem = 0; nAtElem < vState.GetDim(); nAtElem++)
	{
		int nCurrBeam;
		int nCurrBeamlet;

		GetPrescription(nScale)->GetBeamletFromSVElem(nAtElem, &nCurrBeam, &nCurrBeamlet);
		if (nCurrBeam == nBeam)
		{
			vState[nAtElem] = pIntensityMap->GetBufferPointer()[nCurrBeamlet + nBeamletOffset];
		}
	}

}	// PlanOptimizer::BeamletWeightsToStateVector

}	// namespace dH
