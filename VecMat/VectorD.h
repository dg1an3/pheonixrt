//////////////////////////////////////////////////////////////////////
// Vector.h: declaration and definition of the CVectorD template class.
//
// Copyright (C) 1999-2003 Derek G Lane
// $Id: VectorD.h,v 1.8 2003/04/26 20:38:19 default Exp $
//////////////////////////////////////////////////////////////////////

#if !defined(VECTOR_H)
#define VECTOR_H

// math utilities
#include "MathUtil.h"

// base class include
#include "VectorBase.h"


//////////////////////////////////////////////////////////////////////
// class CVectorD<DIM, TYPE>
//
// represents a mathematical vector with dimension and type given
//////////////////////////////////////////////////////////////////////
template<int DIM = 4, class TYPE = double>
class CVectorD : public CVectorBase<TYPE>
{
public:
	// constructors / destructor
	CVectorD();
	CVectorD(TYPE x);
	CVectorD(TYPE x, TYPE y);
	CVectorD(TYPE x, TYPE y, TYPE z);
	CVectorD(TYPE x, TYPE y, TYPE z, TYPE w);
	CVectorD(const CVectorD& vFrom);
	explicit CVectorD(const CVectorBase<TYPE>& vFrom);
#ifdef __AFX_H__
	CVectorD(const CPoint& pt);
#endif
	~CVectorD();

	// assignment
	CVectorD& operator=(const CVectorD& vFrom);

	// conversion
#ifdef __AFX_H__
	operator CPoint() const;
#endif

	// tests for approximate equality using the EPS defined at the 
	//		top of this file
	BOOL IsApproxEqual(const CVectorD& v, TYPE epsilon = DEFAULT_EPSILON) const;

	// assignment operators -- requires proper type for strict
	//		compile-time check
	CVectorD& operator+=(const CVectorD& vRight);
	CVectorD& operator-=(const CVectorD& vRight);

private:
	// the vector's elements
	TYPE m_arrElements[DIM];
};


//////////////////////////////////////////////////////////////////
// CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>() 
//
// default constructor
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CVectorD<DIM, TYPE>::CVectorD() 
{
	// set the elements pointer to the static array 
	//		and initialize the dimension
	SetElements(DIM, &m_arrElements[0], FALSE);

	// clear to all zeros
	memset((*this), 0, DIM * sizeof(TYPE));

}	// CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>() 


//////////////////////////////////////////////////////////////////
// CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(TYPE x) 
//
// construct from one element
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CVectorD<DIM, TYPE>::CVectorD(TYPE x) 
{
	// set the elements pointer to the static array 
	//		and initialize the dimension
	SetElements(DIM, &m_arrElements[0], FALSE);

	// set the given elements
	(*this)[0] = x;

	// clear to all zeros
	if (DIM > 1)
	{
		memset(&(*this)[1], 0, (DIM-1) * sizeof(TYPE));
	}

}	// CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(TYPE x) 


//////////////////////////////////////////////////////////////////
// CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(TYPE x, TYPE y) 
//
// construct from two elements
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CVectorD<DIM, TYPE>::CVectorD(TYPE x, TYPE y) 
{
	// set the elements pointer to the static array 
	//		and initialize the dimension
	SetElements(DIM, &m_arrElements[0], FALSE);

	// set the given elements
	(*this)[0] = x;
	(*this)[1] = y;

	// clear to all zeros
	if (DIM > 2)
	{
		memset(&(*this)[2], 0, (DIM-2) * sizeof(TYPE));
	}

}	// CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(TYPE x, TYPE y) 


//////////////////////////////////////////////////////////////////
// CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(TYPE x, TYPE y, TYPE z) 
//
// construct from three elements
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CVectorD<DIM, TYPE>::CVectorD(TYPE x, TYPE y, TYPE z) 
{
	// set the elements pointer to the static array 
	//		and initialize the dimension
	SetElements(DIM, &m_arrElements[0], FALSE);

	// set the given elements
	(*this)[0] = x;
	(*this)[1] = y;
	(*this)[2] = z;

	// clear to all zeros
	if (DIM > 3)
	{
		memset(&(*this)[3], 0, (DIM-3) * sizeof(TYPE));
	}

}	// CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(TYPE x, TYPE y, TYPE z) 


