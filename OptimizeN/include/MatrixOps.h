// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: MatrixOps.h 643 2009-11-05 17:46:17Z dglane001 $
#if !defined(MATRIXOPS_H)
#define MATRIXOPS_H

// valarray helper class
#undef min
#undef max

#include <valarray>
using namespace std;

#define INLINE __forceinline


//////////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
void 
	MultMatrixVector(TYPE *vProd, 
			const TYPE * mLeft, int nCols, int nRows,
			const TYPE *vRight)
	// multiply the vector by matrix
{
	// clear product
	ZeroValues(vProd, nRows);

	// step through the rows of the matrix
	for (int nAtRow = 0; nAtRow < nRows; nAtRow++)
	{
		// step through the columns of the matrix
		for (int nAtCol = 0, nAtMatElem = nAtRow; nAtCol < nCols; 
				nAtCol++, nAtMatElem += nRows)
		{
			vProd[nAtRow] += mLeft[nAtMatElem] * vRight[nAtCol];
		}
	}

}	// MultMatrixVector

#ifdef NEVER // USE_IPP

//////////////////////////////////////////////////////////////////////
template<> INLINE
void 
	MultMatrixVector(float *vProd, 
			const float *mLeft, int nCols, int nRows,
			const float *vRight)
	// multiply the vector by matrix
{
	CK_IPP(ippmMul_mTv_32f(mLeft, 
		nRows * sizeof(float),
		nRows, nCols,
		vRight, nCols,											
		vProd));

}	// MultMatrixVector


//////////////////////////////////////////////////////////////////////
template<> INLINE
void 
	MultMatrixVector(double *vProd, 
			const double *mLeft, int nCols, int nRows,
			const double *vRight)
	// multiply the vector by matrix
{
	CK_IPP(ippmMul_mTv_64f(mLeft, 
		nRows * sizeof(double),
		nRows, nCols,
		vRight, nCols,											
		vProd));

}	// MultMatrixVector


#endif	// USE_IPP



//////////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
void 
	OuterProdN(TYPE *mProd, 
				const TYPE* vLeft, int nLeftDim,
				const TYPE *vRight, int nRightDim)
	// compute the matrix product
{
	for (int nRow = 0; nRow < nLeftDim; nRow++)
	{
		for (int nCol = 0; nCol < nRightDim; nCol++)
		{
			mProd[nCol * nLeftDim + nRow] = vLeft[nRow] * vRight[nCol];
		}
	}

}	// OuterProdN


#ifdef NEVER // USE_IPP

//////////////////////////////////////////////////////////////////////
#define DECLARE_OUTERPRODN(TYPE, TYPE_IPP) \
template<> INLINE											\
void OuterProdN(TYPE *mProd,								\
				const TYPE* vLeft, int nLeftDim,			\
				const TYPE *vRight, int nRightDim)			\
{															\
	CK_IPP(ippmMul_mm_##TYPE_IPP(							\
		vLeft,												\
			/* src1Stride1 */ 1 * sizeof(TYPE),				\
			1, nLeftDim,									\
		vRight,												\
			/* src2Stride1 */ nRightDim * sizeof(TYPE),		\
			nRightDim, 1,									\
		mProd,												\
			/* dstStride1 */ nRightDim * sizeof(TYPE)));	\
}

DECLARE_OUTERPRODN(float, 32f);
DECLARE_OUTERPRODN(double, 64f);

#endif


//////////////////////////////////////////////////////////////////////
template<class MATRIX_TYPE> INLINE
void 
	MultMatrixMatrix(MATRIX_TYPE& mProd, 
			const MATRIX_TYPE& mLeft, 
			const MATRIX_TYPE& mRight)
	// multiply two matrices
	// TODO: change signature to support IPP over-rides
{
	// check dimensions
	ASSERT(mLeft.GetCols() == mRight.GetRows());
	ASSERT(mProd.GetCols() == mRight.GetCols());
	ASSERT(mProd.GetRows() == mLeft.GetRows());

	// compute the matrix product
	for (int nRow = 0; nRow < mProd.GetRows(); nRow++)
	{
		for (int nCol = 0; nCol < mProd.GetCols(); nCol++)
		{
			mProd[nCol][nRow] = 0.0;

			for (int nMid = 0; nMid < mLeft.GetCols(); nMid++)
			{
				mProd[nCol][nRow] +=
					mLeft[nMid][nRow] * mRight[nCol][nMid];
			}
		}
	}
}

