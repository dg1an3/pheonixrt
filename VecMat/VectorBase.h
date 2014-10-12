//////////////////////////////////////////////////////////////////////
// VectorBase.h: declaration and definition of the CVectorBase 
//		template class.
//
// Copyright (C) 1999-2003 Derek G Lane
// $Id: VectorBase.h,v 1.10 2003/03/01 22:26:45 default Exp $
//////////////////////////////////////////////////////////////////////

#if !defined(VECTORBASE_H)
#define VECTORBASE_H

// for iostream support for vectors
#include <iostream>
using namespace std;

// math utilities
#include "MathUtil.h"

// forward-define CMatrix* as a friend class
template<class TYPE>
class CMatrixBase;

template<int DIM, class TYPE>
class CMatrixD;

template<class TYPE>
class CMatrixNxM;

//////////////////////////////////////////////////////////////////////
// class CVectorBase<TYPE>
//
// base class for mathematical vectors with element type given
//////////////////////////////////////////////////////////////////////
template<class TYPE = double>
class CVectorBase
{
protected:
	// constructors 
	CVectorBase();
	CVectorBase(const CVectorBase& vFrom);

	//// CMatrixBase can construct CVectorBases
	//friend CMatrixBase<TYPE>;

	//// CMatrixD can construct CVectorBases
	//friend CMatrixD<1, TYPE>;
	//friend CMatrixD<2, TYPE>;
	//friend CMatrixD<3, TYPE>;
	//friend CMatrixD<4, TYPE>;

	//// global operator* can construct
	//friend CVectorBase<TYPE> operator*(const CMatrixNxM<TYPE>& mat,
	//	const CVectorBase<TYPE>& v);

public:
	// destructor
	~CVectorBase();

	// assignment
	CVectorBase& operator=(const CVectorBase& vFrom);

	// initializes all elements to zero
	void SetZero();

	// element accessors
	int GetDim() const;
	TYPE& operator[](int nAtRow);
	const TYPE& operator[](int nAtRow) const;

	// TYPE * conversion -- returns a pointer to the first element
	//		WARNING: this allows for no-bounds-checking access
	operator TYPE *();
	operator const TYPE *() const;

	// vector length and normalization
	TYPE GetLength() const;
	void Normalize();

	// approximate equality using the epsilon
	BOOL IsApproxEqual(const CVectorBase& v, TYPE epsilon = DEFAULT_EPSILON) const;

	// in-place vector arithmetic
	CVectorBase& operator+=(const CVectorBase& vRight);
	CVectorBase& operator-=(const CVectorBase& vRight);
	CVectorBase& operator*=(const TYPE& scalar);

	// external element management
	void SetElements(int nDim, TYPE *pElements, BOOL bFreeElements);

protected:
	// dimension of vector
	int m_nDim;

	// vector elements
	TYPE *m_pElements;

	// flag to indicate whether the elements should be freed
	BOOL m_bFreeElements;

};	// class CVectorBase<TYPE>


//////////////////////////////////////////////////////////////////
// CVectorBase<TYPE>::CVectorBase<TYPE>
//
// default constructor -- public can not construct a CVectorBase
//////////////////////////////////////////////////////////////////
template<class TYPE>
CVectorBase<TYPE>::CVectorBase()
	: m_nDim(0),
		m_pElements(NULL),
		m_bFreeElements(TRUE)
{
}	// CVectorBase<TYPE>::CVectorBase<TYPE>


//////////////////////////////////////////////////////////////////
// CVectorBase<TYPE>::CVectorBase<TYPE>(const CVectorBase<TYPE>&)
//									 
// copy constructor
//////////////////////////////////////////////////////////////////
template<class TYPE>
CVectorBase<TYPE>::CVectorBase(const CVectorBase<TYPE>& vFrom)
	: m_nDim(vFrom.GetDim()),
		m_pElements(NULL),
		m_bFreeElements(TRUE)
{
	// set up the elements
	SetElements(m_nDim, new TYPE[m_nDim], TRUE);

	// and copy them
	(*this) = vFrom;

}	// CVectorBase<TYPE>::CVectorBase<TYPE>(const CVectorBase<TYPE>&)