//////////////////////////////////////////////////////////////////
// CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(TYPE x, TYPE y, TYPE z, TYPE w) 
//
// construct from four elements
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CVectorD<DIM, TYPE>::CVectorD(TYPE x, TYPE y, TYPE z, TYPE w) 
{ 
	// set the elements pointer to the static array 
	//		and initialize the dimension
	SetElements(DIM, &m_arrElements[0], FALSE);

	// set the given elements
	(*this)[0] = x;
	(*this)[1] = y;
	(*this)[2] = z;
	(*this)[3] = w;

	// clear to all zeros
	if (DIM > 4)
	{
		memset(&(*this)[4], 0, (DIM-4) * sizeof(TYPE));
	}

}	// CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(TYPE x, TYPE y, TYPE z, TYPE w) 


//////////////////////////////////////////////////////////////////
// CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(const CVectorD<DIM, TYPE>& vFrom) 
//
// copy constructor
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CVectorD<DIM, TYPE>::CVectorD(const CVectorD<DIM, TYPE>& vFrom) 
{
	// set the elements pointer to the static array 
	//		and initialize the dimension
	SetElements(DIM, &m_arrElements[0], FALSE);

	// assign to copy 
	(*this) = vFrom;

}	// CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(const CVectorD<DIM, TYPE>& vFrom) 


//////////////////////////////////////////////////////////////////
// CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(const CVectorBase<TYPE>& vFrom) 
//
// copy constructor from base class object
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CVectorD<DIM, TYPE>::CVectorD(const CVectorBase<TYPE>& vFrom) 
{
	// set the elements pointer to the static array 
	//		and initialize the dimension
	SetElements(DIM, &m_arrElements[0], FALSE);

	// copy the elements
	memcpy((*this), vFrom, __min(GetDim(), vFrom.GetDim()) * sizeof(TYPE));

	// set remainder of elements to 0
	if (GetDim() > vFrom.GetDim())
	{
		memset(&(*this)[vFrom.GetDim()], 0, 
			(GetDim() - vFrom.GetDim()) * sizeof(TYPE));
	}

}	// CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(const CVectorBase<TYPE>& vFrom) 


#ifdef __AFX_H__
//////////////////////////////////////////////////////////////////
// CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(const CPoint& pt)
//
// construct from a windows CPoint
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CVectorD<DIM, TYPE>::CVectorD(const CPoint& pt)
{
	// set the elements pointer to the static array 
	//		and initialize the dimension
	SetElements(DIM, &m_arrElements[0], FALSE);

	// set the given elements
	(*this)[0] = pt.x;
	(*this)[1] = pt.y;

	// clear to all zeros
	if (DIM > 2)
	{
		memset(&(*this)[2], 0, (DIM-2) * sizeof(TYPE));
	}

}	// CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(const CPoint& pt)
#endif


//////////////////////////////////////////////////////////////////
// CVectorD<DIM, TYPE>::~CVectorD<DIM, TYPE>()
//
// destructor
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CVectorD<DIM, TYPE>::~CVectorD()
{
	// ensure no monkey business has occurred
	ASSERT(m_pElements == &m_arrElements[0]);

	// ensure we don't delete anything
	m_pElements = NULL;

}	// CVectorD<DIM, TYPE>::~CVectorD<DIM, TYPE>()


#ifdef __AFX_H__
//////////////////////////////////////////////////////////////////
// CVectorD<DIM, TYPE>::operator CPoint() const
//
// Windows CPoint conversion 
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CVectorD<DIM, TYPE>::operator CPoint() const
{
	return CPoint((int)(*this)[0], (int)(*this)[1]);

}	// CVectorD<DIM, TYPE>::operator CPoint() const
#endif


//////////////////////////////////////////////////////////////////
// CVectorD<DIM, TYPE>& CVectorD<DIM, TYPE>::operator=
//
// assignment operator
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CVectorD<DIM, TYPE>& CVectorD<DIM, TYPE>::operator=(const CVectorD<DIM, TYPE>& vFrom)
{
	// NOTE: this ASSERT would work, were it not for the terrible down-cast
	//		that occurs in CMatrixD::operator[]
	// ASSERT(m_pElements == &m_arrElements[0]);

	// copy the elements
	memcpy((*this), vFrom, GetDim() * sizeof(TYPE));

	// return a reference to this
	return (*this);

}	// CVectorD<DIM, TYPE>& CVectorD<DIM, TYPE>::operator=


