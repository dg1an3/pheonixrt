// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: VectorN.h 605 2008-09-14 18:06:23Z dglane001 $
#if !defined(VECTORN_H)
#define VECTORN_H

#ifdef USE_IPP
#include <ipp.h>
#endif

#include <vnl/vnl_vector_ref.h>
#include <MathUtil.h>

#include <VectorOps.h>

//////////////////////////////////////////////////////////////////////
// class CVectorN<TYPE>
//
// represents a dynamically sizable mathematical vector with element
//		type given
//////////////////////////////////////////////////////////////////////
template<class TYPE = REAL>
class CVectorN
{
	// dimension of vector
	int m_nDim;

	// vector elements
	TYPE *m_pElements;

	// flag to indicate whether the elements should be freed
	bool m_bFreeElements;

	// the contained vector
	vnl_vector_ref<TYPE> *m_pvVnlVector;

public:
	// constructors / destructor
	CVectorN();
	explicit CVectorN(int nDim);
	CVectorN(const CVectorN& vFrom);
	CVectorN(const vnl_vector<TYPE>& vFrom)
	: m_nDim(0),
		m_pElements(NULL),
		m_bFreeElements(TRUE),
		m_pvVnlVector(NULL)
	{
		this->SetDim(vFrom.size());
		(*this) = vFrom;
	}
	~CVectorN();

	// assignment operator
	CVectorN& operator=(const CVectorN& vFrom);
	CVectorN& operator=(const vnl_vector<TYPE>& vFrom)
	{
		::CopyElements<TYPE>(&(*this)[0], &vFrom[0], GetDim());
		return (*this);
	}

	// get the vnl vector
	vnl_vector<TYPE>& GetVnlVector()
	{
		return *m_pvVnlVector;
	}

	// template helper for element type
	typedef TYPE ELEM_TYPE;

	// initializes all elements to zero
	void SetZero() { ZeroValues(&(*this)[0], GetDim()); }

	// element accessors
	TYPE& operator[](int nAtRow);
	const TYPE& operator[](int nAtRow) const;

	// the dimensionality of this vector
	int GetDim() const;
	void SetDim(int nDim);

	// TYPE * conversion -- returns a pointer to the first element
	//		WARNING: this allows for no-bounds-checking access
	operator TYPE *();
	operator const TYPE *() const;

	// vector length and normalization
	TYPE GetLength() const;
	void Normalize();

	// approximate equality using the epsilon
	bool IsApproxEqual(const CVectorN& v, TYPE epsilon = DEFAULT_EPSILON) const;

	// in-place vector arithmetic
	CVectorN& operator+=(const CVectorN& vRight);
	CVectorN& operator-=(const CVectorN& vRight);
	CVectorN& operator*=(const TYPE& scalar);

	//////////////////////////////////////////////////////////////////////
	// low-level element management

	// external element management
	void SetElements(int nDim, TYPE *pElements, bool bFreeElements);

	// copy elements from v, starting at start, for length elements, 
	//		and copy them to the destination position
	int CopyElements(const CVectorN<TYPE>& v, int nStart, int nLength, 
		int nDest = 0);

	// create a printable string representation
	CString ToString();

};	// class CVectorN<TYPE>


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
CVectorN<TYPE>::CVectorN() 
	: m_nDim(0),
		m_pElements(NULL),
		m_bFreeElements(TRUE),
		m_pvVnlVector(NULL)
	// default constructor
{
}	// CVectorN<TYPE>::CVectorN<TYPE>


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
CVectorN<TYPE>::CVectorN(int nDim) 
	: m_nDim(0),
		m_pElements(NULL),
		m_bFreeElements(TRUE),
		m_pvVnlVector(NULL)
	// construct an arbitrary dimensioned vector
{
	// set the dimensionality of the vector
	SetDim(nDim);

}	// CVectorN<TYPE>::CVectorN<TYPE>(int nDim) 


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
CVectorN<TYPE>::CVectorN(const CVectorN<TYPE>& vFrom)
	: m_nDim(0),
		m_pElements(NULL),
		m_bFreeElements(TRUE),
		m_pvVnlVector(NULL)
	// copy constructor
{
	// set the dimensionality of the vector
	SetDim(vFrom.GetDim());

	// copy the elements
	(*this) = vFrom;

}	// CVectorN<TYPE>::CVectorN<TYPE>(const CVectorN<TYPE>& vFrom)


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
CVectorN<TYPE>::~CVectorN()
	// destructor 
{
	if (m_bFreeElements 
		&& m_pElements != NULL)
	{
		// free elements
		FreeValues(m_pElements);
	}

	if (m_pvVnlVector)
	{
		delete m_pvVnlVector;
	}

}	// CVectorN<TYPE>::~CVectorN<TYPE>


