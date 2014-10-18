// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: HistogramGradient.cpp 619 2009-03-01 17:43:35Z dglane001 $
#include "StdAfx.h"
#include "HistogramGradient.h"
#include <itkResampleImageFilter.h>
#include <itkAffineTransform.h>

#ifdef USE_IPP
#include <ippi.h>
#endif

//////////////////////////////////////////////////////////////////////
CHistogramWithGradient::CHistogramWithGradient()
: vInput(NULL)
, vInputTrans(NULL)
{
	m_groupVolBinScaled = VolumeReal::New();
}

//////////////////////////////////////////////////////////////////////
CHistogramWithGradient::~CHistogramWithGradient(void)
{
}


//////////////////////////////////////////////////////////////////////
int 
	CHistogramWithGradient::Get_dVolumeCount() const
	// returns the count of dVolumes
{
	return (int) m_arr_dVolumes.size();

}	// CHistogramWithGradient::Get_dVolumeCount

//////////////////////////////////////////////////////////////////////
int CHistogramWithGradient::GetGroupCount() const
	// returns the count of groups of dVolumes
{
	int nMaxGroup = -1;
	for (int nAt = 0; nAt < m_arrVolumeGroups.GetSize(); nAt++)
	{
		nMaxGroup = __max(nMaxGroup, m_arrVolumeGroups[nAt]);
	}

	return nMaxGroup+1;

}	// CHistogramWithGradient::GetGroupCount

//////////////////////////////////////////////////////////////////////
VolumeReal *
	CHistogramWithGradient::Get_dVolume(int nAt, int *pnGroup) const
	// returns the dVolume
{
	if (pnGroup)
	{
		(*pnGroup) = m_arrVolumeGroups[nAt];
	}

	return m_arr_dVolumes[nAt];

}	// CHistogramWithGradient::Get_dVolume

//////////////////////////////////////////////////////////////////////
int 
	CHistogramWithGradient::Add_dVolume(VolumeReal *p_dVolume, int nGroup)
	// adds another dVolume
{
	m_arr_dVolumes.push_back(p_dVolume); 
	int nNewVolumeIndex = (int) m_arr_dVolumes.size()-1; 
	m_arrVolumeGroups.Add(nGroup);
	while (m_groupVolBinLoInt.size() <= (size_t) nGroup)
	{
		m_groupVolBinLoInt.push_back(VolumeShort::New());
		m_arr_bRecomputeBinVolume.Add(TRUE);

		m_groupVolBinFracHi.push_back(VolumeReal::New());
		m_groupVolBinFracLo.push_back(VolumeReal::New());
		m_groupVolBinFracHi_x_dVolume.push_back(VolumeReal::New());
		m_groupVolBinFracLo_x_dVolume.push_back(VolumeReal::New());

		// Just add a NULL for region rotate, because the logic below will initialize it when
		//		a dVolume is available
		m_groupVolRegion.push_back(NULL);
	} 

	// see if the region rotate is in need of initialization
	if (m_groupVolRegion[nGroup].IsNull()) 
	{
		// rotate region
		m_groupVolRegion[nGroup] = VolumeReal::New();
		ConformTo<VOXEL_REAL,3>(p_dVolume, m_groupVolRegion[nGroup]);
		m_groupVolRegion[nGroup]->FillBuffer(0.0);
		//Resample(GetRegion(), m_groupVolRegion[nGroup], TRUE);
		//Resample3D(GetRegion(), m_groupVolRegion[nGroup], TRUE);
		itk::ResampleImageFilter<VolumeReal, VolumeReal>::Pointer resampler = 
			itk::ResampleImageFilter<VolumeReal, VolumeReal>::New();
		resampler->SetInput(GetRegion());

		typedef itk::AffineTransform<REAL, 3> TransformType;
		TransformType::Pointer transform = TransformType::New();
		transform->SetIdentity();
		resampler->SetTransform(transform);

		typedef itk::LinearInterpolateImageFunction<VolumeReal, REAL> InterpolatorType;
		InterpolatorType::Pointer interpolator = InterpolatorType::New();
		resampler->SetInterpolator( interpolator );

		resampler->SetOutputParametersFromImage(m_groupVolRegion[nGroup]);
		resampler->Update();
		CopyImage<VOXEL_REAL, 3>(resampler->GetOutput(), m_groupVolRegion[nGroup]);
	}

	// set flag for computing bins for new dVolume
	m_arr_bRecompute_dBins.Add(TRUE);

	// add new product volume
	VolumeReal::Pointer p_dVolume_x_Region = VolumeReal::New();
	ConformTo<VOXEL_REAL,3>(p_dVolume, p_dVolume_x_Region);
	m_arr_dVolumes_x_Region.push_back(p_dVolume_x_Region); 

	// add the derivative bins
	m_arr_dBins.SetSize(Get_dVolumeCount());
	m_arr_dGBins.SetSize(Get_dVolumeCount());

	// flag to recompute 
	m_arr_bRecompute_dVolumes_x_Region.Add(TRUE);

	return nNewVolumeIndex;

}	// CHistogramWithGradient::Add_dVolume


