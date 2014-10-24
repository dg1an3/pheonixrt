// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: Histogram.cpp 604 2008-09-14 17:01:30Z dglane001 $
#include "stdafx.h"
#include "Histogram.h"

//#ifdef USE_IPP
//#include <ippi.h>
//#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

const REAL GBINS_KERNEL_WIDTH = 8.0; // 4.0;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CHistogram::CHistogram(VolumeReal *pVolume, VolumeReal *pRegion)
	: m_bRecomputeBins(TRUE)
		, m_bRecomputeCumBins(TRUE)

		, m_pVolume(NULL)
		, m_pRegion(NULL)

		, m_bRecomputeBinScaledVolume(TRUE)

		, m_minValue(0.0)		
		, m_binWidth(0.1)		
		, m_pAV(NULL) 
		, m_varMin(0.1 * 0.1) // binKernelSigma^2
		, m_varMax(0.1 * 0.1) // binKernelSigma^2
		, m_Slice(0)
{
	SetVolume(pVolume);
	SetRegion(pRegion);

	SetBinning((REAL) 0.0, (REAL) 0.1, GBINS_BUFFER);
	SetGBinVar(m_pAV, m_varMin, m_varMax);

	m_volBinScaled = VolumeReal::New();
	m_volBinLoInt = VolumeShort::New(); 

	m_volBinFracHi = VolumeReal::New();
	m_volBinFracHi_x_VarFracLo = VolumeReal::New();
	m_volBinFracHi_x_VarFracHi = VolumeReal::New();

	m_volBinFracLo = VolumeReal::New();
	m_volBinFracLo_x_VarFracHi = VolumeReal::New();
	m_volBinFracLo_x_VarFracLo = VolumeReal::New();

	m_volRegion_x_VarFracHi = VolumeReal::New();
	m_volRegion_x_VarFracLo = VolumeReal::New();

}	// CHistogram::CHistogram

//////////////////////////////////////////////////////////////////////
CHistogram::~CHistogram()
{
}	// CHistogram::~CHistogram

//////////////////////////////////////////////////////////////////////
void 
	CHistogram::SetVolume(VolumeReal *pVolume)
{
	// set the pointer
	m_pVolume = pVolume;

	// update for new volume
	OnVolumeChange(); // NULL, NULL);

	// fire a change event
	//GetChangeEvent().Fire();
	this->DataHasBeenGenerated(); // Modified();

}	// CHistogram::SetVolume

//////////////////////////////////////////////////////////////////////
void 
	CHistogram::SetRegion(VolumeReal *pRegion)
	// sets the computation region for the histogram
{
	// set the pointer
	m_pRegion = pRegion;

	// trigger update
	OnRegionChanged(); // NULL, NULL);

	// flag recomputation
	// m_bRecomputeBins = TRUE;
	// m_bRecomputeCumBins = TRUE;

	// fire a change event
	//GetChangeEvent().Fire();
	this->DataHasBeenGenerated(); // Modified();

}	// CHistogram::SetRegion

//////////////////////////////////////////////////////////////////////
REAL 
	CHistogram::GetBinMinValue() const
	// returns the minimum bin value
{
	return m_minValue;

}	// CHistogram::GetBinMinValue

//////////////////////////////////////////////////////////////////////
REAL 
	CHistogram::GetBinWidth() const
	// returns the bin step value
{
	return m_binWidth;

}	// CHistogram::GetBinWidth

//////////////////////////////////////////////////////////////////////
void 
	CHistogram::SetBinning(REAL min_value, REAL width, REAL sigma_mult)
	// sets up the binning parameters
{
	// only tested with this condition
	ASSERT(sigma_mult == GBINS_BUFFER);

	m_minValue = min_value - sigma_mult * sqrt(m_varMax); // m_binKernelSigma;
	m_binWidth = width;

	// recalculate kernels
	SetGBinVar(m_pAV, m_varMin, m_varMax);

	// no need to fire change here, because SetGBinVar does it
	// fire binning change event
	// GetBinningChangeEvent().Fire();

}	// CHistogram::SetBinning

//////////////////////////////////////////////////////////////////////
const CVectorN<>& 
	CHistogram::GetBinMeans() const
	// returns a vector with mean bin values	
{
	m_arrBinMeans.SetDim(m_arrBins.GetDim());
	for (int nAt = 0; nAt < m_arrBinMeans.GetDim(); nAt++)
	{
		m_arrBinMeans[nAt] = m_minValue + (REAL) nAt * m_binWidth;
	}

	return m_arrBinMeans;

}	// CHistogram::GetBinMeans