#ifdef USE_IPP
// TODO: implement MultMatrixMatrix for IPP calls
#endif

//////////////////////////////////////////////////////////////////////
template<class MATRIX_TYPE>
bool 
	IsOrthogonal(const MATRIX_TYPE& mMat)
	// tests for orthogonality of the matrix
{
	MATRIX_TYPE mTrans = mMat;
	mTrans.Transpose();
	mTrans *= mMat;

	MATRIX_TYPE mIdent = mMat;
	mIdent.SetIdentity();

	return mTrans.IsApproxEqual(mTrans);

}	// IsOrthogonal


//////////////////////////////////////////////////////////////////////
template<class MATRIX_TYPE>
void 
	Orthogonalize(MATRIX_TYPE& mMat)
	// orthogonalizes the input matrix using GSO
{
	// normalize the first column vector
	mMat[0].Normalize();

	// apply to each row vector after the zero-th
	for (int nAtCol = 1; nAtCol < mMat.GetCols(); nAtCol++)
	{
		// normalize the next column vector
		mMat[nAtCol].Normalize();

		// ensure orthogonality with all previous column vectors
		for (int nAtOrthoCol = nAtCol - 1; nAtOrthoCol >= 0; 
				nAtOrthoCol--)
		{
			// compute the scale factor
			MATRIX_TYPE::ELEM_TYPE scalar = (mMat[nAtCol] * mMat[nAtOrthoCol]) 
				/ (mMat[nAtOrthoCol] * mMat[nAtOrthoCol]);

			for (int nAtRow = 0; nAtRow < mMat.GetRows(); nAtRow++)
			{
				mMat[nAtCol][nAtRow] -= scalar * mMat[nAtOrthoCol][nAtRow];
			}

			// make sure we are now orthogonal to this
			ASSERT(mMat[nAtCol] * mMat[nAtOrthoCol] < DEFAULT_EPSILON);
		}
	}

}	// Orthogonalize


//////////////////////////////////////////////////////////////////////
template<class MATRIX_TYPE>
bool 
	Invert(MATRIX_TYPE& mMat, bool bFullPivot = false)
	// invert the matrix
{
	// only invert square matrices
	ASSERT(mMat.GetCols() == mMat.GetRows());

	// Gauss-Jordan elimination
	MATRIX_TYPE mCopy = mMat;		// the copy of this matrix
	MATRIX_TYPE mInv = mMat;		// stores the built inverse
	mInv.SetIdentity();

	// stores the sequence of column swaps for a full pivot
	valarray<int> arrColumnSwaps(mMat.GetCols());

	// helper vectors to hold rows
	MATRIX_TYPE::COL_TYPE vRow(mMat.GetCols());
	MATRIX_TYPE::COL_TYPE vOtherRow(mMat.GetCols());
	
	for (int nCol = 0; nCol < mMat.GetCols(); nCol++)
	{
		if (bFullPivot)
		{
			// CHECK THE FULL PIVOT
			// find the full pivot element
			int nPivotRow, nPivotCol;
			FindPivotElem(mCopy, nCol, &nPivotRow, &nPivotCol);

			// interchange the rows
			InterchangeRows(mCopy, nCol, nPivotRow);
			InterchangeCols(mCopy, nCol, nPivotCol);

			// interchange the columns
			InterchangeRows(mInv, nCol, nPivotRow);
			InterchangeCols(mInv, nCol, nPivotCol);
			
			// store the column interchange for later restoring
			arrColumnSwaps[nCol] = nPivotCol;
		}
		else	// partial pivot
		{
			// pivot if necessary
			int nPivotRow = FindPivotRow(mCopy, nCol);
			InterchangeRows(mCopy, nCol, nPivotRow);
			InterchangeRows(mInv, nCol, nPivotRow);
		}

    	// obtain a 1 in the diagonal position
		//		(the pivot ensures that copy[nCol][nCol] is not zero)
		// make sure we are numerically stable
		if (fabs(mCopy[nCol][nCol]) < 1e-8)
		{
			return FALSE;
		}

		// scale factor to be applied
		MATRIX_TYPE::ELEM_TYPE scale = R(1.0) / mCopy[nCol][nCol];	

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
	  	for (nRow = 0; nRow < mCopy.GetRows(); nRow++) 
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
		for (int nAtCol = mInv.GetCols()-1; nAtCol >= 0; nAtCol--)
		{
			InterchangeCols(mInv, nAtCol, arrColumnSwaps[nAtCol]);
		}
	}

	// assign this to inverse
	mMat = mInv; 

	// return OK
	return true;

}	// Invert