//////////////////////////////////////////////////////////////////
// CVectorD<DIM, TYPE>::IsApproxEqual
//
// tests for approximate equality using the given epsilon
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
BOOL CVectorD<DIM, TYPE>::IsApproxEqual(const CVectorD<DIM, TYPE>& v, 
										TYPE epsilon) const
{
	// form the difference vector
	CVectorD<DIM, TYPE> vDiff(*this);
	vDiff -= v;

	return (vDiff.GetLength() < epsilon);

}	// CVectorD<DIM, TYPE>::IsApproxEqual


//////////////////////////////////////////////////////////////////
// CVectorD<DIM, TYPE>::operator+=
//
// in-place vector addition; returns a reference to this
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CVectorD<DIM, TYPE>& CVectorD<DIM, TYPE>::operator+=(const CVectorD<DIM, TYPE>& vRight)
{
	// use the base-class operator+=
	CVectorBase<TYPE>::operator+=((const CVectorBase<TYPE>&)vRight);

	return (*this);

}	// CVectorD<DIM, TYPE>::operator+=


//////////////////////////////////////////////////////////////////
// CVectorD<DIM, TYPE>::operator-=
//
// in-place vector subtraction; returns a reference to this
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CVectorD<DIM, TYPE>& CVectorD<DIM, TYPE>::operator-=(const CVectorD<DIM, TYPE>& vRight)
{
	// use the base-class operator-=
	CVectorBase<TYPE>::operator-=((const CVectorBase<TYPE>&)vRight);

	return (*this);

}	// CVectorD<DIM, TYPE>::operator-=


//////////////////////////////////////////////////////////////////////
// operator==(const CVectorD, const CVectorD)
//
// friend function to provide equality comparison for vectors.
// use IsApproxEqual for approximate equality.
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline bool operator==(const CVectorD<DIM, TYPE>& vLeft, 
					   const CVectorD<DIM, TYPE>& vRight)
{
	// use the base-class operator==
	return operator==((const CVectorBase<TYPE>&) vLeft,
		(const CVectorBase<TYPE>&) vRight);

}	// operator==(const CVectorD, const CVectorD)


//////////////////////////////////////////////////////////////////////
// operator!=(const CVectorD, const CVectorD)
//
// friend function to provide inequality comparison for vectors.
// use !IsApproxEqual for approximate inequality.
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline bool operator!=(const CVectorD<DIM, TYPE>& vLeft, 
					   const CVectorD<DIM, TYPE>& vRight)
{
	// use the base-class operator!=
	return operator!=((const CVectorBase<TYPE>&) vLeft,
		(const CVectorBase<TYPE>&) vRight);

}	// operator!=(const CVectorD, const CVectorD)


//////////////////////////////////////////////////////////////////////
// operator+(const CVectorD, const CVectorD)
//
// friend function to add two vectors, returning the sum as a new 
//		vector
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline CVectorD<DIM, TYPE> operator+(const CVectorD<DIM, TYPE>& vLeft, 
							  const CVectorD<DIM, TYPE>& vRight)
{
	CVectorD<DIM, TYPE> vSum = vLeft;
	vSum += vRight;

	return vSum;

}	// operator+(const CVectorD, const CVectorD)


//////////////////////////////////////////////////////////////////////
// operator-(const CVectorD, const CVectorD)
//
// friend function to subtract one vector from another, returning 
//		the difference as a new vector
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline CVectorD<DIM, TYPE> operator-(const CVectorD<DIM, TYPE>& vLeft, 
							  const CVectorD<DIM, TYPE>& vRight)
{
	CVectorD<DIM, TYPE> vSum = vLeft;
	vSum -= vRight;

	return vSum;

}	// operator-(const CVectorD, const CVectorD)


//////////////////////////////////////////////////////////////////////
// operator*(const CVectorD, const CVectorD)
//
// friend function for vector inner product
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline TYPE operator*(const CVectorD<DIM, TYPE>& vLeft, 
							  const CVectorD<DIM, TYPE>& vRight)
{
	return operator*((const CVectorBase<TYPE>&) vLeft,
		(const CVectorBase<TYPE>&) vRight);

}	// operator*(const CVectorD, const CVectorD)


