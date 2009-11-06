// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: LineFunction.h 605 2008-09-14 18:06:23Z dglane001 $
#if !defined(LINEFUNCTION_H)
#define LINEFUNCTION_H

// base class
#include "ObjectiveFunction.h"

//////////////////////////////////////////////////////////////////////
// class CLineFunction
// 
// defines a line function given another objective function and
//		a line in the vector-space domain of that objective function
//////////////////////////////////////////////////////////////////////
class CLineFunction : public CObjectiveFunction
{
public:
	// construct a new line function object, given an DIM-dimensional
	//		objective function
	CLineFunction(CObjectiveFunction *pProjFunc);

	// returns the point on the line
	const CVectorN<>& GetPoint() const;

	// returns the direction of the line
	const CVectorN<>& GetDirection() const;

	// sets the line parameters
	void SetLine(const CVectorN<>& vPoint, const CVectorN<>& vDir);

	// evaluates the line function
	virtual REAL operator()(const CVectorN<>& vInput,
		CVectorN<> *pGrad = NULL) const;

private:
	// pointer to the objective function upon which this line function
	//		is based
	CObjectiveFunction *m_pProjFunc;

	// stores a point on the line
	CVectorN<> m_vPoint;

	// stores the direction of the line
	CVectorN<> m_vDirection;

	// temporary store of evaluation point
	mutable CVectorN<> m_vEvalPoint;

	// stores the gradient at the last evaluated point
	mutable CVectorN<> m_vGrad;
};

#endif
