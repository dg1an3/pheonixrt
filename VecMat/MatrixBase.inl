//////////////////////////////////////////////////////////////////////
// MatrixBase.inl : implementation of the CMatrixBase class.
//
// Copyright (C) 2002-2003 Derek G Lane
// $Id: MatrixBase.inl,v 1.5 2003/03/01 22:27:19 default Exp $
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#ifndef _MATRIXBASE_INL_
#define _MATRIXBASE_INL_

// valarray helper class
#undef min
#undef max

#include <valarray>
using namespace std;

// class declaration
#include "MatrixBase.h"

// CVectorN
#include "VectorN.h"

// helper macros
#define SQR(a) ((a)*(a))
#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))

// maximum iterations for SVD
#define MAX_ITER 30

// constant for finding the pivot during a matrix inversion
#define MAX_TO_PIVOT (1.0)


//////////////////////////////////////////////////////////////////////
// pythag
//
// computes the c side of a triangle from a and b
//////////////////////////////////////////////////////////////////////
template<class TYPE>
TYPE pythag(TYPE a, TYPE b)
{
	TYPE absa = (TYPE) fabs(a);
	TYPE absb = (TYPE) fabs(b);

	if (absa > absb)
	{
		return absa * (TYPE) sqrt(1.0 + SQR(absb / absa));
	}
	else
	{
		return (absb == 0.0 ? 0.0 : absb * (TYPE) sqrt(1.0 + SQR(absa / absb)));
	}

}	// pythag


//////////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::SetElements
//
// sets the elements of the matrix
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void CMatrixBase<TYPE>::SetElements(int nCols, int nRows, 
									TYPE *pElements, BOOL bFreeElements)
{
	// delete previous data
	if (m_bFreeElements && NULL != m_pElements)
	{
		delete [] m_pElements;
		m_pElements = NULL;
	}

	if (nCols != m_nCols 
		&& NULL != m_pColumns)
	{
		delete [] m_pColumns;
		m_pColumns = NULL;
	}

	m_nCols = nCols;
	m_nRows = nRows;

	m_pElements = pElements;

	// set up the column vectors
	if (0 != m_nCols)
	{
		if (m_pColumns == NULL)
		{
			// allocate column vectors
			m_pColumns = new CVectorN<TYPE>[GetCols()];
		}

		// initialize the column vectors and the pointers
		for (int nAt = 0; nAt < GetCols(); nAt++)
		{
			// initialize the column vector
			m_pColumns[nAt].SetElements(m_nRows, &m_pElements[nAt * GetRows()],
				FALSE);
		}
	}

	m_bFreeElements = bFreeElements;

}	// CMatrixBase<TYPE>::SetElements


//////////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::Transpose
//
// transposes the matrix
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void CMatrixBase<TYPE>::Transpose()
{
	// allocate the elements of the transposed matrix
	TYPE *pElements = new TYPE[GetRows() * GetCols()];

	{
		// make the transposed matrix
		CMatrixBase<TYPE> mTranspose;
		mTranspose.SetElements(GetRows(), GetCols(), pElements, FALSE);
		
		for (int nCol = 0; nCol < GetCols(); nCol++)
		{
			for (int nRow = 0; nRow < GetRows(); nRow++)
			{
				mTranspose[nRow][nCol] = (*this)[nCol][nRow];
			}
		}
	}

	// set the elements of this to the transposed elements
	SetElements(GetRows(), GetCols(), pElements, TRUE);

}	// CMatrixBase<TYPE>::Transpose


//////////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::IsOrthogonal
//
// tests for orthogonality of the matrix
//////////////////////////////////////////////////////////////////////
template<class TYPE>
BOOL CMatrixBase<TYPE>::IsOrthogonal() const
{
	CMatrixNxM<TYPE> mTrans = (*this);
	mTrans.Transpose();
	mTrans *= (*this);

	CMatrixNxM<TYPE> mIdent(GetCols(), GetCols());
	mIdent.SetIdentity();

	return mTrans.IsApproxEqual(mTrans);

}	// CMatrixBase<TYPE>::IsOrthogonal