//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
CVectorN<TYPE>& 
	CVectorN<TYPE>::operator=(const CVectorN<TYPE>& vFrom)
	// assignment operator
{
	// check the dimensionality of the vector
	ASSERT(GetDim() == vFrom.GetDim());

	if (GetDim() > 0)
	{
		CopyValues(&(*this)[0], &vFrom[0], __min(GetDim(), vFrom.GetDim()));
	}

	// set remainder of elements to 0
	if (GetDim() > vFrom.GetDim())
	{
		// zero initial
		ZeroValues(&(*this)[vFrom.GetDim()], GetDim() - vFrom.GetDim());
	}

	// return a reference to this
	return (*this);

}	// CVectorN<TYPE>::operator=


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
TYPE& 
	CVectorN<TYPE>::operator[](int nAtRow)
	// returns a reference to the specified element.
{
	// check dimensions
	ASSERT(nAtRow >= 0 && nAtRow < GetDim());

	return m_pElements[nAtRow];

}	// CVectorN<TYPE>::operator[]


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
const TYPE& 
	CVectorN<TYPE>::operator[](int nAtRow) const
	// returns a const reference to the specified element.
{
	// check dimensions
	ASSERT(nAtRow >= 0 && nAtRow < GetDim());

	return m_pElements[nAtRow];

}	// CVectorN<TYPE>::operator[] const


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
CVectorN<TYPE>::operator TYPE *()
	// TYPE * conversion -- returns a pointer to the first element
	//		WARNING: this allows for no-bounds-checking access
{
	return &m_pElements[0];

}	// CVectorN<TYPE>::operator TYPE *


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
CVectorN<TYPE>::operator const TYPE *() const
	// const TYPE * conversion -- returns a pointer to the first 
	//		element.
	//		WARNING: this allows for no-bounds-checking access
{
	return &m_pElements[0];

}	// CVectorN<TYPE>::operator const TYPE *


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
int 
	CVectorN<TYPE>::GetDim() const
	// returns the dimensionality of the vector
{
	return m_nDim;

}	// CVectorN<TYPE>::GetDim


//////////////////////////////////////////////////////////////////
template<class TYPE>
void 
	CVectorN<TYPE>::SetDim(int nDim)
	// sets the dimensionality of this vector
{
	// do nothing if the dim is already correct
	if (m_nDim != nDim)
	{
		// store pointer to old elements
		TYPE *pOldElements = m_pElements;
		m_pElements = NULL;

		if (m_pvVnlVector)
		{
			delete m_pvVnlVector;
			m_pvVnlVector = NULL;
		}

		// set the new dimensionality
		int nOldDim = m_nDim;
		m_nDim = nDim;

		// allocate new elements, if needed
		if (m_nDim > 0)
		{
			m_pElements = NULL;
			AllocValues(m_nDim, m_pElements);

			if (pOldElements)
			{
				// copy the elements
				CopyValues(&(*this)[0], pOldElements, __min(GetDim(), nOldDim));
			}

			// set remainder of elements to 0
			if (GetDim() > nOldDim)
			{
				ZeroValues(&(*this)[nOldDim], GetDim() - nOldDim);
			}

			m_pvVnlVector = new vnl_vector_ref<TYPE>(GetDim(), &(*this)[0]);
		}

		// free the elements, if needed
		if (pOldElements != NULL)
		{
			FreeValues(pOldElements);
		}

	}

}	// CVectorN<TYPE>::SetDim