//////////////////////////////////////////////////////////////////
// CVectorBase<TYPE>::~CVectorBase<TYPE>
//
// destructor 
//////////////////////////////////////////////////////////////////
template<class TYPE>
CVectorBase<TYPE>::~CVectorBase()
{
	if (m_bFreeElements 
		&& m_pElements != NULL)
	{
		// free elements
		delete [] m_pElements;
	}

}	// CVectorBase<TYPE>::~CVectorBase<TYPE>


//////////////////////////////////////////////////////////////////
// CVectorBase<TYPE>::operator=
//
// assignment operator -- copies arbitrary dimensioned vector to
//		this, zero-filling or truncating the elements
//////////////////////////////////////////////////////////////////
template<class TYPE>
CVectorBase<TYPE>& CVectorBase<TYPE>::operator=(const CVectorBase<TYPE>& vFrom)
{
	// copy the elements
	ASSERT(m_pElements == (*this));
	ASSERT(vFrom == (vFrom.m_pElements));
	memcpy((*this), vFrom, __min(GetDim(), vFrom.GetDim()) * sizeof(TYPE));

	// set remainder of elements to 0
	if (GetDim() > vFrom.GetDim())
	{
		memset(&(*this)[vFrom.GetDim()], 0, 
			(GetDim() - vFrom.GetDim()) * sizeof(TYPE));
	}

	return (*this);

}	// CVectorBase<TYPE>::operator=


//////////////////////////////////////////////////////////////////
// CVectorBase<TYPE>::SetZero
//
// initializes all elements to zero
//////////////////////////////////////////////////////////////////
template<class TYPE>
void CVectorBase<TYPE>::SetZero() 
{
	// zero all elements
	memset((*this), 0, GetDim() * sizeof(TYPE));

}	// CVectorBase<TYPE>::SetZero


//////////////////////////////////////////////////////////////////
// CVectorBase<TYPE>::GetDim
//
// returns the dimensionality of the vector
//////////////////////////////////////////////////////////////////
template<class TYPE>
int CVectorBase<TYPE>::GetDim() const
{
	return m_nDim;

}	// CVectorBase<TYPE>::GetDim


//////////////////////////////////////////////////////////////////
// CVectorBase<TYPE>::operator[]
//
// returns a reference to the specified element.
//////////////////////////////////////////////////////////////////
template<class TYPE>
TYPE& CVectorBase<TYPE>::operator[](int nAtRow)
{
	// check dimensions
	ASSERT(nAtRow >= 0 && nAtRow < GetDim());

	return m_pElements[nAtRow];

}	// CVectorBase<TYPE>::operator[]


//////////////////////////////////////////////////////////////////
// CVectorBase<TYPE>::operator[] const
//
// returns a const reference to the specified element.
//////////////////////////////////////////////////////////////////
template<class TYPE>
const TYPE& CVectorBase<TYPE>::operator[](int nAtRow) const
{
	// check dimensions
	ASSERT(nAtRow >= 0 && nAtRow < GetDim());

	return m_pElements[nAtRow];

}	// CVectorBase<TYPE>::operator[] const


//////////////////////////////////////////////////////////////////
// CVectorBase<TYPE>::operator TYPE *
//
// TYPE * conversion -- returns a pointer to the first element
//		WARNING: this allows for no-bounds-checking access
//////////////////////////////////////////////////////////////////
template<class TYPE>
CVectorBase<TYPE>::operator TYPE *()
{
	return &m_pElements[0];

}	// CVectorBase<TYPE>::operator TYPE *