//////////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::Orthogonalize
//
// orthogonalizes the input matrix using GSO
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void CMatrixBase<TYPE>::Orthogonalize()
{
	// make a working copy of the matrix
	CMatrixBase<TYPE> mOrtho(*this);

	// normalize the first column vector
	mOrtho[0].Normalize();

	// apply to each row vector after the zero-th
	for (int nAtCol = 1; nAtCol < GetCols(); nAtCol++)
	{
		// normalize the next column vector
		mOrtho[nAtCol].Normalize();

		// ensure orthogonality with all previous column vectors
		for (int nAtOrthoCol = nAtCol - 1; nAtOrthoCol >= 0; 
				nAtOrthoCol--)
		{
			// compute the scale factor
			TYPE scalar = (mOrtho[nAtCol] * mOrtho[nAtOrthoCol]) 
				/ (mOrtho[nAtOrthoCol] * mOrtho[nAtOrthoCol]);

			for (int nAtRow = 0; nAtRow < mOrtho.GetRows(); nAtRow++)
				mOrtho[nAtCol][nAtRow] -= scalar * mOrtho[nAtOrthoCol][nAtRow];

			// make sure we are now orthogonal to this
			ASSERT(mOrtho[nAtCol] * mOrtho[nAtOrthoCol] < DEFAULT_EPSILON);
		}
	}

	// assign the result
	(*this) = mOrtho;

}	// CMatrixBase<TYPE>::Orthogonalize


//////////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::Determinant
//
// computes the determinant of the matrix, for square matrices
//////////////////////////////////////////////////////////////////////
template<class TYPE>
TYPE CMatrixBase<TYPE>::Determinant() const
{
	ASSERT(GetCols() == GetRows());

	if (GetCols() > 2) 
	{
		TYPE det = 0.0;
		for (int nAtCol = 0; nAtCol < GetCols(); nAtCol++) 
		{
			CMatrixNxM mMinor(GetCols()-1, GetRows()-1);
			for (nAtRow = 1; nAtRow < GetRows(); nAtRow++) 
			{
				nAtMinorCol = 0;
				for (nAtCol2 = 0; nAtCol2 < GetCols(); nAtCol2++) 
				{
				   if (nAtCol2 != nAtCol)
				   {
					   mMinor[nAtMinorCol][nAtRow-1] = a[nAtCol2][nAtRow];
					   nAtMinorCol++;
				   }
				}
			}
			det += ((nAtCol % 2 == 0) ? 1.0 : -1.0) 
				* (*this)[nAtCol][0] * mMinor.Determinant();
		}

		return det;
	}
	else if (GetCols() > 1) 
	{
		return (*this)[0][0] * (*this)[1][1] - (*this)[1][0] * (*this)[0][1];
	}
	
	return (*this)[0][0];

}	// CMatrixBase<TYPE>::Determinant


