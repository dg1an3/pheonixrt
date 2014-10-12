//////////////////////////////////////////////////////////////////////
// MatrixNxM.h: declaration and definition of the CMatrixNxM template class.
//
// Copyright (C) 1999-2001
// $Id: MatrixNxM.h,v 1.7 2003/11/01 01:39:40 default Exp $
//////////////////////////////////////////////////////////////////////

#if !defined(MATRIXNXM_H)
#define MATRIXNXM_H

#include "MathUtil.h"

#include "MatrixBase.h"
#include "VectorN.h"

//////////////////////////////////////////////////////////////////////
// class CMatrixNxM<TYPE>
//
// represents a square matrix with GetDim()ension and type given.
//////////////////////////////////////////////////////////////////////
template<class TYPE = double>
class CMatrixNxM : public CMatrixBase<TYPE>
{
public:
	// constructors / destructor
	CMatrixNxM();
	CMatrixNxM(int nCols, int nRows);
	CMatrixNxM(const CMatrixNxM& fromMatrix);
	CMatrixNxM(const CMatrixBase<TYPE>& fromMatrix);

	// assignment operator
	CMatrixNxM& operator=(const CMatrixBase<TYPE>& fromMatrix);

	// Reshape -- sets the dimension of the matrix
	void Reshape(int nCols, int nRows);

	// in-place operators
	CMatrixNxM& operator*=(const TYPE& scale);
	CMatrixNxM& operator*=(const CMatrixNxM& mRight);

	//////////////////////////////////////////////////////////////////
	// Pseudoinvert -- in-place Moore-Penrose pseudoinversion
	//////////////////////////////////////////////////////////////////
	BOOL Pseudoinvert();
};


//////////////////////////////////////////////////////////////////
// default constructor -- initializes to 0x0 matrix
//////////////////////////////////////////////////////////////////
template<class TYPE>
CMatrixNxM<TYPE>::CMatrixNxM()
{
}

//////////////////////////////////////////////////////////////////
// constructs a specific-dimensioned matrix
//////////////////////////////////////////////////////////////////
template<class TYPE>
CMatrixNxM<TYPE>::CMatrixNxM(int nCols, int nRows)
{
	Reshape(nCols, nRows);
}

//////////////////////////////////////////////////////////////////
// copy constructor
//////////////////////////////////////////////////////////////////
template<class TYPE>
CMatrixNxM<TYPE>::CMatrixNxM(const CMatrixNxM<TYPE>& fromMatrix)
{
	// sets the dimensions
	Reshape(fromMatrix.GetCols(), fromMatrix.GetRows());

	// SetIdentity to fill unoccupied parts of matrix
	SetIdentity();

	// copy the elements
	for (int nCol = 0; nCol < GetCols(); nCol++)
	{
		for (int nRow = 0; nRow < GetRows(); nRow++)
		{
			(*this)[nCol][nRow] = fromMatrix[nCol][nRow];
		}
	}
}

//////////////////////////////////////////////////////////////////
// copy constructor
//////////////////////////////////////////////////////////////////
template<class TYPE>
CMatrixNxM<TYPE>::CMatrixNxM(const CMatrixBase<TYPE>& fromMatrix)
{
	// sets the dimensions
	Reshape(fromMatrix.GetCols(), fromMatrix.GetRows());

	// SetIdentity to fill unoccupied parts of matrix
	SetIdentity();

	// copy the elements
	for (int nCol = 0; nCol < GetCols(); nCol++)
	{
		for (int nRow = 0; nRow < GetRows(); nRow++)
		{
			(*this)[nCol][nRow] = fromMatrix[nCol][nRow];
		}
	}
}

//////////////////////////////////////////////////////////////////
// assignment operator
//////////////////////////////////////////////////////////////////
template<class TYPE>
CMatrixNxM<TYPE>& CMatrixNxM<TYPE>::operator=(const CMatrixBase<TYPE>& fromMatrix)
{
	// sets the dimensions
	Reshape(fromMatrix.GetCols(), fromMatrix.GetRows());

	// SetIdentity to fill unoccupied parts of matrix
	SetIdentity();

	// copy the elements
	for (int nCol = 0; nCol < GetCols(); nCol++)
	{
		for (int nRow = 0; nRow < GetRows(); nRow++)
		{
			(*this)[nCol][nRow] = fromMatrix[nCol][nRow];
		}
	}

	return (*this);
}

