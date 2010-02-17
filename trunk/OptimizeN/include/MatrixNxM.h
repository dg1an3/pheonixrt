// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: MatrixNxM.h 643 2009-11-05 17:46:17Z dglane001 $
#if !defined(MATRIXNXM_H)
#define MATRIXNXM_H

#include "MathUtil.h"

#include "VectorN.h"

#include "MatrixOps.h"

//////////////////////////////////////////////////////////////////////
// class CMatrixNxM<TYPE>
//
// represents a non-square matrix with type given.
//////////////////////////////////////////////////////////////////////
template<class TYPE = REAL>
class CMatrixNxM
{
	// counts number of columns
	int m_nColumns;

	// the row vectors of the matrix
	CVectorN<TYPE> *m_pColumns;

	// the elements of the matrix
	TYPE *m_pElements;

	// flag to indicate whether elements should be freed
	bool m_bFreeElements;

public:
	// constructors / destructor
	CMatrixNxM();
	explicit CMatrixNxM(int nCols, int nRows);
	CMatrixNxM(const CMatrixNxM& fromMatrix);
	~CMatrixNxM();

	// assignment operator
	CMatrixNxM& operator=(const CMatrixNxM<TYPE>& fromMatrix);

	// SetIdentity -- sets the matrix to an identity matrix
	void SetIdentity();

	// element access
	CVectorN<TYPE>& operator[](int nAtCol);
	const CVectorN<TYPE>& operator[](int nAtCol) const;

	vnl_vector<TYPE>& GetVnlColumn(int nAtCol) { return (*this)[nAtCol].GetVnlVector(); }
	const vnl_vector<TYPE>& GetVnlColumn(int nAtCol) const { return (*this)[nAtCol].GetVnlVector(); }

	// TYPE * conversion -- returns a pointer to the first element
	//		WARNING: this allows for no-bounds-checking access
	operator TYPE *();
	operator const TYPE *() const;

	// matrix size
	int GetCols() const;
	int GetRows() const;

	// sets the dimension of the matrix -- optional preserves
	//		elements
	void Reshape(int nCols, int nRows, bool bPreserve = TRUE);

	// row-vector access
	//void GetRow(int nAtRow, CVectorN<TYPE>& vRow) const;
	//void SetRow(int nAtRow, const CVectorN<TYPE>& vRow);

	// IsApproxEqual -- tests for approximate equality using the EPS
	//		defined at the top of this file
	bool IsApproxEqual(const CMatrixNxM& m, TYPE epsilon = DEFAULT_EPSILON) const;

	// in-place operators
	CMatrixNxM& operator+=(const CMatrixNxM& mRight);
	CMatrixNxM& operator-=(const CMatrixNxM& mRight);
	CMatrixNxM& operator*=(const TYPE& scale);
	CMatrixNxM& operator*=(const CMatrixNxM& mRight);

	// Transpose -- transposes elements of the matrix
	void Transpose();

	// Invert -- inverts the matrix using the Gauss-Jordan 
	//		elimination
	bool Invert(bool bFullPivot = FALSE);

protected:
	// typedef helpers
	typedef TYPE ELEM_TYPE;
	typedef CVectorN<TYPE> COL_TYPE;

