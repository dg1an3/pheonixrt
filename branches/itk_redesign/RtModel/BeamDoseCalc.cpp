// Copyright (C) 2nd Messenger Systems
// $Id: BeamDoseCalc.cpp 602 2008-09-14 16:54:49Z dglane001 $
#include "stdafx.h"
#include "BeamDoseCalc.h"

#include <EnergyDepKernel.h>
#include <Beam.h>
#include <Plan.h>

using namespace itk;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
CBeamDoseCalc::CBeamDoseCalc(CBeam *pBeam, CEnergyDepKernel *pKernel)
:	m_pBeam(pBeam),
		m_pKernel(pKernel),
		m_raysPerVoxel(12)
{
}	// CBeamDoseCalc::CBeamDoseCalc


///////////////////////////////////////////////////////////////////////////////
CBeamDoseCalc::~CBeamDoseCalc()
{
}	// CBeamDoseCalc::~CBeamDoseCalc



///////////////////////////////////////////////////////////////////////////////
void 
	CBeamDoseCalc::InitCalcBeamlets()
	// Calculates pencil beams given the density matrix
{
	// TODO: check that dose matrix is initialized
	ASSERT(m_pBeam->m_dose->GetBufferedRegion().GetSize()[0] > 0);

	m_densityRep = VolumeReal::New();
	ConformTo<VOXEL_REAL,3>(m_pBeam->m_dose, m_densityRep);
	m_densityRep->FillBuffer(0.0); 

	VolumeReal *pMassDensity = m_pBeam->GetPlan()->GetMassDensity();
	// ::Resample(pMassDensity, m_densityRep, TRUE); 
	// Resample3D(pMassDensity, m_densityRep, TRUE); 
	itk::ResampleImageFilter<VolumeReal, VolumeReal>::Pointer resampler = 
		itk::ResampleImageFilter<VolumeReal, VolumeReal>::New();
	resampler->SetInput(pMassDensity);

	typedef itk::AffineTransform<REAL, 3> TransformType;
	TransformType::Pointer transform = TransformType::New();
	transform->SetIdentity();
	resampler->SetTransform(transform);

	typedef itk::LinearInterpolateImageFunction<VolumeReal, REAL> InterpolatorType;
	InterpolatorType::Pointer interpolator = InterpolatorType::New();
	resampler->SetInterpolator( interpolator );

	VolumeReal::Pointer pPointToVolume = static_cast<VolumeReal*>(m_densityRep);
	resampler->SetOutputParametersFromImage(pPointToVolume);
	resampler->Update();
	CopyImage<VOXEL_REAL, 3>(resampler->GetOutput(), m_densityRep);

#ifdef USE_2D
	// TODO: now, replicate slices
	for (int nZ = 1; nZ < m_densityRep->GetBufferedRegion().GetSize()[2]; nZ++)
	{
		int nPlaneStride = m_densityRep->GetBufferedRegion().GetSize()[1]
			* m_densityRep->GetBufferedRegion().GetSize()[0];
		memcpy(&m_densityRep->GetBufferPointer()[nZ * nPlaneStride],
			m_densityRep->GetBufferPointer(), 
			m_densityRep->GetBufferedRegion().GetSize()[0] * m_densityRep->GetBufferedRegion().GetSize()[1] * sizeof(VOXEL_REAL));
	}
#endif

	Matrix<REAL, 4, 4> mBeamBasis;
	CalcBasis<3>(m_pBeam->m_dose, mBeamBasis);
	Matrix<REAL, 4, 4> mBeamBasisInv(mBeamBasis.GetInverse());

	// calc isocenter position
	Vector<REAL, 3> vIsocenter = m_pBeam->GetIsocenter();
	MultHG(mBeamBasisInv, vIsocenter, m_vIsocenter_vxl);
	m_vIsocenter_vxl[2] = vIsocenter[2];
	//m_vIsocenter_vxl[0] -= m_pBeam->m_dose->GetOrigin()[0];
	//m_vIsocenter_vxl[1] -= m_pBeam->m_dose->GetOrigin()[1];
	m_vIsocenter_vxl[2] -= m_pBeam->m_dose->GetOrigin()[2];
	//m_vIsocenter_vxl[0] /= m_pBeam->m_dose->GetSpacing()[0];
	//m_vIsocenter_vxl[1] /= m_pBeam->m_dose->GetSpacing()[1];
	m_vIsocenter_vxl[2] /= m_pBeam->m_dose->GetSpacing()[2];
	m_vIsocenter_vxl[2] = Round<int>(m_vIsocenter_vxl[2]);

	// calc source position
	// TODO: get SAD from beam
	m_vSource_vxl = m_vIsocenter_vxl;
	m_vSource_vxl[0] -= 1000.0 / m_pBeam->GetPlan()->GetDoseResolution();

	// clear existing beamlets
	//m_pBeam->m_arrBeamletsSub[0].clear();
}