//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
TYPE 
	CVectorN<TYPE>::GetLength() const
	// returns the euclidean length of the vector
{
	return VectorLength(&(*this)[0], GetDim());

}	// CVectorN<TYPE>::GetLength


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
void 
	CVectorN<TYPE>::Normalize()
	// scales the vector so its length is 1.0
{
	TYPE len = GetLength();
	if (len > 0.0)
	{
		MultValues(&(*this)[0], (TYPE) 1.0 / len, GetDim());
	}

}	// CVectorN<TYPE>::Normalize


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
bool 
	CVectorN<TYPE>::IsApproxEqual(const CVectorN<TYPE>& v, 
			TYPE epsilon) const
	// tests for approximate equality using the EPS defined at 
	//		the top of this file
{
	// form the difference vector
	CVectorN<TYPE> vDiff(*this);
	vDiff -= v;

	return (vDiff.GetLength() < epsilon);

}	// CVectorN<TYPE>::IsApproxEqual


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
CVectorN<TYPE>& 
	CVectorN<TYPE>::operator+=(const CVectorN<TYPE>& vRight)
	// in-place vector addition; returns a reference to this	
{
	SumValues(&(*this)[0], &vRight[0], GetDim());

	return (*this);

}	// CVectorN<TYPE>::operator+=


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
CVectorN<TYPE>& 
	CVectorN<TYPE>::operator-=(const CVectorN<TYPE>& vRight)
	// in-place vector subtraction; returns a reference to this
{
	DiffValues(&(*this)[0], &vRight[0], GetDim());

	return (*this);

}	// CVectorN<TYPE>::operator-=


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
CVectorN<TYPE>& 
	CVectorN<TYPE>::operator*=(const TYPE& scalar)
	// in-place scalar multiplication; returns a reference to this
{
	MultValues(&(*this)[0], scalar, GetDim());

	return (*this);

}	// CVectorN<TYPE>::operator*=

//////////////////////////////////////////////////////////////////
template<class TYPE>
void 
	CVectorN<TYPE>::SetElements(int nDim, TYPE *pElements,
			bool bFreeElements)
	// management for external elements
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

}	// CVectorN<TYPE>::SetElements

//////////////////////////////////////////////////////////////////
template<class TYPE>
int 
	CVectorN<TYPE>::CopyElements(const CVectorN<TYPE>& v, 
			int nStart, int nLength, int nDest)
	// copy elements from v, starting at start, for length elements, 
	//		and copy them to the destination position
{
	int nLastSrcPos = __min(nStart + nLength, v.GetDim());
	int nLastDstPos = __min(nDest + nLength, GetDim());
	nLength = __min(nLength, nLastSrcPos - nStart);
	nLength = __min(nLength, nLastDstPos - nDest);
	if (nLength > 0
		&& &(*this)[nDest] != &v[nStart])
	{
		CopyValues(&(*this)[nDest], &v[nStart], nLength);
	}

	return nLength;

}	// CVectorN<TYPE>::CopyElements


//////////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
bool 
	operator==(const CVectorN<TYPE>& vLeft, 
			const CVectorN<TYPE>& vRight)
	// friend function to provide exact equality comparison for vectors.
	// use IsApproxEqual for approximate equality.
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

}	// operator==(const CVectorN, const CVectorN)


//////////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
bool 
	operator!=(const CVectorN<TYPE>& vLeft, 
			const CVectorN<TYPE>& vRight)
	// friend function to provide exact inequality comparison for vectors.
	// use !IsApproxEqual for approximate inequality.
{
	return !(vLeft == vRight);

}	// operator!=(const CVectorN, const CVectorN)


//////////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
CVectorN<TYPE> 
	operator+(const CVectorN<TYPE>& vLeft, 
			const CVectorN<TYPE>& vRight)
	// friend function to add two vectors, returning the sum as a new 
	//		vector
{
	CVectorN<TYPE> vSum(vLeft);
	vSum += vRight;

	return vSum;

}	// operator+(const CVectorN, const CVectorN)


//////////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
CVectorN<TYPE> 
	operator-(const CVectorN<TYPE>& vLeft, 
			const CVectorN<TYPE>& vRight)
	// friend function to subtract one vector from another, returning 
	//		the difference as a new vector
{
	CVectorN<TYPE> vDiff(vLeft);
	vDiff -= vRight;

	return vDiff;

}	// operator-(const CVectorN, const CVectorN)


//////////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
TYPE 
	operator*(const CVectorN<TYPE>& vLeft, 
			const CVectorN<TYPE>& vRight)
	// friend function for vector inner product
{
	return DotProduct(&vLeft[0], &vRight[0], vLeft.GetDim());

}	// operator*(const CVectorN, const CVectorN)