	// SetElements -- sets the elements to an external pointer
	void SetElements(int nCols, int nRows, TYPE *pElements, bool bFreeElements);

};	// class CMatrixNxM


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
CMatrixNxM<TYPE>::CMatrixNxM()
	// default constructor -- initializes to 0x0 matrix
	: m_nColumns(0),
		m_pColumns(NULL),
		m_pElements(NULL),
		m_bFreeElements(TRUE)
{
}	// CMatrixNxM<TYPE>::CMatrixNxM<TYPE>


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
CMatrixNxM<TYPE>::CMatrixNxM(int nCols, int nRows)
	// constructs a specific-dimensioned matrix
	: m_nColumns(0),
		m_pColumns(NULL),
		m_pElements(NULL),
		m_bFreeElements(TRUE)
{
	Reshape(nCols, nRows);

}	// CMatrixNxM<TYPE>::CMatrixNxM<TYPE>(int nCols, int nRows)


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
CMatrixNxM<TYPE>::CMatrixNxM(const CMatrixNxM<TYPE>& fromMatrix)
	// copy constructor
	: m_nColumns(0),
		m_pColumns(NULL),
		m_pElements(NULL),
		m_bFreeElements(TRUE)
{
	// sets the dimensions
	Reshape(fromMatrix.GetCols(), fromMatrix.GetRows());

	(*this) = fromMatrix;

}	// CMatrixNxM<TYPE>::CMatrixNxM<TYPE>(
	//		const CMatrixNxM<TYPE>& fromMatrix)


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
CMatrixNxM<TYPE>::~CMatrixNxM()
	// destructor
{
	// frees any elements, if needed
	SetElements(0, 0, NULL, TRUE);

}	// CMatrixNxM<TYPE>::~CMatrixNxM<TYPE>


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
CMatrixNxM<TYPE>& 
	CMatrixNxM<TYPE>::operator=(const CMatrixNxM<TYPE>& fromMatrix)
	// assignment operator
{
	// checks the dimensions
	ASSERT(GetCols() == fromMatrix.GetCols());
	ASSERT(GetRows() == fromMatrix.GetRows());

	if (GetCols() > 0)
	{
		// assign values
		CopyValues(&(*this)[0][0], &fromMatrix[0][0], GetCols() * GetRows());
	}

	return (*this);

}	// CMatrixNxM<TYPE>::operator=


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
void 
	CMatrixNxM<TYPE>::SetIdentity()
	// sets the matrix to an identity matrix
{
	ZeroValues(&(*this)[0][0], GetRows() * GetCols());

	// for each element in the matrix,
	for (int nAt = 0; nAt < __min(GetCols(), GetRows()); nAt++)
	{
		(*this)[nAt][nAt] = TYPE(1.0);
	}

}	// CMatrixD<TYPE>::SetIdentity


