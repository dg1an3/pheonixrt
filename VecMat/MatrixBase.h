//////////////////////////////////////////////////////////////////////
// MatrixBase.h: declaration and definition of the CMatrixBase 
//		template class.
//
// Copyright (C) 1999-2003 Derek G Lane
// $Id: MatrixBase.h,v 1.13 2003/03/01 22:27:19 default Exp $
//////////////////////////////////////////////////////////////////////

#if !defined(MATRIXBASE_H)
#define MATRIXBASE_H

// vector base class
#include "VectorBase.h"

//////////////////////////////////////////////////////////////////////
// class CMatrixBase<TYPE>
//
// represents a fixed-size matrix with dimension and type given.
//////////////////////////////////////////////////////////////////////
template<class TYPE = double>
class CMatrixBase
{
protected:
	// constructors
	CMatrixBase();
	CMatrixBase(const CMatrixBase& fromMatrix);

public:
	// destructor
	~CMatrixBase();

	// assignment
	CMatrixBase& operator=(const CMatrixBase& fromMatrix);

	// SetIdentity -- sets the matrix to an identity matrix
	void SetIdentity();

	// matrix size
	int GetCols() const;
	int GetRows() const;

	// element access
	CVectorBase<TYPE>& operator[](int nAtCol);
	const CVectorBase<TYPE>& operator[](int nAtCol) const;

	// TYPE * conversion -- returns a pointer to the first element
	//		WARNING: this allows for no-bounds-checking access
	operator TYPE *();
	operator const TYPE *() const;

	// row-vector access
	void GetRow(int nAtRow, CVectorBase<TYPE>& vRow) const;
	void SetRow(int nAtRow, const CVectorBase<TYPE>& vRow);

	// row / column interchange
	void InterchangeRows(int nRow1, int nRow2);
	void InterchangeCols(int nCol1, int nCol2);

	// IsApproxEqual -- tests for approximate equality using the EPS
	//		defined at the top of this file
	BOOL IsApproxEqual(const CMatrixBase& m, TYPE epsilon = DEFAULT_EPSILON) const;

	// in-place operators
	CMatrixBase& operator+=(const CMatrixBase& mRight);
	CMatrixBase& operator-=(const CMatrixBase& mRight);
	CMatrixBase& operator*=(const TYPE& scale);

	// Transpose -- transposes elements of the matrix
	void Transpose();

	// orthogonality test
	BOOL IsOrthogonal() const;

	// Orthogonalize 
	void Orthogonalize();

	// computes the determinant of the matrix
	TYPE Determinant() const;

	// Invert -- inverts the matrix using the Gauss-Jordan 
	//		elimination
	void Invert(BOOL bFullPivot = FALSE);

	// SVD -- singular-valued decomposition -- returns the singular
	//		value matrix as a vector of diagonal values w, and 
	//		returns V (not V^T)
	BOOL SVD(CVectorBase<TYPE>& w, CMatrixBase& v);

protected:
	// SetElements -- sets the elements to an external pointer
	void SetElements(int nCols, int nRows, TYPE *pElements, BOOL bFreeElements);

	// helper functions for Invert

	// FindPivotRow -- determines the best other row to be 
	//		exchanged so that no zero elements occur on the diagonal
	int FindPivotRow(int nDiag);

	// FindPivotElem -- determines the best pivot element
	void FindPivotElem(int nDiag, int *nRow, int *nCol);

	// helper function for SVD to compute the Householder decomposition
	//		of this matrix, and to accumulate the rh and lh singular
	//		values
	TYPE Householder(CVectorBase<TYPE>& w, CVectorBase<TYPE>& rv1);
	void AccumulateRH(CMatrixBase& v, const CVectorBase<TYPE>& rv1);
	void AccumulateLH(CVectorBase<TYPE>& w);

protected:
	// the matrix shape
	int m_nRows;
	int m_nCols;

	// the row vectors of the matrix
	CVectorBase<TYPE> *m_pColumns;

	// the elements of the matrix
	TYPE *m_pElements;