///////////////////////////////////////////////////////////////////////////////////////
void CBeamDoseCalc::CalcBeamlet(int nBeamlet)
{
	// determine beamlet spacing
	// TODO: fix this
	// TODO: reconcile with beamlet spacing in PlanPyramid
	REAL beamletSpacing = 4.0;  

	// set beamlet size
	Vector<REAL,2> vMin = MakeVector<2>(((REAL) nBeamlet - 0.5) * beamletSpacing, -10.0); // -5.0);
	Vector<REAL,2> vMax = MakeVector<2>(((REAL) nBeamlet + 0.5) * beamletSpacing,  10.0); // 5.0);
	SetBeamletMinMax(vMin, vMax);

	// calculate terma for pencil beam
	CalcTerma();

	// convolve terma with energy deposition kernel to form dose
	int nSlice = Round<int>(m_vIsocenter_vxl[2]);
	CString strSlice;
	strSlice.Format(_T("Calc dose for slice %i\n"), nSlice);
	TRACE(strSlice);
	// ::AfxMessageBox(strSlice);
	m_pEnergy = m_pKernel->CalcSphereConvolve(m_densityRep, m_pTerma, Round<int>(m_vIsocenter_vxl[2]));

#ifdef USE_2D
	// copy voxels from 3D energy to 2D array
	//int nStride = m_pTerma->GetBufferedRegion().GetSize()[0]
	//	* m_pTerma->GetBufferedRegion().GetSize()[1]
	//	* Round<int>(m_vIsocenter_vxl[2]);
	//memcpy(m_pTerma->GetBufferPointer(), 
	//	&m_pTerma->GetBufferPointer()[nStride], 
	//	m_pTerma->GetBufferedRegion().GetSize()[0] * m_pTerma->GetBufferedRegion().GetSize()[1] * sizeof(VOXEL_REAL));

	// copy voxels from 3D energy to 2D array
	{
	int nStride = m_pEnergy->GetBufferedRegion().GetSize()[0]
		* m_pEnergy->GetBufferedRegion().GetSize()[1]
		* Round<int>(m_vIsocenter_vxl[2]);
	memcpy(m_pEnergy->GetBufferPointer(), 
		&m_pEnergy->GetBufferPointer()[nStride], 
		m_pEnergy->GetBufferedRegion().GetSize()[0] * m_pEnergy->GetBufferedRegion().GetSize()[1] * sizeof(VOXEL_REAL));
	}
#endif

	// set pencil beam
	m_pBeam->m_arrBeamlets.push_back(m_pEnergy); // m_pTerma); // // pEnergy2D);
	m_pEnergy = NULL;
	m_pTerma = NULL;	// reset so that new one will be generated

}	// CBeamDoseCalc::CalcBeamlet


// consts for index positions
const int X = 1;
const int Y = 2;
const int Z = 0;

///////////////////////////////////////////////////////////////////////////////////////
void CBeamDoseCalc::SetBeamletMinMax(const Vector<REAL,2>& vMin_in,
				const Vector<REAL,2>& vMax_in)
	// sets the rectangular region for the current beamlet, in IEC beam coordinates on
	//		the isocentric plane
{
	// vPixSpacing is spacing factor for three coordinates
	//		= conversion from voxel to physical coordinates
	const itk::Vector<REAL>& vPixSpacing = m_densityRep->GetSpacing();

	// calculate conversion from isocenter to top boundary
	const REAL convIso2Top = 
		(-0.5 - m_vSource_vxl[Z])		// z-distance from upper boundary to source
			/ (m_vIsocenter_vxl[Z] - m_vSource_vxl[Z]);

	// set up starting min vector position in voxel coords
	m_vMin_vxl = m_vIsocenter_vxl;
	m_vMin_vxl[X] += convIso2Top * vMin_in[0] / vPixSpacing[X];
	m_vMin_vxl[Y] += convIso2Top * vMin_in[1] / vPixSpacing[Y];

	// set up max vector position in voxel coords
	m_vMax_vxl = m_vIsocenter_vxl;
	m_vMax_vxl[X] += convIso2Top * vMax_in[0] / vPixSpacing[X];
	m_vMax_vxl[Y] += convIso2Top * vMax_in[1] / vPixSpacing[Y];

	// now set starting z-coord (at upper boundary of voxels)
	m_vMin_vxl[Z] = m_vMax_vxl[Z] = -0.5;
}