//////////////////////////////////////////////////////////////////////
REAL 
	CHistogram::GetGBinVarMin(void) const
	// returns the GBin variance max parameter
{
	return m_varMin;

}	// CHistogram::GetGBinVarMin

//////////////////////////////////////////////////////////////////////
REAL 
	CHistogram::GetGBinVarMax(void) const
{
	return m_varMax;

}	// CHistogram::GetGBinVarMax

//////////////////////////////////////////////////////////////////////
void 
	CHistogram::SetGBinVar(CVectorN<> *pAV, REAL varMin, REAL varMax)
	// sets up the gbinning variance parameters
{
	m_pAV = pAV; 
	m_varMin = varMin;
	m_varMax = varMax;

	REAL dx = GetBinWidth();

	REAL binKernelSigmaMax = sqrt(varMax);
    int nNeighborhood = (int) ceil(GBINS_KERNEL_WIDTH * binKernelSigmaMax / GetBinWidth());
	m_binKernelVarMax.SetDim(nNeighborhood * 2 + 1);
    for (int nZ = -nNeighborhood; nZ <= nNeighborhood; nZ++)
	{
        m_binKernelVarMax[nZ + nNeighborhood] = 
			dx * Gauss<REAL>(nZ * dx, binKernelSigmaMax);
	}

#ifdef _DEBUG
	double sum = 0.0;
	ITERATE_VECTOR(m_binKernelVarMax, nAt, sum += m_binKernelVarMax[nAt]);
	ASSERT(IsApproxEqual(sum, 1.0));
#endif

	m_bin_dKernelVarMax.SetDim(nNeighborhood * 2 + 1);
    for (int nZ = -nNeighborhood; nZ <= nNeighborhood; nZ++)
	{
        m_bin_dKernelVarMax[nZ + nNeighborhood] = 
			dx * dGauss<REAL>(-nZ * dx, binKernelSigmaMax);
	}

	REAL binKernelSigmaMin = sqrt(varMin);
	m_binKernelVarMin.SetDim(nNeighborhood * 2 + 1);
    for (int nZ = -nNeighborhood; nZ <= nNeighborhood; nZ++)
	{
        m_binKernelVarMin[nZ + nNeighborhood] = 
			dx * Gauss<REAL>(nZ * dx, binKernelSigmaMin);
	}

	m_bin_dKernelVarMin.SetDim(nNeighborhood * 2 + 1);
    for (int nZ = -nNeighborhood; nZ <= nNeighborhood; nZ++)
	{
        m_bin_dKernelVarMin[nZ + nNeighborhood] = 
			dx * dGauss<REAL>(-nZ * dx, binKernelSigmaMin);
	}

	// make sure recompute bin scaled volume
	m_bRecomputeBinScaledVolume = TRUE;

	// fire binning change event
	//GetBinningChangeEvent().Fire();

	// also firing main for legacy reasons
	//GetChangeEvent().Fire();
	this->DataHasBeenGenerated(); // Modified();

}	// CHistogram::SetGBinVar

//////////////////////////////////////////////////////////////////////
const CVectorN<>& 
	CHistogram::GetKernelVarMin(void) const
{
	return m_binKernelVarMin;

}	// CHistogram::GetKernelVarMin
	
//////////////////////////////////////////////////////////////////////
const CVectorN<>& 
	CHistogram::GetKernelVarMax(void) const
{
	return m_binKernelVarMax;

}	// CHistogram::GetKernelVarMax