	// flag to indicate whether elements should be freed
	BOOL m_bFreeElements;
};


//////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::CMatrixBase<TYPE>
//
// default constructor -- initializes to 0x0 matrix
//////////////////////////////////////////////////////////////////
template<class TYPE>
CMatrixBase<TYPE>::CMatrixBase()
	: m_nCols(0),
		m_nRows(0),
		m_pColumns(NULL),
		m_pElements(NULL),
		m_bFreeElements(TRUE)
{
}	// CMatrixBase<TYPE>::CMatrixBase<TYPE>


//////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::CMatrixBase<TYPE>
//
// copy constructor
//////////////////////////////////////////////////////////////////
template<class TYPE>
CMatrixBase<TYPE>::CMatrixBase(const CMatrixBase<TYPE>& fromMatrix)
	: m_nCols(0),
		m_nRows(0),
		m_pColumns(NULL),
		m_pElements(NULL),
		m_bFreeElements(TRUE)
{
	// allocate the elements
	SetElements(fromMatrix.GetCols(), fromMatrix.GetRows(),
		new TYPE[fromMatrix.GetCols() * fromMatrix.GetRows()], TRUE);

	(*this) = fromMatrix;

}	// CMatrixBase<TYPE>::CMatrixBase<TYPE>


//////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::~CMatrixBase<TYPE>
//
// destructor
//////////////////////////////////////////////////////////////////
template<class TYPE>
CMatrixBase<TYPE>::~CMatrixBase()
{
	// frees any elements, if needed
	SetElements(0, 0, NULL, TRUE);

}	// CMatrixBase<TYPE>::~CMatrixBase<TYPE>


//////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::operator=
//
// assignment operator
//////////////////////////////////////////////////////////////////
template<class TYPE>
CMatrixBase<TYPE>& CMatrixBase<TYPE>::operator=(const CMatrixBase<TYPE>& fromMatrix)
{
	ASSERT(GetCols() == fromMatrix.GetCols());

	// assign successive column vectors
	for (int nAt = 0; nAt < GetCols(); nAt++)
	{
		(*this)[nAt] = fromMatrix[nAt];
	}

	return (*this);

}	// CMatrixBase<TYPE>::operator=


//////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::SetIdentity
//
// sets the matrix to an identity matrix
//////////////////////////////////////////////////////////////////
template<class TYPE>
void CMatrixBase<TYPE>::SetIdentity()
{
	// for each element in the matrix,
	for (int nCol = 0; nCol < GetCols(); nCol++)
	{
		for (int nRow = 0; nRow < GetRows(); nRow++)
		{
			(*this)[nCol][nRow] = 
				(TYPE)((nRow == nCol) ? 1.0 : 0.0);
		}
	}

}	// CMatrixBase<TYPE>::SetIdentity


//////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::operator[]
//
// retrieves a reference to a column vector
//////////////////////////////////////////////////////////////////
template<class TYPE>
CVectorBase<TYPE>& CMatrixBase<TYPE>::operator[](int nAtCol)
{
	// bounds check on the index
	ASSERT(nAtCol >= 0 && nAtCol < GetCols());

	// return a reference to the column vector
	return m_pColumns[nAtCol];

}	// CMatrixBase<TYPE>::operator[]


//////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::operator[]
//
// retrieves a reference to a column vector
//////////////////////////////////////////////////////////////////
template<class TYPE>
const CVectorBase<TYPE>& CMatrixBase<TYPE>::operator[](int nAtCol) const
{
	// bounds check on the index
	ASSERT(nAtCol >= 0 && nAtCol < GetCols());

	// return a reference to the column vector
	return m_pColumns[nAtCol];

}	// CMatrixBase<TYPE>::operator[]


//////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::GetCols
//
// returns the number of columns of the matrix
//////////////////////////////////////////////////////////////////
template<class TYPE>
int CMatrixBase<TYPE>::GetCols() const
{
	return m_nCols;

}	// CMatrixBase<TYPE>::GetCols