///////////////////////////////////////////////////////////////////////////////
void 
	CBeamDoseCalc::CalcTerma()
	// Calculates TERMA for the given source geometry, and mass density field
	// vMin, vMax in physical coords at isocentric plane
{
	// construct and initialize the terma volume
	m_pTerma = VolumeReal::New();
	ConformTo<VOXEL_REAL,3>(m_densityRep, m_pTerma);
	m_pTerma->FillBuffer(0.0);

	// based on rays per voxel -- in voxel coordinates
	const REAL deltaRay = 1.0 / m_raysPerVoxel;

	// initial fluence per ray -- set so incident fluence is constant
	const Vector<REAL>& vPixSpacing = m_densityRep->GetSpacing();
	const REAL fluence0 = vPixSpacing[X] * vPixSpacing[Y] * deltaRay * deltaRay;

	// initialize surface integral of fluence 
	m_fluenceSurfIntegral = 0.0;

	// iterate over X & Y voxel positions
	for (Vector<REAL> vX = m_vMin_vxl; vX[X] < m_vMax_vxl[X]; vX[X] += deltaRay)
	{
		for (Vector<REAL> vY = vX; vY[Y] < m_vMax_vxl[Y]; vY[Y] += deltaRay)
		{
			TraceRayTerma(vY, fluence0);
		}	
	}	
}

///////////////////////////////////////////////////////////////////////////////
REAL 
	MinDistToIntersectPlan(const Vector<REAL>& vRay,
			const Vector<REAL>& vDir,
			VolumeReal::IndexType& nNdx)
	// Helper function to compute min distance of a vector component to interset the
	//	next plane at a 0.5-boundary
{
	const REAL EPS = (REAL) 1e-6;
	REAL minDist = 1e+6;

	for (int nDim = 0; nDim <= 2; nDim++)
	{
		if (vDir[nDim] > 0)
		{
			nNdx[nDim] = (int) floor(vRay[nDim] + 0.5 + EPS);
			minDist = __min(minDist, 
				((REAL(nNdx[nDim]) + 0.5) - vRay[nDim]) / vDir[nDim]);
		}
		else
		{
			nNdx[nDim] = (int) ceil(vRay[nDim] - 0.5 - EPS);
			minDist = __min(minDist, 
				((REAL(nNdx[nDim]) - 0.5) - vRay[nDim]) / vDir[nDim]);
		}
	}

	return minDist;
}

//////////////////////////////////////////////////////////////////////////////
void 
	CBeamDoseCalc::TraceRayTerma(Vector<REAL> vRay, const REAL fluence0)
{
	// unit ray direction vector (voxel coordinates)
	Vector<REAL> vDir = vRay - m_vSource_vxl;
	vDir.Normalize();

	// calc physical length of vDir, in cm!!! (not mm)
	const REAL dirLength = GetPhysicalLength(vDir);

	// stores mu
	const REAL mu = m_pKernel->Get_mu();

	// initialize radiological path length for raytrace
	REAL path = 0.0;

	// increment by incident fluence
	m_fluenceSurfIntegral += fluence0;

	// stores current voxel indices
	VolumeReal::IndexType nNdx;

	// compute initial min distance for the ray
	REAL minDist = MinDistToIntersectPlan(vRay, vDir, nNdx);

	// iterate ray trace until volume boundary is reached 
	// TODO: with nNdx as an IndexType, is there a function to directly test for containment?
	while (nNdx[X] >= 1 
		&& nNdx[X] < m_pTerma->GetBufferedRegion().GetSize()[1]-1
		&& nNdx[Y] >= 1 
		&& nNdx[Y] < m_pTerma->GetBufferedRegion().GetSize()[2]-1
		&& nNdx[Z] < m_pTerma->GetBufferedRegion().GetSize()[0]-1)
	{
		// compute avg position of ray within voxel
		//		use this position for trilinear weights
		Vector<REAL> vPos = vRay + (REAL) (0.5 * minDist) * vDir;

		// compute tri-linear interpolation weights
		REAL weights[3][3];
		// interpolate the deltaPath from the density, storing trilinear weights for later use
		REAL deltaPath = TrilinearInterpDensity(vPos, nNdx, weights);

		// update radiological path for length of partial volume traversal
		deltaPath *= minDist * dirLength;

		// update cumulative path
		path += deltaPath;

		// add to terma = -(derivative of fluence along ray)	
		REAL fluenceInc = fluence0 * exp(-mu * path) * mu * deltaPath;

		// update the neighborhood terma voxels using the previously calculated trilinear weights
		UpdateTermaNeighborhood(nNdx, weights, fluenceInc);

		// update current ray position to move to the given plane intersection
		vRay += minDist * vDir;

		// find next min distance
		minDist = MinDistToIntersectPlan(vRay, vDir, nNdx);

	}	// while

	// reduce by amount of remaining (un-attenuated) fluence exiting volume
	m_fluenceSurfIntegral -= fluence0 * exp(-mu * path) ;
}