//////////////////////////////////////////////////////////////////
// Reshape -- sets the dimension of the matrix
//////////////////////////////////////////////////////////////////
template<class TYPE>
void CMatrixNxM<TYPE>::Reshape(int nCols, int nRows)
{
	if (GetRows() == nRows 
		&& GetCols() == nCols)
	{
		return;
	}

	// preserve existing elements
	int nOldRows = GetRows();
	int nOldCols = GetCols();
	TYPE *pOldElements = m_pElements;

	// allocate and set the new elements, but do not free the old
	TYPE *pNewElements = new TYPE[nCols * nRows];

	// don't free the existing elements, as they will be copied
	m_bFreeElements = FALSE;

	// set the new elements
	SetElements(nCols, nRows, pNewElements, TRUE);

	// if there were old elements, 
	if (pOldElements)
	{
		// set the new elements to 0 initially
		memset(pNewElements, 0, sizeof(TYPE) * nCols * nRows);

		// create a temporary matrix to hold the old elements
		CMatrixNxM<> mTemp;
		mTemp.SetElements(nOldCols, nOldRows, pOldElements, TRUE);

		// and assign
		for (int nAtCol = 0; nAtCol < __min(GetCols(), mTemp.GetCols()); nAtCol++)
		{
			for (int nAtRow = 0; nAtRow < __min(GetRows(), mTemp.GetRows()); nAtRow++)
			{
				(*this)[nAtCol][nAtRow] = mTemp[nAtCol][nAtRow];
			}
		}
	}
	else
	{
		// populate as an identity matrix
		SetIdentity();
	}
}


//////////////////////////////////////////////////////////////////
// CMatrixNxM<TYPE>::operator*=
//
// in-place matrix multiplication; returns a reference to this
//////////////////////////////////////////////////////////////////
template<class TYPE>
CMatrixNxM<TYPE>& CMatrixNxM<TYPE>::operator*=(const CMatrixNxM<TYPE>& mRight)
{
	CMatrixNxM<TYPE> mProduct = (*this) * mRight;

	// and assign
	Reshape(mProduct.GetCols(), mProduct.GetRows());
	(*this) = mProduct;

	// return a reference to this
	return (*this);

}	// CMatrixNxM<TYPE>::operator*=


//////////////////////////////////////////////////////////////////
// CMatrixNxM<TYPE>::operator*=
//
// in-place matrix multiplication; returns a reference to this
//////////////////////////////////////////////////////////////////
template<class TYPE>
CMatrixNxM<TYPE>& CMatrixNxM<TYPE>::operator*=(const TYPE& scale)
{
	// element-by-element difference of the matrix
	for (int nRow = 0; nRow < GetRows(); nRow++)
	{
		for (int nCol = 0; nCol < GetCols(); nCol++)
		{
			(*this)[nCol][nRow] *= scale;
		}
	}

	// return a reference to this
	return (*this);

}	// CMatrixNxM<TYPE>::operator*=


//////////////////////////////////////////////////////////////////
// CMatrixNxM<TYPE>::Pseudoinvert
// 
// in-place Moore-Penrose pseudoinversion
//////////////////////////////////////////////////////////////////
template<class TYPE>
BOOL CMatrixNxM<TYPE>::Pseudoinvert()
{
	CVectorN<TYPE> w(GetCols());
	CMatrixNxM<TYPE> v(GetCols(), GetCols());
	if (!SVD(w, v))
	{
		return FALSE;
	}

	// using the formula (A+)^T = U * {1/w} * V^T

	// form the S matrix (S^T * S)^-1 * S^T
	CMatrixNxM<TYPE> s(GetCols(), GetCols());
	for (int nAt = 0; nAt < GetCols(); nAt++)
	{
		s[nAt][nAt] = (w[nAt] > 1e-8) ? 1.0 / w[nAt] : 0.0;
	}
	(*this) *= s;

	// multiply by V^T
	v.Transpose();
	(*this) *= v;

	// and transpose the result to finish with A+
	Transpose();

	return TRUE;

}	// CMatrixNxM<TYPE>::Pseudoinvert


//////////////////////////////////////////////////////////////////////
// operator+(const CMatrixNxM<TYPE>&, const CMatrixNxM<TYPE>&)
//
// matrix addition
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline CMatrixNxM<TYPE> operator+(const CMatrixNxM<TYPE>& mLeft, 
									const CMatrixNxM<TYPE>& mRight)
{
	// create the product
	CMatrixNxM<TYPE> mSum(mLeft);

	// use in-place multiplication
	mSum += mRight;

	// return the product
	return mSum;

}	// operator+(const CMatrixNxM<TYPE>&, const CMatrixNxM<TYPE>&)


