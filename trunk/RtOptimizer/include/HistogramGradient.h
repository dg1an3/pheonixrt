// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: HistogramGradient.h 603 2008-09-14 16:58:43Z dglane001 $
#pragma once

#include <Histogram.h>

class CHistogramWithGradient : public CHistogram
{
public:
	CHistogramWithGradient();
	virtual ~CHistogramWithGradient(void);

	// partial derivative volumes
	int Get_dVolumeCount() const;
	int GetGroupCount() const;
	VolumeReal *Get_dVolume(int nAt, int *pnGroup = NULL) const;
	int Add_dVolume(VolumeReal *p_dVolume, int nGroup);

	// partial derivatives
	const CVectorN<>& Get_dBins(int nAt) const;
	const CVectorN<>& Get_dGBins(int nAt) const;

	const CVectorN<>* vInput;
	const CVectorN<>* vInputTrans;

protected:
	// helpers

	// calculates the bin volume, rotated for basis group N
	const VolumeShort * GetBinVolume(int nAt) const;

	// calculates the dVolume x region, rotated for basis group N
	const VolumeReal * Get_dVolume_x_Region(int nAt) const;

	// convolve helper
	void Conv_dGauss(const CVectorN<>& buffer_in, const CVectorN<>& kernel_in,
							CVectorN<>& buffer_out) const;

protected:

	// array of rotated regions, per group
	std::vector< VolumeReal::Pointer > m_groupVolRegion;	

	// helper for rotating bin scaled volume (only one declared, because it is used
	//		only temporarily)
	mutable VolumeReal::Pointer m_groupVolBinScaled;	

	// int bin indices for each voxel, per group
	mutable std::vector< VolumeShort::Pointer > m_groupVolBinLoInt;

	// bin frac hi / lo volumes, per group
	mutable std::vector< VolumeReal::Pointer > m_groupVolBinFracHi;	
	mutable std::vector< VolumeReal::Pointer > m_groupVolBinFracLo;

	// flags for recomputing binning volumes
	// mutable CArray<bool, bool> m_arr_bRecomputeBinVolume;	// per group

	// stores product of bin fracs and dVolume (and Region)
	mutable std::vector< VolumeReal::Pointer > m_groupVolBinFracHi_x_dVolume;
	mutable std::vector< VolumeReal::Pointer > m_groupVolBinFracLo_x_dVolume;


	// array of partial derivative volumes
	std::vector< VolumeReal::Pointer > m_arr_dVolumes;
	CArray<int, int> m_arrVolumeGroups;

	// array of partial derivative X region
	std::vector< VolumeReal::Pointer > m_arr_dVolumes_x_Region;

	//// flags for recalc
	//mutable CArray<bool, bool> m_arr_bRecompute_dVolumes_x_Region;

	// partial derivative histogram bins
	mutable CArray<CVectorN<>, CVectorN<>&> m_arr_dBins;

	// partial derivative histogram bins
	mutable CArray<CVectorN<>, CVectorN<>&> m_arr_dGBins;

	//// flags for recalc
	//mutable CArray<bool, bool> m_arr_bRecompute_dBins;

};