//////////////////////////////////////////////////////////////////////
const CVectorN<>& 
	CHistogramWithGradient::Get_dBins(int nAt_dBin) const
	// computes and returns the d/dx bins
{
	// recompute dBins if needed
	if (m_arr_bRecompute_dBins[nAt_dBin])
	{
		// set size of dBins & dGBins

		// initialize reference to proper dBins and zero
		CVectorN<>& arr_dBins = m_arr_dBins[nAt_dBin];
		VOXEL_REAL maxValue = GetMax<VOXEL_REAL>(GetVolume());
		int nBins = GetBinForValue(maxValue)+2;
		arr_dBins.SetDim(nBins);
		arr_dBins.SetZero();

		// now compute bins
		if (GetRegion())
		{
			// get dVoxels * Region
			Get_dVolume_x_Region(nAt_dBin);

			// get the bin voxels, recompute if needed
			GetBinVolume(nAt_dBin);

			int nGroup = m_arrVolumeGroups[nAt_dBin];

			// and do the binning
			/// TODO: extend this to non-zed planar / 3D
			int nCount = GetRegion()->GetBufferedRegion().GetSize()[0] 
				* GetRegion()->GetBufferedRegion().GetSize()[1]; 
			int nStrideZ = GetRegion()->GetBufferedRegion().GetSize()[0] 
				* GetRegion()->GetBufferedRegion().GetSize()[1];
			const VolumeShort *pBinVolume = GetBinVolume(nAt_dBin);
			for (int nZ = 0; nZ < /*1*/ GetRegion()->GetBufferedRegion().GetSize()[2]; nZ++)
			{
				for (int nAtVoxel = /*0*/ /*GetSlice()*/nZ * nCount;
					nAtVoxel < /*nCount*/((/*GetSlice()*/nZ+1) * nCount); nAtVoxel++)
				//for (int nAtVoxel = /*0*/GetSlice() * nCount; 
				//	nAtVoxel < /*nCount*/((GetSlice()+1) * nCount); nAtVoxel++)
				{
					int nBin = pBinVolume->GetBufferPointer()[nAtVoxel]; 
					arr_dBins[nBin] -= m_groupVolBinFracLo_x_dVolume[nGroup]->GetBufferPointer()[nAtVoxel]; 
					arr_dBins[nBin+1] += m_groupVolBinFracHi_x_dVolume[nGroup]->GetBufferPointer()[nAtVoxel]; 
				}
			}
		}
		else
		{
			/// TODO: extend this to non-zed planar / 3D
			/// ACTUALLY -- is this being called anywhere?

			// get the dVoxels
			const VOXEL_REAL *p_dVoxels = Get_dVolume(nAt_dBin)->GetBufferPointer(); ;

			const short *pBinVolumeVoxels = GetBinVolume(nAt_dBin)->GetBufferPointer(); 

			for (int nAtVoxel = 0; nAtVoxel < GetVolume()->GetBufferedRegion().GetNumberOfPixels(); nAtVoxel++)
			{
				int nBin = pBinVolumeVoxels[nAtVoxel];
				arr_dBins[nBin] += -p_dVoxels[nAtVoxel];
			}
		}

		// convolve for dGBins, if needed
		REAL binKernelSigma = sqrt(m_varMax);
		if (binKernelSigma > 0.0)
		{	
			static CVectorN<> arr_dGBinsVarMin;
			static CVectorN<> arr_dGBinsVarMax;

			Conv_dGauss(arr_dBins, m_bin_dKernelVarMax, arr_dGBinsVarMax);
			Conv_dGauss(arr_dBins, m_bin_dKernelVarMin, arr_dGBinsVarMin);
			ASSERT(arr_dGBinsVarMax.GetDim() == arr_dGBinsVarMin.GetDim());


			// determine variance using dSigmoid
			REAL varSlope = 1.0;
			REAL varWeight = 1.0;
			REAL m_inputScale = 0.5;	// should get this from the registry
			const REAL SIGMOID_SCALE = 0.2; // 0.1; // 0.3; // 0.1; // 1.0;
				// should get this from Prescription
			// calculate variance adjustment due to sigmoid transform
			varSlope = 
				SIGMOID_SCALE * dSigmoid<REAL>((*vInput)[nAt_dBin], m_inputScale);

			// this is equivalent to scaling the level sigma's so that their current
			//	value is the equal to that at optimizer value -4.0
			varSlope /= SIGMOID_SCALE * dSigmoid<REAL>(0.0, m_inputScale);

			// compute the variance adjustment for the beamlet weight
			varWeight = (*vInputTrans)[nAt_dBin];

			// normalize so that beamlet weight at scale / 2 is 1.0
			varWeight /= SIGMOID_SCALE / 2.0;
			REAL actVar = (*m_pAV)[nAt_dBin] * varSlope * varSlope * varWeight * varWeight;

			REAL fracMax = (actVar - m_varMin) / (m_varMax - m_varMin);
			fracMax = __min(fracMax, 1.0);
			fracMax = __max(fracMax, 0.0);
			const REAL fracMin = 1.0 - fracMax; 

			arr_dGBinsVarMax *= fracMax;
			arr_dGBinsVarMin *= fracMin;

			m_arr_dGBins[nAt_dBin].SetDim(arr_dGBinsVarMax.GetDim());
			m_arr_dGBins[nAt_dBin] = arr_dGBinsVarMax;
			m_arr_dGBins[nAt_dBin] += arr_dGBinsVarMin;

			// now normalize
			REAL calcSum = 0.0;
#ifdef STANDARD_SUM
			calcSum = GetSum<VOXEL_REAL>(GetRegion());
#else
			// NOTE: this needs to cover the same voxels as the above binning loop
			int nCount = GetRegion()->GetBufferedRegion().GetSize()[0] 
				* GetRegion()->GetBufferedRegion().GetSize()[1];
			for (int nZ = 0; nZ < /*1*/GetRegion()->GetBufferedRegion().GetSize()[2]; nZ++)
			{
				for (int nAtVoxel = /*0*/ /*GetSlice()*/nZ * nCount;
					nAtVoxel < /*nCount*/((/*GetSlice()*/nZ+1) * nCount); nAtVoxel++)
				//for (int nAtVoxel = /*0*/GetSlice() * nCount; 
				//	nAtVoxel < /*nCount*/((GetSlice()+1) * nCount); nAtVoxel++)
				{
					calcSum += GetRegion()->GetBufferPointer()[nAtVoxel]; 
				}
			}
#endif
			if (calcSum > 0.0)
			{
				// normalize this bin
				m_arr_dGBins[nAt_dBin] *= R(1.0 / ((double) calcSum));
			}
		}
		m_arr_bRecompute_dBins[nAt_dBin] = FALSE;
	}

	return m_arr_dBins[nAt_dBin];

}	// CHistogramWithGradient::Get_dBins


