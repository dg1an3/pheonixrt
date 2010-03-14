// Copyright (C) 2nd Messenger Systems
// $Id: EnergyDepKernel.cpp 602 2008-09-14 16:54:49Z dglane001 $
#include "stdafx.h"
#include "EnergyDepKernel.h"

#include <direct.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CEnergyDepKernel::CEnergyDepKernel(REAL energy)
	: m_energy(energy)
{
	LoadKernel();
}

//////////////////////////////////////////////////////////////////////
CEnergyDepKernel::~CEnergyDepKernel()
{
}


////////////////////////////////////////////////////////////////////////////////////////
inline int 
	CEnergyDepKernel::GetNumPhi()
{
	return m_mCumEnergy.GetCols();
}

////////////////////////////////////////////////////////////////////////////////////////
inline const VolumeReal::OffsetType& 
	CEnergyDepKernel::GetIndexOffset(int nTheta, int nPhi, int nRadial)																			
{
	return m_radialToOffset[nTheta-1][nPhi-1][nRadial-1];
}

////////////////////////////////////////////////////////////////////////////////////////
inline double 
	CEnergyDepKernel::GetRadius(int nTheta, int nPhi, int nRadial)
	// returns radius in cm
{
	return m_radius[nTheta-1][nPhi-1][nRadial];
}

////////////////////////////////////////////////////////////////////////////////////////
inline double 
	CEnergyDepKernel::GetCumEnergy(int nPhi, double rad_dist /* cm */)
{
	// The resolution of the array in the radial direction is every mm
	//	hence the multiply by 10.0 in the arguement.
	int nRadial = (int) floor(rad_dist * 10.0 + 0.5);

	// check for overflow
	nRadial = __min(nRadial, m_mCumEnergy.GetRows()-1);

	return m_mCumEnergy[nPhi-1][nRadial];
}

//////////////////////////////////////////////////////////////////////////////
VolumeReal::Pointer
	CEnergyDepKernel::CalcSphereConvolve(VolumeReal *pDensity, VolumeReal *pTerma, int nSlice) 
	// spherical convolution
{
	// initialize energy
	VolumeReal::Pointer pEnergy = VolumeReal::New();
	ConformTo<VOXEL_REAL,3>(pTerma, pEnergy);
	pEnergy->FillBuffer(0.0);

	// set up pixel spacing
	itk::Vector<REAL> vPixSpacing = pDensity->GetSpacing();

	// convert to cm and set up lookup table for sphere convolve
	vPixSpacing *= (REAL) 0.1;
	SetupRadialLUT(vPixSpacing);

	// Now do the convolution.
	VolumeReal::IndexType nNdx;
	nNdx[2] = nSlice; // Round<int>(m_vIsocenter_vxl[2]);	// TODO: pass this in
	// for (nNdx[2] = 0; nNdx[2] < pDensity->GetBufferedRegion().GetSize()[2]; nNdx[2]++)         
	{
		for (nNdx[1] = 0; nNdx[1] < pDensity->GetBufferedRegion().GetSize()[1]; nNdx[1]++)        
		{
			for (nNdx[0] = 0; nNdx[0] < pDensity->GetBufferedRegion().GetSize()[0]; nNdx[0]++)          
			{
				// dose at zero density?
				if (pDensity->GetPixel(nNdx) > 0.01) 
				{
					// spherical convolution at this point
					CalcSphereTrace(pDensity, pTerma, nNdx, pEnergy); 

					// Convert the energy to dose by dividing by mass
					// convert to Gy cm**2 and take into account the azimuthal sum
					pEnergy->GetPixel(nNdx) *= 1.0 / (REAL) NUM_THETA;
					// 	(VOXEL_REAL) (1.602e-10 / (REAL) NUM_THETA);

					if (pDensity->GetPixel(nNdx) > 0.25)
						pEnergy->GetPixel(nNdx) /= pDensity->GetPixel(nNdx);
					else
						pEnergy->GetPixel(nNdx) = 0.0;
				}
			}
		}
	}

	// now copy isocenter slice to others
	int nCount = pEnergy->GetBufferedRegion().GetSize()[1] * pEnergy->GetBufferedRegion().GetSize()[0];
	VOXEL_REAL *pSrc = &pEnergy->GetBufferPointer()[nSlice * nCount];
	for (nNdx[2] = 0; nNdx[2] < pEnergy->GetBufferedRegion().GetSize()[2]; nNdx[2]++)         
	{
		if (nNdx[2] != nSlice)
		{
			VOXEL_REAL *pDst = &pEnergy->GetBufferPointer()[nNdx[2] * nCount];
			CopyValues<VOXEL_REAL>(pDst, pSrc, nCount);
		}
	}

	typedef itk::ImageRegionConstIterator< VolumeReal > ConstIteratorType;
	typedef itk::ImageRegionIterator< VolumeReal > IteratorType;

	// get new maximum for energy dist
	REAL dmax = -1e+8;
	ConstIteratorType inputIt( pEnergy, pEnergy->GetBufferedRegion() );
	for ( inputIt.GoToBegin(); !inputIt.IsAtEnd(); ++inputIt)
	{
		dmax = __max(inputIt.Get(), dmax);
	}

	// now normalize to dmax
	if (dmax > 0.0)
	{
		IteratorType outputIt( pEnergy, pEnergy->GetBufferedRegion() );
		for ( outputIt.GoToBegin(); !outputIt.IsAtEnd(); ++outputIt)
		{
			outputIt.Value() /= dmax;
		}
	}

	return pEnergy;

}	// CEnergyDepKernel::CalcSphereConvolve

