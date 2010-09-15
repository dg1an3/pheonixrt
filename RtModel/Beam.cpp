// Copyright (C) 2nd Messenger Systems
// $Id: Beam.cpp 640 2009-06-13 05:06:50Z dglane001 $
#include "stdafx.h"

#include <itkEuler3DTransform.h>

// class declaration
#include <Beam.h>
#include <Plan.h>

namespace dH
{

///////////////////////////////////////////////////////////////////////////////
Beam::Beam()
	// constructs a new CBeam object
	: m_pPlan(NULL)
		, m_gantryAngle(PI)
		, m_bRecalcDose(TRUE)
		, m_bRecalcBeamlets(true)
{
	m_vBeamletWeights = IntensityMap::New();
	m_dose = VolumeReal::New();
	m_doseAccumBuffer = VolumeReal::New();

}

//////////////////////////////////////////////////////////////////////
Beam::~Beam()
	// destroys the CBeam object
{
}	

//////////////////////////////////////////////////////////////////////
double 
	Beam::GetGantryAngle() const
{
	return m_gantryAngle;
}

//////////////////////////////////////////////////////////////////////
void 
	Beam::SetGantryAngle(double gantryAngle)
	// sets the gantry angle value
{
	m_gantryAngle = gantryAngle;

	// make sure the plan's dose matrix is initialized
	GetPlan()->SetDoseResolution(GetPlan()->GetDoseResolution());

	// now set up the rotated dose matrix
	ConformTo<VOXEL_REAL,3>(GetPlan()->m_pDose, m_dose);

	itk::Euler3DTransform<REAL>::Pointer rotXform = itk::Euler3DTransform<REAL>::New();
	rotXform->SetRotation(0.0, 0.0, gantryAngle);
	
	// set the center of rotation
	itk::Vector<REAL, 3> vCenter_vxl;
	vCenter_vxl[0] = m_dose->GetBufferedRegion().GetSize()[0]/2;
	vCenter_vxl[1] = m_dose->GetBufferedRegion().GetSize()[1]/2;
	vCenter_vxl[2] = m_dose->GetBufferedRegion().GetSize()[2]/2;

	itk::Matrix<REAL, 4, 4> mDoseBasis;
	CalcBasis<3>(m_dose, mDoseBasis);
	itk::Vector<REAL, 3> vCenter;
	MultHG(mDoseBasis, vCenter_vxl, vCenter);
	rotXform->SetCenter(MakePoint<3>(vCenter));

	itk::Matrix<REAL, 3, 3> mRot = 
		rotXform->GetMatrix() * m_dose->GetDirection();
	m_dose->SetDirection(mRot);

	itk::Point<REAL, 3> vOrigin = rotXform->TransformPoint(m_dose->GetOrigin());
	m_dose->SetOrigin(vOrigin);
	
	/// TODO: this should be done using the dose calc region

#ifdef USING_MATRIXHG
	CMatrixD<4> mDoseBasis = m_dose.GetMatrixHG();

	// rotate basis to beam orientation
	CMatrixD<2> mRotateBasis2D = CreateRotate(gantryAngle);

	CMatrixD<4> mRotateBasisHG;
	mRotateBasisHG[0][0] = mRotateBasis2D[0][0];
	mRotateBasisHG[0][1] = mRotateBasis2D[0][1];
	mRotateBasisHG[1][0] = mRotateBasis2D[1][0];
	mRotateBasisHG[1][1] = mRotateBasis2D[1][1];

	// rotate about center of volume
	CMatrixD<4> mBeamBasis = mRotateBasisHG * mDoseBasis;

	m_dose.SetMatrixHG(mBeamBasis);
	TRACE_MATRIX("CBeam::m_dose Basis", mBeamBasis);
#endif

	// flag that change has occurred
	Modified();
}

///////////////////////////////////////////////////////////////////////////////
int 
	Beam::GetBeamletCount()
{
	return (int) m_arrBeamlets.size(); 
}	

/////////////////////////////////////////////////////////////////////////////// 
VolumeReal * 
	Beam::GetBeamlet(int nShift)
{
	int nBeamletAt = nShift + GetBeamletCount() / 2;
	if (nBeamletAt >= 0 
		&& nBeamletAt < m_arrBeamlets.size())
	{
		return m_arrBeamlets[nBeamletAt];
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
Beam::IntensityMap * 
	Beam::GetIntensityMap() const
{
	return m_vBeamletWeights;
}

///////////////////////////////////////////////////////////////////////////////
void 
	Beam::SetIntensityMap(const CVectorN<>& vWeights)
{
	m_vBeamletWeights->SetRegions(MakeSize(vWeights.GetDim()));
	m_vBeamletWeights->Allocate();
	for (int nAt = 0; nAt < vWeights.GetDim(); nAt++)
		m_vBeamletWeights->GetBufferPointer()[nAt] = vWeights[nAt];

	// flag dose recalc
	m_bRecalcDose = TRUE;

	// flag that change has occurred
	Modified();
}

///////////////////////////////////////////////////////////////////////////////
void 
	Beam::OnIntensityMapChanged() 
{
	// must be odd-sized
	ASSERT(m_vBeamletWeights->GetBufferedRegion().GetSize()[0] % 2 == 1);

	// set up the number of beamelts
	if (m_vBeamletWeights->GetBufferedRegion().GetSize()[0] != m_arrBeamlets.size())
	{
		m_arrBeamlets.clear();
		for (int nAt = 0; nAt < m_vBeamletWeights->GetBufferedRegion().GetSize()[0]; nAt++)
			m_arrBeamlets.push_back(VolumeReal::New());
	}

	// flag dose recalc
	m_bRecalcDose = TRUE;

	// flag that change has occurred
	Modified();
}

//////////////////////////////////////////////////////////////////////
VolumeReal *
	Beam::GetDoseMatrix()
	// the computed dose for this beam (NULL if no dose exists)
{
	if (m_bRecalcDose 
		 && m_vBeamletWeights->GetBufferedRegion().GetSize()[0] == m_arrBeamlets.size())
		 // && m_vBeamletWeights.GetDim() == m_arrBeamlets.size())
	{ 
		// set dose matrix size
		ConformTo<VOXEL_REAL,3>(m_arrBeamlets[0], m_dose);

		// clear voxels for accumulation
		m_dose->FillBuffer(0.0);

		for (int nAt = 0; nAt < m_arrBeamlets.size(); nAt++)
		{
			VolumeReal *pBeamlet = m_arrBeamlets[nAt];
			ConformTo<VOXEL_REAL,3>(m_dose, m_doseAccumBuffer);
			Accumulate3D<VOXEL_REAL>(pBeamlet, m_vBeamletWeights->GetBufferPointer()[nAt], m_dose, 
				m_doseAccumBuffer); 
		}

		m_bRecalcDose = FALSE;
		DataHasBeenGenerated();
	}  

	return m_dose;

}

}