//////////////////////////////////////////////////////////////////
template<class TYPE>
void 
	CMatrixNxM<TYPE>::Reshape(int nCols, int nRows, bool bPreserve)
	// sets the dimension of the matrix
{
	// check if we need to reshape
	if (GetRows() == nRows 
		&& GetCols() == nCols)
	{
		// no reshape, but if we aren't preserving elements
		if (!bPreserve)
		{
			// then set to identity
			SetIdentity();
		}

		return;
	}

	// preserve existing elements
	int nOldRows = GetRows();
	int nOldCols = GetCols();

	// store pointer to old elements, if we are preserving
	if (!bPreserve)
	{
		FreeValues(m_pElements);
	}
	TYPE *pOldElements = m_pElements;

	// allocate and set the new elements, but do not free the old
	TYPE *pNewElements = NULL;
	AllocValues(nCols * nRows, pNewElements);

	// don't free the existing elements, as they will be copied
	m_bFreeElements = FALSE;

	// set the new elements
	SetElements(nCols, nRows, pNewElements, TRUE);

	// if there were old elements, 
	if (pOldElements)
	{
		// set the new elements to 0 initially
		ZeroValues(pNewElements, nCols * nRows);

		// create a temporary matrix to hold the old elements
		CMatrixNxM<TYPE> mTemp;
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

}	// CMatrixNxM<TYPE>::Reshape


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
CVectorN<TYPE>& 
	CMatrixNxM<TYPE>::operator[](int nAtCol)
	// retrieves a reference to a column vector
{
	// bounds check on the index
	ASSERT(nAtCol >= 0 && nAtCol < GetCols());

	// return a reference to the column vector
	return m_pColumns[nAtCol];

}	// CMatrixNxM<TYPE>::operator[]


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
const CVectorN<TYPE>& 
	CMatrixNxM<TYPE>::operator[](int nAtCol) const
	// retrieves a reference to a column vector
{
	// bounds check on the index
	ASSERT(nAtCol >= 0 && nAtCol < GetCols());

	// return a reference to the column vector
	return m_pColumns[nAtCol];

}	// CMatrixNxM<TYPE>::operator[]


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
int 
	CMatrixNxM<TYPE>::GetCols() const
	// returns the number of columns of the matrix
{
	return m_nColumns;

}	// CMatrixNxM<TYPE>::GetCols


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
int 
	CMatrixNxM<TYPE>::GetRows() const
	// returns the number of rows of the matrix
{
	if (m_pColumns)
	{
		return m_pColumns->GetDim();
	}

	return 0;

}	// CMatrixNxM<TYPE>::GetRows


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
CMatrixNxM<TYPE>::operator TYPE *()
	// TYPE * conversion -- returns a pointer to the first element
	//		WARNING: this allows for no-bounds-checking access

{
	return &m_pElements[0];

}	// CMatrixNxM<TYPE>::operator TYPE *


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
CMatrixNxM<TYPE>::operator const TYPE *() const
	// const TYPE * conversion -- returns a pointer to the first 
	//		element.
	//		WARNING: this allows for no-bounds-checking access
{
	return &m_pElements[0];

}	// CMatrixNxM<TYPE>::operator const TYPE *

#ifdef USE_ROW_OPERATIONS
//////////////////////////////////////////////////////////////////
template<class TYPE>
void 
	CMatrixNxM<TYPE>::GetRow(int nAtRow, CVectorN<TYPE>& vRow) const
	// constructs and returns a row vector
{
	// make the row vector the same size
	ASSERT(vRow.GetDim() == GetCols());

	// populate the row vector
	for (int nAtCol = 0; nAtCol < GetCols(); nAtCol++)
	{
		vRow[nAtCol] = (*this)[nAtCol][nAtRow];
	}

}	// CMatrixNxM<TYPE>::GetRow


//////////////////////////////////////////////////////////////////
template<class TYPE>
void 
	CMatrixNxM<TYPE>::SetRow(int nAtRow, const CVectorN<TYPE>& vRow)
	// sets the rows vector
{
	if (vRow.GetDim() == GetCols())
	{
		// de-populate the row vector
		for (int nAtCol = 0; nAtCol < GetCols(); nAtCol++)
		{
			(*this)[nAtCol][nAtRow] = vRow[nAtCol];
		}
	}

}	// CMatrixNxM<TYPE>::SetRow
#endif

//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
bool 
	CMatrixNxM<TYPE>::IsApproxEqual(const CMatrixNxM& m, TYPE epsilon) const
	// tests for approximate equality using the EPS 
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

}	// CMatrixNxM<TYPE>::IsApproxEqual


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
CMatrixNxM<TYPE>& 
	CMatrixNxM<TYPE>::operator+=(const CMatrixNxM<TYPE>& mRight)
	// in-place matrix addition; returns a reference to this
{
	ASSERT(GetCols() == mRight.GetCols());
	ASSERT(GetRows() == mRight.GetRows());

	SumValues(&(*this)[0][0], &mRight[0][0], GetCols() * GetRows());

	// return a reference to this
	return (*this);

}	// CMatrixNxM<TYPE>::operator+=


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
CMatrixNxM<TYPE>& 
	CMatrixNxM<TYPE>::operator-=(const CMatrixNxM<TYPE>& mRight)
	// in-place matrix subtraction; returns a reference to 	this
{
	ASSERT(GetCols() == mRight.GetCols());
	ASSERT(GetRows() == mRight.GetRows());

	DiffValues(&(*this)[0][0], &mRight[0][0], GetCols() * GetRows());

	// return a reference to this
	return (*this);

}	// CMatrixNxM<TYPE>::operator-=


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
CMatrixNxM<TYPE>& 
	CMatrixNxM<TYPE>::operator*=(const CMatrixNxM<TYPE>& mRight)
	// in-place matrix multiplication; returns a reference to this
{
	CMatrixNxM<TYPE> mProduct = (*this) * mRight;

	// and assign
	Reshape(mProduct.GetCols(), mProduct.GetRows());
	(*this) = mProduct;

	// return a reference to this
	return (*this);

}	// CMatrixNxM<TYPE>::operator*=


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
CMatrixNxM<TYPE>& 
	CMatrixNxM<TYPE>::operator*=(const TYPE& scale)
	// in-place matrix multiplication; returns a reference to this
{
	MultValues(&(*this)[0][0], scale, GetCols() * GetRows());

	// return a reference to this
	return (*this);

}	// CMatrixNxM<TYPE>::operator*=


//////////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
void 
	CMatrixNxM<TYPE>::Transpose()
	// transposes the matrix
{
	// allocate the elements of the transposed matrix
	TYPE *pElements = NULL;
	AllocValues(GetRows() * GetCols(), pElements);

	// make the transposed matrix
	CMatrixNxM<TYPE> mTranspose;
	mTranspose.SetElements(GetRows(), GetCols(), pElements, FALSE);
	
	for (int nCol = 0; nCol < GetCols(); nCol++)
	{
		for (int nRow = 0; nRow < GetRows(); nRow++)
		{
			mTranspose[nRow][nCol] = (*this)[nCol][nRow];
		}
	}

	// set the elements of this to the transposed elements
	SetElements(GetRows(), GetCols(), pElements, TRUE);

}	// CMatrixD<TYPE>::Transpose


#ifdef NEVER // USE_IPP