///////////////////////////////////////////////////////////////////////////////
void 
	CEnergyDepKernel::CalcSphereTrace(VolumeReal *pDensity, VolumeReal *pTerma, 
			const VolumeReal::IndexType& nNdx, VolumeReal *pEnergy)
	// helper function to convolve at a single point in the energy volume
{
	// TODO: move this to EnergyDepKernel
	const REAL kernelDensity = 1.0;

	// do for all azimuthal angles
	for (int nTheta = 1; nTheta <= NUM_THETA; nTheta++)            
	{
		// do for zenith angles 
		for (int nPhi = 1; nPhi <= GetNumPhi(); nPhi++)
		{
			// stores total radiological distance traversed
			REAL radDist = 0.0;

			// stores previous cumulative energy value
			REAL prevEnergy = 0.0;
			
			// loop over radial increments
			for (int nRadial = 1; nRadial <= NUM_RADIAL_STEPS; nRadial++)       
			{
				// integer distances between the interaction and the dose depostion voxels
				VolumeReal::IndexType nKernelNdx = nNdx;
				nKernelNdx -= GetIndexOffset(nTheta, nPhi, nRadial);
				if (!pDensity->GetBufferedRegion().IsInside(nKernelNdx))
				{
					break;
					// continue;
				}

				// compute physical path length increment (in cm)
				REAL deltaPhysDist = GetRadius(nTheta, nPhi, nRadial);

				// compute radiological path length increment
				REAL deltaRadDist = deltaPhysDist 
						* 1.0 // convolution
						// * pDensity->GetPixel(nKernelNdx) // superposition
						/ kernelDensity;
				
				// update radiological path
				radDist += deltaRadDist;

				// quit after 60cm rad dist
				if (radDist > 4.0) // 2.0) // 10.0) // 60.0)
				{
					break;
				}

				// Use lookup table to find the value of the cumulative energy
				// deposited up to this radius. No interpolation is done. 
				REAL totalEnergy = GetCumEnergy(nPhi, radDist); 
			
				// Subtract the last cumulative energy from the new cumulative energy
				// to get the amount of energy deposited in this interval and set the
				// last cumulative energy for the next time the lookup table is used.
				REAL energy = totalEnergy - prevEnergy;
				prevEnergy = totalEnergy;             
				
				// The energy is accumulated - superposition
				pEnergy->GetPixel(nNdx) +=
					(VOXEL_REAL) (energy * pTerma->GetPixel(nKernelNdx));
			}     
		}	
	}

}	// CEnergyDepKernel::CalcSphereTrace


