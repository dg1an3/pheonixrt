// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: Prescription.h 640 2009-06-13 05:06:50Z dglane001 $
// #include <ModelObject.h>

#include <ObjectiveFunction.h>
// #include <Optimizer.h>

#include <Histogram.h>
#include <KLDivTerm.h>

#include <Structure.h>
#include <Plan.h>

#pragma once

namespace dH
{

class PlanOptimizer;

///////////////////////////////////////////////////////////////////////////////
// class CPrescription
// 
// represents the Prescription objective function
///////////////////////////////////////////////////////////////////////////////
class Prescription : public DynamicCovarianceCostFunction /* CObjectiveFunction */
{
public:
	// constructors / destructors
	Prescription(CPlan *pPlan = NULL/*, int nLevel = 0*/);
	virtual ~Prescription();

	// my plan
	DECLARE_ATTRIBUTE_PTR(Plan, CPlan);

	// for 2D operation, determines slice to use
	DECLARE_ATTRIBUTE(Slice, int);

	// independent terms of the function
	VOITerm *GetStructureTerm(Structure *pStruct);

	// accessors for the structure terms
	void AddStructureTerm(VOITerm *pST);
	void RemoveStructureTerm(Structure *pStruct);

	// updates term parameters from another CPrescription object
	void UpdateTerms(Prescription *pPresc);

	// sets up adaptive variance
	void SetGBinVar(REAL varMin, REAL varMax);

	//////////////////////////////////////////////////////////////////////////
	// optimization and helpers

	// evaluates the objective function
	virtual REAL operator()(const CVectorN<>& vInput, 
		CVectorN<> *pGrad = NULL) const;

	// flag to indicate whether the transform slope variance correction should be applied
	DECLARE_ATTRIBUTE(TransformSlopeVariance, bool);

	// initial step in objective function -- forming sum and histogram
	void CalcSumSigmoid(CHistogramWithGradient *pHisto, const CVectorN<>& vInput,
		const CVectorN<>& vInputTrans,
		const CArray<BOOL, BOOL>& arrInclude) const;

	// transform function from linear to other parameter space
	virtual void Transform(CVectorN<> *pvInOut) const;
	virtual void dTransform(CVectorN<> *pvInOut) const;
	virtual void InvTransform(CVectorN<> *pvInOut) const;

	// mux / demux of vector elements
	void GetBeamletFromSVElem(int nElem, int *pnBeam, int *pnBeamlet) const;

	// helper to update the histogram regions
	void UpdateHistogramRegions();

	// helper to set up element include flags
	void SetElementInclude();

public:
	// sigmoid for parameter transform
	REAL m_inputScale;

	// the sum volume used for histogram
	VolumeReal::Pointer m_sumVolume;

	// helper variables for CalcSumSigmoid eval

	// volGroupMin/MaxVar holds the accumulation of the beamlets for a group, 
	//		scale by the beamlet's var min / max fraction
	mutable VolumeReal::Pointer m_volGroupMaxVar;
	mutable VolumeReal::Pointer m_volGroupMinVar;

	// volGroupMainMin/MaxVar holds the volGroupMin/MaxVar, resampled to the main basis
	mutable VolumeReal::Pointer m_volGroupMainMaxVar;
	mutable VolumeReal::Pointer m_volGroupMainMinVar;

	// volMainMin/MaxVar holds the accumulated var min / max fractions for all groups,
	//		and at the end of CalcSumSigmoid is normalized so that the proper fractions remain
	mutable VolumeReal::Pointer m_volMainMinVar;
	mutable VolumeReal::Pointer m_volMainMaxVar;

	// temp accumulator
	mutable VolumeReal::Pointer m_volTemp;

	// stores the actual (i.e. accounting for transform slope) variance vector
	mutable CVectorN<> m_ActualAV;

	// stores the VOITs
	/// TODO: change this to std::map
	CTypedPtrMap<CMapPtrToPtr, Structure*, VOITerm*> m_mapVOITs;

	// helpers for Eval_TotalEntropy
	mutable CVectorN<> m_vPartGrad;

	// array of flags for element inclusion
	/// TODO: change this to std::vector
	CArray<BOOL, BOOL> m_arrIncludeElement;

};	// class Prescription

}	// namespace dH