//////////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::Invert
//
// swaps two rows of the matrix
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void CMatrixBase<TYPE>::Invert(BOOL bFullPivot)
{
	// only invert square matrices
	ASSERT(GetCols() == GetRows());

	// Gauss-Jordan elimination
	CMatrixBase<TYPE> mCopy(*this);		// the copy of this matrix
	CMatrixBase<TYPE> mInv(*this);		// stores the built inverse
	mInv.SetIdentity();

	// stores the sequence of column swaps for a full pivot
	valarray<int> arrColumnSwaps(GetCols());

	// helper vectors to hold rows
	CVectorN<TYPE> vRow(GetCols());
	CVectorN<TYPE> vOtherRow(GetCols());
		
	for (int nCol = 0; nCol < GetCols(); nCol++)
	{
		if (bFullPivot)
		{
			// CHECK THE FULL PIVOT
			// find the full pivot element
			int nPivotRow, nPivotCol;
			mCopy.FindPivotElem(nCol, &nPivotRow, &nPivotCol);

			// interchange the rows
			mCopy.InterchangeRows(nCol, nPivotRow);
			mCopy.InterchangeCols(nCol, nPivotCol);

			// interchange the columns
			mInv.InterchangeRows(nCol, nPivotRow);
			mInv.InterchangeCols(nCol, nPivotCol);
			
			// store the column interchange for later restoring
			arrColumnSwaps[nCol] = nPivotCol;
		}
		else	// partial pivot
		{
			// pivot if necessary
			int nPivotRow = mCopy.FindPivotRow(nCol);
			mCopy.InterchangeRows(nCol, nPivotRow);
			mInv.InterchangeRows(nCol, nPivotRow);
		}

    	// obtain a 1 in the diagonal position
		//		(the pivot ensures that copy[nCol][nCol] is not zero)
		// make sure we are numerically stable
		ASSERT(fabs(mCopy[nCol][nCol]) > 1e-8);

		// scale factor to be applied
    	TYPE scale = 1.0 / mCopy[nCol][nCol];	

		// scale the copy to get a 1.0 in the diagonal
		mCopy.GetRow(nCol, vRow);
		vRow *= scale;
		mCopy.SetRow(nCol, vRow);

		// scale the inverse by the same amount
		mInv.GetRow(nCol, vRow);
		vRow *= scale;
		mInv.SetRow(nCol, vRow);

		// obtain zeros in the off-diagonal elements
		int nRow;	// for index
	  	for (nRow = 0; nRow < GetRows(); nRow++) 
		{
    		if (nRow != nCol) 
			{
				// get the scale factor to be applied
    			scale = -mCopy[nCol][nRow];

				// add a scaled version of the diagonal row
				//		to obtain a zero at this row and column
				mCopy.GetRow(nCol, vRow);
				mCopy.GetRow(nRow, vOtherRow);
				vOtherRow += vRow * scale;
				mCopy.SetRow(nRow, vOtherRow);

				// same operation on the inverse
				mInv.GetRow(nCol, vRow);
				mInv.GetRow(nRow, vOtherRow);
				vOtherRow += vRow * scale;
				mInv.SetRow(nRow, vOtherRow);
    		}
    	}
	}

	if (bFullPivot)
	{
		// restores the sequence of columns
		for (int nAtCol = GetCols()-1; nAtCol >= 0; nAtCol--)
		{
			mInv.InterchangeCols(nAtCol, arrColumnSwaps[nAtCol]);
		}
	}

	// assign this to inverse
	(*this) = mInv; 

}	// CMatrixBase<TYPE>::Invert


//////////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::InterchangeRows
//
// swaps two rows of the matrix
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void CMatrixBase<TYPE>::InterchangeRows(int nRow1, int nRow2) 
{
	// check that the rows are not the same
	if (nRow1 != nRow2)
	{
		for (int nAtCol = 0; nAtCol < GetCols(); nAtCol++)
		{
			TYPE temp = (*this)[nAtCol][nRow1];
			(*this)[nAtCol][nRow1] = (*this)[nAtCol][nRow2];
			(*this)[nAtCol][nRow2] = temp;
		}
	}

}	// CMatrixBase<TYPE>::InterchangeRows

    
//////////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::InterchangeCols
//
// swaps two COLUMNS of the matrix
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void CMatrixBase<TYPE>::InterchangeCols(int nCol1, int nCol2) 
{
	// check that the cols are not the same
	if (nCol1 != nCol2)
	{
		// temporary vector storage
		CVectorN<TYPE> vTemp = (*this)[nCol1];	

		// and swap the two rows
		(*this)[nCol1] = (*this)[nCol2];
		(*this)[nCol2] = vTemp;
	}

}	// CMatrixBase<TYPE>::InterchangeRows

    
//////////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::FindPivotRow
//
// finds the pivot element (returns the row of the element in the
//		the given column)
//////////////////////////////////////////////////////////////////////
template<class TYPE>
int CMatrixBase<TYPE>::FindPivotRow(int nDiag)
{
	int nBestRow = nDiag;	// stores the current best row

	if (fabs((*this)[nDiag][nDiag]) < MAX_TO_PIVOT) 
	{
		int nRow;	// for index
    	for (nRow = nDiag + 1; nRow < GetRows(); nRow++)
		{
			if (fabs((*this)[nDiag][nRow]) > fabs((*this)[nDiag][nBestRow]))
			{
    			nBestRow = nRow;
			}
		}
	}

	return nBestRow;

}	// CMatrixBase<TYPE>::FindPivotRow