//////////////////////////////////////////////////////////////////////
void 
	CHistogram::SetVarFracVolumes(VolumeReal *volVarFracLo, VolumeReal *volVarFracHi)
{
	typedef itk::ImageRegionConstIterator< VolumeReal > ConstIteratorType;
	typedef itk::ImageRegionIterator< VolumeReal > IteratorType;

	CopyImage<VOXEL_REAL,3>(volVarFracLo, m_volRegion_x_VarFracLo);

	/// TODO: extend this to 3D
	int nStrideZ = m_pRegion->GetBufferedRegion().GetSize()[0] 
		* m_pRegion->GetBufferedRegion().GetSize()[1];
	//for (int nZ = 0; nZ < m_pRegion->GetBufferedRegion().GetSize()[2]; nZ++)
	//{
	//	CK_IPP(ippiMul_32f_C1IR(
	//		&m_pRegion->GetBufferPointer()[nZ * nStrideZ],	Stride<VOXEL_REAL,3>(m_pRegion),
	//		&m_volRegion_x_VarFracLo->GetBufferPointer()[nZ * nStrideZ], Stride<VOXEL_REAL,3>(m_volRegion_x_VarFracLo), 
	//		MakeIppiSize<3>(m_pRegion->GetBufferedRegion())));
	//}

	ConstIteratorType regionIt( m_pRegion, m_pRegion->GetBufferedRegion() );

	{
	IteratorType srcDstIt( m_volRegion_x_VarFracLo, m_volRegion_x_VarFracLo->GetBufferedRegion() );
	for ( srcDstIt.GoToBegin(), regionIt.GoToBegin(); 
		!srcDstIt.IsAtEnd(); ++srcDstIt, ++regionIt)
	{
		srcDstIt.Set(srcDstIt.Get()*regionIt.Get());
	}
	}

	CopyImage<VOXEL_REAL,3>(volVarFracHi, m_volRegion_x_VarFracHi); 

	/// TODO: extend this to 3D
	//for (int nZ = 0; nZ < m_pRegion->GetBufferedRegion().GetSize()[2]; nZ++)
	//{
	//	CK_IPP(ippiMul_32f_C1IR(
	//		&m_pRegion->GetBufferPointer()[nZ * nStrideZ], Stride<VOXEL_REAL,3>(m_pRegion),
	//		&m_volRegion_x_VarFracHi->GetBufferPointer()[nZ * nStrideZ], Stride<VOXEL_REAL,3>(m_volRegion_x_VarFracHi),
	//		MakeIppiSize<3>(m_pRegion->GetBufferedRegion())));
	//}

	{
	IteratorType srcDstIt( m_volRegion_x_VarFracHi, m_volRegion_x_VarFracHi->GetBufferedRegion() );
	for ( srcDstIt.GoToBegin(), regionIt.GoToBegin(); 
		!srcDstIt.IsAtEnd(); ++srcDstIt, ++regionIt)
	{
		srcDstIt.Set(srcDstIt.Get()*regionIt.Get());
	}
	}


}	// CHistogram::SetVarFracVolumes