//////////////////////////////////////////////////////////////////////
template<> INLINE									
void 
	CMatrixNxM<float>::Transpose()					
	// transposes the matrix
{													
	float *pElements = NULL;							
	AllocValues(GetRows() * GetCols(), pElements);	

	CK_IPP(ippmTranspose_m_32f(&(*this)[0][0],		
		GetRows() * sizeof(float),					
		GetRows(), GetCols(),						
		pElements, GetCols() * sizeof(float)));		

	SetElements(GetRows(), GetCols(), pElements, true);

}	// CMatrixNxM<TYPE>::Transpose

//////////////////////////////////////////////////////////////////////
template<> INLINE									
void 
	CMatrixNxM<double>::Transpose()					
	// transposes the matrix
{							
	double *pElements = NULL;							
	AllocValues(GetRows() * GetCols(), pElements);	

	CK_IPP(ippmTranspose_m_64f(&(*this)[0][0],		
		GetRows() * sizeof(double),					
		GetRows(), GetCols(),						
		pElements, GetCols() * sizeof(double)));		

	SetElements(GetRows(), GetCols(), pElements, true);

}	// CMatrixNxM<TYPE>::Transpose

#endif


#ifdef NEVER // USE_IPP

//////////////////////////////////////////////////////////////////////
template<> INLINE
bool 
	CMatrixNxM<float>::Invert(bool bFlag)					
	// invert the matrix, with full pivot or not???
{															
	ASSERT(GetRows() == GetCols());							
	float *pElements = NULL;									
	AllocValues(GetRows() * GetCols(), pElements);			
	__declspec(thread) static float *arrBuffer = NULL;		
	__declspec(thread) static int nLength = 0;				
	if (nLength < GetRows() * GetCols())					
	{														
		FreeValues(arrBuffer);								
		nLength = GetRows() * GetCols();					
		/* TODO: fix this memory leak */					
		AllocValues(2 * nLength, arrBuffer);				
	}

	IppStatus stat = ippmInvert_m_32f(&(*this)[0][0],
		GetRows() * sizeof(float), sizeof(float),							
		arrBuffer,											
		pElements, 
		GetRows() * sizeof(float), sizeof(float),
		GetRows());

	if (stat == ippStsOk)									
	{														
		SetElements(GetCols(), GetRows(), pElements, TRUE);	
	}										

	return (stat == ippStsOk);								

}	// CMatrixNxM<TYPE>::Invert

//////////////////////////////////////////////////////////////////////
template<> INLINE
bool 
	CMatrixNxM<double>::Invert(bool bFlag)					
	// invert the matrix, with full pivot or not???
{															
	ASSERT(GetRows() == GetCols());							
	double *pElements = NULL;									
	AllocValues(GetRows() * GetCols(), pElements);			
	__declspec(thread) static double *arrBuffer = NULL;		
	__declspec(thread) static int nLength = 0;				
	if (nLength < GetRows() * GetCols())					
	{														
		FreeValues(arrBuffer);								
		nLength = GetRows() * GetCols();					
		/* TODO: fix this memory leak */					
		AllocValues(2 * nLength, arrBuffer);				
	}

	IppStatus stat = ippmInvert_m_64f(&(*this)[0][0],
		GetRows() * sizeof(double), sizeof(double),							
		arrBuffer,											
		pElements, 
		GetRows() * sizeof(double), sizeof(double),
		GetRows());
	if (stat == ippStsOk)									
	{														
		SetElements(GetCols(), GetRows(), pElements, TRUE);	
	}										

	return (stat == ippStsOk);								

}	// CMatrixNxM<TYPE>::Invert

#endif

//////////////////////////////////////////////////////////////////////
template<class TYPE>
void 
	CMatrixNxM<TYPE>::SetElements(int nCols, int nRows, 
		TYPE *pElements, bool bFreeElements)
	// sets the elements of the matrix
{
	// delete previous data
	if (m_bFreeElements && NULL != m_pElements)
	{
		FreeValues(m_pElements);
	}

	if (nCols != m_nColumns 
		&& NULL != m_pColumns)
	{
		delete [] m_pColumns;
		m_pColumns = NULL;
	}

	m_nColumns = nCols;
	// m_nRows = nRows;

	m_pElements = pElements;

	// set up the column vectors
	if (0 != m_nColumns)
	{
		if (m_pColumns == NULL)
		{
			// allocate column vectors
			m_pColumns = new COL_TYPE[GetCols()];
		}

		// initialize the column vectors and the pointers
		for (int nAt = 0; nAt < GetCols(); nAt++)
		{
			// initialize the column vector
			m_pColumns[nAt].SetElements(nRows, &m_pElements[nAt * GetRows()],
				FALSE);
		}
	}

	m_bFreeElements = bFreeElements;

}	// CMatrixNxM<TYPE>::SetElements