//////////////////////////////////////////////////////////////////////
// operator-(const CMatrixNxM<TYPE>&, const CMatrixNxM<TYPE>&)
//
// matrix subtraction
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline CMatrixNxM<TYPE> operator-(const CMatrixNxM<TYPE>& mLeft, 
									const CMatrixNxM<TYPE>& mRight)
{
	// create the difference
	CMatrixNxM<TYPE> mDiff(mLeft);

	// use in-place subtraction
	mDiff -= mRight;

	// return the difference
	return mDiff;

}	// operator-(const CMatrixNxM<TYPE>&, const CMatrixNxM<TYPE>&)


//////////////////////////////////////////////////////////////////////
// operator*(const CMatrixNxM<TYPE>&, double scale)
//
// matrix scalar multiplication
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline CMatrixNxM<TYPE> operator*(const CMatrixNxM<TYPE>& mat,
									double scale)
{
	// stored the product
	CMatrixNxM<TYPE> mProduct(mat);

	// scalar multiplication
	mProduct *= scale;

	// return product
	return mProduct;

}	// operator*(const CMatrixNxM<TYPE>&, double scale)


//////////////////////////////////////////////////////////////////////
// operator*(double scale, const CMatrixNxM<TYPE>&)
//
// matrix scalar multiplication
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline CMatrixNxM<TYPE> operator*(double scale, 
								   const CMatrixNxM<TYPE>& mat)
{
	// stored the product
	CMatrixNxM<TYPE> mProduct(mat);

	// scalar multiplication
	mProduct *= scale;

	// return product
	return mProduct;

}	// operator*(double scale, const CMatrixNxM<TYPE>&)


//////////////////////////////////////////////////////////////////////
// operator*(const CMatrixNxM<TYPE>&, const CVectorBase<TYPE>&)
//
// matrix-vector multiplication
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline CVectorN<TYPE> operator*(const CMatrixNxM<TYPE>& mat,
									const CVectorBase<TYPE>& v)
{
	ASSERT(mat.GetRows() == v.GetDim());

	// stored the product
	CVectorN<TYPE> vProduct(v);
	vProduct.SetZero();

	// step through the rows of the matrix
	for (int nRow = 0; nRow < mat.GetRows(); nRow++)
	{
		ASSERT(vProduct[nRow] == 0.0);

		// step through the columns of the matrix
		for (int nCol = 0; nCol < mat.GetCols(); nCol++)
		{
			vProduct[nRow] += mat[nCol][nRow] * v[nCol];
		}
	}

	// return the product
	return vProduct;

}	// operator*(const CMatrixNxM<TYPE>&, const CVectorBase<TYPE>&)


//////////////////////////////////////////////////////////////////////
// operator*(const CMatrixNxM<TYPE>&, const CMatrixNxM<TYPE>&)
//
// matrix multiplication
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline CMatrixNxM<TYPE> operator*(const CMatrixNxM<TYPE>& mLeft, 
									const CMatrixNxM<TYPE>& mRight)
{
	ASSERT(mLeft.GetCols() == mRight.GetRows());

	// create the product
	CMatrixNxM<TYPE> mProduct(mRight.GetCols(), mLeft.GetRows());

	// compute the matrix product
	for (int nRow = 0; nRow < mProduct.GetRows(); nRow++)
	{
		for (int nCol = 0; nCol < mProduct.GetCols(); nCol++)
		{
			mProduct[nCol][nRow] = 0.0;

			for (int nMid = 0; nMid < mLeft.GetCols(); nMid++)
			{
				mProduct[nCol][nRow] +=
					mLeft[nMid][nRow] * mRight[nCol][nMid];
			}
		}
	}

	// return the product
	return mProduct;

}	// operator*(const CMatrixNxM<TYPE>&, const CMatrixNxM<TYPE>&)


#ifdef __AFX_H__

//////////////////////////////////////////////////////////////////////
// function operator<<
//
// matrix serialization
//////////////////////////////////////////////////////////////////////
template<class TYPE>
CArchive& operator<<(CArchive &ar, CMatrixNxM<TYPE> m)
{
	// serialize the dimension
	ar << m.GetCols();
	ar << m.GetRows();

	// serialize the individual elements
	ar.Write((TYPE *) m, m.GetCols() * m.GetRows() * sizeof(TYPE));


	// return the archive object
	return ar;
}

//////////////////////////////////////////////////////////////////////
// function operator>>
//
// matrix serialization
//////////////////////////////////////////////////////////////////////
template<class TYPE>
CArchive& operator>>(CArchive &ar, CMatrixNxM<TYPE>& m)
{
	// serialize the dimension
	int nCols, nRows;
	ar >> nCols >> nRows;
	m.Reshape(nCols, nRows);

	// serialize the individual elements
	ar.Read((TYPE *) m, nCols * nRows * sizeof(TYPE));

	// return the archive object
	return ar;
}

#endif

#endif