//////////////////////////////////////////////////////////////////////
void CEnergyDepKernel::LoadKernel()
{
	CString strFilename;

	// form current path
	HMODULE hCurrModule = ::GetModuleHandle(NULL);
	::GetModuleFileName(hCurrModule, strFilename.GetBuffer(255), 255);
	strFilename.ReleaseBuffer();

	int nLastSlash = strFilename.ReverseFind('\\');
	strFilename = strFilename.Left(nLastSlash);

	if (IsApproxEqual(m_energy, 15.0))
	{
		strFilename += "\\15MV_kernel.dat";
		Set_mu(1.941E-02);
	}
	else if (IsApproxEqual(m_energy, 6.0))
	{
		strFilename += "\\6MV_kernel.dat";
		Set_mu(2.770E-02);
	}
	else if (IsApproxEqual(m_energy, 2.0))
	{
		strFilename += "\\2MV_kernel.dat";
		Set_mu(4.942E-02);

	}
	else
	{
		ASSERT(FALSE);
	}

	// The dose spread arrays produced by SUM_ELEMENT.FOR are read.
	FILE *pFile = NULL;
	_tfopen_s(&pFile, strFilename, _T("rt"));
	if (pFile == NULL)
	{
		::AfxMessageBox(_T("Problem reading kernel..."));
		return;
	}

	static char pszLine[10000];
	
	fscanf_s(pFile, "%[^\n]\n", pszLine, 1000);
	fscanf_s(pFile, "%[^\n]\n", pszLine, 1000);

	int nNumPhiIn;
	int nNumRadIn;

	fscanf_s(pFile, "%i\n", &nNumPhiIn);	
	fscanf_s(pFile, "%i\n", &nNumRadIn);	

	fscanf_s(pFile, "%[^\n]\n", pszLine, 1000);
	fscanf_s(pFile, "%[^\n]\n", pszLine, 1000);
	
	
	// set up increment energy array
	CMatrixNxM<REAL> mIncEnergyIn;
	mIncEnergyIn.Reshape(nNumPhiIn+1, nNumRadIn+1);
	for (int nA = 1; nA <= nNumPhiIn; nA++)
	{
		mIncEnergyIn[nA-1][0] = 0.0;
		for (int nR = 1; nR <= nNumRadIn; nR++)
		{
			// read dose spread values      
			fscanf_s(pFile, "%lf", &mIncEnergyIn[nA-1][nR]);
			mIncEnergyIn[nA-1][nR] += mIncEnergyIn[nA-1][nR-1];
		}
	}  

	// read (1,*)
	fscanf_s(pFile, "%s", pszLine, 1000);
	fscanf_s(pFile, "%[^\n]\n", pszLine, 1000);		
	
	// set up angle vector
	m_vAnglesIn.SetDim(nNumPhiIn+1);
	for (int nA = 1; nA <= nNumPhiIn; nA++)
	{        
		// read mean angle of spherical voxels 
		fscanf_s(pFile, "%lf", &m_vAnglesIn[nA-1]);      
	} 
	
	// set up radial bounds vector
	// read (1,*)
	fscanf_s(pFile, "%s", pszLine, 1000);	
	fscanf_s(pFile, "%[^\n]\n", pszLine, 1000);	
	
	CVectorN<REAL> vRadialBoundsIn;
	vRadialBoundsIn.SetDim(nNumRadIn+1);
	vRadialBoundsIn[0] = 0.0;
	for (int nR = 1; nR <= nNumRadIn; nR++)
	{              
		// read radial boundaries of spherical voxels
		fscanf_s(pFile, "%lf", &vRadialBoundsIn[nR]);   
	} 

	fclose(pFile);

	// now interpolate values to mm resolution
	InterpCumEnergy(mIncEnergyIn, vRadialBoundsIn);
}