//////////////////////////////////////////////////////////////////////
void //const VolumeReal * 
	CHistogram::CalcBinningVolumes() const
	// calculates / returns the bin scaled volume
{
	if (true) // m_bRecomputeBinScaledVolume)
	{
		typedef itk::ImageRegionConstIterator< VolumeReal > ConstIteratorType;
		typedef itk::ImageRegionIterator< VolumeReal > IteratorType;

		ConformTo<VOXEL_REAL,3>(m_pVolume, m_volBinScaled);
		m_volBinScaled->FillBuffer(0.0);

		// subtract min,
		/// TODO: extend this to 3D
		int nStrideZ = m_pRegion->GetBufferedRegion().GetSize()[0] 
			* m_pRegion->GetBufferedRegion().GetSize()[1];

		{
		IteratorType dstIt( m_volBinScaled, m_volBinScaled->GetBufferedRegion() );
		ConstIteratorType volumeIt( m_pVolume, m_pVolume->GetBufferedRegion() );
		for ( dstIt.GoToBegin(), volumeIt.GoToBegin(); 
			!dstIt.IsAtEnd(); ++dstIt, ++volumeIt)
		{
			dstIt.Set(volumeIt.Get()-m_minValue);
		}
		}

		//for (int nZ = 0; nZ < m_pRegion->GetBufferedRegion().GetSize()[2]; nZ++)
		//{
		//	CK_IPP( ippiSubC_32f_C1R(
		//		&m_pVolume->GetBufferPointer()[nZ * nStrideZ], Stride<VOXEL_REAL,3>(m_pVolume), 
		//		(Ipp32f) m_minValue,
		//		&m_volBinScaled->GetBufferPointer()[nZ * nStrideZ], Stride<VOXEL_REAL,3>(m_volBinScaled), 
		//		MakeIppiSize<3>(m_pRegion->GetBufferedRegion())) );
		//}

		// and scale,
		{
		IteratorType dstIt( m_volBinScaled, m_volBinScaled->GetBufferedRegion() );
		//ConstIteratorType volumeIt( m_pVolume, m_pVolume->GetBufferedRegion() );
		for ( dstIt.GoToBegin()/*, volumeIt.GoToBegin()*/; 
			!dstIt.IsAtEnd(); ++dstIt/*, ++volumeIt*/)
		{
			dstIt.Set(dstIt.Get()*(1.0/m_binWidth));
		}
		}
		/// TODO: extend this to 3D
		//for (int nZ = 0; nZ < m_pRegion->GetBufferedRegion().GetSize()[2]; nZ++)
		//{
		//	CK_IPP( ippiMulC_32f_C1IR((Ipp32f)(1.0 / m_binWidth),
		//		&m_volBinScaled->GetBufferPointer()[nZ * nStrideZ], Stride<VOXEL_REAL,3>(m_volBinScaled), 
		//		MakeIppiSize<3>(m_pRegion->GetBufferedRegion())) ); 
		//}

		typedef itk::ImageRegionConstIterator< VolumeShort > ConstShortIteratorType;
		typedef itk::ImageRegionIterator< VolumeShort > ShortIteratorType;

		// and compute the bin int volume
		ConformTo<short,3>(m_volBinScaled, m_volBinLoInt);

		{
		ShortIteratorType dstIt( m_volBinLoInt, m_volBinLoInt->GetBufferedRegion() );
		ConstIteratorType srcIt( m_volBinScaled, m_volBinScaled->GetBufferedRegion() );
		for ( dstIt.GoToBegin(), srcIt.GoToBegin(); 
			!dstIt.IsAtEnd(); ++dstIt, ++srcIt)
		{
			dstIt.Set((short)floor(srcIt.Get()));
		}
		}

		///// TODO: extend this to 3D
		//for (int nZ = 0; nZ < m_pRegion->GetBufferedRegion().GetSize()[2]; nZ++)
		//{
		//	CK_IPP( ippiConvert_32f16s_C1R(
		//		&m_volBinScaled->GetBufferPointer()[nZ * nStrideZ], Stride<VOXEL_REAL,3>(m_volBinScaled), 
		//		&m_volBinLoInt->GetBufferPointer()[nZ * nStrideZ], Stride<short,3>(m_volBinLoInt), 
		//		MakeIppiSize<3>(m_pRegion->GetBufferedRegion()), 
		//		ippRndZero) );
		//}

		////////////////////////////////////////////////////////////////////
		// calc bin frac Hi volume

		ConformTo<VOXEL_REAL,3>(m_volBinLoInt, m_volBinFracHi);

		{
		IteratorType dstIt( m_volBinFracHi, m_volBinFracHi->GetBufferedRegion() );
		ConstShortIteratorType srcIt( m_volBinLoInt, m_volBinLoInt->GetBufferedRegion() );
		for ( dstIt.GoToBegin(), srcIt.GoToBegin(); 
			!dstIt.IsAtEnd(); ++dstIt, ++srcIt)
		{
			dstIt.Set(srcIt.Get());
		}
		}

		/// TODO: extend this to 3D
		//for (int nZ = 0; nZ < m_pRegion->GetBufferedRegion().GetSize()[2]; nZ++)
		//{
		//	CK_IPP( ippiConvert_16s32f_C1R(
		//		&m_volBinLoInt->GetBufferPointer()[nZ * nStrideZ], Stride<short,3>(m_volBinLoInt), 
		//		&m_volBinFracHi->GetBufferPointer()[nZ * nStrideZ], Stride<VOXEL_REAL,3>(m_volBinFracHi), 
		//		MakeIppiSize<3>(m_pRegion->GetBufferedRegion())) );
		//}

		// leaves Frac = -High Fraction

		{
		IteratorType dstIt( m_volBinFracHi, m_volBinFracHi->GetBufferedRegion() );
		ConstIteratorType srcIt( m_volBinScaled, m_volBinScaled->GetBufferedRegion() );
		for ( dstIt.GoToBegin(), srcIt.GoToBegin(); 
			!dstIt.IsAtEnd(); ++dstIt, ++srcIt)
		{
			dstIt.Set(-(srcIt.Get()-dstIt.Get()));
		}
		}

		/// TODO: extend this to 3D
		//for (int nZ = 0; nZ < m_pRegion->GetBufferedRegion().GetSize()[2]; nZ++)
		//{
		//	CK_IPP( ippiSub_32f_C1IR(
		//		&m_volBinScaled->GetBufferPointer()[nZ * nStrideZ], Stride<VOXEL_REAL,3>(m_volBinScaled), 
		//		&m_volBinFracHi->GetBufferPointer()[nZ * nStrideZ], Stride<VOXEL_REAL,3>(m_volBinFracHi), 
		//		MakeIppiSize<3>(m_pRegion->GetBufferedRegion())) );
		//}

		////////////////////////////////////////////////////////////////////
		// calc bin frac Lo volume

		ConformTo<VOXEL_REAL,3>(m_volBinFracHi, m_volBinFracLo);

		{
		IteratorType dstIt( m_volBinFracLo, m_volBinFracLo->GetBufferedRegion() );
		ConstIteratorType srcIt( m_volBinFracHi, m_volBinFracHi->GetBufferedRegion() );
		for ( dstIt.GoToBegin(), srcIt.GoToBegin(); 
			!dstIt.IsAtEnd(); ++dstIt, ++srcIt)
		{
			dstIt.Set(srcIt.Get()+1.0);
		}
		}

		/// TODO: extend this to 3D
		//for (int nZ = 0; nZ < m_pRegion->GetBufferedRegion().GetSize()[2]; nZ++)
		//{
		//	CK_IPP( ippiAddC_32f_C1R(
		//		&m_volBinFracHi->GetBufferPointer()[nZ * nStrideZ], Stride<VOXEL_REAL,3>(m_volBinFracHi), 
		//		1.0,
		//		&m_volBinFracLo->GetBufferPointer()[nZ * nStrideZ], Stride<VOXEL_REAL,3>(m_volBinFracLo), 
		//		MakeIppiSize<3>(m_pRegion->GetBufferedRegion())) ); 
		//}

		////////////////////////////////////////////////////////////////////
		// calc bin frac Hi x var frac volumes

		CopyImage<VOXEL_REAL,3>(m_volBinFracHi, m_volBinFracHi_x_VarFracHi);

		{
		IteratorType dstIt( m_volBinFracHi_x_VarFracHi, m_volBinFracHi_x_VarFracHi->GetBufferedRegion() );
		ConstIteratorType srcIt( m_volRegion_x_VarFracHi, m_volRegion_x_VarFracHi->GetBufferedRegion() );
		for ( dstIt.GoToBegin(), srcIt.GoToBegin(); 
			!dstIt.IsAtEnd(); ++dstIt, ++srcIt)
		{
			dstIt.Set(dstIt.Get() * srcIt.Get());
		}
		}

		/// TODO: extend this to 3D
		//for (int nZ = 0; nZ < m_pRegion->GetBufferedRegion().GetSize()[2]; nZ++)
		//{
		//	CK_IPP( ippiMul_32f_C1IR(
		//		&m_volRegion_x_VarFracHi->GetBufferPointer()[nZ * nStrideZ], Stride<VOXEL_REAL,3>(m_volRegion_x_VarFracHi), 
		//		&m_volBinFracHi_x_VarFracHi->GetBufferPointer()[nZ * nStrideZ], Stride<VOXEL_REAL,3>(m_volBinFracHi_x_VarFracHi), 
		//		MakeIppiSize<3>(m_pRegion->GetBufferedRegion()))); 
		//}

		CopyImage<VOXEL_REAL,3>(m_volBinFracHi, m_volBinFracHi_x_VarFracLo);

		{
		IteratorType dstIt( m_volBinFracHi_x_VarFracLo, m_volBinFracHi_x_VarFracLo->GetBufferedRegion() );
		ConstIteratorType srcIt( m_volRegion_x_VarFracLo, m_volRegion_x_VarFracLo->GetBufferedRegion() );
		for ( dstIt.GoToBegin(), srcIt.GoToBegin(); 
			!dstIt.IsAtEnd(); ++dstIt, ++srcIt)
		{
			dstIt.Set(dstIt.Get() * srcIt.Get());
		}
		}

		/// TODO: extend this to 3D
		//for (int nZ = 0; nZ < m_pRegion->GetBufferedRegion().GetSize()[2]; nZ++)
		//{
		//	CK_IPP( ippiMul_32f_C1IR(
		//		&m_volRegion_x_VarFracLo->GetBufferPointer()[nZ * nStrideZ], Stride<VOXEL_REAL,3>(m_volRegion_x_VarFracLo), 
		//		&m_volBinFracHi_x_VarFracLo->GetBufferPointer()[nZ * nStrideZ], Stride<VOXEL_REAL,3>(m_volBinFracHi_x_VarFracLo), 
		//		MakeIppiSize<3>(m_pRegion->GetBufferedRegion())));
		//}

		////////////////////////////////////////////////////////////////////
		// calc bin frac Lo x var frac volumes

		CopyImage<VOXEL_REAL,3>(m_volBinFracLo, m_volBinFracLo_x_VarFracHi);

		{
		IteratorType dstIt( m_volBinFracLo_x_VarFracHi, m_volBinFracLo_x_VarFracHi->GetBufferedRegion() );
		ConstIteratorType srcIt( m_volRegion_x_VarFracHi, m_volRegion_x_VarFracHi->GetBufferedRegion() );
		for ( dstIt.GoToBegin(), srcIt.GoToBegin(); 
			!dstIt.IsAtEnd(); ++dstIt, ++srcIt)
		{
			dstIt.Set(dstIt.Get() * srcIt.Get());
		}
		}

		/// TODO: extend this to 3D
		//for (int nZ = 0; nZ < m_pRegion->GetBufferedRegion().GetSize()[2]; nZ++)
		//{
		//	CK_IPP( ippiMul_32f_C1IR(
		//		&m_volRegion_x_VarFracHi->GetBufferPointer()[nZ * nStrideZ], Stride<VOXEL_REAL,3>(m_volRegion_x_VarFracHi), 
		//		&m_volBinFracLo_x_VarFracHi->GetBufferPointer()[nZ * nStrideZ],	Stride<VOXEL_REAL,3>(m_volBinFracLo_x_VarFracHi), 
		//		MakeIppiSize<3>(m_pRegion->GetBufferedRegion())) ); 
		//}

		CopyImage<VOXEL_REAL,3>(m_volBinFracLo, m_volBinFracLo_x_VarFracLo);

		{
		IteratorType dstIt( m_volBinFracLo_x_VarFracLo, m_volBinFracLo_x_VarFracLo->GetBufferedRegion() );
		ConstIteratorType srcIt( m_volRegion_x_VarFracLo, m_volRegion_x_VarFracLo->GetBufferedRegion() );
		for ( dstIt.GoToBegin(), srcIt.GoToBegin(); 
			!dstIt.IsAtEnd(); ++dstIt, ++srcIt)
		{
			dstIt.Set(dstIt.Get() * srcIt.Get());
		}
		}

		/// TODO: extend this to 3D
		//for (int nZ = 0; nZ < m_pRegion->GetBufferedRegion().GetSize()[2]; nZ++)
		//{
		//	CK_IPP( ippiMul_32f_C1IR(
		//		&m_volRegion_x_VarFracLo->GetBufferPointer()[nZ * nStrideZ], Stride<VOXEL_REAL,3>(m_volRegion_x_VarFracLo), 
		//		&m_volBinFracLo_x_VarFracLo->GetBufferPointer()[nZ * nStrideZ],	Stride<VOXEL_REAL,3>(m_volBinFracLo_x_VarFracLo),
		//		MakeIppiSize<3>(m_pRegion->GetBufferedRegion())) );
		//}

		m_bRecomputeBinScaledVolume = FALSE;
	}

	//return m_volBinScaled;

}	// CHistogram::CalcBinningVolumes