//////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::GetRows
//
// returns the number of rows of the matrix
//////////////////////////////////////////////////////////////////
template<class TYPE>
int CMatrixBase<TYPE>::GetRows() const
{
	return m_nRows;

}	// CMatrixBase<TYPE>::GetRows


//////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::operator TYPE *
//
// TYPE * conversion -- returns a pointer to the first element
//		WARNING: this allows for no-bounds-checking access
//////////////////////////////////////////////////////////////////
template<class TYPE>
CMatrixBase<TYPE>::operator TYPE *()
{
	return &m_pElements[0];

}	// CMatrixBase<TYPE>::operator TYPE *


//////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::operator const TYPE *
//
// const TYPE * conversion -- returns a pointer to the first 
//		element.
//		WARNING: this allows for no-bounds-checking access
//////////////////////////////////////////////////////////////////
template<class TYPE>
CMatrixBase<TYPE>::operator const TYPE *() const
{
	return &m_pElements[0];

}	// CMatrixBase<TYPE>::operator const TYPE *



//////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::GetRow
//
// constructs and returns a row vector
//////////////////////////////////////////////////////////////////
template<class TYPE>
void CMatrixBase<TYPE>::GetRow(int nAtRow, CVectorBase<TYPE>& vRow) const
{
	// make the row vector the same size
	ASSERT(vRow.GetDim() == GetCols());

	// populate the row vector
	for (int nAtCol = 0; nAtCol < GetCols(); nAtCol++)
	{
		vRow[nAtCol] = (*this)[nAtCol][nAtRow];
	}

}	// CMatrixBase<TYPE>::GetRow


//////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::SetRow
// 
// sets the rows vector
//////////////////////////////////////////////////////////////////
template<class TYPE>
void CMatrixBase<TYPE>::SetRow(int nAtRow, const CVectorBase<TYPE>& vRow)
{
	if (vRow.GetDim() == GetCols())
	{
		// de-populate the row vector
		for (int nAtCol = 0; nAtCol < GetCols(); nAtCol++)
		{
			(*this)[nAtCol][nAtRow] = vRow[nAtCol];
		}
	}

}	// CMatrixBase<TYPE>::SetRow


//////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::IsApproxEqual
//
// tests for approximate equality using the EPS 
//////////////////////////////////////////////////////////////////
template<class TYPE>
BOOL CMatrixBase<TYPE>::IsApproxEqual(const CMatrixBase& m, TYPE epsilon) const
{
	ASSERT(GetCols() == m.GetCols());

	for (int nAtCol = 0; nAtCol < GetCols(); nAtCol++)
	{
		if (!(*this)[nAtCol].IsApproxEqual(m[nAtCol], epsilon))
		{
			return FALSE;
		}
	}

	return TRUE;

}	// CMatrixBase<TYPE>::IsApproxEqual


//////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::operator+=
//
// in-place matrix addition; returns a reference to this
//////////////////////////////////////////////////////////////////
template<class TYPE>
CMatrixBase<TYPE>& CMatrixBase<TYPE>::operator+=(const CMatrixBase<TYPE>& mRight)
{
	ASSERT(GetCols() == mRight.GetCols());
	ASSERT(GetRows() == mRight.GetRows());

	// element-by-element sum of the matrix
	for (int nCol = 0; nCol < GetCols(); nCol++)
	{
		for (int nRow = 0; nRow < GetRows(); nRow++)
		{
			(*this)[nCol][nRow] += mRight[nCol][nRow];
		}
	}

	// return a reference to this
	return (*this);

}	// CMatrixBase<TYPE>::operator+=


//////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::operator-=
//
// in-place matrix subtraction; returns a reference to 	this
//////////////////////////////////////////////////////////////////
template<class TYPE>
CMatrixBase<TYPE>& CMatrixBase<TYPE>::operator-=(const CMatrixBase<TYPE>& mRight)
{
	ASSERT(GetCols() == mRight.GetCols());
	ASSERT(GetRows() == mRight.GetRows());

	// element-by-element difference of the matrix
	for (int nCol = 0; nCol < GetCols(); nCol++)
	{
		for (int nRow = 0; nRow < GetRows(); nRow++)
		{
			(*this)[nCol][nRow] -= mRight[nCol][nRow];
		}
	}

	// return a reference to this
	return (*this);

}	// CMatrixBase<TYPE>::operator-=