//////////////////////////////////////////////////////////////////////////////
void 
	CEnergyDepKernel::InterpCumEnergy(const CMatrixNxM<>& mIncEnergy, 
									   const CVectorN<>& vRadialBounds)
	// looks up the energies for a phi angle setting 
{
	// TODO: fix hard-coding for 60.0 cm
	m_mCumEnergy.Reshape(mIncEnergy.GetCols(), 600);

	// these should be read in the table
	for (int nPhi = 1; nPhi <= m_mCumEnergy.GetCols()-1; nPhi++)
	{
		m_mCumEnergy[nPhi-1][0] = 0.0;
		
		int nRadial = 1;
		for (int nI = 1; nI < m_mCumEnergy.GetRows(); nI++)
		{
			// distance in cm
			double radialDist = 0.1 * nI;
			
			while (nRadial < vRadialBounds.GetDim()
				&& vRadialBounds[nRadial] < radialDist)
			{
				nRadial++;
			}

			if (nRadial < vRadialBounds.GetDim())
			{
				// energy at lower boundary
				double incEnergy = mIncEnergy[nPhi-1][nRadial-1];

				// increase in energy between lower and upper
				double incEnergyDelta = mIncEnergy[nPhi-1][nRadial] - incEnergy;

				// linear interpolate using lookup table values
				incEnergy += incEnergyDelta 
					* (radialDist - vRadialBounds[nRadial-1]) 
						/ (vRadialBounds[nRadial] - vRadialBounds[nRadial-1]);

				m_mCumEnergy[nPhi-1][nI] = incEnergy;
			}
			else
			{
				// if over end of radial bounds, just fill out with same value
				m_mCumEnergy[nPhi-1][nI] = m_mCumEnergy[nPhi-1][nI-1];
			}
		}
	}

}	// CEnergyDepKernel::InterpCumEnergy


//////////////////////////////////////////////////////////////////////////////
void 
	CalcBoundaryOffsets(const Vector<REAL>& vDir,	// direction vector
					int nDim, // dimension to use to form steps
				  REAL (&radius_out)[NUM_RADIAL_STEPS],      // list of lengths thru a voxel
					VolumeReal::OffsetType (&offset)[NUM_RADIAL_STEPS])	// list of offsets
{                 
	// At each ray-trace step determine the radius and the voxel number 
	// along each coordinate direction.                       
	for (int nI = 0; nI < NUM_RADIAL_STEPS; nI++)
	{
        // distance to the end of a voxel
		REAL d = REAL(nI) + 0.5;                 
		
		if (fabs(vDir[nDim]) >= 1e-04) 
		{	
			// physical radius to intersection point
			radius_out[nI] = fabs(d / vDir[nDim]);
		}
		else
		{
			// effectively infinity
			radius_out[nI] = 100000.0;                 
		}
		
		// Calculate a distance along a coordinate direction and find the nearest
		// integer to specify the voxel direction.
		for (int nN = 0; nN < 3; nN++)
		{
			REAL factor = (nN == nDim) ? 0.99 : 1.0;	
				// 0.99 prevents being exactly on the voxel boundary
			offset[nI][nN] = Round<int>(factor * radius_out[nI] * vDir[nN]);
		}
	}

}	// CEnergyDepKernel::MakeVector