//////////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
bool 
	operator==(const CMatrixNxM<TYPE>& mLeft, 
		const CMatrixNxM<TYPE>& mRight)
	// exact matrix equality
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

}	// operator==(const CMatrixNxM<TYPE>&, const CMatrixNxM<TYPE>&)


//////////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
bool 
	operator!=(const CMatrixNxM<TYPE>& mLeft, 
		const CMatrixNxM<TYPE>& mRight)
	// exact matrix inequality
{
	return !(mLeft == mRight);

}	// operator!=(const CMatrixNxM<TYPE>&, const CMatrixNxM<TYPE>&)


//////////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
CMatrixNxM<TYPE> 
	operator+(const CMatrixNxM<TYPE>& mLeft, 
		const CMatrixNxM<TYPE>& mRight)
	// matrix addition
{
	// create the product
	CMatrixNxM<TYPE> mSum(mLeft);
	mSum += mRight;

	// return the product
	return mSum;

}	// operator+(const CMatrixNxM<TYPE>&, const CMatrixNxM<TYPE>&)


//////////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
CMatrixNxM<TYPE> 
	operator-(const CMatrixNxM<TYPE>& mLeft, 
		const CMatrixNxM<TYPE>& mRight)
	// matrix subtraction
{
	// create the difference
	CMatrixNxM<TYPE> mDiff(mLeft);
	mDiff -= mRight;

	// return the difference
	return mDiff;

}	// operator-(const CMatrixNxM<TYPE>&, const CMatrixNxM<TYPE>&)


//////////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
CMatrixNxM<TYPE> 
	operator*(const CMatrixNxM<TYPE>& mat, double scale)
	// matrix scalar multiplication
{
	// stored the product
	CMatrixNxM<TYPE> mProduct(mat);
	mProduct *= scale;

	// return product
	return mProduct;

}	// operator*(const CMatrixNxM<TYPE>&, double scale)


//////////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
CMatrixNxM<TYPE> 
	operator*(double scale, const CMatrixNxM<TYPE>& mat)
	// matrix scalar multiplication, contrariwise
{
	// stored the product
	CMatrixNxM<TYPE> mProduct(mat);
	mProduct *= scale;

	// return product
	return mProduct;

}	// operator*(double scale, const CMatrixNxM<TYPE>&)


//////////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
CVectorN<TYPE> 
	operator*(const CMatrixNxM<TYPE>& mat, const CVectorN<TYPE>& v)
	// matrix-vector multiplication
{
	// stored the product
	CVectorN<TYPE> vProduct(mat.GetRows());
	MultMatrixVector(&vProduct[0], 
		&mat[0][0], mat.GetCols(), mat.GetRows(), 
		&v[0]);

	// return the product
	return vProduct;

}	// operator*(const CMatrixNxM<TYPE>&, const CVectorN<TYPE>&)


#ifdef NEVER // USE_IPP

//////////////////////////////////////////////////////////////////////
template<> INLINE
CVectorN<float> 
	operator*(const CMatrixNxM<float>& mat,
			const CVectorN<float>& v)
	// matrix-vector multiplication for float override
{											
	// stores the product
	CVectorN<float> vProduct(mat.GetRows());			

	CK_IPP(ippmMul_mTv_32f(&mat[0][0], 
		mat.GetRows() * sizeof(float),
		mat.GetRows(), mat.GetCols(),
		&v[0], v.GetDim(),											
		&vProduct[0]));

	// return the product
	return vProduct;

}	// operator*(const CMatrixNxM<TYPE>&, const CVectorN<TYPE>&)

//////////////////////////////////////////////////////////////////////
template<> INLINE
CVectorN<double> 
	operator*(const CMatrixNxM<double>& mat,
			const CVectorN<double>& v)
	// matrix-vector multiplication for float override
{											
	// stores the product
	CVectorN<double> vProduct(mat.GetRows());			

	CK_IPP(ippmMul_mTv_64f(&mat[0][0], 
		mat.GetRows() * sizeof(double),
		mat.GetRows(), mat.GetCols(),
		&v[0], v.GetDim(),											
		&vProduct[0]));

	// return the product
	return vProduct;

}	// operator*(const CMatrixNxM<TYPE>&, const CVectorN<TYPE>&)

