// Copyright (C) 2nd Messenger Systems
// $Id: Beam.cpp 640 2009-06-13 05:06:50Z dglane001 $
#include "stdafx.h"

#include <itkEuler3DTransform.h>

// class declaration
#include <Beam.h>
#include <Plan.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
CBeam::CBeam()
	// constructs a new CBeam object
	: m_pPlan(NULL)
		, m_collimAngle(0.0)
		, m_gantryAngle(PI)
		, m_couchAngle(0.0)
		, m_bRecalcDose(TRUE)
		, m_bRecalcBeamlets(true)
{
	m_vBeamletWeights = IntensityMap::New();
	m_dose = VolumeReal::New();
	m_doseAccumBuffer = VolumeReal::New();

}	// CBeam::CBeam

//////////////////////////////////////////////////////////////////////
CBeam::~CBeam()
	// destroys the CBeam object
{
	// delete the blocks
	//for (int nAt = 0; nAt < m_arrBlocks.GetSize(); nAt++)
	//{
	//	delete m_arrBlocks[nAt];
	//}

}	// CBeam::~CBeam

//////////////////////////////////////////////////////////////////////
double CBeam::GetGantryAngle() const
{
	return m_gantryAngle;

}	// CBeam::GetGantryAngle

//////////////////////////////////////////////////////////////////////
void 
	CBeam::SetGantryAngle(double gantryAngle)
	// sets the gantry angle value
	/// TODO: fix this
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

	// finally change event
	GetChangeEvent().Fire();

}	// CBeam::SetGantryAngle

//////////////////////////////////////////////////////////////////////
double 
	CBeam::GetCouchAngle() const
	// returns the couch angle value
{
	return m_couchAngle;

}	// CBeam::GetCouchAngle

//////////////////////////////////////////////////////////////////////
void 
	CBeam::SetCouchAngle(double couchAngle)
	// sets the couch angle value
{
	m_couchAngle = couchAngle;
	GetChangeEvent().Fire();

}	// CBeam::SetCouchAngle

///////////////////////////////////////////////////////////////////////////////
int 
	CBeam::GetBeamletCount()
{
	return (int) m_arrBeamlets.size(); 

}	// CBeam::GetBeamletCount

/////////////////////////////////////////////////////////////////////////////// 
VolumeReal * 
	CBeam::GetBeamlet(int nShift)
{
	int nBeamletAt = nShift + GetBeamletCount() / 2;
	if (nBeamletAt >= 0 
		&& nBeamletAt < m_arrBeamlets.size())
	{
		return m_arrBeamlets[nBeamletAt];
	}

	return NULL;

}	// CBeam::GetBeamlet

///////////////////////////////////////////////////////////////////////////////
CBeam::IntensityMap * 
	CBeam::GetIntensityMap() const
{
	return m_vBeamletWeights;

}	// CBeam::GetIntensityMap

///////////////////////////////////////////////////////////////////////////////
void 
	CBeam::SetIntensityMap(const CVectorN<>& vWeights)
{
	m_vBeamletWeights->SetRegions(MakeSize(vWeights.GetDim()));
	m_vBeamletWeights->Allocate();
	for (int nAt = 0; nAt < vWeights.GetDim(); nAt++)
		m_vBeamletWeights->GetBufferPointer()[nAt] = vWeights[nAt];

	// flag dose recalc
	m_bRecalcDose = TRUE;

	// fire change
	GetChangeEvent().Fire();

}	// CBeam::SetIntensityMap

///////////////////////////////////////////////////////////////////////////////
void 
	CBeam::OnIntensityMapChanged() 
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

	// fire change
	GetChangeEvent().Fire();
}

//////////////////////////////////////////////////////////////////////
VolumeReal *
	CBeam::GetDoseMatrix()
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
	}  

	return m_dose;

}	// CBeam::GetDoseMatrix

#ifdef USE_MFC_SERIALIZAION

//////////////////////////////////////////////////////////////////////
// CBeam serialization
// 
// supports serialization of beam and subordinate objects
//////////////////////////////////////////////////////////////////////
#define BEAM_SCHEMA 6
	// Schema 1: geometry description, blocks
	// Schema 2: + dose matrix
	// Schema 3: + call to CModelObject base class serialization
	// Schema 4: + weight
	// Schema 5: + beamlets
	// Schema 6: + subbeamlets & beamlets

IMPLEMENT_SERIAL(CBeam, CModelObject, VERSIONABLE_SCHEMA | BEAM_SCHEMA)