//////////////////////////////////////////////////////////////////////////////
void 
	CEnergyDepKernel::SetupRadialLUT(const itk::Vector<REAL>& vPixSpacing)
	// sets up the ray trace for conv.
{
	// check if we are already set up
	if (IsApproxEqual<3>(m_vPixSpacing, vPixSpacing))
	{
		return;
	}
	
	// loop thru all zenith angles
	for (int nPhi = 1; nPhi <= m_vAnglesIn.GetDim()-1; nPhi++)              
	{
		// trig. for zenith angles
		const REAL sphi = sin(m_vAnglesIn[nPhi]);
		const REAL cphi = cos(m_vAnglesIn[nPhi]);

		// loop thru all azimuthal angles
		// the azimuthal angle increment is calculated
		const REAL thetaStep = 2.0 * PI / double(NUM_THETA); 
		for (int nTheta = 1; nTheta <= NUM_THETA; nTheta++)                  
		{
			// trig. for azimuthal angles
			const REAL sthet = sin(double(nTheta) * thetaStep);    
			const REAL cthet = cos(double(nTheta) * thetaStep);

			// calculate the direction in physical coordinates
			itk::Vector<REAL> vDir;
			vDir[0] = cphi / vPixSpacing[0];
			vDir[1] = sphi * cthet / vPixSpacing[1];
			vDir[2] = sphi * sthet / vPixSpacing[2];

			// CalcBoundaryOffsets is called for each direction. It returns the distance from
			// the intersection of a plane defined by a coordinate value and the ray along
			// each direction.                                
			
			// Call for the y-z plane crossing list. Plane defined by the x-coordinate.
			REAL radiusX[NUM_RADIAL_STEPS];
			VolumeReal::OffsetType offsetsX[NUM_RADIAL_STEPS];
			CalcBoundaryOffsets(vDir,	0, radiusX, offsetsX);

			// Call for the x-z plane crossing list. Plane defined by the y-coordinate.
			REAL radiusY[NUM_RADIAL_STEPS];
			VolumeReal::OffsetType offsetsY[NUM_RADIAL_STEPS];
			CalcBoundaryOffsets(vDir,	1, radiusY, offsetsY);

			// Call for the x-y plane crossing list. Plane defined by the z-coordinate.		
			REAL radiusZ[NUM_RADIAL_STEPS];
			VolumeReal::OffsetType offsetsZ[NUM_RADIAL_STEPS];
			CalcBoundaryOffsets(vDir, 2, radiusZ,	offsetsZ);

			// counters for the three direction arrays
			int nX = 0;
			int nY = 0;
			int nZ = 0;
			
			// radius at origin is 0
			m_radius[nTheta-1][nPhi-1][0] = 0.0;             
			REAL last_radius = 0.0;
			
			// The following sorts through the distance vectors, rx,ry,rz
			// to find the next smallest value. This will be the next plane crossed.
			// A merged vector is created that lists the location of the voxel crossed
			// and the length thru it in the order of crossings.			
			for (int nN = 1; nN <= NUM_RADIAL_STEPS; nN++)
			{
				// done if plane defined by the x-coord crossed
				if (radiusX[nX] <= radiusY[nY] 
						&& radiusX[nX] <= radiusZ[nZ])
				{
					// length thru voxel
					m_radius[nTheta-1][nPhi-1][nN] = radiusX[nX] - last_radius;	
					last_radius = radiusX[nX];

					// index offset for this point
					m_radialToOffset[nTheta-1][nPhi-1][nN-1] = offsetsX[nX];	
					nX++;											
				}
				// done if plane defined by the y-coord crossed
				else if (radiusY[nY] <= radiusX[nX] 
						&& radiusY[nY] <= radiusZ[nZ])    
				{
					// length thru voxel
					m_radius[nTheta-1][nPhi-1][nN] = radiusY[nY] - last_radius;
					last_radius = radiusY[nY];

					// index offset for this point
					m_radialToOffset[nTheta-1][nPhi-1][nN-1] = offsetsY[nY];	
					nY++;											
				}
				// done if plane defined by the z-coord crossed 
				else if (radiusZ[nZ] <= radiusX[nX] 
						&& radiusZ[nZ] <= radiusY[nY])     
				{
					// length thru voxel
					m_radius[nTheta-1][nPhi-1][nN] = radiusZ[nZ] - last_radius;		
					last_radius = radiusZ[nZ];								

					// index offset for this point
					m_radialToOffset[nTheta-1][nPhi-1][nN-1] = offsetsZ[nZ];
					nZ++;
				}
			}
		}
	}

	// store the pixel spacing used
	m_vPixSpacing = vPixSpacing;

}