//////////////////////////////////////////////////////////////////////
const CVectorN<>& 
	CHistogram::GetBins() const
	// retrieves the bins for this histogram
{
	if (true) // m_bRecomputeBins)
	{
		// calculate all binning volumes
		CalcBinningVolumes();

		// now set up the bins
		REAL maxValue = GetMax<VOXEL_REAL>(GetVolume());
		int nBins = GetBinForValue(maxValue)+2;
		m_arrBinsVarMax.SetDim(nBins);
		m_arrBinsVarMax.SetZero();
		m_arrBinsVarMin.SetDim(nBins);
		m_arrBinsVarMin.SetZero();

		// and do the binning
		/// TODO: extend this to non-zed planar / 3D
		int nStrideZ = m_pRegion->GetBufferedRegion().GetSize()[0] 
			* m_pRegion->GetBufferedRegion().GetSize()[1];
		int nCount = m_pRegion->GetBufferedRegion().GetSize()[0] 
			* m_pRegion->GetBufferedRegion().GetSize()[1]; 
		for (int nZ = 0; nZ < /*1*/ m_pRegion->GetBufferedRegion().GetSize()[2]; nZ++)
		{
			for (int nAt = /*0*/ /*GetSlice()*/nZ * nCount;
				nAt < /*nCount*/((/*GetSlice()*/nZ+1) * nCount); nAt++)
			{
				const int nLowBin = m_volBinLoInt->GetBufferPointer()[nAt];

				// check that region is positive definite
				ASSERT(GetRegion()->GetBufferPointer()[nAt] >= 0.0);

				m_arrBinsVarMax[nLowBin] += m_volBinFracLo_x_VarFracHi->GetBufferPointer()[nAt];
				m_arrBinsVarMin[nLowBin] += m_volBinFracLo_x_VarFracLo->GetBufferPointer()[nAt];

				m_arrBinsVarMax[nLowBin+1] -= m_volBinFracHi_x_VarFracHi->GetBufferPointer()[nAt]; 
				m_arrBinsVarMin[nLowBin+1] -= m_volBinFracHi_x_VarFracLo->GetBufferPointer()[nAt]; 
			}
		}

		// now calculate total bins
		m_arrBins.SetDim(nBins);
		m_arrBins.SetZero();
		m_arrBins = m_arrBinsVarMax;
		m_arrBins += m_arrBinsVarMin;

		REAL binKernelSigma = sqrt(m_varMax);
		if (binKernelSigma > 0.0)
		{
			ConvGauss(m_arrBinsVarMax, m_binKernelVarMax, m_arrGBinsVarMax);
			ConvGauss(m_arrBinsVarMin, m_binKernelVarMin, m_arrGBinsVarMin);
			ASSERT(m_arrGBinsVarMax.GetDim() == m_arrGBinsVarMin.GetDim());

			m_arrGBins.SetDim(m_arrGBinsVarMax.GetDim());
			m_arrGBins = m_arrGBinsVarMax;
			m_arrGBins += m_arrGBinsVarMin;
		}

		// now normalize
		REAL calcSum = 0.0;
#ifdef STANDARD_SUM
		calcSum = GetSum<VOXEL_REAL>(GetRegion());
#else
		// NOTE: this needs to cover the same voxels as the above binning loop
		//int nCount = GetRegion()->GetBufferedRegion().GetSize()[0] 
		//	* GetRegion()->GetBufferedRegion().GetSize()[1];
		for (int nZ = 0; nZ < /*1*/ m_pRegion->GetBufferedRegion().GetSize()[2]; nZ++)
		{
			for (int nAt = /*0*/ /*GetSlice()*/nZ * nCount;
				nAt < /*nCount*/((/*GetSlice()*/nZ+1) * nCount); nAt++)
			{
				//for (int nAt = /*0*/ GetSlice() * nCount;
				//	nAt < /*nCount*/((GetSlice()+1) * nCount); nAt++)
				// for (int nAt = 0; nAt < nCount; nAt++)
				calcSum += GetRegion()->GetBufferPointer()[nAt]; 
			}
		}
#endif

		if (calcSum > 0.0)
		{
			// normalize each bin
			m_arrGBins *= R(1.0 / ((double) calcSum));
		}

		m_bRecomputeBins = FALSE;
	}

	return m_arrBins;

}	// CHistogram::GetBins