//////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::operator*=
//
// in-place scalar multiplication
//////////////////////////////////////////////////////////////////
template<class TYPE>
CMatrixBase<TYPE>& CMatrixBase<TYPE>::operator*=(const TYPE& scale)
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

}	// CMatrixBase<TYPE>::operator*=


//////////////////////////////////////////////////////////////////////
// operator==(const CMatrixBase<TYPE>&, const CMatrixBase<TYPE>&)
//
// exact matrix equality
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline bool operator==(const CMatrixBase<TYPE>& mLeft, 
					   const CMatrixBase<TYPE>& mRight)
{
	// element-by-element comparison
	for (int nCol = 0; nCol < mLeft.GetCols(); nCol++)
	{
		for (int nRow = 0; nRow < mLeft.GetRows(); nRow++)
		{
			if (mLeft[nCol][nRow] != mRight[nCol][nRow])
			{
				return false;
			}
		}
	}

	return true;

}	// operator==(const CMatrixBase<TYPE>&, const CMatrixBase<TYPE>&)


//////////////////////////////////////////////////////////////////////
// operator!=(const CMatrixBase<TYPE>&, const CMatrixBase<TYPE>&)
//
// exact matrix inequality
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline bool operator!=(const CMatrixBase<TYPE>& mLeft, 
					   const CMatrixBase<TYPE>& mRight)
{
	return !(mLeft == mRight);

}	// operator!=(const CMatrixBase<TYPE>&, const CMatrixBase<TYPE>&)


//////////////////////////////////////////////////////////////////////
// operator<<(ostream& ar, CMatrixBase)
//
// stream output of a matrix
//////////////////////////////////////////////////////////////////////
template<class TYPE>
ostream& operator<<(ostream& os, const CMatrixBase<TYPE>& m)
{
	cout << strMessage << " = \n";

	for (int nAtRow = 0; nAtRow < m.GetRows(); nAtRow++)
	{
		cout << "\t<";
		for (int nAtCol = 0; nAtCol < m.GetCols(); nAtCol++)
		{
			cout << (double) m[nAtCol][nAtRow] << "\t";
		}
		cout << ">\n";
	} 

	return os;

}	// operator<<(ostream& ar, CMatrixBase)


//////////////////////////////////////////////////////////////////////
// TraceMatrix
//
// outputs the matrix for tracing
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void TraceMatrix(const char *pszMessage, const CMatrixBase<TYPE>& m)
{
	TRACE("%s = \n", pszMessage);

	// MATLAB output
	TRACE("\t[");

	for (int nAtRow = 0; nAtRow < m.GetRows(); nAtRow++)
	{
		for (int nAtCol = 0; nAtCol < m.GetCols(); nAtCol++)
		{
			TRACE("%10.4lf\t", (double) m[nAtCol][nAtRow]);
		}

		if (nAtRow < m.GetRows()-1)
		{
			// MATLAB output
			TRACE(";\n\t");
		}
	} 

	// MATLAB output
	TRACE("]\n");

}	// TraceMatrix



//////////////////////////////////////////////////////////////////////
// macro TRACE_MATRIX
//
// macro to trace matrix -- only compiles in debug version
//////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#define TRACE_MATRIX(strMessage, m) \
	TraceMatrix(strMessage, m);	
#else
#define TRACE_MATRIX(strMessage, m)
#endif


#ifdef _DEBUG
//////////////////////////////////////////////////////////////////////
// MatrixValid
//
// asserts that the matrix elements are valid values
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void MatrixValid(const CMatrixBase<TYPE>& m)
{
	for (int nAt = 0; nAt < m.GetCols(); nAt++)
	{
		VectorValid(m[nAt]);
	}

}	// MatrixValid

#endif

#endif