//////////////////////////////////////////////////////////////////////
void 
	CBeam::Serialize(CArchive &ar)
	// loads/saves beam to archive
{
	// store the schema for the beam object
	UINT nSchema = ar.IsLoading() ? ar.GetObjectSchema() : BEAM_SCHEMA;

	// call base class for schema >= 3
	if (nSchema >= 3)
	{
		CModelObject::Serialize(ar);
	}
	else
	{
		SERIALIZE_VALUE(ar, m_strName);
	}

	// serialize the machine description
	// m_Machine.Serialize(ar);

	///////////////////////////////////////////////////////

	// machine identification
	/// TODO: get rid of these (except SAD)
	CString m_strManufacturer;
	CString m_strModel;
	CString m_strSerialNumber;

	SERIALIZE_VALUE(ar, m_strManufacturer);
	SERIALIZE_VALUE(ar, m_strModel);
	SERIALIZE_VALUE(ar, m_strSerialNumber);

	// machine geometry description
	double m_SAD = 0.0;	// source-axis distance
	double m_SCD = 0.0;	// source-collimator distance
	double m_SID = 0.0;	// source-image distance

	SERIALIZE_VALUE(ar, m_SAD);
	SERIALIZE_VALUE(ar, m_SCD);
	SERIALIZE_VALUE(ar, m_SID);

	///////////////////////////////////////////////////////


	// serialize angles
	/// TODO: get rid of collim angle
	SERIALIZE_VALUE(ar, m_collimAngle);
	SERIALIZE_VALUE(ar, m_gantryAngle);
	SERIALIZE_VALUE(ar, m_couchAngle);

	// TODO: is this really necessary?
	if (ar.IsLoading())
	{
		m_collimAngle = 0.0;
		m_gantryAngle = 0.0;
		m_couchAngle = 0.0;
	}

	// serialize table parameters
	/// TODO: get rid of table offset
	SERIALIZE_VALUE(ar, m_vTableOffset);

	// serialize the collimator jaw settings
	SERIALIZE_VALUE(ar, m_vCollimMin);
	SERIALIZE_VALUE(ar, m_vCollimMax);

	// serialize the block(s) -- first prepare the array
	/// TODO: get rid of blocks
	//if (ar.IsLoading())
	//{
	//	// delete any existing structures
	//	for (int nAt = 0; nAt < m_arrBlocks.GetSize(); nAt++)
	//	{
	//		delete m_arrBlocks[nAt];
	//	}
	//	m_arrBlocks.SetSize(0);

	//	DWORD nCount = (DWORD) ar.ReadCount();
	//	for (int nAt = 0; nAt < (int) nCount; nAt++)
	//	{
	//		// and add it to the array
	//		m_arrBlocks.Add(new CPolygon());
	//	}
	//}
	//else
	//{
	//	ar.WriteCount(m_arrBlocks.GetSize());
	//}

	//// now serialize the blocks
	//for (int nAt = 0; nAt < m_arrBlocks.GetSize(); nAt++)
	//{
	//	m_arrBlocks[nAt]->Serialize(ar);
	//}

	// check the beam object's schema; only serialize the dose if 
	//		we are storing or if we are loading with schema >= 2
	if (nSchema >= 2)
	{
		// WAS m_bDoseValid flag (deprecated)
		BOOL bDoseValid = TRUE;
		SERIALIZE_VALUE(ar, bDoseValid);

		// serialize the dose matrix
		SerializeVolume<VOXEL_REAL>(ar, m_dose);
	}

	// serialize the beam weight
	if (nSchema >= 4)
	{
		double m_weight = 1.0;
		SERIALIZE_VALUE(ar, m_weight);
	}

	// serialize the beamlets
	int nBeamlets0 = 0;
	if (nSchema >= 5)
	{
		int nLevels = dH::Structure::MAX_SCALES;
		SERIALIZE_VALUE(ar, nLevels);
		ASSERT(nLevels <= dH::Structure::MAX_SCALES);

		/// TODO: get rid of this beamlet serialization (the one below does the trick)
		for (int nAtLevel = 0; nAtLevel < nLevels; nAtLevel++)
		{
			int nBeamlets = 0; 
			SERIALIZE_VALUE(ar, nBeamlets);
			if (nAtLevel == 0) nBeamlets0 = nBeamlets;


			for (int nAtBeamlet = 0; nAtBeamlet < nBeamlets; nAtBeamlet++)
			{
				// dummny volume for serialization of (deprecated) sub beamlets)
				VolumeReal::Pointer ptr = VolumeReal::New();
				SerializeVolume<VOXEL_REAL>(ar, ptr);
			}
		}
		
		// serialize the weights as well
		CVectorN<> vBeamletWeights;
		if (ar.IsStoring())
		{
			vBeamletWeights.SetDim(m_vBeamletWeights->GetBufferedRegion().GetSize()[0]);
			for (int nAt = 0; nAt < vBeamletWeights.GetDim(); nAt++)
				vBeamletWeights[nAt] = m_vBeamletWeights->GetBufferPointer()[nAt];
		}
		SERIALIZE_VALUE(ar, vBeamletWeights);
		if (ar.IsLoading())
		{
			SetIntensityMap(vBeamletWeights);
		}
	}

	// serialize the beamlets + subbeamlets
	if (nSchema >= 6)
	{
		for (int nAtBeamlet = 0; nAtBeamlet < nBeamlets0; nAtBeamlet++)
		{
			if (ar.IsLoading())
			{
				m_arrBeamlets.push_back(VolumeReal::New());
			}
			SerializeVolume<VOXEL_REAL>(ar, m_arrBeamlets[nAtBeamlet]);
		}
	}

	// set the flag to trigger recalc of beamlets
	m_bRecalcBeamlets = true;

}	// CBeam::Serialize

#endif