//////////////////////////////////////////////////////////////////////
const CVectorN<>& 
	CHistogram::GetCumBins() const
	// computes and returns the cumulative bins
{
	if (m_bRecomputeCumBins)
	{
		GetBins();

		m_arrCumBins.SetDim(m_arrBins.GetDim());
		m_arrCumBins.SetZero();

		REAL sum = 0.0;
		for (int nAt = m_arrBins.GetDim()-1; nAt >= 0; nAt--)
		{
			sum += m_arrBins[nAt];
			m_arrCumBins[nAt] = sum;
		} 
		m_bRecomputeCumBins = FALSE;
	}

	return m_arrCumBins;

}	// CHistogram::GetCumBins

//////////////////////////////////////////////////////////////////////
const CVectorN<>& 
	CHistogram::GetGBins() const
	// computes and returns the GHistogram
{
	GetBins();
	if (m_varMax == 0.0)
	{
		ASSERT(FALSE);
		return m_arrBins;
	}

	return m_arrGBins;

}	// CHistogram::GetGBins

//////////////////////////////////////////////////////////////////////
const CVectorN<>& 
	CHistogram::GetGBinMeans() const
	// returns a vector with mean GBin values
{
	m_arrGBinMeans.SetDim(m_arrGBins.GetDim());
	for (int nAt = 0; nAt < m_arrGBinMeans.GetDim(); nAt++)
	{
		m_arrGBinMeans[nAt] = m_minValue + (REAL) nAt * m_binWidth;
	}

	return m_arrGBinMeans;

}	// CHistogram::GetGBinMeans

