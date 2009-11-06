// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: LineFunction.cpp 606 2008-09-14 18:06:57Z dglane001 $
#include "stdafx.h"

// the class definition
#include "LineFunction.h"

//////////////////////////////////////////////////////////////////////
// CLineFunction::CLineFunction
// 
// construct a new line function object, given an DIM-dimensional
//		objective function
//////////////////////////////////////////////////////////////////////
CLineFunction::CLineFunction(CObjectiveFunction *pProjFunc)
	: CObjectiveFunction(pProjFunc->HasGradientInfo()),
		m_pProjFunc(pProjFunc)
{
}

//////////////////////////////////////////////////////////////////////
// CLineFunction::GetPoint
// 
// returns the point on the line
//////////////////////////////////////////////////////////////////////
const CVectorN<>& CLineFunction::GetPoint() const
{
	return m_vPoint;
}

//////////////////////////////////////////////////////////////////////
// CLineFunction::GetDirection
// 
// returns the direction of the line
//////////////////////////////////////////////////////////////////////
const CVectorN<>& CLineFunction::GetDirection() const
{
	return m_vDirection;
}

//////////////////////////////////////////////////////////////////////
// CLineFunction::SetLine
// 
// sets the line parameters
//////////////////////////////////////////////////////////////////////
void CLineFunction::SetLine(const CVectorN<>& vPoint, 
							const CVectorN<>& vDir)
{
	ASSERT(vPoint.GetDim() == vDir.GetDim());

	m_vPoint.SetDim(vPoint.GetDim());
	m_vPoint = vPoint;
	m_vDirection.SetDim(vDir.GetDim());
	m_vDirection = vDir;
}

//////////////////////////////////////////////////////////////////////
// CLineFunction::operator()
// 
// evaluates the line function
//////////////////////////////////////////////////////////////////////
REAL CLineFunction::operator()(const CVectorN<>& vInput, 
							   CVectorN<> *pGrad) const
{
	// ensure this is a one-dimensional input vector
	ASSERT(vInput.GetDim() == 1);

	if (pGrad)
	{
		// set dimension of intermediate gradient vector
		m_vGrad.SetDim(GetPoint().GetDim());
	}

	// form the point at which evaluation is to occur
	// m_vEvalPoint = GetPoint() + vInput[0] * GetDirection();
	m_vEvalPoint.SetDim(m_vDirection.GetDim());
	m_vEvalPoint = GetDirection();
	m_vEvalPoint *= vInput[0];
	m_vEvalPoint += GetPoint();

	// evaluate the function and gradient, if needed
	REAL value = (*m_pProjFunc)(m_vEvalPoint, pGrad ? &m_vGrad : NULL);

	// assign
	if (pGrad)
	{
		// set the dimension
		pGrad->SetDim(1);

		// and assign (dot product of gradient and direction)
		(*pGrad)[0] = m_vGrad * GetDirection();
	}

	return value;
}