//////////////////////////////////////////////////////////////////////
const CVectorN<>& 
	CHistogramWithGradient::Get_dGBins(int nAt/*dBin*/) const
	// computes and returns the d/dx GHistogram
{
	if (m_varMax == 0.0)
	{
		ASSERT(FALSE);	// not OK, because we need to normalize
		return Get_dBins(nAt);
	}

	Get_dBins(nAt);

	return m_arr_dGBins[(int) nAt];

}	// CHistogramWithGradient::Get_dGBins


//////////////////////////////////////////////////////////////////////
const VolumeReal * 
	CHistogramWithGradient::Get_dVolume_x_Region(int nAt/*Group*/) const
	// calculates / returns the masked dVolume
{
	if (m_arr_bRecompute_dVolumes_x_Region[nAt])
	{
		int nGroup = m_arrVolumeGroups[nAt];

		typedef itk::ImageRegionConstIterator< VolumeReal > ConstIteratorType;
		typedef itk::ImageRegionIterator< VolumeReal > IteratorType;

		IteratorType dstIt( m_arr_dVolumes_x_Region[nAt], m_arr_dVolumes_x_Region[nAt]->GetBufferedRegion() );
		ConstIteratorType groupVolRegionIt( m_groupVolRegion[nGroup], m_groupVolRegion[nGroup]->GetBufferedRegion() );
		ConstIteratorType dVolIt( Get_dVolume(nAt), Get_dVolume(nAt)->GetBufferedRegion() );
		for ( dstIt.GoToBegin(), groupVolRegionIt.GoToBegin(), dVolIt.GoToBegin(); 
			!dstIt.IsAtEnd(); ++dstIt, ++groupVolRegionIt, ++dVolIt )
		{
			dstIt.Set(groupVolRegionIt.Get() * dVolIt.Get());
		}

		/// TODO: extend to 3D
		//int nStrideZ = m_groupVolRegion[nGroup]->GetBufferedRegion().GetSize()[0] 
		//	* m_groupVolRegion[nGroup]->GetBufferedRegion().GetSize()[1];
		//for (int nZ = 0; nZ < m_groupVolRegion[nGroup]->GetBufferedRegion().GetSize()[2]; nZ++)
		//{
		//	CK_IPP( ippiMul_32f_C1R(
		//		&m_groupVolRegion[nGroup]->GetBufferPointer()[nZ * nStrideZ], Stride<VOXEL_REAL,3>(m_groupVolRegion[nGroup]), 
		//		&Get_dVolume(nAt)->GetBufferPointer()[nZ * nStrideZ], Stride<VOXEL_REAL,3>(Get_dVolume(nAt)), 
		//		&m_arr_dVolumes_x_Region[nAt]->GetBufferPointer()[nZ * nStrideZ], Stride<VOXEL_REAL,3>(m_arr_dVolumes_x_Region[nAt]), 
		//		MakeIppiSize<3>(m_arr_dVolumes_x_Region[nAt]->GetBufferedRegion())) ); 
		//}

		m_arr_bRecompute_dVolumes_x_Region[nAt] = FALSE;
	}

	return m_arr_dVolumes_x_Region[nAt];

}	// CHistogramWithGradient::Get_dVolume_x_Region