//////////////////////////////////////////////////////////////////////
// operator*(TYPE scalar, const CVectorD)
//
// friend function for scalar multiplication of a vector
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline CVectorD<DIM, TYPE> operator*(TYPE scalar, 
							  const CVectorD<DIM, TYPE>& vRight)
{
	CVectorD<DIM, TYPE> vProd = vRight;
	vProd *= scalar;

	return vProd;

}	// operator*(TYPE scalar, const CVectorD)


//////////////////////////////////////////////////////////////////////
// operator*(const CVectorD, TYPE scalar)
//
// friend function for scalar multiplication of a vector
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline CVectorD<DIM, TYPE> operator*(const CVectorD<DIM, TYPE>& vLeft, 
							  TYPE scalar)
{
	CVectorD<DIM, TYPE> vProd = vLeft;
	vProd *= scalar;

	return vProd;

}	// operator*(const CVectorD, TYPE scalar)


//////////////////////////////////////////////////////////////////////
// Cross(const CVectorD<2>&, const CVectorD<2>&)
//
// friend function for vector cross product of 2-d vectors
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline TYPE Cross(const CVectorD<2, TYPE>& vLeft, 
					const CVectorD<2, TYPE>& vRight)
{
	return vLeft[0] * vRight[1] - vLeft[1] * vRight[0];

}	// Cross(const CVectorD<2>&, const CVectorD<2>&)


//////////////////////////////////////////////////////////////////////
// Cross(const CVectorD<3>&, const CVectorD<3>&)
//
// friend function for vector cross product of 3-d vectors
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline CVectorD<3, TYPE> Cross(const CVectorD<3, TYPE>& vLeft, 
						      const CVectorD<3, TYPE>& vRight)
{
	CVectorD<3, TYPE> vProd;

	vProd[0] =   vLeft[1] * vRight[2] - vLeft[2] * vRight[1];
	vProd[1] = -(vLeft[0] * vRight[2] - vLeft[2] * vRight[0]);
	vProd[2] =   vLeft[0] * vRight[1] - vLeft[1] * vRight[0];

	return vProd;

}	// Cross(const CVectorD<3>&, const CVectorD<3>&)


//////////////////////////////////////////////////////////////////////
// ToHomogeneous(const CVectorD&)
//
// converts an N-dimensional vector to an N+1-dimensional homogeneous
//		vector
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline CVectorD<DIM + 1, TYPE> ToHG(const CVectorD<DIM, TYPE>& v) 
{
	// create the homogeneous vector
	CVectorD<DIM + 1, TYPE> vh;

	// fill with the given vector's elements
	for (int nAt = 0; nAt < DIM; nAt++)
	{
		vh[nAt] = v[nAt];
	}

	// set last element to 1.0
	vh[DIM] = (TYPE) 1.0;

	return vh;

}	// ToHomogeneous(const CVectorD&)

#define ToHomogeneous ToHG

//////////////////////////////////////////////////////////////////////
// FromHomogeneous(const CVectorD&)
//
// converts an N+1-dimensional homogeneous vector to an N-dimensional 
//		vector
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline CVectorD<DIM, TYPE> FromHG(const CVectorD<DIM + 1, TYPE>& vh) 
{
	// create the non-homogeneous vector
	CVectorD<DIM, TYPE> v;

	// normalize the first DIM-1 elements by the last element
	for (int nAt = 0; nAt < DIM; nAt++)
	{
		v[nAt] = vh[nAt] / vh[DIM];
	}

	return v;

}	// FromHomogeneous(const CVectorD&)

#define FromHomogeneous FromHG

#ifdef __AFX_H__
//////////////////////////////////////////////////////////////////////
// operator<<(CArchive &ar, CVectorD)
//
// CArchive serialization of a vector
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CArchive& operator<<(CArchive &ar, CVectorD<DIM, TYPE> v)
{
	for (int nAt = 0; nAt < DIM; nAt++)
	{
		ar << v[nAt];
	}

	return ar;

}	// operator<<(CArchive &ar, CVectorD)


//////////////////////////////////////////////////////////////////////
// operator>>(CArchive &ar, CVectorD)
//
// CArchive de-serialization of a vector
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CArchive& operator>>(CArchive &ar, CVectorD<DIM, TYPE>& v)
{
	for (int nAt = 0; nAt < DIM; nAt++)
	{
		ar >> v[nAt];
	}

	return ar;

}	// operator>>(CArchive &ar, CVectorD)
#endif

#endif	// !defined(VECTOR_H)