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
	//for (int nLevel = 1; nLevel < m_arrPlans.size(); nLevel++)
	//	delete m_arrPlans[nLevel];
	//m_arrPlans.clear();
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
			CBeam *pNextBeam = NULL;
			if (pNextPlan->GetBeamCount() <= nAt)
			{
				pNextBeam = new CBeam(/*pPrevBeam*/);
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

REAL GetSliceMax(VolumeReal *pVol, int nSlice)
{
	typedef itk::ImageRegionConstIterator< VolumeReal > ConstIteratorType;
	typedef itk::ImageRegionIterator< VolumeReal > IteratorType;

	VolumeReal::RegionType inputRegion;
	VolumeReal::RegionType::IndexType inputStart = pVol->GetBufferedRegion().GetIndex();
	inputStart[0] = 0;
	inputStart[1] = 0;
	inputStart[2] = nSlice;

	VolumeReal::RegionType::SizeType size = pVol->GetBufferedRegion().GetSize();
	//size[0] = ;
	//size[1] = ;
	size[2] = 1;

	inputRegion.SetSize( size );
	inputRegion.SetIndex( inputStart );

	ConstIteratorType inputIt( pVol, inputRegion );

	REAL maxVoxel = -1e+6;
	for ( inputIt.GoToBegin(); !inputIt.IsAtEnd(); ++inputIt)
	{
		maxVoxel = __max(inputIt.Get(), maxVoxel);
	}

	return maxVoxel;
}


///////////////////////////////////////////////////////////////////////////////
void 
	PlanPyramid::CalcPencilSubBeamlets(int nBeam)
{
	// this re-generates all sub-plans
	// SetPlan(m_pPlan);

	//REAL doseResolution = GetPlan()->GetDoseResolution();
	//CPlan *pPrevPlan = GetPlan();
	//for (int nLevel = 1; nLevel < MAX_SCALES; nLevel++)
	//{
	//	doseResolution *= 2.0;
	//	CPlan *pNextPlan = GetPlan(nLevel); // new CPlan();
	//	//pNextPlan->SetSeries(m_pPlan->GetSeries());
	//	//pNextPlan->SetDoseResolution(doseResolution);
	//	//for (int nAt = 0; nAt < GetPlan()->GetBeamCount(); nAt++)
	//	//{
	//		CBeam *pPrevBeam = pPrevPlan->GetBeamAt(nBeam);
	//		CBeam *pNextBeam = new CBeam(/*pPrevBeam*/);
	//		// need to add the beam first, because the plan is needed to set gantry angle
	//		pNextPlan->AddBeam(pNextBeam);

	//		pNextBeam->SetGantryAngle(pPrevBeam->GetGantryAngle());
	//		pNextBeam->SetIsocenter(pPrevBeam->GetIsocenter());

	//	// }
	//	//m_arrPlans.push_back(pNextPlan);
	//	pPrevPlan = pNextPlan;
	//}

	// TODO: fix this -- should actually check whether beamlets need recalc
	//if (GetPlan(1)->GetBeamCount() > 0)
	//	return;

#ifdef NEVER
	// replicated from SetPlan
	CPlan *pPrevPlan = m_pPlan;
	for (int nLevel = 1; nLevel < MAX_SCALES; nLevel++)
	{
		CPlan *pNextPlan = GetPlan(nLevel); // new CPlan();
		pNextPlan->m_arrBeams.RemoveAll();
		ASSERT(pNextPlan->GetBeamCount() == 0);

		// pNextPlan->SetSeries(m_pPlan->GetSeries());
		for (int nAt = 0; nAt < GetPlan()->GetBeamCount(); nAt++)
		{
			CBeam *pPrevBeam = pPrevPlan->GetBeamAt(nAt);
			CBeam *pNextBeam = new CBeam(/*pPrevBeam*/);
			// need to add the beam first, because the plan is needed to set gantry angle
			pNextPlan->AddBeam(pNextBeam);

			pNextBeam->SetGantryAngle(pPrevBeam->GetGantryAngle());
			pNextBeam->SetIsocenter(pPrevBeam->GetIsocenter());
		}
	}
#endif

	// make sure pencil subbeamlets are properly generated
	for (int nAt = ((nBeam == -1) ? (GetPlan()->GetBeamCount()-1) : nBeam); nAt >= ((nBeam == -1) ? 0 : nBeam); nAt--)
	{
		CBeam *pBeam = GetPlan()->GetBeamAt(nAt);

		// only recalc if they need to be
		if (!pBeam->m_bRecalcBeamlets)
			return;

			// stores beamlet count for level N
		int nBeamletCount = /*m_nBeamletCount*/ // 5; // 
			19;
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

				//pPyramid->SetOutput(0, NULL);
				//pPyramid->GetOutput(1)->DisconnectPipeline();
				//pPyramid->GetOutput(0)->DisconnectPipeline();
				// pPyramid->SetInput(NULL);

				// explicitly release: why?
				// pPyramid->UnRegister();
				//pPyramid = NULL;

				//VolumeReal *pOutput = pBeamSub->GetBeamlet(nAtShift);
				//REAL maxOutput = GetSliceMax(pOutput, 0);
				//REAL maxInput = GetSliceMax(beamlet, 0);

				//VolumeReal::RegionType inputRegion;
				//VolumeReal::RegionType::IndexType inputStart = pOutput->GetBufferedRegion().GetIndex();
				//VolumeReal::RegionType::SizeType size = pOutput->GetBufferedRegion().GetSize();

				//inputRegion.SetSize( size );
				//inputRegion.SetIndex( inputStart );

				//VolumeRealIterator voxelIt( pOutput, inputRegion );
				//for ( voxelIt.GoToBegin(); !voxelIt.IsAtEnd(); ++voxelIt)
				//{
				//	voxelIt.Set(voxelIt.Get() / maxOutput * maxInput);
				//}
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

// #define FUCK_FILTER
#ifdef FUCK_FILTER
	{
	typedef itk::ResampleImageFilter<CBeam::IntensityMap, CBeam::IntensityMap> ResamplerType;
	ResamplerType::Pointer filter = ResamplerType::New();

	typedef itk::AffineTransform< REAL, 1 > TransformType;
	TransformType::Pointer transform = TransformType::New();
	transform->SetIdentity();
	filter->SetTransform( transform );

	typedef itk::LinearInterpolateImageFunction<CBeam::IntensityMap, REAL> InterpolatorType;
	InterpolatorType::Pointer interpolator = InterpolatorType::New();
	filter->SetInterpolator( interpolator );

	filter->SetDefaultPixelValue( 1e-4 );

	filter->SetInput(vWeights);
	filter->SetReferenceImage(vFiltWeights);
	filter->SetUseReferenceImage(true);
	// filter->SetOutputParametersFromImage( vFiltWeights );

	//filter->SetOutputSpacing( vFiltWeights->GetSpacing() );
	//filter->SetOutputOrigin(  vFiltWeights->GetOrigin() );
	//filter->GetOutput()->SetRegions(vFiltWeights->GetBufferedRegion());
	//filter->GetOutput()->Allocate();
	// filter->SetSize( vFiltWeights->GetBufferedRegion() );

	filter->Update();


	CBeam::IntensityMap::Pointer ptrOutput = filter->GetOutput();
	ASSERT(ptrOutput->GetBufferedRegion().GetSize()[0] == vFiltWeights->GetBufferedRegion().GetSize()[0]);
	for (int nAt = 0; nAt < vFiltWeights->GetBufferedRegion().GetSize()[0]; nAt++)
	{
		vFiltWeights->GetBufferPointer()[nAt] = /*2.0 **/ ptrOutput->GetBufferPointer()[nAt];
		ASSERT(_finite(vFiltWeights->GetBufferPointer()[nAt]));
	}
	TRACE("1 Input int map: ");
	for (int nAt = 0; nAt < vWeights->GetBufferedRegion().GetSize()[0]; nAt++)
	{
		TRACE("%f\t", vWeights->GetBufferPointer()[nAt]);
	}
	TRACE("\n1Output int map: ");
	for (int nAt = 0; nAt < vFiltWeights->GetBufferedRegion().GetSize()[0]; nAt++)
	{
		TRACE("%f\t", vFiltWeights->GetBufferPointer()[nAt]);
	}
	}
#endif

}	// PlanPyramid::InvFiltIntensityMap
//
/////////////////////////////////////////////////////////////////////////////////
//const CMatrixNxM<>& 
//	PlanPyramid::GetFilterMat(int nLevel)
//{
//	int nBeamletCount = GetPlan(nLevel)->GetBeamAt(0)->GetBeamletCount();
//
//	if (m_mFilter[nLevel].GetCols() != nBeamletCount)
//	{
//		// set up the filter matrix
//		m_mFilter[nLevel].Reshape(nBeamletCount, nBeamletCount);
//		ZeroMemory(m_mFilter[nLevel], 
//			m_mFilter[nLevel].GetRows() * m_mFilter[nLevel].GetCols() * sizeof(REAL));
//
//		for (int nAt = 0; nAt < m_mFilter[nLevel].GetRows(); nAt++)
//		{
//			if (nAt > 0)
//			{
//				m_mFilter[nLevel][nAt - 1][nAt] = m_vWeightFilter[0];
//			}
//
//			m_mFilter[nLevel][nAt][nAt] = m_vWeightFilter[1];
//
//			if (nAt < m_mFilter[nLevel].GetRows()-1)
//			{
//				m_mFilter[nLevel][nAt + 1][nAt] = m_vWeightFilter[2];
//			}
//		}
//	}
//
//	return m_mFilter[nLevel];
//
//}	// PlanPyramid::GetFilterMat

}	// namespace dH