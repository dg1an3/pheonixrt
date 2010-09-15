// Copyright (C) 2nd Messenger Systems
// $Id: Plan.h 640 2009-06-13 05:06:50Z dglane001 $
#if !defined(_PLAN_H__INCLUDED_)
#define _PLAN_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// series upon which plan is based
#include <Series.h>

// beams belonging to the plan
#include <Beam.h>

// histograms for the plan
#ifdef USE_RTOPT
#include <Histogram.h>
#endif

class CHistogram;
class CEnergyDepKernel;

namespace dH
{

// forward declaration for pyramid class
class PlanPyramid;

/**
 * Plan represents a treatment plan: beams, dose, DVHs
 */
class Plan : public DataObject
{
	Plan();
	virtual ~Plan();

public:
	/** itk typedefs */
	typedef Plan Self;
	typedef DataObject Superclass;
	typedef SmartPointer<Self> Pointer;
	typedef SmartPointer<const Self> ConstPointer;

	itkNewMacro(Self);

	/** series accessor */
	DECLARE_ATTRIBUTE_PTR_GI(Series, dH::Series);

	/** histogram accessor / creator */
	CHistogram *GetHistogram(dH::Structure *pSurface, bool bCreate);
	void RemoveHistogram(dH::Structure *pStructure);

	/** the beams for this plan */
	int GetBeamCount() const;
	CBeam * GetBeamAt(int nAt);
	int AddBeam(CBeam *pBeam);

	/** helper functions */
	int GetTotalBeamletCount();

	/** helper to get formatted mass density volume */
	VolumeReal * GetMassDensity();

	/** the computed dose for this plan (NULL if no dose exists) */
	VolumeReal * GetDoseMatrix();

	/** calls update on all internal histograms */
	void UpdateAllHisto();

	/** sets shape for dose matrix */
	DECLARE_ATTRIBUTE_GI(DoseResolution, REAL);

	/** stores the energy dep kernel */
	CEnergyDepKernel * m_pKernel;

protected:
	friend class dH::PlanPyramid;

	/** the plan's beams */
	std::vector< dH::Beam::Pointer > m_arrBeams;

private:
	/** storing resampled mass density */
	VolumeReal::Pointer m_pMassDensity;

public:
	/** the dose matrix for the plan */
	VolumeReal::Pointer m_pDose;

	/** helper volumes for dose summation */
	VolumeReal::Pointer m_pTempBuffer;
	VolumeReal::Pointer m_pBeamDoseRot;

private:
	/** the histograms */
	CTypedPtrMap<CMapStringToOb, CString, CHistogram*> m_mapHistograms;

};	// class Plan

}

typedef dH::Plan CPlan;

#endif // !defined(_PLAN_H__INCLUDED_)
