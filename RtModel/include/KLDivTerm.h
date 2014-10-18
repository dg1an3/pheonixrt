// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: KLDivTerm.h 640 2009-06-13 05:06:50Z dglane001 $
#pragma once

#include <VOITerm.h>

namespace dH
{

///////////////////////////////////////////////////////////////////////////////
// class KLDivTerm
// 
// represents a Kullback-Liebler divergence match of a DVH to a target curve
///////////////////////////////////////////////////////////////////////////////
class KLDivTerm : public VOITerm
{
	// constructor / destructor
	KLDivTerm(Structure *pStructure = NULL, REAL weight = 0.0);
	virtual ~KLDivTerm();

public:
	// standard class typedefs
	typedef KLDivTerm Self;
	typedef VOITerm Superclass;
	typedef itk::SmartPointer< Self > Pointer;
	typedef itk::SmartPointer< const Self > ConstPointer;
	
	// defines a New for the object
	itkNewMacro(KLDivTerm);

	// update operator
	virtual void UpdateFrom(const VOITerm *otherTerm);

	// sets the bins to an interval distribution
	const CMatrixNxM<>& GetDVPs() const;
	void SetDVPs(const CMatrixNxM<>& mDVP);

	// special-purpose
	void SetInterval(REAL low, REAL high, REAL fraction, BOOL bMid);

	// returns minimum / maximum target dose to the structure
	REAL GetMinDose(void) const;
	REAL GetMaxDose(void) const;	
	
private:
	// accessor for target bins
	const CVectorN<>& GetTargetBins() const;
	const CVectorN<>& GetTargetGBins() const;
public:
	// evaluates the term
	virtual REAL Eval(CVectorN<> *pvGrad, const CArray<BOOL, BOOL>& arrInclude);

	// over-ride to create subcopy
	virtual VOITerm *Clone();

	// change handler for when the volume or region changes
	void OnHistogramBinningChange(); // CObservableEvent *pSource = NULL, void *pVoid = NULL);

private:
	// DVPs
	CMatrixNxM<> m_mDVPs;

	// the target bins
	mutable CVectorN<> m_vTargetBins;
	mutable bool m_bRecompTarget;

	// the convolved bins
	mutable CVectorN<> m_vTargetGBins;
	mutable bool m_bReconvolve;

	// temp to hold calc + EPS
	CVectorN<> m_vCalc_EPS;
	CVectorN<> m_vTarget_div_Calc;
	CVectorN<> m_vTarget_div_Calc_EPS;
	CVectorN<> m_vLogTarget_div_Calc;
	CVectorN<> m_v_dx_Target_div_Calc;
	CVectorN<> m_v_dVol_Target;

	// use if cross entropy of calc w.r.t. target is needed
	/// TODO: document this and get rid of it
	bool m_bTargetCrossEntropy;

};	// class KLDivTerm

}	// namespace dH
