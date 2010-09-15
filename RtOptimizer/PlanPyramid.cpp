// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: PlanPyramid.cpp 647 2009-11-05 21:52:59Z dglane001 $
#include "StdAfx.h"
#include "PlanPyramid.h"

#include "itkBinomialBlurImageFilter.h"
#include "itkResampleImageFilter.h"
#include "itkAffineTransform.h"
#include "itkLinearInterpolateImageFunction.h"


namespace dH
{

///////////////////////////////////////////////////////////////////////////////
PlanPyramid::PlanPyramid(CPlan *pPlan)
{
	SetPlan(pPlan);

	if (m_vWeightFilter.GetDim() == 0)
	{
		m_vWeightFilter.SetDim(3);
		m_vWeightFilter[0] = 0.25;
		m_vWeightFilter[1] = 0.50;
		m_vWeightFilter[2] = 0.25;
	}
}

///////////////////////////////////////////////////////////////////////////////
PlanPyramid::~PlanPyramid(void)
{
	for (int nLevel = 1; nLevel < MAX_SCALES; nLevel++)
		delete m_arrPlans[nLevel];
}

///////////////////////////////////////////////////////////////////////////////
void 
	PlanPyramid::SetPlan(CPlan *pPlan)
{
	m_pPlan = pPlan;


	// generate sub-plans
	CPlan *pPrevPlan = GetPlan();
	if (m_arrPlans.size() == 0)
		m_arrPlans.push_back(pPrevPlan);

	REAL doseResolution = pPlan->GetDoseResolution();
	for (int nLevel = 1; nLevel < MAX_SCALES; nLevel++)
	{
		doseResolution *= 2.0;
		CPlan *pNextPlan = NULL; 
		if (m_arrPlans.size() <= nLevel)
		{
			pNextPlan = new CPlan();
			m_arrPlans.push_back(pNextPlan);
		}
		else
		{
			pNextPlan = m_arrPlans[nLevel];
		}

		pNextPlan->SetSeries(m_pPlan->GetSeries());
		pNextPlan->SetDoseResolution(doseResolution);
		for (int nAt = 0; nAt < GetPlan()->GetBeamCount(); nAt++)
		{
			CBeam *pPrevBeam = pPrevPlan->GetBeamAt(nAt);
			CBeam::Pointer pNextBeam;
			if (pNextPlan->GetBeamCount() <= nAt)
			{
				pNextBeam = dH::Beam::New(); // CBeam(/*pPrevBeam*/);
				// need to add the beam first, because the plan is needed to set gantry angle
				pNextPlan->AddBeam(pNextBeam);
			}
			else
			{
				pNextBeam = pNextPlan->GetBeamAt(nAt);
			}

			pNextBeam->SetGantryAngle(pPrevBeam->GetGantryAngle());
			pNextBeam->SetIsocenter(pPrevBeam->GetIsocenter());

		}
		ASSERT(pPrevPlan->GetBeamCount() == pNextPlan->GetBeamCount());

		pPrevPlan = pNextPlan;
	}

}	// PlanPyramid::SetPlan

///////////////////////////////////////////////////////////////////////////////
CPlan *
	PlanPyramid::GetPlan(int nLevel)
{
	return m_arrPlans[nLevel];

}	// PlanPyramid::SetPlan

///////////////////////////////////////////////////////////////////////////////
void 
	PlanPyramid::CalcPencilSubBeamlets(int nBeam)
{
	// make sure pencil subbeamlets are properly generated
	for (int nAt = ((nBeam == -1) ? (GetPlan()->GetBeamCount()-1) : nBeam); nAt >= ((nBeam == -1) ? 0 : nBeam); nAt--)
	{
		CBeam *pBeam = GetPlan()->GetBeamAt(nAt);

		// only recalc if they need to be
		if (!pBeam->m_bRecalcBeamlets)
			return;

			// stores beamlet count for level N
		int nBeamletCount = 19;
		// TODO: reconcile this with nBeamletCount used in PlanPyramid

		REAL beamletSpacing = 4.0; // 2.0;
		// TODO: reconcile this with beamletSpacing used in BeamDoseCalc

		// now generate level 1..n beamlets
		for (int nAtScale = 1; nAtScale < MAX_SCALES; nAtScale++)
		{
			CBeam *pBeamSub = m_arrPlans[nAtScale]->GetBeamAt(nAt);
			CBeam *pBeamSubPrev = m_arrPlans[nAtScale-1]->GetBeamAt(nAt);

			// each level halves the number of beamlets
			nBeamletCount /= 2;
			beamletSpacing *= 2.0;

			// set up the beams beamlets; do this by defining the intensity map parameters
			CBeam::IntensityMap *pIM = pBeamSub->GetIntensityMap();

			// set up the intensity map indexing
			CBeam::IntensityMap::RegionType region;
			itk::Index<1> index = {{-nBeamletCount}};
			region.SetIndex(index);
			region.SetSize(MakeSize(nBeamletCount*2 + 1));
			pIM->SetRegions(region);	/// TODO: make this index from -n/2..n/2
			pIM->Allocate();

			// set up the intensity map spacing
			pIM->SetSpacing(beamletSpacing);
			/// TODO: figure out the origin
			REAL origin[] = {0}; 
				// {-beamletSpacing * nBeamletCount};
			pIM->SetOrigin(origin);
			pIM->FillBuffer(0);

			// this will allocate the necessary beamlets
			pBeamSub->OnIntensityMapChanged();

			typedef itk::MultiResolutionPyramidImageFilter<VolumeReal, VolumeReal> PyramidType;
			PyramidType::Pointer pPyramid = PyramidType::New();
			pPyramid->SetNumberOfLevels(2);

			VolumeReal::Pointer beamlet = // const_cast<VolumeReal*>(pPyramid->GetInput()); // 
				VolumeReal::New();
			pPyramid->SetInput(beamlet);
			VolumeReal::Pointer beamletAccum = VolumeReal::New();

			ConformTo<VOXEL_REAL,3>(pBeamSubPrev->GetBeamlet(0), beamlet);
			ConformTo<VOXEL_REAL,3>(pBeamSubPrev->GetBeamlet(0), beamletAccum);

			// generate beamlets for base scale
			for (int nAtShift = -nBeamletCount; nAtShift <= nBeamletCount; nAtShift++)
			{
				// helpers for calculating sub beamlets
				beamlet->FillBuffer(0.0);

				VolumeReal * pPrevBeamletLow = pBeamSubPrev->GetBeamlet(nAtShift * 2 - 1);
				VolumeReal * pPrevBeamletHigh = pBeamSubPrev->GetBeamlet(nAtShift * 2 + 1);
				if (pPrevBeamletLow != NULL)
				{
					// NOTE: these are all * 2.0 because there are only half as many sub-beamlets 
					//		contributing; this means that the intensity map interpolation needs 
					//		no scaling
					Accumulate3D<VOXEL_REAL>(pPrevBeamletLow, 
						(pPrevBeamletHigh != NULL) 
						? 2.0 * m_vWeightFilter[0] 
						: 2.0 * m_vWeightFilter[0] /*/ 0.75*/, 
						beamlet, beamletAccum);
				}

				Accumulate3D<VOXEL_REAL>(pBeamSubPrev->GetBeamlet(nAtShift * 2 + 0), 
					(pPrevBeamletHigh != NULL && pPrevBeamletLow != NULL) 
						? 2.0 * m_vWeightFilter[1] 
						: 2.0 * m_vWeightFilter[1] /*/ 0.75*/, 
					beamlet, beamletAccum); 


				if (pPrevBeamletHigh != NULL)
				{
					Accumulate3D<VOXEL_REAL>(pPrevBeamletHigh, 
						(pPrevBeamletLow != NULL) 
						? 2.0 * m_vWeightFilter[2] 
						: 2.0 * m_vWeightFilter[2] /*/ 0.75*/,
						beamlet, beamletAccum);
				}
				// pPyramid->ResetPipeline();
				pPyramid->SetNumberOfLevels(2); // ->Update();
				pPyramid->Modified();
				pPyramid->GetOutput(0)->Update();
				// TODO: investigate whether resulting filtered beamlet is scaled properly

				CopyImage<VOXEL_REAL,3>(pPyramid->GetOutput(0), pBeamSub->GetBeamlet(nAtShift));

				// check that resolution is correct
				ASSERT(pBeamSub->GetBeamlet(nAtShift)->GetSpacing()[0] == pBeamSub->GetPlan()->GetDoseResolution());
			}
		}

		/// TODO: move this flag to PlanPyramid::m_bRecalcBeamlets
		pBeam->m_bRecalcBeamlets = false;
	}

}	// PlanPyramid::CalcPencilSubBeamlets

///////////////////////////////////////////////////////////////////////////////
void
PlanPyramid::InvFiltIntensityMap(int nLevel, const CBeam::IntensityMap * vWeights,
								CBeam::IntensityMap * vFiltWeights)
{
#define MANUAL_INTERP
#ifdef MANUAL_INTERP
	{
	ASSERT(vWeights->GetBufferedRegion().GetSize()[0] == GetPlan(nLevel)->GetBeamAt(0)->GetBeamletCount());
	ASSERT(vFiltWeights->GetBufferedRegion().GetSize()[0] == GetPlan(nLevel-1)->GetBeamAt(0)->GetBeamletCount());

	int nBeamletCountPrev = GetPlan(nLevel)->GetBeamAt(0)->GetBeamletCount() / 2;
	int nBeamletCountNext = GetPlan(nLevel-1)->GetBeamAt(0)->GetBeamletCount() / 2;

	// generate beamlets for base scale
	for (int nAtShift = -nBeamletCountNext; nAtShift <= nBeamletCountNext; nAtShift++)
	{
		if (abs(nAtShift) % 2 == 0)
		{
			vFiltWeights->GetBufferPointer()[nAtShift + nBeamletCountNext] =
				vWeights->GetBufferPointer()[nAtShift/2 + nBeamletCountPrev];
		}
		else
		{
			int nLower = floor((double) nAtShift / 2.0);
			int nHigher = ceil((double) nAtShift / 2.0);
			if (nLower + nBeamletCountPrev < 0)
			{
				vFiltWeights->GetBufferPointer()[nAtShift + nBeamletCountNext] = 
					0.5 * vWeights->GetBufferPointer()[nHigher + nBeamletCountPrev];
			}
			else if (nHigher + nBeamletCountPrev >= vWeights->GetBufferedRegion().GetSize()[0])
			{
				vFiltWeights->GetBufferPointer()[nAtShift + nBeamletCountNext] =  
					0.5 * vWeights->GetBufferPointer()[nLower + nBeamletCountPrev];
			}
			else
			{
				vFiltWeights->GetBufferPointer()[nAtShift + nBeamletCountNext] = 
					0.5 * vWeights->GetBufferPointer()[nLower + nBeamletCountPrev]
						+ 0.5 * vWeights->GetBufferPointer()[nHigher + nBeamletCountPrev];
			}
		}
	}

	}
#endif


}	// PlanPyramid::InvFiltIntensityMap

}	// namespace dH