#endif


//////////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
CMatrixNxM<TYPE> operator*(const CMatrixNxM<TYPE>& mLeft, 
									const CMatrixNxM<TYPE>& mRight)
	// matrix multiplication
{
	// create the product
	CMatrixNxM<TYPE> mProduct(mRight.GetCols(), mLeft.GetRows());
	MultMatrixMatrix(mProduct, mLeft, mRight);

	// return the product
	return mProduct;

}	// operator*(const CMatrixNxM<TYPE>&, const CMatrixNxM<TYPE>&)


//////////////////////////////////////////////////////////////////////
template<class TYPE>
TYPE Determinant(const CMatrixNxM<TYPE>& mMat) 
	// computes the determinant of the matrix, for square matrices
	// TODO: move this to MatrixOps
	// TODO: implement IPP determinant
{
	ASSERT(mMat.GetCols() == mMat.GetRows());

	if (mMat.GetCols() > 2) 
	{
		TYPE det = 0.0;
		for (int nAtCol = 0; nAtCol < mMat.GetCols(); nAtCol++) 
		{
			CMatrixNxM<TYPE> mMinor(mMat.GetCols()-1, mMat.GetRows()-1);
			for (int nAtRow = 1; nAtRow < mMat.GetRows(); nAtRow++) 
			{
				int nAtMinorCol = 0;
				for (int nAtCol2 = 0; nAtCol2 < mMat.GetCols(); nAtCol2++) 
				{
				   if (nAtCol2 != nAtCol)
				   {
					   mMinor[nAtMinorCol][nAtRow-1] = mMat[nAtCol2][nAtRow];
					   nAtMinorCol++;
				   }
				}
			}
			det += ((nAtCol % 2 == 0) ? 1.0 : -1.0) 
				* mMat[nAtCol][0] * Determinant(mMinor);
		}

		return det;
	}
	else if (mMat.GetCols() > 1) 
	{
		return mMat[0][0] * mMat[1][1] - mMat[1][0] * mMat[0][1];
	}
	
	return mMat[0][0];

}	// CMatrixNxM<TYPE>::Determinant


// operator overloads for serialization
#ifdef __AFX_H__

//////////////////////////////////////////////////////////////////////
template<class TYPE>
CArchive& 
	operator<<(CArchive &ar, CMatrixNxM<TYPE> m)
	// matrix serialization in
{
	// serialize the dimension
	ar << m.GetCols();
	ar << m.GetRows();

	// serialize the individual elements
	ar.Write((TYPE *) m, m.GetCols() * m.GetRows() * sizeof(TYPE));

	// return the archive object
	return ar;

}	// operator<<(CArchive &ar, CMatrixNxM<TYPE> m)

//////////////////////////////////////////////////////////////////////
template<class TYPE>
CArchive& 
	operator>>(CArchive &ar, CMatrixNxM<TYPE>& m)
	// matrix serialization out
{
	// serialize the dimension
	int nCols, nRows;
	ar >> nCols >> nRows;
	m.Reshape(nCols, nRows);

	// serialize the individual elements
	ar.Read((TYPE *) m, nCols * nRows * sizeof(TYPE));

	// return the archive object
	return ar;

}	// operator>>(CArchive &ar, CMatrixNxM<TYPE>& m)

#endif	// __AFX_H__


#ifdef USE_XMLLOGGING

//////////////////////////////////////////////////////////////////////
template<typename TYPE>
void 
	LogExprExt(const CMatrixNxM<TYPE>& mMat, 
			const char *pszName, const char *pszModule)
	// helper function for XML logging of matrices
{
	// get the global log file
	CXMLLogFile *pLog = CXMLLogFile::GetLogFile();

	// create a new expression element
	CXMLElement *pVarElem = pLog->NewElement("lx", pszModule);

	// if there is a name,
	if (strlen(pszName) > 0)
	{
		// set it.
		pVarElem->Attribute("name", pszName);
	}

	// set type to generice "CVector"
	pVarElem->Attribute("type", "CMatrix");

	// stores each row
	CVectorN<TYPE> vRow(mMat.GetCols());
	for (int nAtRow = 0; nAtRow < mMat.GetRows(); nAtRow++)
	{
		// get each row
		mMat.GetRow(nAtRow, vRow);

		// and output as sub-element
		LogExprExt(vRow, "", pszModule);
	}

	// close the element
	pLog->CloseElement();

}	// LogExprExt

#endif	// USE_XMLLOGGING

#endif	// MATRIXNXM_H