//////////////////////////////////////////////////////////////////
// CVectorBase<TYPE>::operator const TYPE *
//
// const TYPE * conversion -- returns a pointer to the first 
//		element.
//		WARNING: this allows for no-bounds-checking access
//////////////////////////////////////////////////////////////////
template<class TYPE>
CVectorBase<TYPE>::operator const TYPE *() const
{
	return &m_pElements[0];

}	// CVectorBase<TYPE>::operator const TYPE *


//////////////////////////////////////////////////////////////////
// CVectorBase<TYPE>::SetElements
//
// management for external elements
//////////////////////////////////////////////////////////////////
template<class TYPE>
void CVectorBase<TYPE>::SetElements(int nDim, TYPE *pElements,
									BOOL bFreeElements)
{
	if (m_bFreeElements 
		&& m_pElements != NULL)
	{
		delete [] m_pElements;
		m_pElements = NULL;
	}

	m_nDim = nDim;
	m_pElements = pElements;
	m_bFreeElements = bFreeElements;

}	// CVectorBase<TYPE>::SetElements


//////////////////////////////////////////////////////////////////
// CVectorBase<TYPE>::GetLength
//
// returns the euclidean length of the vector
//////////////////////////////////////////////////////////////////
template<class TYPE>
TYPE CVectorBase<TYPE>::GetLength() const
{
	// form the sum of the square of each element
	TYPE len = 0.0;
	for (int nAt = 0; nAt < GetDim(); nAt++)
	{
		len += (*this)[nAt] * (*this)[nAt];
	}
	
	// return the square root of this
	return static_cast<TYPE>(sqrt(len));

}	// CVectorBase<TYPE>::GetLength


//////////////////////////////////////////////////////////////////
// CVectorBase<TYPE>::Normalize
//
// scales the vector so its length is 1.0
//////////////////////////////////////////////////////////////////
template<class TYPE>
void CVectorBase<TYPE>::Normalize()
{
	// store the computed lendth
	TYPE len = GetLength();

	// don't normalize zero vectors
	if (len > 0.0)
	{
		// normalize each element by length
		for (int nAt = 0; nAt < GetDim(); nAt++)
		{
			(*this)[nAt] /= len;
		}
	}

}	// CVectorBase<TYPE>::Normalize


//////////////////////////////////////////////////////////////////
// CVectorBase<TYPE>::IsApproxEqual
//	
// tests for approximate equality using the EPS defined at 
//		the top of this file
//////////////////////////////////////////////////////////////////
template<class TYPE>
BOOL CVectorBase<TYPE>::IsApproxEqual(const CVectorBase<TYPE>& v, 
									  TYPE epsilon) const
{
	// form the difference vector
	CVectorBase<TYPE> vDiff(*this);
	vDiff -= v;

	return (vDiff.GetLength() < epsilon);

}	// CVectorBase<TYPE>::IsApproxEqual


//////////////////////////////////////////////////////////////////
// CVectorBase<TYPE>::operator+=
//
// in-place vector addition; returns a reference to this
//////////////////////////////////////////////////////////////////
template<class TYPE>
CVectorBase<TYPE>& CVectorBase<TYPE>::operator+=(const CVectorBase<TYPE>& vRight)
{
	for (int nAt = 0; nAt < GetDim(); nAt++)
	{
		(*this)[nAt] += vRight[nAt];
	}

	return (*this);

}	// CVectorBase<TYPE>::operator+=


//////////////////////////////////////////////////////////////////
// CVectorBase<TYPE>::operator-=
//
// in-place vector subtraction; returns a reference to this
//////////////////////////////////////////////////////////////////
template<class TYPE>
CVectorBase<TYPE>& CVectorBase<TYPE>::operator-=(const CVectorBase<TYPE>& vRight)
{
	for (int nAt = 0; nAt < GetDim(); nAt++)
	{
		(*this)[nAt] -= vRight[nAt];
	}

	return (*this);

}	// CVectorBase<TYPE>::operator-=