///////////////////////////////////////////////////////////////////////////////
void 
	CHistogram::ConvGauss(const CVectorN<>& buffer_in, const CVectorN<>& kernel_in,
						   CVectorN<>& buffer_out) const
{
	BeginLogSection(_T("CHistogram::ConvGauss"));

	TraceVector(_T("kernel_in"), kernel_in);
	TraceVector(_T("buffer_in"), buffer_in);

	buffer_out.SetDim(buffer_in.GetDim() + kernel_in.GetDim() - 1);
	buffer_out.SetZero();

	// make sure REAL is double
	ASSERT(sizeof(REAL) == 8);

	IppStatus stat = ippsConv_64f(
		&buffer_in[0], buffer_in.GetDim(),
		&kernel_in[0], kernel_in.GetDim(),
		&buffer_out[0]);
	ASSERT(stat == ippStsNoErr);

	TraceVector(_T("buffer_out"), buffer_out);

	EndLogSection();

}	// CHistogram::ConvGauss

//////////////////////////////////////////////////////////////////////
bool 
	CHistogram::IsContributing(int nElement)
	// determines if dVolume is contribution to the histogram
{
	return true;

	//const C Volume<VOXEL_REAL>* pVolume = Get_dVolume_x_Region(nElement);

	//return (pVolume->GetThresholdBounds().left < pVolume->GetThresholdBounds().right
	//	&& pVolume->GetThresholdBounds().top < pVolume->GetThresholdBounds().bottom);

}	// CHistogram::IsContributing

