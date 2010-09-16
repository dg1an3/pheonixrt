// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: Histogram.h 603 2008-09-14 16:58:43Z dglane001 $
#pragma once

#include <VectorN.h>
#include <ItkUtils.h>
// #include <ModelObject.h>

const REAL GBINS_BUFFER = R(8.0);

//////////////////////////////////////////////////////////////////////
// class CHistogram
//
// forms a histogram of a volume, possibly within a "region" which
//		is a binary volume
//////////////////////////////////////////////////////////////////////
class CHistogram : public itk::DataObject  
{
public:
	// constructor from a volume and a "region"
	CHistogram(VolumeReal *pImage = NULL, 
		VolumeReal *pRegion = NULL);

	// destructor
	virtual ~CHistogram();

	// association to the volume over which the histogram is formed
	DECLARE_ATTRIBUTE_PTR_GI(Volume, VolumeReal);

	// association to a congruent volume describing the region over
	//		which the histogram is formed -- contains a 1.0 for voxels
	//		within the region, 0.0 elsewhere
	DECLARE_ATTRIBUTE_PTR_GI(Region, VolumeReal);

	// for 2D operation, determines slice to use
	DECLARE_ATTRIBUTE(Slice, int);

	// histogram parameters
	REAL GetBinMinValue() const;
	REAL GetBinWidth() const;
	void SetBinning(REAL min_value, REAL width, 
		REAL sigma_mult = 0.0);
	int GetBinForValue(REAL value) const;
	const CVectorN<>& GetBinMeans() const;

	// Gbinning parameters
	REAL GetGBinVarMin(void) const;
	REAL GetGBinVarMax(void) const;

	// adaptive variance
	void SetGBinVar(CVectorN<> *pAV, REAL varMin, REAL varMax);

	// returns reference to kernels
	const CVectorN<>& GetKernelVarMin(void) const;
	const CVectorN<>& GetKernelVarMax(void) const;

	// sets the fraction volumes
	void SetVarFracVolumes(VolumeReal *pVarMinVolume, VolumeReal *pVarMaxVolume);

	// returns a reference to change event representing changes in binning parameters
	// CObservableEvent& GetBinningChangeEvent() { return m_eventBinningChange; }

	// accessors for bin data
	const CVectorN<>& GetBins() const;
	const CVectorN<>& GetCumBins() const;

	// Gbin accessor
	const CVectorN<>& GetGBins() const;
	const CVectorN<>& GetGBinMeans() const;

	// determines if the given dVolume is contributing to the masked region
	bool IsContributing(int nElement);

	// convolve helpers
	void ConvGauss(const CVectorN<>& buffer_in, const CVectorN<>& kernel_in,
							CVectorN<>& buffer_out) const;

	// change handler for when the volume or region changes
	void OnVolumeChange(); // CObservableEvent *pSource, void *);

	// called when region updated
	void OnRegionChanged(); // CObservableEvent * pEvt, void * pParam);

protected:

	// helpers
	void CalcBinningVolumes() const;

protected:

	// binning parameters
	REAL m_minValue;
	REAL m_binWidth;

	// the change event for this object
	// CObservableEvent m_eventBinningChange;

	mutable CVectorN<> m_arrBins;
	mutable CVectorN<> m_arrBinsVarMax;
	mutable CVectorN<> m_arrBinsVarMin;
	mutable CVectorN<> m_arrBinMeans;

	// flag to indicate bins should be recomputed
	mutable BOOL m_bRecomputeBins;

	mutable VolumeReal::Pointer m_volBinScaled;
	mutable bool m_bRecomputeBinScaledVolume;

	mutable VolumeShort::Pointer m_volBinLoInt;

	mutable VolumeReal::Pointer m_volBinFracHi;
	mutable VolumeReal::Pointer m_volBinFracHi_x_VarFracLo;
	mutable VolumeReal::Pointer m_volBinFracHi_x_VarFracHi;

	mutable VolumeReal::Pointer m_volBinFracLo;
	mutable VolumeReal::Pointer m_volBinFracLo_x_VarFracLo;
	mutable VolumeReal::Pointer m_volBinFracLo_x_VarFracHi;

	mutable	VolumeReal::Pointer m_volRegion_x_VarFracHi;
	mutable VolumeReal::Pointer m_volRegion_x_VarFracLo;

	//////////////////////////////////////////////////////////////////////////

	// cumulative bins
	mutable CVectorN<> m_arrCumBins;

	// flag to indicate cumulative bins should be recomputed
	mutable bool m_bRecomputeCumBins;

	//////////////////////////////////////////////////////////////////////////

	// the binning kernel widths
	REAL m_varMin;
	REAL m_varMax;

	// the adaptive variance vector
	CVectorN<> *m_pAV;

	// the binning kernels
	CVectorN<> m_binKernelVarMax;
	CVectorN<> m_binKernelVarMin;
	CVectorN<> m_bin_dKernelVarMax;		// TODO: move these to HistoGrad
	CVectorN<> m_bin_dKernelVarMin;		// TODO: move these to HistoGrad

	// array of GBins + means
	mutable CVectorN<> m_arrGBins;
	mutable CVectorN<> m_arrGBinsVarMax;
	mutable CVectorN<> m_arrGBinsVarMin;
	mutable CVectorN<> m_arrGBinMeans;

	// these are here for processing in OnVolumeChanged, though they
	// should be moved to CHistogramGradient class

	// flags for recalc
	mutable CArray<bool, bool> m_arr_bRecomputeBinVolume;	// per group

	//// flags for recalc
	mutable CArray<bool, bool> m_arr_bRecompute_dVolumes_x_Region;

	// flags for recalc
	mutable CArray<bool, bool> m_arr_bRecompute_dBins;

};	// class CHistogram


//////////////////////////////////////////////////////////////////////
// CHistogram::GetBinForValue
// 
// computes the bin for a particular intensity value
//////////////////////////////////////////////////////////////////////
inline int CHistogram::GetBinForValue(REAL value) const
{
	return Round<int>((value - m_minValue) / m_binWidth);

}	// CHistogram::GetBinForValue

