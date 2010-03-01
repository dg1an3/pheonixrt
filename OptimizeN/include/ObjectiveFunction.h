// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: ObjectiveFunction.h 605 2008-09-14 18:06:23Z dglane001 $
#if !defined(OBJECTIVEFUNCTION_H)
#define OBJECTIVEFUNCTION_H

#include <vnl/vnl_cost_function.h>

// objective functions are vector-domained functions
#include <VectorN.h>

//////////////////////////////////////////////////////////////////////
class DynamicCovarianceCostFunction : public vnl_cost_function
{
public:
	// constructs an objective function; gets flag to indicate
	//		whether gradient information is available
	DynamicCovarianceCostFunction(); // BOOL bHasGradientInfo);

	// evaluates the objective function
	virtual REAL operator()(const CVectorN<>& vInput, 
		CVectorN<> *pGrad = NULL) const = 0;

	//:  Compute one or both of f and g.
	//   Normally implemented in terms of the above two, but may be faster if specialized. f != 0 => compute f
	virtual void compute(vnl_vector<double> const& x, 
		double *f, vnl_vector<double>* g);

	// whether gradient information is available
	//BOOL HasGradientInfo() const;

	// transform function from linear to other parameter space
	virtual void Transform(CVectorN<> *pvInOut) const;
	virtual void dTransform(CVectorN<> *pvInOut) const;
	virtual void InvTransform(CVectorN<> *pvInOut) const;

	// function to evaluate gradiant at a point (uses difference method)
	//void Gradient(const CVectorN<>& vIn, REAL epsilon, 
	//			CVectorN<>& vGrad_out) const;

	// sets the OF to use adaptive variance
	void SetAdaptiveVariance(CVectorN<> *pAV, REAL varMin, REAL varMax);

protected:
	// pointer to adaptive variance vector, if enabled
	CVectorN<> *m_pAV;

	// min and max for AV
	REAL m_varMin;
	REAL m_varMax;

//private:
//	// flag to indicate that gradient information is available
//	BOOL m_bHasGradientInfo;

};	// class CObjectiveFunction

#endif