//////////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::FindPivotElem
//
// finds the pivot element, returning the row and column
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void CMatrixBase<TYPE>::FindPivotElem(int nDiag, 
		int *pBestRow, int *pBestCol)
{
	(*pBestCol) = nDiag;
	(*pBestRow) = nDiag;

	if (fabs((*this)[nDiag][nDiag]) < MAX_TO_PIVOT) 
	{
		for (int nCol = nDiag; nCol < GetCols(); nCol++)
		{
    		for (int nRow = nDiag; nRow < GetRows(); nRow++)
			{
				if (fabs((*this)[nCol][nRow]) 
					> fabs((*this)[*pBestCol][*pBestRow]))
				{
					(*pBestCol) = nCol;
					(*pBestRow) = nRow;
				}
			}
		}
	}

}	// CMatrixBase<TYPE>::FindPivotElem


//////////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::SVD
//
// computes the singular-valued decomposition of this matrix,
//		leaving U in the matrix and returning the singular values
//		in w and v (not v^T)
//////////////////////////////////////////////////////////////////////
template<class TYPE>
BOOL CMatrixBase<TYPE>::SVD(CVectorBase<TYPE>& w, CMatrixBase<TYPE>& v)
{
	// stored reduction vector
	CVectorN<TYPE> rv1(GetCols());

	// perform the householder reduction
	TYPE anorm = Householder(w, rv1);

	// accumulation of right-hand transformations
	AccumulateRH(v, rv1);

	// accumulation of left-hand transformations
	AccumulateLH(w);

	// Diagonalization of the bidiagonal form
	for (int nK = GetCols()-1; nK >= 0; nK--)
	{
		for (int nIter = 1; nIter < MAX_ITER; nIter++)
		{
			for (int nL = nK; nL >= 0; nL--)
			{
				if (((TYPE) fabs(rv1[nL]) + anorm) == anorm)
				{
					break;
				}

				if (nL == 0 
					|| (((TYPE) fabs(w[nL - 1]) + anorm) == anorm))
				{
					TYPE c = 0.0;
					TYPE s = 1.0;
					for (int nI = nL; nI <= nK; nI++)
					{
						TYPE f = s * rv1[nI];
						rv1[nI] = c * rv1[nI];

						if ((TYPE)(fabs(f) + anorm) == anorm)
						{
							break;
						}

						TYPE g = w[nI];
						TYPE h = pythag(f,g);
						w[nI] = h;
						h = 1.0 / h;
						c = g * h;
						s = -f * h;
						for (int nJ = 0; nJ < GetRows(); nJ++)
						{
							TYPE y = (*this)[nL - 1][nJ];
							TYPE z = (*this)[nI][nJ];
							(*this)[nL - 1][nJ] = y * c + z * s;
							(*this)[nI][nJ] = z * c - y * s;
						}	// for
					}	// for

					break;

				}	// if
			}

			// test for convergence
			if (nL == nK)
			{
				if (w[nK] < 0.0)
				{
					// singular value is made non-negative
					w[nK] = -w[nK];
					for (int nJ = 0; nJ < GetCols(); nJ++)
					{
						v[nK][nJ] = -v[nK][nJ];
					}
				}

				break;
			}	// if

			TYPE x = w[nL];	// shift from bottom 2x2 minor
			TYPE y = w[nK - 1];
			TYPE z = w[nK];

			TYPE g = rv1[nK - 1];
			TYPE h = rv1[nK];
			TYPE f = ((y - z) * (y + z) + (g - h) * (g + h)) 
				/ (2.0 * h * y);
			g = pythag<TYPE>(f, 1.0);
			f = ((x - z) * (x + z) + h * ((y / (f + SIGN(g,f))) - h))
				/ x;

			TYPE c = 1.0;	// next QR transformation:
			TYPE s = 1.0;		
			for (int nJ = 0; nJ <= nK - 1; nJ++)
			{
				g = rv1[nJ + 1];
				y = w[nJ + 1];
				h = s * g;
				g = c * g;
				z = pythag<TYPE>(f, h);
				rv1[nJ] = z;
				c = f / z;
				s = h / z;
				f = x * c + g * s;
				g = g * c - x * s;
				h = y * s;
				y *= c;
				for (int nJJ = 0; nJJ < GetCols(); nJJ++)
				{
					x = v[nJ][nJJ];
					z = v[nJ + 1][nJJ];
					v[nJ][nJJ] = x * c + z * s;
					v[nJ + 1][nJJ] = z * c - x * s;
				}
				z = pythag(f,h);
				w[nJ] = z;
				if (z != 0.0)
				{
					z = 1.0/z;
					c = f * z;
					s = h * z;
				}
				f = c * g + s * y;
				x = c * y - s * g;
				for (nJJ = 0; nJJ < GetRows(); nJJ++)
				{
					y = (*this)[nJ][nJJ];
					z = (*this)[nJ + 1][nJJ];
					(*this)[nJ][nJJ] = y * c + z * s;
					(*this)[nJ + 1][nJJ] = z * c - y * s;
				}
			}	// for

			rv1[nL] = 0.0;
			rv1[nK] = f;
			w[nK] = x;
		}

		if (nIter == MAX_ITER)
		{
			TRACE("SVD: no convergence in %i steps\n", MAX_ITER);
			return FALSE;
		}
	}

	return TRUE;

}	// CMatrixBase<TYPE>::SVD


