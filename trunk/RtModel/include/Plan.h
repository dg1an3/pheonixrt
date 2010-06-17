// Copyright (C) 2nd Messenger Systems
// $Id: Plan.h 640 2009-06-13 05:06:50Z dglane001 $
#if !defined(PLAN_H)
#define PLAN_H

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
	class PlanPyramid;
}

//////////////////////////////////////////////////////////////////////
// class CPlan
//
// represents a treatment plan
//////////////////////////////////////////////////////////////////////
class CPlan : public CModelObject
{
public:
	// constructor
	CPlan();
	virtual ~CPlan();

#ifdef USE_MFC_SERIALIZATION
	// dynamic create
	DECLARE_SERIAL(CPlan)
#endif

	// series accessor
	DECLARE_ATTRIBUTE_PTR_GI(Series, dH::Series);

	// histogram accessor / creator
	CHistogram *GetHistogram(dH::Structure *pSurface, bool bCreate);
	void RemoveHistogram(dH::Structure *pStructure);

	// the beams for this plan
	int GetBeamCount() const;
	CBeam * GetBeamAt(int nAt);
	int AddBeam(CBeam *pBeam);

	// helper functions
	int GetTotalBeamletCount();

	// helper to get formatted mass density volume
	/// TODO: move this to dose calc
	VolumeReal * GetMassDensity();

	// the computed dose for this plan (NULL if no dose exists)
	VolumeReal * GetDoseMatrix();

	// calls update on all internal histograms
	void UpdateAllHisto();

	// sets shape for dose matrix
	DECLARE_ATTRIBUTE_GI(DoseResolution, REAL);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlan)
	public:
#ifdef USE_MFC_SERIALIZATION
	virtual void Serialize(CArchive& ar);
#endif
	//}}AFX_VIRTUAL

// Implementation
public:

	// stores the energy dep kernel
	/// TODO: move this elsewhere
	CEnergyDepKernel * m_pKernel;


#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	friend class dH::PlanPyramid;

	// the plan's beams
	CTypedPtrArray<CPtrArray, CBeam*> m_arrBeams;

private:
	// storing resampled mass density
	VolumeReal::Pointer m_pMassDensity;

public:
	// the dose matrix for the plan
	VolumeReal::Pointer m_pDose;

	// helper volumes for dose summation
	VolumeReal::Pointer m_pTempBuffer;
	VolumeReal::Pointer m_pBeamDoseRot;

private:
	// the histograms
	CTypedPtrMap<CMapStringToOb, CString, CHistogram*> m_mapHistograms;

};	// class CPlan

#ifdef USING_CLI
namespace dH
{

public ref class Plan : public System::Object
{
	Plan();

	property System::String Name;
	property System::Collection::Generic::List<Beam^>^ Beams;
};

}
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLAN_H__71D1495A_EE39_11D4_9E36_00B0D0609AB0__INCLUDED_)