//////////////////////////////////////////////////////////////////
// CVectorBase<TYPE>::operator*=
//
// in-place scalar multiplication; returns a reference to this
//////////////////////////////////////////////////////////////////
template<class TYPE>
CVectorBase<TYPE>& CVectorBase<TYPE>::operator*=(const TYPE& scalar)
{
	for (int nAt = 0; nAt < GetDim(); nAt++)
	{
		(*this)[nAt] *= scalar;
	}

	return (*this);

}	// CVectorBase<TYPE>::operator*=


//////////////////////////////////////////////////////////////////////
// operator==(const CVectorBase, const CVectorBase)
//
// friend function to provide exact equality comparison for vectors.
// use IsApproxEqual for approximate equality.
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline bool operator==(const CVectorBase<TYPE>& vLeft, 
					   const CVectorBase<TYPE>& vRight)
{
	// test for element-wise equality
	for (int nAt = 0; nAt < vLeft.GetDim(); nAt++)
	{
		if (vLeft[nAt] != vRight[nAt])
		{
			return false;
		}
	}

	return true;

}	// operator==(const CVectorBase, const CVectorBase)


//////////////////////////////////////////////////////////////////////
// operator!=(const CVectorBase, const CVectorBase)
//
// friend function to provide exact inequality comparison for vectors.
// use !IsApproxEqual for approximate inequality.
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline bool operator!=(const CVectorBase<TYPE>& vLeft, 
					   const CVectorBase<TYPE>& vRight)
{
	return !(vLeft == vRight);

}	// operator!=(const CVectorBase, const CVectorBase)


//////////////////////////////////////////////////////////////////////
// operator*(const CVectorBase, const CVectorBase)
//
// friend function for vector inner product
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline TYPE operator*(const CVectorBase<TYPE>& vLeft, 
							  const CVectorBase<TYPE>& vRight)
{
	// stores the dot product
	TYPE prod = (TYPE) 0.0;

	// sum the element-wise multiplication
	for (int nAt = 0; nAt < vLeft.GetDim(); nAt++)
	{
		prod += vLeft[nAt] * vRight[nAt];
	}

	// return the product
	return prod;

}	// operator*(const CVectorBase, const CVectorBase)


//////////////////////////////////////////////////////////////////////
// operator<<(ostream& ar, CVectorBase)
//
// stream output of a vector
//////////////////////////////////////////////////////////////////////
template<class TYPE>
ostream& operator<<(ostream& os, CVectorBase<TYPE> v)
{
	os << "<";
	for (int nAt = 0; nAt < v.GetDim(); nAt++)
	{
		os << v[nAt] << "\t";
	}
	os << ">\n";

	return os;

}	// operator<<(ostream& ar, CVectorBase)


//////////////////////////////////////////////////////////////////////
// TraceVector
//
// helper function to output a vector for debugging
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void TraceVector(const CVectorBase<TYPE>& vTrace)
{
#ifdef _DEBUG
	TRACE("<");
	for (int nAt = 0; nAt < vTrace.GetDim(); nAt++)
	{
		TRACE("%lf\t", vTrace[nAt]);
	}
	TRACE(">\n");
#endif

}	// TraceVector


//////////////////////////////////////////////////////////////////////
// TRACE_VECTOR
//
// macro to trace matrix -- only compiles in debug version
//////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#define TRACE_VECTOR(strMessage, v) \
	TRACE(strMessage);				\
	TraceVector(v);					\
	TRACE("\n");
#else
#define TRACE_VECTOR(strMessage, v)
#endif


#ifdef _DEBUG
//////////////////////////////////////////////////////////////////////
// VectorValid
//
// asserts that the vector has valid values
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void VectorValid(const CVectorBase<TYPE>& v)
{
	for (int nAt = 0; nAt < v.GetDim(); nAt++)
	{
		ASSERT(_finite(v[nAt]) && !_isnan(v[nAt]));
	}

}	// VectorValid
#endif


#endif	// !defined(VECTORBASE_H)