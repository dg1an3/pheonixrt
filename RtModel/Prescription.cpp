// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: Prescription.cpp 640 2009-06-13 05:06:50Z dglane001 $
#include "stdafx.h"
#include "Prescription.h"

#include <iostream>

#include <ConjGradOptimizer.h>
#include <HistogramGradient.h>

namespace dH
{

const REAL SIGMOID_SCALE = 0.2; // 0.1; // 0.3; // 0.1; // 1.0;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
Prescription::Prescription(CPlan *pPlan/*, int nLevel*/)
	: /*CObjectiveFunction(FALSE)
		, */m_pPlan(pPlan)
		, m_inputScale(GetProfileReal("Prescription", "InputScale", 0.5))
		, m_Slice(0)
		, m_TransformSlopeVariance(true)
{
	m_sumVolume = VolumeReal::New();

	m_volGroupMaxVar = VolumeReal::New();
	m_volGroupMinVar = VolumeReal::New();

	m_volGroupMainMaxVar = VolumeReal::New();
	m_volGroupMainMinVar = VolumeReal::New();

	m_volMainMinVar = VolumeReal::New();
	m_volMainMaxVar = VolumeReal::New();

	m_volTemp = VolumeReal::New();

}	// Prescription::Prescription

///////////////////////////////////////////////////////////////////////////////
Prescription::~Prescription()
{
	dH::Structure *pStruct = NULL;
	VOITerm *pVOIT = NULL;
	POSITION pos = m_mapVOITs.GetStartPosition();
	while (pos != NULL)
	{
		m_mapVOITs.GetNextAssoc(pos, pStruct, pVOIT);
		delete pVOIT;
	}

}	// Prescription::~Prescription

///////////////////////////////////////////////////////////////////////////////
VOITerm *
	Prescription::GetStructureTerm(Structure *pStruct)
{
	VOITerm *pVOIT = NULL;
	m_mapVOITs.Lookup(pStruct, pVOIT);

	return pVOIT;

}	// Prescription::GetStructureTerm

//static REAL GetSliceMax(VolumeReal *pVol, int nSlice)
//{
//	typedef itk::ImageRegionConstIterator< VolumeReal > ConstIteratorType;
//	typedef itk::ImageRegionIterator< VolumeReal > IteratorType;
//
//	VolumeReal::RegionType inputRegion;
//	VolumeReal::RegionType::IndexType inputStart = pVol->GetBufferedRegion().GetIndex();
//	inputStart[0] = 0;
//	inputStart[1] = 0;
//	inputStart[2] = nSlice;
//
//	VolumeReal::RegionType::SizeType size = pVol->GetBufferedRegion().GetSize();
//	//size[0] = ;
//	//size[1] = ;
//	size[2] = 1;
//
//	inputRegion.SetSize( size );
//	inputRegion.SetIndex( inputStart );
//
//	ConstIteratorType inputIt( pVol, inputRegion );
//
//	REAL maxVoxel = -1e+6;
//	for ( inputIt.GoToBegin(); !inputIt.IsAtEnd(); ++inputIt)
//	{
//		maxVoxel = __max(inputIt.Get(), maxVoxel);
//	}
//
//	return maxVoxel;
//}

///////////////////////////////////////////////////////////////////////////////
void 
	Prescription::AddStructureTerm(VOITerm *pVOIT)
{
	// get any beam (as an exemplar)
	CBeam *pBeam = m_pPlan->GetBeamAt(m_pPlan->GetBeamCount()-1);

	// initialize the sum volume, so as to coincide with the beamlets
	VolumeReal *pBeamlet = pBeam/*m_pPlan->GetBeamAt(m_pPlan->GetBeamCount()-1)*/->GetBeamlet(0);
	ConformTo<VOXEL_REAL,3>(pBeamlet, m_sumVolume);

	// initialize the histogram region
	// TODO: fix this memory leak
	VolumeReal *pResampRegion = pVOIT->GetVOI()->GetConformRegion(m_sumVolume);

	// set histogram options
	CHistogramWithGradient *pHisto = pVOIT->GetHistogram();
	pHisto->SetVolume(m_sumVolume);
	pHisto->SetRegion(pResampRegion);

	// calculate slice number for the isocenter
	REAL sliceZ = pBeam->GetIsocenter()[2];
	int nSlice = Round<int>((sliceZ - m_sumVolume->GetOrigin()[2]) / m_sumVolume->GetSpacing()[2]);
	SetSlice(nSlice);
	pHisto->SetSlice(nSlice);

	// check that slice is non-zero
	REAL sliceMax = GetSliceMax(pResampRegion, nSlice);
	if (sliceMax < 1e-3)
	{
		for (int nOtherSlice = 0; 
			nOtherSlice < pResampRegion->GetBufferedRegion().GetSize()[2];
			nOtherSlice++)
		{
			sliceMax = GetSliceMax(pResampRegion, nOtherSlice);
			TRACE("sliceMax on slice %i = %lf\n", nOtherSlice, sliceMax);
		}
	}

	// need to do this before initialize target bins
	//	 original = 0.0125	0.006125  // * pow(2.0, (double) m_nLevel));
	REAL binWidth = R(0.01); // R(0.02); // R(0.0030625);  
	pHisto->SetBinning(0.0, binWidth, GBINS_BUFFER);
	pHisto->SetGBinVar(&m_ActualAV, m_varMin, m_varMax);

	// set up dVolumes
	for (int nAtElem = 0; nAtElem < m_pPlan->GetTotalBeamletCount(); nAtElem++)
	{
		int nBeam;
		int nBeamlet;
		GetBeamletFromSVElem(nAtElem, &nBeam, &nBeamlet);

		VolumeReal *pBeamlet = m_pPlan->GetBeamAt(nBeam)->GetBeamlet(nBeamlet);
		pHisto->Add_dVolume(pBeamlet, nBeam);
	}

	// add to the current prescription
	m_mapVOITs[pVOIT->GetVOI()] = pVOIT;

	SetElementInclude();

}	// Prescription::AddStructureTerm

///////////////////////////////////////////////////////////////////////////////
void 
	Prescription::RemoveStructureTerm(Structure *pStruct)
{
	VOITerm *pVOIT = NULL;
	if (m_mapVOITs.Lookup(pStruct, pVOIT))
	{
		m_mapVOITs.RemoveKey(pStruct);
		delete pVOIT;
	}

}	// Prescription::RemoveStructureTerm

///////////////////////////////////////////////////////////////////////////////
void 
	Prescription::UpdateTerms(Prescription *pPresc)
	// updates terms from another prescription object
{
	// now for the terms
	POSITION pos = pPresc->m_mapVOITs.GetStartPosition();
	while (pos != NULL)
	{
		Structure * pStruct = NULL;
		VOITerm *pVOIT = NULL;
		pPresc->m_mapVOITs.GetNextAssoc(pos, pStruct, pVOIT);

		VOITerm *pMyVOIT = NULL;
		BOOL bFound = m_mapVOITs.Lookup(pStruct, pMyVOIT);
		ASSERT(bFound);

		// assignment operator -- copies all relevent fields
		pMyVOIT->UpdateFrom(pVOIT);
	}

}	// Prescription::UpdateTerms

//////////////////////////////////////////////////////////////////////////////
void 
	Prescription::SetGBinVar(REAL varMin, REAL varMax)
	// sets up adaptive variance
{
	// NOTE: this is an over-ride that requires that the optimizer's SetGBinVar be called first
	m_varMin = varMin = 0.01; // 0.0001;

	// varMax *= 1.5; // 2.0;
	// if (!IsApproxEqual(m_varMax, varMax))
	//	::AfxMessageBox(_T("Problem!!!"));
	m_varMax = varMax;
	if (GetTransformSlopeVariance())
	{
		// compute the maximum slope for the transform function
		REAL varSlope = SIGMOID_SCALE * dSigmoid(0.0, m_inputScale);
		
		// this is equivalent to scaling the level sigma's so that their current
		//	value is the equal to that at optimizer value -4.0
		varSlope /= SIGMOID_SCALE * dSigmoid(0.0, m_inputScale);

		// adjust variance max for the maximum slope
		m_varMax = varMax * varSlope * varSlope;
	}

	// now set up the histogram variances
	POSITION pos = m_mapVOITs.GetStartPosition();
	while (pos != NULL)
	{
		Structure *pStruct = NULL;
		VOITerm *pVOIT = NULL;
		m_mapVOITs.GetNextAssoc(pos, pStruct, pVOIT);

		pVOIT->GetHistogram()->SetGBinVar(&m_ActualAV/*m_pAV*/, m_varMin, m_varMax);
	}

}	// Prescription::SetGBinVar

///////////////////////////////////////////////////////////////////////////////
REAL 
	Prescription::operator()(const CVectorN<>& vInput, CVectorN<> *pGrad ) const
	// objective function evaluator
{
	USES_CONVERSION;

	// initialize total sum of objective function
	REAL totalSum = 0.0;

	BeginLogSection(_T("Prescription::operator()"));

	TraceVector(_T("vInput"), vInput);

	// transform input for calc purposes
	CVectorN<> vInputTrans = vInput;
	Transform(&vInputTrans);
	TraceVector(_T("vInputTrans"), vInputTrans);

	// dTransform of the input -- flag is used to only calculate it if needed (if there is a gradient)
	CVectorN<> v_dInputTrans;

	// initialization for gradient calc
	if (pGrad)
	{
		// initialize gradient vector
		pGrad->SetDim(vInput.GetDim());
		pGrad->SetZero();

		// dTransform of input
		v_dInputTrans.SetDim(vInput.GetDim());
		v_dInputTrans = vInput;
		dTransform(&v_dInputTrans);
		TraceVector(_T("v_dInputTrans"), v_dInputTrans);
	}

	// flag to indicate need to call CalcSumSigmoid
	bool bCalcSum = true;

	// iterate over the VOITerms
	POSITION pos = m_mapVOITs.GetStartPosition();
	while (pos != NULL)
	{
		Structure *pStruct = NULL;
		VOITerm *pVOIT = NULL;
		m_mapVOITs.GetNextAssoc(pos, pStruct, pVOIT);

		// calculate the summed volume, if this is the first VOIT
		if (bCalcSum)
		{
			CalcSumSigmoid(pVOIT->GetHistogram(), vInput, vInputTrans, m_arrIncludeElement);
			bCalcSum = false;
		}

		if (pVOIT->GetWeight() >= DEFAULT_EPSILON)
		{
			CString strMessage;
			strMessage.Format(_T("VOI = %s\n"), 
				A2W(pVOIT->GetVOI()->GetName().c_str()));
			OutputDebugString(strMessage);

			// set fractions to histo
			pVOIT->GetHistogram()->SetVarFracVolumes(m_volMainMinVar, m_volMainMaxVar);
			dynamic_cast<CHistogramWithGradient*>(pVOIT->GetHistogram())->vInput = &vInput;
			dynamic_cast<CHistogramWithGradient*>(pVOIT->GetHistogram())->vInputTrans = &vInputTrans;

			// trigger change
			pVOIT->GetHistogram()->OnVolumeChange(); //NULL, NULL);
			// TODO: what is updated here?

			if (pGrad)
			{
				// initialize partial gradient vector
				m_vPartGrad.SetDim(vInput.GetDim());
				m_vPartGrad.SetZero();

				// evaluate the VOITerm
				totalSum += pVOIT->Eval(&m_vPartGrad, m_arrIncludeElement);

				// apply the chain rule for the sigmoid
				for (int nAt = 0; nAt < m_vPartGrad.GetDim(); nAt++)
				{
					// use dTransform'd vInput
					m_vPartGrad[nAt] *= v_dInputTrans[nAt]; 
				}

				TraceVector(_T("m_vPartGrad"), m_vPartGrad);

				// add the partial gradient to the total
				(*pGrad) += m_vPartGrad;
			}
			else
			{
				totalSum += pVOIT->Eval(NULL, m_arrIncludeElement);
			}
		}
	}

	// good to catch an NANs
	ASSERT(_finite(totalSum));

	// DGL: adding 0.1 to hold it up off 0.0 
	totalSum += 0.1;

	EndLogSection();

	return totalSum;

}	// Prescription::operator()


///////////////////////////////////////////////////////////////////////////////
void 
	Prescription::CalcSumSigmoid(CHistogramWithGradient *pHisto, 
								   const CVectorN<>& vInput,
								   const CVectorN<>& vInputTrans, 
								   const CArray<BOOL, BOOL>& arrInclude) const
	// computes the sum of weights from an input vector
{
	BeginLogSection(_T("Prescription::CalcSumSigmoid"));

	// get the main volume
	VolumeReal *pVolume = pHisto->GetVolume();
	pVolume->FillBuffer(0.0);

	ConformTo<VOXEL_REAL,3>(pVolume, m_volMainMinVar);
	m_volMainMinVar->FillBuffer(0.0);

	ConformTo<VOXEL_REAL,3>(pVolume, m_volMainMaxVar);
	m_volMainMaxVar->FillBuffer(0.0);

	// iterate over the component volumes, accumulating the weighted volumes
	ASSERT(vInputTrans.GetDim() == pHisto->Get_dVolumeCount());

	int nMaxGroup = pHisto->GetGroupCount();
	for (int nAtGroup = 0; nAtGroup < nMaxGroup; nAtGroup++)
	{
		BOOL bInitVolGroup = TRUE;

		for (int nAt_dVolume = 0; nAt_dVolume < pHisto->Get_dVolumeCount();
			nAt_dVolume++)
		{
			int nGroup = 0;
			VolumeReal *p_dVolume = pHisto->Get_dVolume(nAt_dVolume, &nGroup);

			if (nGroup == nAtGroup)
			{
				if (bInitVolGroup)
				{

					ConformTo<VOXEL_REAL,3>(p_dVolume, m_volGroupMaxVar);
					m_volGroupMaxVar->FillBuffer(0.0); 

					ConformTo<VOXEL_REAL,3>(p_dVolume, m_volGroupMinVar);
					m_volGroupMinVar->FillBuffer(0.0); 

					bInitVolGroup = FALSE;
				}

				// add to weighted sum
				if (arrInclude[nAt_dVolume])
				{
					if (m_ActualAV.GetDim() != m_pAV->GetDim())
					{
						m_ActualAV.SetDim(m_pAV->GetDim());
						m_ActualAV.SetZero();
					}

					// check adaptive variance value
					// TODO why is this not true?
					ASSERT((*m_pAV)[nAt_dVolume] <= (m_varMax + 1e-6));
					ASSERT((*m_pAV)[nAt_dVolume] >= (m_varMin - 1e-6));

					// determine variance using dSigmoid
					REAL varSlope = 1.0;
					REAL varWeight = 1.0;
					if (GetTransformSlopeVariance())
					{
						// calculate variance adjustment due to sigmoid transform
						varSlope = 
							SIGMOID_SCALE * dSigmoid(vInput[nAt_dVolume], m_inputScale);

						// this is equivalent to scaling the level sigma's so that their current
						//	value is the equal to that at optimizer value -4.0
						varSlope /= SIGMOID_SCALE * dSigmoid(0.0, m_inputScale);

						// compute the variance adjustment for the beamlet weight
						varWeight = vInputTrans[nAt_dVolume];

						// normalize so that beamlet weight at scale / 2 is 1.0
						varWeight /= SIGMOID_SCALE / 2.0;
					}
					REAL actVar = m_ActualAV[nAt_dVolume] = 
						(*m_pAV)[nAt_dVolume] * varSlope * varSlope * varWeight * varWeight;
					actVar = __max(actVar, m_varMin);
					actVar = __min(actVar, m_varMax);

					// calculate fractional parts
					const REAL fracMax = // ((*m_pAV)[nAt_dVolume] - m_varMin) / (m_varMax - m_varMin);
						(actVar - m_varMin) / (m_varMax - m_varMin);
					const REAL fracMin = 1.0 - fracMax; 

					// use Transform'd input to calc sigmoid

					// calculate max part
					const REAL weightMaxVar = vInputTrans[nAt_dVolume] * fracMax; 
					ConformTo<VOXEL_REAL,3>(m_volGroupMaxVar, m_volTemp);
					// Accumulate<VOXEL_REAL>(p_dVolume, weightMaxVar, 
					Accumulate3D<VOXEL_REAL>(p_dVolume, weightMaxVar, 
						m_volGroupMaxVar, m_volTemp); 

					// calculate min part
					const REAL weightMinVar = vInputTrans[nAt_dVolume] * fracMin; 
					ConformTo<VOXEL_REAL,3>(m_volGroupMinVar, m_volTemp);
					// Accumulate<VOXEL_REAL>(p_dVolume, weightMinVar,
					Accumulate3D<VOXEL_REAL>(p_dVolume, weightMinVar,
						m_volGroupMinVar, m_volTemp); 
				}
			}
		}

		// now rotate the groups sum to the main sumVolume basis
		ConformTo<VOXEL_REAL,3>(pVolume, m_volGroupMainMaxVar);
		m_volGroupMainMaxVar->FillBuffer(0.0); 
		//Resample(m_volGroupMaxVar, m_volGroupMainMaxVar, TRUE);
		//Resample3D(m_volGroupMaxVar, m_volGroupMainMaxVar, TRUE);
		itk::ResampleImageFilter<VolumeReal, VolumeReal>::Pointer resamplerMax = 
			itk::ResampleImageFilter<VolumeReal, VolumeReal>::New();
		resamplerMax->SetInput(m_volGroupMaxVar);

		typedef itk::AffineTransform<REAL, 3> TransformType;
		TransformType::Pointer transform = TransformType::New();
		transform->SetIdentity();
		resamplerMax->SetTransform(transform);

		typedef itk::LinearInterpolateImageFunction<VolumeReal, REAL> InterpolatorType;
		InterpolatorType::Pointer interpolator = InterpolatorType::New();
		resamplerMax->SetInterpolator( interpolator );

		VolumeReal::Pointer pPointToVolume = static_cast<VolumeReal*>(m_volGroupMainMaxVar);
		resamplerMax->SetOutputParametersFromImage(pPointToVolume);
		resamplerMax->Update();
		CopyImage<VOXEL_REAL, 3>(resamplerMax->GetOutput(), m_volGroupMainMaxVar);




		ConformTo<VOXEL_REAL,3>(pVolume, m_volGroupMainMinVar);
		m_volGroupMainMinVar->FillBuffer(0.0); 
		//Resample(m_volGroupMinVar, m_volGroupMainMinVar, TRUE);
		//Resample3D(m_volGroupMinVar, m_volGroupMainMinVar, TRUE);


		itk::ResampleImageFilter<VolumeReal, VolumeReal>::Pointer resamplerMin = 
			itk::ResampleImageFilter<VolumeReal, VolumeReal>::New();
		resamplerMin->SetInput(m_volGroupMinVar);

		//typedef itk::AffineTransform<REAL, 3> TransformType;
		//TransformType::Pointer transform = TransformType::New();
		//transform->SetIdentity();
		resamplerMin->SetTransform(transform);

		//typedef itk::LinearInterpolateImageFunction<VolumeReal, REAL> InterpolatorType;
		//InterpolatorType::Pointer interpolator = InterpolatorType::New();
		resamplerMin->SetInterpolator( interpolator );

		/*VolumeReal::Pointer*/ pPointToVolume = static_cast<VolumeReal*>(m_volGroupMainMinVar);
		resamplerMin->SetOutputParametersFromImage(pPointToVolume);
		resamplerMin->Update();
		CopyImage<VOXEL_REAL, 3>(resamplerMin->GetOutput(), m_volGroupMainMinVar);



		/// TODO check this
		ConformTo<VOXEL_REAL,3>(m_volMainMinVar, m_volTemp);	
		// Accumulate<VOXEL_REAL>(m_volGroupMainMinVar, 1.0,
		Accumulate3D<VOXEL_REAL>(m_volGroupMainMinVar, 1.0,		
			m_volMainMinVar, m_volTemp);

		ConformTo<VOXEL_REAL,3>(m_volMainMaxVar, m_volTemp);	
		// Accumulate<VOXEL_REAL>(m_volGroupMainMaxVar, 1.0, 
		Accumulate3D<VOXEL_REAL>(m_volGroupMainMaxVar, 1.0, 
			m_volMainMaxVar, m_volTemp); 
	}

	// and sum to histo volume
	ConformTo<VOXEL_REAL,3>(pVolume, m_volTemp);
	//Accumulate<VOXEL_REAL>(m_volMainMaxVar, 1.0,
	Accumulate3D<VOXEL_REAL>(m_volMainMaxVar, 1.0,
		pVolume, m_volTemp); 
	//Accumulate<VOXEL_REAL>(m_volMainMinVar, 1.0, 
	Accumulate3D<VOXEL_REAL>(m_volMainMinVar, 1.0, 
		pVolume, m_volTemp); 

	// now calculate fractions
	/// TODO: make this a normal (i.e. itk::Image parametered) call
	DivVoxels(m_volMainMaxVar->GetBufferPointer(), 
		m_volMainMaxVar->GetBufferedRegion().GetSize()[0],
		pVolume->GetBufferPointer(), 
		pVolume->GetBufferedRegion().GetSize()[0], 
		m_volMainMaxVar->GetBufferedRegion().GetSize()); 

	DivVoxels(m_volMainMinVar->GetBufferPointer(),
		m_volMainMinVar->GetBufferedRegion().GetSize()[0],
		pVolume->GetBufferPointer(), 
		pVolume->GetBufferedRegion().GetSize()[0], 
		m_volMainMinVar->GetBufferedRegion().GetSize());

	// fire change???
	EndLogSection();


}	// Prescription::CalcSumSigmoid

///////////////////////////////////////////////////////////////////////////////
void 
	Prescription::Transform(CVectorN<> *pvInOut) const
	// transform function from linear to sigmoid parameter space
{
	ITERATE_VECTOR((*pvInOut), nAt, (*pvInOut)[nAt] = 
		SIGMOID_SCALE * Sigmoid((*pvInOut)[nAt], m_inputScale));

}	// Prescription::Transform

///////////////////////////////////////////////////////////////////////////////
void 
	Prescription::dTransform(CVectorN<> *pvInOut) const
	// derivative transform function from linear to sigmoid parameter space
{
	ITERATE_VECTOR((*pvInOut), nAt, (*pvInOut)[nAt] = 
		SIGMOID_SCALE * dSigmoid((*pvInOut)[nAt], m_inputScale));

}	// Prescription::dTransform

///////////////////////////////////////////////////////////////////////////////
void 
	Prescription::InvTransform(CVectorN<> *pvInOut) const
	// inverse transform function from linear to sigmoid parameter space
{
	ITERATE_VECTOR((*pvInOut), nAt, (*pvInOut)[nAt] = 
		InvSigmoid((*pvInOut)[nAt] / SIGMOID_SCALE, m_inputScale));

}	// Prescription::InvTransform

///////////////////////////////////////////////////////////////////////////////
void 
	Prescription::GetBeamletFromSVElem(int nElem, int *pnBeam, int *pnBeamlet) const
{
	int nBeamletCount = m_pPlan->GetBeamAt(0)->GetBeamletCount();
	(*pnBeam) = nElem / nBeamletCount;
	(*pnBeamlet) = nElem % nBeamletCount - nBeamletCount / 2;

}	// Prescription::GetBeamletFromSVElem

///////////////////////////////////////////////////////////////////////////////
void 
	Prescription::UpdateHistogramRegions()
	// helper to update the histogram regions
{
	// set up the histogram regions
	for (POSITION pos = m_mapVOITs.GetStartPosition(); pos != NULL;)
	{
		Structure *pStruct = NULL;
		VOITerm *pVOIT = NULL;
		m_mapVOITs.GetNextAssoc(pos, pStruct, pVOIT);

		// now reset the conform regions for the VOIT
		VolumeReal *pResampRegion = pVOIT->GetVOI()->GetConformRegion(m_sumVolume);

		// set histogram options
		CHistogram *pHisto = pVOIT->GetHistogram();
		pHisto->SetRegion(pResampRegion);
	}
}

/// TODO: figure out where SetElementInclude is / should be called
/// TODO: figure out where SetElementInclude is / should be called
/// TODO: figure out where SetElementInclude is / should be called

///////////////////////////////////////////////////////////////////////////////
void 
	Prescription::SetElementInclude()
	// determines array of flags of beamlets (elements) to include in optimization
{
	// iterate over terms to find target term
	POSITION pos = m_mapVOITs.GetStartPosition();
	while (pos != NULL)
	{
		Structure *pStruct = NULL;
		VOITerm *pVOIT = NULL;
		m_mapVOITs.GetNextAssoc(pos, pStruct, pVOIT);

		CHistogramWithGradient *pHisto = pVOIT->GetHistogram();

		if (m_arrIncludeElement.GetSize() < pHisto->Get_dVolumeCount())
		{
			m_arrIncludeElement.SetSize(pHisto->Get_dVolumeCount());

			for (int nAt = 0; nAt < m_arrIncludeElement.GetSize(); nAt++)
			{
				m_arrIncludeElement[nAt] = true;
			}
		}

		// check if its a target
		if (Structure::eTARGET == pVOIT->GetVOI()->GetType()) 
		{
			// iterate over beamlets, seeing which are included in target term
			for (int nAt = 0; nAt < pHisto->Get_dVolumeCount(); nAt++)
			{
				// TODO: fix this for multiple targets
				m_arrIncludeElement[nAt] = 
					m_arrIncludeElement[nAt]
						|| pHisto->IsContributing(nAt);
			}
		}
	}

}	// Prescription::SetElementInclude

}	// namespace dH