//////////////////////////////////////////////////////////////////////
const VolumeShort *
	CHistogramWithGradient::GetBinVolume(int nAt/*Group*/) const
	// calculates / returns the bin volume
{
	int nGroup = m_arrVolumeGroups[nAt];

	if (m_arr_bRecomputeBinVolume[nGroup])
	{
		CalcBinningVolumes();

		// rotate bin scaled volume to group orientation
		ConformTo<VOXEL_REAL,3>(Get_dVolume(nAt), m_groupVolBinScaled);
		m_groupVolBinScaled->FillBuffer(0.0);
		// Resample(m_volBinScaled, m_groupVolBinScaled, TRUE);
		// Resample3D(m_volBinScaled, m_groupVolBinScaled, TRUE);

		itk::ResampleImageFilter<VolumeReal, VolumeReal>::Pointer resampler = 
			itk::ResampleImageFilter<VolumeReal, VolumeReal>::New();
		resampler->SetInput(m_volBinScaled);

		typedef itk::AffineTransform<REAL, 3> TransformType;
		TransformType::Pointer transform = TransformType::New();
		transform->SetIdentity();
		resampler->SetTransform(transform);

		typedef itk::LinearInterpolateImageFunction<VolumeReal, REAL> InterpolatorType;
		InterpolatorType::Pointer interpolator = InterpolatorType::New();
		resampler->SetInterpolator( interpolator );

		resampler->SetOutputParametersFromImage(m_groupVolBinScaled);
		resampler->Update();
		CopyImage<VOXEL_REAL, 3>(resampler->GetOutput(), m_groupVolBinScaled);


		// now convert to integer bin values

		// get the main volume voxels
		ConformTo<short,3>(m_groupVolBinScaled, m_groupVolBinLoInt[nGroup]);
		m_groupVolBinLoInt[nGroup]->FillBuffer(0.0); 

		typedef itk::ImageRegionConstIterator< VolumeReal > ConstIteratorType;
		typedef itk::ImageRegionIterator< VolumeReal > IteratorType;
		typedef itk::ImageRegionIterator< VolumeShort > ShortIteratorType;
		typedef itk::ImageRegionConstIterator< VolumeShort > ShortConstIteratorType;

		{
		ShortIteratorType dstIt( m_groupVolBinLoInt[nGroup], m_groupVolBinLoInt[nGroup]->GetBufferedRegion() );
		ConstIteratorType groupVolBinScaledIt( m_groupVolBinScaled, m_groupVolBinScaled->GetBufferedRegion() );
		for ( dstIt.GoToBegin(), groupVolBinScaledIt.GoToBegin(); 
			!dstIt.IsAtEnd(); ++dstIt, ++groupVolBinScaledIt )
		{
			dstIt.Set((short)floor(groupVolBinScaledIt.Get()));
		}
		}

		/// TODO: extend to 3D
		int nStrideZ = m_groupVolRegion[nGroup]->GetBufferedRegion().GetSize()[0] 
			* m_groupVolRegion[nGroup]->GetBufferedRegion().GetSize()[1];
		//for (int nZ = 0; nZ < m_groupVolRegion[nGroup]->GetBufferedRegion().GetSize()[2]; nZ++)
		//{
		//	CK_IPP( ippiConvert_32f16s_C1R(
		//		&m_groupVolBinScaled->GetBufferPointer()[nZ * nStrideZ], Stride<VOXEL_REAL,3>(m_groupVolBinScaled), 
		//		&m_groupVolBinLoInt[nGroup]->GetBufferPointer()[nZ * nStrideZ], Stride<short,3>(m_groupVolBinLoInt[nGroup]), 
		//		MakeIppiSize<3>(m_groupVolRegion[nGroup]->GetBufferedRegion()),
		//		ippRndZero) ); // DGL: using zero for interp // ippRndNear) );
		//}

		////////////////////////////////////////////////////////////////////
		// get bin frac volume

		ConformTo<VOXEL_REAL,3>(m_groupVolBinLoInt[nGroup], m_groupVolBinFracHi[nGroup]);

		{
		IteratorType dstIt(m_groupVolBinFracHi[nGroup], m_groupVolBinFracHi[nGroup]->GetBufferedRegion() );
		ShortConstIteratorType groupVolBinLoIntIt( m_groupVolBinLoInt[nGroup], m_groupVolBinLoInt[nGroup]->GetBufferedRegion() );
		for ( dstIt.GoToBegin(), groupVolBinLoIntIt.GoToBegin(); 
			!dstIt.IsAtEnd(); ++dstIt, ++groupVolBinLoIntIt)
		{
			dstIt.Set(groupVolBinLoIntIt.Get());
		}
		}

		/// TODO: extend to 3D
		//for (int nZ = 0; nZ < m_groupVolRegion[nGroup]->GetBufferedRegion().GetSize()[2]; nZ++)
		//{
		//	CK_IPP( ippiConvert_16s32f_C1R(
		//		&m_groupVolBinLoInt[nGroup]->GetBufferPointer()[nZ * nStrideZ], Stride<short,3>(m_groupVolBinLoInt[nGroup]), 
		//		&m_groupVolBinFracHi[nGroup]->GetBufferPointer()[nZ * nStrideZ], Stride<VOXEL_REAL,3>(m_groupVolBinFracHi[nGroup]),
		//		MakeIppiSize<3>(m_groupVolRegion[nGroup]->GetBufferedRegion())) ); 
		//}

		// leaves Frac = -High Fraction

		{
		IteratorType dstIt(m_groupVolBinFracHi[nGroup], m_groupVolBinFracHi[nGroup]->GetBufferedRegion() );
		ConstIteratorType groupVolBinScaledIt( m_groupVolBinScaled, m_groupVolBinScaled->GetBufferedRegion() );
		for ( dstIt.GoToBegin(), groupVolBinScaledIt.GoToBegin(); 
			!dstIt.IsAtEnd(); ++dstIt, ++groupVolBinScaledIt)
		{
			dstIt.Set(-(groupVolBinScaledIt.Get()-dstIt.Get()));
		}
		}


		/// TODO: extend to 3D
		//for (int nZ = 0; nZ < m_groupVolRegion[nGroup]->GetBufferedRegion().GetSize()[2]; nZ++)
		//{		
		//	CK_IPP( ippiSub_32f_C1IR(
		//		&m_groupVolBinScaled->GetBufferPointer()[nZ * nStrideZ], Stride<VOXEL_REAL,3>(m_groupVolBinScaled),
		//		&m_groupVolBinFracHi[nGroup]->GetBufferPointer()[nZ * nStrideZ], Stride<VOXEL_REAL,3>(m_groupVolBinFracHi[nGroup]),
		//		MakeIppiSize<3>(m_groupVolRegion[nGroup]->GetBufferedRegion())) ); 
		//}

		////////////////////////////////////////////////////////////////////
		// get bin frac low volume

		ConformTo<VOXEL_REAL,3>(m_groupVolBinFracHi[nGroup], m_groupVolBinFracLo[nGroup]);

		{
		IteratorType dstIt(m_groupVolBinFracLo[nGroup], m_groupVolBinFracLo[nGroup]->GetBufferedRegion() );
		ConstIteratorType groupVolBinFracHiIt( m_groupVolBinFracHi[nGroup], m_groupVolBinFracHi[nGroup]->GetBufferedRegion() );
		for ( dstIt.GoToBegin(), groupVolBinFracHiIt.GoToBegin(); 
			!dstIt.IsAtEnd(); ++dstIt, ++groupVolBinFracHiIt)
		{
			dstIt.Set(groupVolBinFracHiIt.Get()+1.0);
		}
		}

		/// TODO: extend to 3D
		//for (int nZ = 0; nZ < m_groupVolRegion[nGroup]->GetBufferedRegion().GetSize()[2]; nZ++)
		//{		
		//	CK_IPP( ippiAddC_32f_C1R(
		//		&m_groupVolBinFracHi[nGroup]->GetBufferPointer()[nZ * nStrideZ], Stride<VOXEL_REAL,3>(m_groupVolBinFracHi[nGroup]), 
		//		1.0,
		//		&m_groupVolBinFracLo[nGroup]->GetBufferPointer()[nZ * nStrideZ], Stride<VOXEL_REAL,3>(m_groupVolBinFracLo[nGroup]), 
		//		MakeIppiSize<3>(m_groupVolRegion[nGroup]->GetBufferedRegion())) );
		//}

		////////////////////////////////////////////////////////////////////
		// now get final bin frac volume

		// scale by region x beamlet
		ConformTo<VOXEL_REAL,3>(m_groupVolBinFracLo[nGroup], m_groupVolBinFracLo_x_dVolume[nGroup]);

		{
		IteratorType dstIt(m_groupVolBinFracLo_x_dVolume[nGroup], m_groupVolBinFracLo_x_dVolume[nGroup]->GetBufferedRegion() );
		ConstIteratorType dVolume_x_RegionIt( Get_dVolume_x_Region(nAt), Get_dVolume_x_Region(nAt)->GetBufferedRegion() );
		ConstIteratorType groupVolBinFracLoIt( m_groupVolBinFracLo[nGroup], m_groupVolBinFracLo[nGroup]->GetBufferedRegion() );
		for ( dstIt.GoToBegin(), dVolume_x_RegionIt.GoToBegin(), groupVolBinFracLoIt.GoToBegin(); 
			!dstIt.IsAtEnd(); ++dstIt, ++dVolume_x_RegionIt, ++groupVolBinFracLoIt )
		{
			dstIt.Set(dVolume_x_RegionIt.Get()*groupVolBinFracLoIt.Get());
		}
		}

		/// TODO: extend to 3D
		//for (int nZ = 0; nZ < m_groupVolRegion[nGroup]->GetBufferedRegion().GetSize()[2]; nZ++)
		//{		
		//	CK_IPP( ippiMul_32f_C1R(
		//		&Get_dVolume_x_Region(nAt)->GetBufferPointer()[nZ * nStrideZ], Stride<VOXEL_REAL,3>(Get_dVolume_x_Region(nAt)), 
		//		&m_groupVolBinFracLo[nGroup]->GetBufferPointer()[nZ * nStrideZ], Stride<VOXEL_REAL,3>(m_groupVolBinFracLo[nGroup]), 
		//		&m_groupVolBinFracLo_x_dVolume[nGroup]->GetBufferPointer()[nZ * nStrideZ], Stride<VOXEL_REAL,3>(m_groupVolBinFracLo_x_dVolume[nGroup]), 
		//		MakeIppiSize<3>(m_arr_dVolumes_x_Region[nAt]->GetBufferedRegion())) ); 
		//}

		// scale by region x beamlet
		ConformTo<VOXEL_REAL,3>(m_groupVolBinFracHi[nGroup], m_groupVolBinFracHi_x_dVolume[nGroup]);

		{
		IteratorType dstIt(m_groupVolBinFracHi_x_dVolume[nGroup], m_groupVolBinFracHi_x_dVolume[nGroup]->GetBufferedRegion() );
		ConstIteratorType dVolume_x_RegionIt( Get_dVolume_x_Region(nAt), Get_dVolume_x_Region(nAt)->GetBufferedRegion() );
		ConstIteratorType groupVolBinFracHiIt( m_groupVolBinFracHi[nGroup], m_groupVolBinFracHi[nGroup]->GetBufferedRegion() );
		for ( dstIt.GoToBegin(), dVolume_x_RegionIt.GoToBegin(), groupVolBinFracHiIt.GoToBegin(); 
			!dstIt.IsAtEnd(); ++dstIt, ++dVolume_x_RegionIt, ++groupVolBinFracHiIt )
		{
			dstIt.Set(dVolume_x_RegionIt.Get()*groupVolBinFracHiIt.Get());
		}
		}

		/// TODO: extend to 3D
		//for (int nZ = 0; nZ < m_groupVolRegion[nGroup]->GetBufferedRegion().GetSize()[2]; nZ++)
		//{
		//	CK_IPP( ippiMul_32f_C1R(
		//		&Get_dVolume_x_Region(nAt)->GetBufferPointer()[nZ * nStrideZ], Stride<VOXEL_REAL,3>(Get_dVolume_x_Region(nAt)),
		//		&m_groupVolBinFracHi[nGroup]->GetBufferPointer()[nZ * nStrideZ], Stride<VOXEL_REAL,3>(m_groupVolBinFracHi[nGroup]), 
		//		&m_groupVolBinFracHi_x_dVolume[nGroup]->GetBufferPointer()[nZ * nStrideZ], Stride<VOXEL_REAL,3>(m_groupVolBinFracHi_x_dVolume[nGroup]), 
		//		MakeIppiSize<3>(m_arr_dVolumes_x_Region[nAt]->GetBufferedRegion())) ); 
		//}

		// flag change
		m_arr_bRecomputeBinVolume[nGroup] = TRUE;
	}

	return m_groupVolBinLoInt[nGroup];

}	// CHistogramWithGradient::GetBinVolume


///////////////////////////////////////////////////////////////////////////////
void CHistogramWithGradient::Conv_dGauss(const CVectorN<>& buffer_in, 
										 const CVectorN<>& kernel_in,
										 CVectorN<>& buffer_out) const
{
	buffer_out.SetDim(buffer_in.GetDim() + kernel_in.GetDim() - 1);
	buffer_out.SetZero();

#ifdef REAL_FLOAT
#error REAL_FLOAT not supported!
	IppStatus stat = ippsConv_32f(
		&buffer_in[0], buffer_in.GetDim(),
		&m_bin_dKernel[0], m_bin_dKernel.GetDim(),
		&buffer_out_temp[0]);
#else
	IppStatus stat = ippsConv_64f(
		&buffer_in[0], buffer_in.GetDim(),
		&kernel_in[0], kernel_in.GetDim(),
		&buffer_out[0]);
	ASSERT(stat == ippStsNoErr);
#endif

}	// CHistogramWithGradient::Conv_dGauss

