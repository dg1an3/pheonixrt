#include "StdAfx.h"
#include "InPlaneResampleImageFilter.h"

namespace dH {

//////////////////////////////////////////////////////////////////////////////
InPlaneResampleImageFilter::InPlaneResampleImageFilter(void)
: m_bResampleLinear(true)
{
	m_pBuffer = OutputImageType::New();
}

//////////////////////////////////////////////////////////////////////////////
InPlaneResampleImageFilter::~InPlaneResampleImageFilter(void)
{
}

//////////////////////////////////////////////////////////////////////////////
void 
InPlaneResampleImageFilter::GenerateData()
// generates the data on update
{
	// Call a method that can be overriden by a subclass to allocate
	// memory for the filter's outputs
	this->AllocateOutputs();

	VolumeReal::ConstPointer pInput = GetInput();
	const VolumeReal::RegionType& inRegion = pInput->GetBufferedRegion();

	VolumeReal::Pointer pOutput = GetOutput();
	const VolumeReal::RegionType& outRegion = pOutput->GetBufferedRegion();

	// calculate warping coefficients 

	itk::Matrix<REAL, 4, 4> mBasisInput;
	CalcBasis<3>(pInput, mBasisInput);

	itk::Matrix<REAL, 4, 4> mBasisOutput;
	CalcBasis<3>(pOutput, mBasisOutput);

	itk::Matrix<REAL, 4, 4> mXform = mBasisInput.GetInverse();
	mXform *= mBasisOutput;

	double xformCoeffs[2][3];
	xformCoeffs[0][0] = mXform(0, 0);
	xformCoeffs[0][1] = mXform(0, 1);
	xformCoeffs[0][2] = mXform(0, 3);
	xformCoeffs[1][0] = mXform(1, 0);
	xformCoeffs[1][1] = mXform(1, 1);
	xformCoeffs[1][2] = mXform(1, 3);

	// fill to black
	pOutput->FillBuffer(0.0);

	// access input / output geometries

	const VolumeReal::PointType& inOrigin = pInput->GetOrigin();
	const VolumeReal::SpacingType& inSpacing = pInput->GetSpacing();

	const VolumeReal::PointType& outOrigin = pOutput->GetOrigin();
	const VolumeReal::SpacingType& outSpacing = pOutput->GetSpacing();

	// iterate over output planes
	VolumeReal::IndexType outIndex = outRegion.GetIndex();
	for (outIndex[2] = 0; outIndex[2] < outRegion.GetSize()[2]; outIndex[2]++)
	{
		// calculate input plane position
		REAL planeZ = (outOrigin[2] + outSpacing[2] * outIndex[2] 
		- inOrigin[2]) / inSpacing[2];

		VolumeReal::IndexType inIndex = inRegion.GetIndex();
		if (m_bResampleLinear)
		{
			inIndex[2] = floor(planeZ);
			ResamplePlaneLinearZ(planeZ, inIndex, outIndex, xformCoeffs);
		}
		else
		{
			inIndex[2] = Round<int>(planeZ);
			ResamplePlaneNNeighborZ(inIndex, outIndex, xformCoeffs);
		}
	}

	// set modified flag
	pOutput->Modified();

	// find out why the fuck the conform region iterator has a value of xxx
	ImageRegionConstIterator<VolumeReal> fuckIterIn(GetInput(), GetInput()->GetBufferedRegion());
	double maxVoxelIn = 0.0; // -1e+10;
	for (; !fuckIterIn.IsAtEnd(); ++fuckIterIn)
	{
		maxVoxelIn = __max(maxVoxelIn, fabs(fuckIterIn.Value()));
	}

	// find out why the fuck the conform region iterator has a value of xxx
	ImageRegionConstIterator<VolumeReal> fuckIterOut(GetOutput(), GetOutput()->GetBufferedRegion());
	double maxVoxelOut = 0.0; // -1e+10;
	for (; !fuckIterOut.IsAtEnd(); ++fuckIterOut)
	{
		maxVoxelOut = __max(maxVoxelOut, fabs(fuckIterOut.Value()));
	}
}

//////////////////////////////////////////////////////////////////////////////
void 
InPlaneResampleImageFilter::ResamplePlaneLinearZ(
	REAL planeZ, 
	VolumeReal::IndexType& inIndex, 
	VolumeReal::IndexType& outIndex, 
	const double coeffs[][3])
{
	VolumeReal::ConstPointer pInput = GetInput();
	const VolumeReal::RegionType& inRegion = pInput->GetBufferedRegion();

	VolumeReal::Pointer pOutput = GetOutput();
	const VolumeReal::RegionType& outRegion = pOutput->GetBufferedRegion();

	// allocate the temp buffer
	m_pBuffer->SetRegions(pOutput->GetBufferedRegion());
	m_pBuffer->Allocate();
	m_pBuffer->FillBuffer(0.0);

	if (inRegion.IsInside(inIndex))
	{
		// warp the plane
		CK_IPP(ippiWarpAffineBack_32f_C1R(

			&pInput->GetPixel(inIndex),									// Src
			MakeIppiSize(inRegion),											// srcSize				
			inRegion.GetSize()[0] * sizeof(VOXEL_REAL), // srcStep
			MakeIppiRect(inRegion),											// srcRoi

			&pOutput->GetPixel(outIndex),								// pDst
			outRegion.GetSize()[0] * sizeof(VOXEL_REAL), // dstStep
			MakeIppiRect(outRegion),										// dstRoi

			coeffs, 
			IPPI_INTER_LINEAR));

		// scale by interpolation factor
		REAL scale = 1.0 - (planeZ - (REAL) inIndex[2]);
		CK_IPP( ippiMulC_32f_C1IR(
			scale,
			&pOutput->GetPixel(outIndex), Stride<VOXEL_REAL, 3>(pOutput), 
			MakeIppiSize(outRegion)));
	}
	inIndex[2]++;
	if (inRegion.IsInside(inIndex))
	{
		// warp the plane
		CK_IPP(ippiWarpAffineBack_32f_C1R(

			&pInput->GetPixel(inIndex),									// Src
			MakeIppiSize(inRegion),											// srcSize				
			inRegion.GetSize()[0] * sizeof(VOXEL_REAL), // srcStep
			MakeIppiRect(inRegion),											// srcRoi

			&m_pBuffer->GetPixel(outIndex),							// pDst
			outRegion.GetSize()[0] * sizeof(VOXEL_REAL), // dstStep
			MakeIppiRect(outRegion),										// dstRoi

			coeffs, 
			IPPI_INTER_LINEAR));

		// scale by interpolation factor
		REAL scale = 1.0 - ((REAL) inIndex[2] - planeZ);
		CK_IPP( ippiMulC_32f_C1IR(
			scale,
			&m_pBuffer->GetPixel(outIndex), Stride<VOXEL_REAL, 3>(m_pBuffer), 
			MakeIppiSize(outRegion)));

		// and add to output
		CK_IPP( ippiAdd_32f_C1IR(
			&m_pBuffer->GetPixel(outIndex), Stride<VOXEL_REAL, 3>(m_pBuffer), 
			&pOutput->GetPixel(outIndex), Stride<VOXEL_REAL, 3>(pOutput), 
			MakeIppiSize(outRegion)));
	}
}

//////////////////////////////////////////////////////////////////////////////
void 
InPlaneResampleImageFilter::ResamplePlaneNNeighborZ(
	VolumeReal::IndexType& inIndex, 
	VolumeReal::IndexType& outIndex,
	const double coeffs[][3])
{
	VolumeReal::ConstPointer pInput = GetInput();
	const VolumeReal::RegionType& inRegion = pInput->GetBufferedRegion();

	VolumeReal::Pointer pOutput = GetOutput();
	const VolumeReal::RegionType& outRegion = pOutput->GetBufferedRegion();

	if (inRegion.IsInside(inIndex))
	{
		// warp the plane
		CK_IPP(ippiWarpAffineBack_32f_C1R(

			&pInput->GetPixel(inIndex),									// Src
			MakeIppiSize(inRegion),											// srcSize				
			inRegion.GetSize()[0] * sizeof(VOXEL_REAL), // srcStep
			MakeIppiRect(inRegion),											// srcRoi

			&pOutput->GetPixel(outIndex),								// pDst
			outRegion.GetSize()[0] * sizeof(VOXEL_REAL), // dstStep
			MakeIppiRect(outRegion),										// dstRoi

			coeffs, 
			IPPI_INTER_LINEAR));
	}
}

}