//////////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
CVectorN<TYPE> 
	operator*(const TYPE& scalar, 
			const CVectorN<TYPE>& vRight)
	// friend function for scalar multiplication of a vector
{
	// copy vector to intermediate product value
	CVectorN<TYPE> vProd(vRight);

	// in-place scalar multiplication
	vProd *= scalar;

	// return the result
	return vProd;

}	// operator*(TYPE scalar, const CVectorN)


//////////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
CVectorN<TYPE> 
	operator*(const CVectorN<TYPE>& vLeft, 
			const TYPE& scalar)
	// friend function for scalar multiplication of a vector
{
	// copy vector to intermediate product value
	CVectorN<TYPE> vProd(vLeft);

	// in-place scalar multiplication
	vProd *= scalar;

	// return the result
	return vProd;

}	// operator*(const CVectorN, TYPE scalar)


///////////////////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
void 
	CalcBinomialCoeff(CVectorN<TYPE>& vCoeff)
	// calculates the binomial coefficients, returns in the vector
{
	CVectorN<TYPE> vAltCoeff;
	vAltCoeff.SetDim(vCoeff.GetDim());

	for (int nAt = 0; nAt < vCoeff.GetDim()-1; nAt++)
	{
		vCoeff[0] = 1.0;
		for (int nAtElem = 0; nAtElem < nAt; nAtElem++)
		{
			vCoeff[nAtElem+1] = vAltCoeff[nAtElem] + vAltCoeff[nAtElem+1];
		}
		vCoeff[nAt+1] = 1.0;

		vAltCoeff = vCoeff;
	}

}	// CalcBinomialCoeff


inline void Trace(LPTSTR label, double value)
{
	CString str;
	str.Format(_T("%s =\t% .4lf\n"), label, value);
	OutputDebugString(str.GetBuffer());
}	

//////////////////////////////////////////////////////////////////////
template<class TYPE>
void 
	TraceVector(LPTSTR label, const CVectorN<TYPE>& vTrace)
	// helper function to output a vector for debugging
{
	CString str;
	str.Format(_T("%s[%d] =\t<"), label, vTrace.GetDim());
#ifdef TRACE_VECTOR_NUMERIC
	for (int nAt = 0; nAt < vTrace.GetDim(); nAt++)
	{
		str.AppendFormat(_T("% .4lf|"), vTrace[nAt]);
	}
#else
	double maxElement = -1e-20;
	for (int nAt = 0; nAt < vTrace.GetDim(); nAt++)
		maxElement = __max(maxElement, vTrace[nAt]);

	str.AppendFormat(_T("% .4lf"), maxElement);

	for (int nAt = 0; nAt < vTrace.GetDim(); nAt++)
	{
		if (vTrace[nAt] < maxElement * 0.1)
			str.AppendFormat(_T(" "));
		else if (vTrace[nAt] < maxElement * 0.25)
			str.AppendFormat(_T("."));
		else if (vTrace[nAt] < maxElement * 0.85)
			str.AppendFormat(_T(":"));
		else
			str.AppendFormat(_T("|"));
	}
#endif
	str.AppendFormat(_T(">\n"));
	OutputDebugString(str.GetBuffer());

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



#ifdef __AFX_H__

//////////////////////////////////////////////////////////////////////
template<class TYPE>
CArchive& 
	operator<<(CArchive &ar, CVectorN<TYPE> v)
	// CArchive serialization of a vector
{
	// store the dimension first
	ar << v.GetDim();

	// store the elements
	for (int nAt = 0; nAt < v.GetDim(); nAt++)
	{
		ar << v[nAt];
	}

	return ar;

}	// operator<<(CArchive &ar, CVectorN<TYPE> v)

//////////////////////////////////////////////////////////////////////
template<class TYPE>
CArchive& 
	operator>>(CArchive &ar, CVectorN<TYPE>& v)
	// CArchive de-serialization of a vector
{
	// retrieve the dimension first
	int nDim;
	ar >> nDim;
	v.SetDim(nDim);

	// retrieve the elements
	for (int nAt = 0; nAt < v.GetDim(); nAt++)
	{
		ar >> v[nAt];
	}

	return ar;

}	// operator>>(CArchive &ar, CVectorN<TYPE>& v)

#endif	// __AFX_H__

#endif	// !defined(VECTORN_H)