//////////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::Householder
//
// helper function for SVD to perform Householder decomposition
//////////////////////////////////////////////////////////////////////
template<class TYPE>
TYPE CMatrixBase<TYPE>::Householder(CVectorBase<TYPE>& w, 
									CVectorBase<TYPE>& rv1)
{
	TYPE anorm = 0.0;

	// Householder reduction to bidiagonal form
	TYPE g = 0.0;
	TYPE scale = 0.0;
	for (int nI = 0; nI < GetCols(); nI++)
	{
		rv1[nI] = scale * g;
		g = scale = 0.0;
		if (nI <= GetRows()-1)
		{
			for (int nK = nI; nK < GetRows(); nK++)
			{
				scale += fabs((*this)[nI][nK]);
			}
			if (scale != 0.0)
			{
				TYPE s = 0.0;
				for (int nK = nI; nK < GetRows(); nK++)
				{
					(*this)[nI][nK] /= scale;
					s += (*this)[nI][nK] * (*this)[nI][nK];
				}

				TYPE f = (*this)[nI][nI];
				g = -SIGN(sqrt(s),f);
				TYPE h = f * g - s;
				(*this)[nI][nI] = f - g;

				for (int nJ = nI + 1; nJ < GetCols(); nJ++)
				{
					TYPE s = 0.0;
					for (int nK = nI; nK < GetRows(); nK++)
					{
						s += (*this)[nI][nK] * (*this)[nJ][nK];
					}
					TYPE f = s / h;
					for (nK = nI; nK < GetRows(); nK++)
					{
						(*this)[nJ][nK] += f * (*this)[nI][nK];
					}
				}	// for

				for (nK = nI; nK < GetRows(); nK++)
				{
					(*this)[nI][nK] *= scale;
				}
			}	// if
		}	// if

		w[nI] = scale * g;
		g = scale = 0.0;
		if (nI < GetRows() && nI != GetCols()-1)
		{
			for (int nK = nI + 1; nK < GetCols(); nK++)
			{
				scale += fabs((*this)[nK][nI]);
			}
			if (scale != 0.0)
			{
				TYPE s = 0.0;
				for (int nK = nI + 1; nK < GetCols(); nK++)
				{
					(*this)[nK][nI] /= scale;
					s += (*this)[nK][nI] * (*this)[nK][nI];
				}

				TYPE f = (*this)[nI + 1][nI];
				g = -SIGN(sqrt(s), f);
				TYPE h = f * g - s;
				(*this)[nI + 1][nI] = f - g;

				for (nK = nI + 1; nK < GetCols(); nK++)
				{
					rv1[nK] = (*this)[nK][nI] / h;
				}

				for (int nJ = nI + 1; nJ < GetRows(); nJ++)
				{
					TYPE s = 0.0;
					for (int nK = nI + 1; nK < GetCols(); nK++)
					{
						s += (*this)[nK][nJ] * (*this)[nK][nI];
					}
					for (nK = nI + 1; nK < GetCols(); nK++)
					{
						(*this)[nK][nJ] += s * rv1[nK];
					}
				}

				for (nK = nI + 1; nK < GetCols(); nK++)
				{
					(*this)[nK][nI] *= scale;
				}
			}	// if
		}	// if

		anorm = __max(anorm, (fabs(w[nI]) + fabs(rv1[nI])));

	}	// for

	return anorm;

}	// CMatrixBase<TYPE>::Householder