//////////////////////////////////////////////////////////////////////
void 
	CHistogram::OnVolumeChange() // CObservableEvent *pSource, void *)
	// triggers update of histogram
{
	// flag recomputation
	m_bRecomputeBins = TRUE;
	m_bRecomputeCumBins = TRUE;
	m_bRecomputeBinScaledVolume = TRUE;

	//int nGroups = GetGroupCount();
	for (int nAt = 0; nAt < m_arr_bRecomputeBinVolume.GetSize(); nAt++)
	{
		m_arr_bRecomputeBinVolume[nAt] = TRUE;
	}

	// set recalc flag for dBins
	for (int nAt = 0; nAt < m_arr_bRecompute_dBins.GetSize(); nAt++)
	{
		m_arr_bRecompute_dBins[nAt] = TRUE;
	}

	// TODO: flag recompute dVolume_x_Region

	// fire a change event
	//GetChangeEvent().Fire();
	this->DataHasBeenGenerated(); // Modified();

}	// CHistogram::OnVolumeChange

//////////////////////////////////////////////////////////////////////
void 
	CHistogram::OnRegionChanged() // CObservableEvent * pEvt, void * pParam)
	// called when region updated
{
	for (int nAt = 0; nAt < m_arr_bRecompute_dVolumes_x_Region.GetSize(); nAt++)
	{
		m_arr_bRecompute_dVolumes_x_Region[nAt] = TRUE;
	}

	m_bRecomputeBins = TRUE;
	m_bRecomputeCumBins = TRUE;


	if (m_pRegion != NULL)
	{
		// set up the constant fractional variance volumes
		// initialize max to all 1.0s and min to all 0.0s to allow basic computation
		VolumeReal::Pointer constVolVarFracLo = VolumeReal::New(); 
		ConformTo<VOXEL_REAL,3>(m_pRegion, constVolVarFracLo);
		constVolVarFracLo->FillBuffer(0.0);

		VolumeReal::Pointer constVolVarFracHi = VolumeReal::New(); 
		ConformTo<VOXEL_REAL,3>(m_pRegion, constVolVarFracHi);
		constVolVarFracHi->FillBuffer(1.0);

		SetVarFracVolumes(constVolVarFracLo, constVolVarFracHi);
	}
}