///////////////////////////////////////////////////////////////////////////////
REAL 
	CBeamDoseCalc::GetPhysicalLength(const Vector<REAL>& vDir)
	// Helper function to calculate physical length of a normalized direction
	//		vector, based on the pixel spacing
	// NOTE: Returned value is in CM, not MM!
{
	const Vector<REAL>& vPixSpacing = m_densityRep->GetSpacing();
	Vector<REAL> vDirPhysical = vDir;
	for (int n = 0; n < 2; n++)
		vDirPhysical[n] *= vPixSpacing[n];
	return 0.1 * vDirPhysical.GetNorm();
}

///////////////////////////////////////////////////////////////////////////////
REAL 
	CBeamDoseCalc::TrilinearInterpDensity(const Vector<REAL>& vPos, 
														const VolumeReal::IndexType& nNdx, 
														REAL (&weights)[3][3])
	// Helper function to trilinear interpolate the density rep at the given 
	//		position
{
	// calculate trilinear weights
	for (int nDim = 0; nDim < 3; nDim++)
	{
		weights[nDim][-1 + 1] = vPos[nDim] < (REAL) nNdx[nDim] 
			? fabs((REAL) nNdx[nDim] - vPos[nDim]) : 0.0;
		weights[nDim][ 0 + 1] = 1.0 - fabs((REAL) nNdx[nDim] - vPos[nDim]);
		weights[nDim][ 1 + 1] = vPos[nDim] > (REAL) nNdx[nDim] 
			? fabs((REAL) nNdx[nDim] - vPos[nDim]) : 0.0;
	}

	// delta path = radiological path (based on mass density) through voxel
	REAL density = 0.0;	
	VolumeReal::OffsetType offset;
	for (offset[Z] = -1; offset[Z] <= 1; offset[Z]++)
	{
		for (offset[Y] = -1; offset[Y] <= 1; offset[Y]++)
		{
			for (offset[X] = -1; offset[X] <= 1; offset[X]++)
			{
				density += 
					weights[X][offset[X]+1] 
					* weights[Y][offset[Y]+1] 
					* weights[Z][offset[Z]+1] 
					* m_densityRep->GetPixel(nNdx+offset);
			}
		}
	}

	return density;
}

///////////////////////////////////////////////////////////////////////////////
void
	CBeamDoseCalc::UpdateTermaNeighborhood(
														const VolumeReal::IndexType& nNdx, 
														REAL (&weights)[3][3], 
														REAL value)
	// Helper function to trilinear interpolate the density rep at the given 
	//		position
{
	// iterate over neighborhood
	VolumeReal::OffsetType offset;
	for (offset[Z] = -1; offset[Z] <= 1; offset[Z]++)
	{
		for (offset[Y] = -1; offset[Y] <= 1; offset[Y]++)
		{
			for (offset[X] = -1; offset[X] <= 1; offset[X]++)
			{
				//	use trilinear interpolation weights to update all neighboring 
				//		terma voxels
				m_pTerma->GetPixel(nNdx+offset) += (VOXEL_REAL)
					( weights[X][offset[X]+1] 
					* weights[Y][offset[Y]+1] 
					* weights[Z][offset[Z]+1] 
					* value );
			}
		}
	}
}