//////////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::AccumulateRH
//
// helper function for SVD to accumulate right-hand products
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void CMatrixBase<TYPE>::AccumulateRH(CMatrixBase<TYPE>& v, 
									 const CVectorBase<TYPE>& rv1)
{
	// Accumulation of right-hand transformations
	for (int nI = GetCols() - 2; nI >= 0; nI--)
	{
		v[nI + 1][nI + 1] = 1.0;

		if (rv1[nI + 1] != 0.0)
		{
			for (int nJ = nI + 1; nJ < GetCols(); nJ++)
			{
				v[nI][nJ] = ((*this)[nJ][nI] / (*this)[nI + 1][nI]) 
					/ rv1[nI + 1];
			}

			for (nJ = nI + 1; nJ < GetCols(); nJ++)
			{
				TYPE s = 0.0;
				for (int nK = nI + 1; nK < GetCols(); nK++)
				{
					s += (*this)[nK][nI] * v[nJ][nK];
				}

				for (nK = nI + 1; nK < GetCols(); nK++)
				{
					v[nJ][nK] += s * v[nI][nK];
				}
			}	// for
		}	// if

		for (int nJ = nI + 1; nJ < GetCols(); nJ++)
		{
			v[nJ][nI] = v[nI][nJ] = 0.0;
		}
	}

}	// CMatrixBase<TYPE>::AccumulateRH


//////////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::AccumulateLH
//
// helper function for SVD to accumulate left-hand products
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void CMatrixBase<TYPE>::AccumulateLH(CVectorBase<TYPE>& w)
{
	for (int nI = __min(GetRows()-1, GetCols()-1); nI >= 0; nI--)
	{
		for (int nJ = nI + 1; nJ < GetCols(); nJ++)
		{
			(*this)[nJ][nI] = 0.0;
		}

		if (w[nI] != 0.0)
		{
			TYPE g = 1.0 / w[nI];
			for (int nJ = nI + 1; nJ < GetCols(); nJ++)
			{
				TYPE s = 0.0;
				for (int nK = nI + 1; nK < GetRows(); nK++)
				{
					s += (*this)[nI][nK] * (*this)[nJ][nK];
				}

				TYPE f = (s / (*this)[nI][nI]) * g;
				for (nK = nI; nK < GetRows(); nK++)
				{
					(*this)[nJ][nK] += f * (*this)[nI][nK];
				}

			}	// for

			for (nJ = nI; nJ < GetRows(); nJ++)
			{
				(*this)[nI][nJ] *= g;
			}
		}
		else
		{
			for (nJ = nI; nJ < GetRows(); nJ++)
			{
				(*this)[nI][nJ] = 0.0;
			}
		}

		++(*this)[nI][nI];
	}

}	// CMatrixBase<TYPE>::AccumulateLH


#endif // define _MATRIXBASE_INL_