//////////////////////////////////////////////////////////////////////
template<class MATRIX_TYPE>
void 
	InterchangeRows(MATRIX_TYPE& mMat, int nRow1, int nRow2) 
	// swaps two rows of the matrix
{
	// check that the rows are not the same
	if (nRow1 != nRow2)
	{
		for (int nAtCol = 0; nAtCol < mMat.GetCols(); nAtCol++)
		{
			MATRIX_TYPE::ELEM_TYPE temp = mMat[nAtCol][nRow1];
			mMat[nAtCol][nRow1] = mMat[nAtCol][nRow2];
			mMat[nAtCol][nRow2] = temp;
		}
	}

}	// InterchangeRows

    
//////////////////////////////////////////////////////////////////////
template<class MATRIX_TYPE>
void 
	InterchangeCols(MATRIX_TYPE& mMat, int nCol1, int nCol2) 
	// swaps two COLUMNS of the matrix
{
	// check that the cols are not the same
	if (nCol1 != nCol2)
	{
		// temporary vector storage
		MATRIX_TYPE::COL_TYPE vTemp = mMat[nCol1];	

		// and swap the two rows
		mMat[nCol1] = mMat[nCol2];
		mMat[nCol2] = vTemp;
	}

}	// InterchangeRows

    
// constant for finding the pivot during a matrix inversion
#define MAX_TO_PIVOT (100.0)

//////////////////////////////////////////////////////////////////////
template<class MATRIX_TYPE>
int 
	FindPivotRow(MATRIX_TYPE& mMat, int nDiag)
	// finds the pivot element (returns the row of the element in the
	//		the given column)
{
	int nBestRow = nDiag;	// stores the current best row

	if (fabs(mMat[nDiag][nDiag]) < MAX_TO_PIVOT) 
	{
		int nRow;	// for index
    	for (nRow = nDiag + 1; nRow < mMat.GetRows(); nRow++)
		{
			if (fabs(mMat[nDiag][nRow]) > fabs(mMat[nDiag][nBestRow]))
			{
    			nBestRow = nRow;
			}
		}
	}

	return nBestRow;

}	// FindPivotRow


//////////////////////////////////////////////////////////////////////
template<class MATRIX_TYPE>
void 
	FindPivotElem(MATRIX_TYPE& mMat, int nDiag, int *pBestRow, int *pBestCol)
	// finds the pivot element, returning the row and column
{
	(*pBestCol) = nDiag;
	(*pBestRow) = nDiag;

	if (fabs(mMat[nDiag][nDiag]) < MAX_TO_PIVOT) 
	{
		for (int nCol = nDiag; nCol < mMat.GetCols(); nCol++)
		{
    		for (int nRow = nDiag; nRow < mMat.GetRows(); nRow++)
			{
				if (fabs(mMat[nCol][nRow]) 
					> fabs(mMat[*pBestCol][*pBestRow]))
				{
					(*pBestCol) = nCol;
					(*pBestRow) = nRow;
				}
			}
		}
	}

}	// FindPivotElem


//////////////////////////////////////////////////////////////////////
template<class MATRIX_TYPE>
void 
	TraceMatrix(const char *pszMessage, const MATRIX_TYPE& m)
	// outputs the matrix for tracing
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

#endif

