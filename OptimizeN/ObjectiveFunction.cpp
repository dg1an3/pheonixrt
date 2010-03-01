// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: ObjectiveFunction.cpp 606 2008-09-14 18:06:57Z dglane001 $
#include "stdafx.h"

// the class definition
#include "ObjectiveFunction.h"


///////////////////////////////////////////////////////////////////////////////
// CObjectiveFunction::CObjectiveFunction
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
DynamicCovarianceCostFunction::DynamicCovarianceCostFunction(/*BOOL bHasGradientInfo*/)
	: /*m_bHasGradientInfo(bHasGradientInfo)
	, */m_pAV(NULL)
{
}	// CObjectiveFunction::CObjectiveFunction

///////////////////////////////////////////////////////////////////////////////
void 
	DynamicCovarianceCostFunction::compute(vnl_vector<double> const& x, 
			double *f, vnl_vector<double>* g)
{
	CVectorN<REAL> vX(x.size());
	CopyElements<REAL>(&vX[0], &x[0], x.size());

	if (g != NULL)
	{
		CVectorN<REAL> vG(g->size());
		if (f != NULL)
		{
			(*f) = (*this)(vX, &vG);
		}
		else
		{
			(*this)(vX, &vG);
		}
		CopyElements<REAL>(&(*g)[0], &vG[0], x.size());
	}
	else
	{
		(*f) = (*this)(vX);
	}
}

///////////////////////////////////////////////////////////////////////////////
// CObjectiveFunction::HasGradientInfo
// 
// whether gradient information is available
///////////////////////////////////////////////////////////////////////////////
//BOOL DynamicCovarianceCostFunction::HasGradientInfo() const
//{
//	return m_bHasGradientInfo;
//
//}	// CObjectiveFunction::HasGradientInfo


///////////////////////////////////////////////////////////////////////////////
// CObjectiveFunction::Transform
// 
// over-ride for function to transform parameters
///////////////////////////////////////////////////////////////////////////////
void DynamicCovarianceCostFunction::Transform(CVectorN<> *pvInOut) const
{
	// default is identity transform
	ASSERT(false);

}	// CObjectiveFunction::Transform

///////////////////////////////////////////////////////////////////////////////
// CObjectiveFunction::dTransform
// 
// over-ride for derivative transform parameters
///////////////////////////////////////////////////////////////////////////////
void DynamicCovarianceCostFunction::dTransform(CVectorN<> *pvInOut) const
{
	// set to all 1.0 for identity transform
	ITERATE_VECTOR((*pvInOut), nAt, (*pvInOut)[nAt] = 1.0);

}	// CObjectiveFunction::dTransform

///////////////////////////////////////////////////////////////////////////////
// CObjectiveFunction::InvTransform
// 
// inverse of parameter transform
///////////////////////////////////////////////////////////////////////////////
void DynamicCovarianceCostFunction::InvTransform(CVectorN<> *pvInOut) const
{
	// default is identity transform
	ASSERT(false);

}	// CObjectiveFunction::InvTransform


///////////////////////////////////////////////////////////////////////////////
// CObjectiveFunction::Gradient
// 
// approximates gradient using difference method
///////////////////////////////////////////////////////////////////////////////
//void DynamicCovarianceCostFunction::Gradient(const CVectorN<>& vIn, REAL epsilon, 
//				CVectorN<>& vGrad_out) const
//{
//	REAL res = 0.0;
//	BEGIN_LOG_SECTION(CObjectiveFunction::Gradient());
//
//	// get epsilon
//	REAL elem_max = 0.0;
//	for (int nAt = 0; nAt < vIn.GetDim(); nAt++)
//	{
//		elem_max = __max(elem_max, fabs(vIn[nAt]));
//	}
//	epsilon = elem_max * epsilon;
//
//	// numerically evaluate the gradiant
//	CVectorN<> vParam = vIn;
//	const REAL fp = (*this)(vParam);
//
//	vGrad_out.SetDim(vParam.GetDim());
//	for (int nAt = 0; nAt < vParam.GetDim(); nAt++)
//	{
//		vParam[nAt] += epsilon;
//
//		const REAL fp_del = (*this)(vParam);
//		vGrad_out[nAt] = (fp_del - fp) / epsilon;
//
//		vParam[nAt] -= epsilon;
//	}
//	LOG_EXPR_EXT(vGrad_out);
//
//	END_LOG_SECTION();	// CObjectiveFunction::Gradient
//
//}	// CObjectiveFunction::Gradient

//////////////////////////////////////////////////////////////////////////////
void 
	DynamicCovarianceCostFunction::SetAdaptiveVariance(CVectorN<> *pAV, REAL varMin, REAL varMax)
	// sets the OF to use adaptive variance
{
	// store pointer to the AV vector
	m_pAV = pAV;

	// stores min / max
	m_varMin = varMin;
	m_varMax = varMax;

}	// CObjectiveFunction::SetAdaptiveVariance

