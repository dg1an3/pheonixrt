//////////////////////////////////////////////////////////////////////
// MatrixD.h: declaration and definition of the CMatrixD template class.
//
// Copyright (C) 1999-2003 Derek G Lane
// $Id: MatrixD.h,v 1.6 2003/03/01 22:28:16 default Exp $
//////////////////////////////////////////////////////////////////////

#if !defined(MATRIXD_H)
#define MATRIXD_H

#include <MathUtil.h>

#include "MatrixBase.h"
#include "VectorD.h"

//////////////////////////////////////////////////////////////////////
// class CMatrixD<DIM, TYPE>
//
// represents a square matrix with dimension and type given.
//////////////////////////////////////////////////////////////////////
template<int DIM = 4, class TYPE = double>
class CMatrixD : public CMatrixBase<TYPE>
{
public:
	// constructors / destructore
	CMatrixD();
	CMatrixD(const CMatrixD& fromMatrix);
	explicit CMatrixD(const CMatrixBase<TYPE>& fromMatrix);
	~CMatrixD();

	// assignment operator
	CMatrixD& operator=(const CMatrixD& fromMatrix);

	// operator[] -- retrieves a reference to a Column vector
	CVectorD<DIM, TYPE>& operator[](int nAtCol);
	const CVectorD<DIM, TYPE>& operator[](int nAtCol) const;

	// IsApproxEqual -- tests for approximate equality using the EPS
	//		defined at the top of this file
	BOOL IsApproxEqual(const CMatrixD& m, TYPE epsilon = EPS) const;

	// in-place operators
	CMatrixD& operator+=(const CMatrixD& mRight);
	CMatrixD& operator-=(const CMatrixD& mRight);
	CMatrixD& operator*=(const TYPE& scale);
	CMatrixD& operator*=(const CMatrixD& mRight);

	// Transpose -- transposes elements of the matrix
	void Transpose();

#ifdef DIRECT3D_VERSION
	// conversion to / from D3DMATRIX
	CMatrixD(const D3DMATRIX& matr);
	operator D3DMATRIX() const;
#endif

private:
	// column vectors
	CVectorBase<TYPE> m_arrColumns[DIM];

	// elements
	TYPE m_arrElements[DIM * DIM];
};

//////////////////////////////////////////////////////////////////
// CMatrixD<DIM,TYPE>::CMatrixD<DIM,TYPE>
//
// default constructor -- initializes to an identity matrix
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CMatrixD<DIM,TYPE>::CMatrixD<DIM,TYPE>()
{
	// we have to assign the columns before calling 
	//		CMatrixBase::SetElements, because otherwise
	//		SetElements will allocate the column vectors
	//		(and ours are part of the CMatrixD)
	m_pColumns = &m_arrColumns[0];
	m_nCols = DIM;

	// allocate the elements
	SetElements(DIM, DIM, &m_arrElements[0], FALSE);

	// populate as an identity matrix
	SetIdentity();

}	// CMatrixD<DIM,TYPE>::CMatrixD<DIM,TYPE>


//////////////////////////////////////////////////////////////////
// CMatrixD<DIM,TYPE>::CMatrixD<DIM,TYPE>
// 
// copy constructor
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CMatrixD<DIM,TYPE>::CMatrixD<DIM,TYPE>(const CMatrixD& fromMatrix)
{
	// we have to assign the columns before calling 
	//		CMatrixBase::SetElements, because otherwise
	//		SetElements will allocate the column vectors
	//		(and ours are part of the CMatrixD)
	m_pColumns = &m_arrColumns[0];
	m_nCols = DIM;

	// allocate the elements
	SetElements(DIM, DIM, &m_arrElements[0], FALSE);

	// populate from other matrix
	for (int nAt = 0; nAt < DIM; nAt++)
	{
		(*this)[nAt] = fromMatrix[nAt];
	}

}	// CMatrixD<DIM,TYPE>::CMatrixD<DIM,TYPE>


//////////////////////////////////////////////////////////////////
// CMatrixD<DIM,TYPE>::CMatrixD<DIM,TYPE>
//
// copy constructor
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CMatrixD<DIM,TYPE>::CMatrixD<DIM,TYPE>(const CMatrixBase<TYPE>& fromMatrix)
{
	// we have to assign the columns before calling 
	//		CMatrixBase::SetElements, because otherwise
	//		SetElements will allocate the column vectors
	//		(and ours are part of the CMatrixD)
	m_pColumns = &m_arrColumns[0];
	m_nCols = DIM;

	// allocate the elements
	SetElements(DIM, DIM, &m_arrElements[0], FALSE);

	// set to identity (for partial fills)
	SetIdentity();

	// populate from other matrix
	for (int nAt = 0; nAt < __min(GetCols(), fromMatrix.GetCols()); nAt++)
	{
		(*this)[nAt] = CVectorD<DIM, TYPE>(fromMatrix[nAt]);
	}

}	// CMatrixD<DIM,TYPE>::CMatrixD<DIM,TYPE>


//////////////////////////////////////////////////////////////////
// CMatrixD<DIM,TYPE>::~CMatrixD<DIM,TYPE>
//
// destructor -- frees the row vectors
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CMatrixD<DIM,TYPE>::~CMatrixD<DIM,TYPE>()
{
	// ensure no monkey business has occurred
	ASSERT(m_pElements == &m_arrElements[0]);
	ASSERT(m_pColumns == &m_arrColumns[0]);

	// don't delete anything, as it was all statically allocated
	m_pElements = NULL;
	m_pColumns = NULL;

}	// CMatrixD<DIM,TYPE>::~CMatrixD<DIM,TYPE>


//////////////////////////////////////////////////////////////////
// CMatrixD<DIM,TYPE>::operator=
//
// assignment operator
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CMatrixD<DIM,TYPE>& CMatrixD<DIM,TYPE>::operator=(const CMatrixD<DIM,TYPE>& fromMatrix)
{
	for (int nAt = 0; nAt < DIM; nAt++)
	{
		(*this)[nAt] = fromMatrix[nAt];
	}

	return (*this);

}	// CMatrixD<DIM,TYPE>::operator=


//////////////////////////////////////////////////////////////////
// CMatrixD<DIM,TYPE>::operator[]
//
// retrieves a reference to a Column vector
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CVectorD<DIM, TYPE>& CMatrixD<DIM,TYPE>::operator[](int nAtCol)
{
	// return a reference to the row vector
	// NOTE: this is a really terrible thing to do, to down-cast
	//		the column vectors
	return static_cast<CVectorD<DIM, TYPE>&>(m_arrColumns[nAtCol]);

}	// CMatrixD<DIM,TYPE>::operator[]


//////////////////////////////////////////////////////////////////
// CMatrixD<DIM,TYPE>::operator[] const
//
// retrieves a const reference to a Column vector
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
const CVectorD<DIM, TYPE>& CMatrixD<DIM,TYPE>::operator[](int nAtCol) const
{
	// return a reference to the row vector
	// NOTE: this is a really terrible thing to do, to down-cast
	//		the column vectors
	return static_cast<const CVectorD<DIM, TYPE>&>(m_arrColumns[nAtCol]);

}	// CMatrixD<DIM,TYPE>::operator[] const


//////////////////////////////////////////////////////////////////
// CMatrixD<DIM,TYPE>::IsApproxEqual
//
// tests for approximate equality using the EPS defined at the 
//		top of this file
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
BOOL CMatrixD<DIM,TYPE>::IsApproxEqual(const CMatrixD<DIM,TYPE>& m, TYPE epsilon) const
{
	return CMatrixBase<TYPE>::IsApproxEqual(
		(const CMatrixBase<TYPE>&) m, epsilon);

}	// CMatrixD<DIM,TYPE>::IsApproxEqual


//////////////////////////////////////////////////////////////////
// CMatrixD<DIM,TYPE>::operator+=
//
// in-place matrix addition; returns a reference to this
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CMatrixD<DIM,TYPE>& CMatrixD<DIM,TYPE>::operator+=(const CMatrixD<DIM,TYPE>& mRight)
{
	// element-by-element sum of the matrix
	for (int nCol = 0; nCol < DIM; nCol++)
	{
		for (int nRow = 0; nRow < DIM; nRow++)
		{
			(*this)[nCol][nRow] += mRight[nCol][nRow];
		}
	}

	// return a reference to this
	return (*this);

}	// CMatrixD<DIM,TYPE>::operator+=


//////////////////////////////////////////////////////////////////
// CMatrixD<DIM,TYPE>::operator-= 
//
// in-place matrix subtraction; returns a reference to this
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CMatrixD<DIM,TYPE>& CMatrixD<DIM,TYPE>::operator-=(const CMatrixD& mRight)
{
	// element-by-element difference of the matrix
	for (int nCol = 0; nCol < DIM; nCol++)
	{
		for (int nRow = 0; nRow < DIM; nRow++)
		{
			(*this)[nCol][nRow] -= mRight[nCol][nRow];
		}
	}

	// return a reference to this
	return (*this);

}	// CMatrixD<DIM,TYPE>::operator-= 


//////////////////////////////////////////////////////////////////
// CMatrixD<DIM,TYPE>::operator*= 
//
// in-place scalar multiplication
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CMatrixD<DIM,TYPE>& CMatrixD<DIM,TYPE>::operator*=(const TYPE& scale)
{
	// element-by-element scalar multiply of the matrix
	for (int nCol = 0; nCol < DIM; nCol++)
	{
		for (int nRow = 0; nRow < DIM; nRow++)
		{
			(*this)[nCol][nRow] *= scale;
		}
	}

	// return a reference to this
	return (*this);

}	// CMatrixD<DIM,TYPE>::operator*= 

	
//////////////////////////////////////////////////////////////////
// CMatrixD<DIM,TYPE>::operator*=
//
// in-place matrix multiplication; returns a reference to this
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CMatrixD<DIM,TYPE>& CMatrixD<DIM,TYPE>::operator*=(const CMatrixD<DIM,TYPE>& mRight)
{
	// assign final product to this
	(*this) = (*this) * mRight;

	// return a reference to this
	return (*this);

}	// CMatrixD<DIM,TYPE>::operator*=


//////////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::Transpose
//
// transposes the matrix
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
void CMatrixD<DIM,TYPE>::Transpose()
{
	for (int nCol = 0; nCol < DIM; nCol++)
	{
		for (int nRow = nCol+1; nRow < DIM; nRow++)
		{
			TYPE temp = (*this)[nCol][nRow];
			(*this)[nCol][nRow] = (*this)[nRow][nCol];
			(*this)[nRow][nCol] = temp;
		}
	}

}	// CMatrixBase<TYPE>::Transpose


//////////////////////////////////////////////////////////////////////
// operator==(const CMatrixD<DIM, TYPE>&, const CMatrixD<DIM, TYPE>&)
//
// exact matrix equality
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline bool operator==(const CMatrixD<DIM, TYPE>& mLeft, 
					   const CMatrixD<DIM, TYPE>& mRight)
{
	// call CMatrixBase version
	return operator==((const CMatrixBase<TYPE>&) mLeft,
		(const CMatrixBase<TYPE>&) mRight);

}	// operator==(const CMatrixD<DIM, TYPE>&, const CMatrixD<DIM, TYPE>&)


//////////////////////////////////////////////////////////////////////
// operator==(const CMatrixD<DIM, TYPE>&, const CMatrixD<DIM, TYPE>&)
//
// exact matrix inequality
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline bool operator!=(const CMatrixD<DIM, TYPE>& mLeft, 
					   const CMatrixD<DIM, TYPE>& mRight)
{
	// call CMatrixBase version
	return operator!=((const CMatrixBase<TYPE>&) mLeft,
		(const CMatrixBase<TYPE>&) mRight);

}	// operator==(const CMatrixD<DIM, TYPE>&, const CMatrixD<DIM, TYPE>&)


//////////////////////////////////////////////////////////////////////
// operator*(const CMatrixD<DIM, TYPE>&, const CVectorD<DIM, TYPE>&)
//
// matrix-vector multiplication
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline CVectorD<DIM, TYPE> operator*(const CMatrixD<DIM, TYPE>& mat,
									const CVectorD<DIM, TYPE>& v)
{
	// stored the product
	CVectorD<DIM, TYPE> vProduct;

	// step through the rows of the product
	for (int nRow = 0; nRow < DIM; nRow++)
	{
		ASSERT(vProduct[nRow] == 0.0);

		// step through the columns of the matrix
		for (int nCol = 0; nCol < DIM; nCol++)
		{
			vProduct[nRow] += mat[nCol][nRow] * v[nCol];
		}
	}

	// return the product
	return vProduct;

}	// operator*(const CMatrixD<DIM, TYPE>&, const CVectorD<DIM, TYPE>&)


//////////////////////////////////////////////////////////////////////
// operator*(const CMatrixD<DIM, TYPE>&, const CMatrixD<DIM, TYPE>&)
//
// matrix multiplication
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline CMatrixD<DIM, TYPE> operator*(const CMatrixD<DIM, TYPE>& mLeft, 
									const CMatrixD<DIM, TYPE>& mRight)
{
	// holds the final product
	CMatrixD<DIM, TYPE> mProduct;

	// compute the matrix product
	for (int nCol = 0; nCol < DIM; nCol++)
	{
		for (int nRow = 0; nRow < DIM; nRow++)
		{
			mProduct[nCol][nRow] = 0.0;
			for (int nMid = 0; nMid < DIM; nMid++)
			{
				mProduct[nCol][nRow] +=
					mLeft[nMid][nRow] * mRight[nCol][nMid];
			}
		}
	}

	// return the product
	return mProduct;

}	// operator*(const CMatrixD<DIM, TYPE>&, const CMatrixD<DIM, TYPE>&)


//////////////////////////////////////////////////////////////////////
// operator*(const CMatrixD<DIM, TYPE>&, const CMatrixD<DIM, TYPE>&)
//
// matrix multiplication
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline CMatrixD<DIM, TYPE> operator*(const CMatrixD<DIM, TYPE>& mLeft, 
									const TYPE& scalar)
{
	// create the product
	CMatrixD<DIM, TYPE> mProduct(mLeft);

	// use in-place multiplication
	mProduct *= scalar;

	// return the product
	return mProduct;

}	// operator*(const CMatrixD<DIM, TYPE>&, const CMatrixD<DIM, TYPE>&)

	
//////////////////////////////////////////////////////////////////////
// operator*(const CMatrixD<DIM, TYPE>&, const CMatrixD<DIM, TYPE>&)
//
// matrix multiplication
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline CMatrixD<DIM, TYPE> operator*(const TYPE& scalar, 
									const CMatrixD<DIM, TYPE>& mRight)
{
	// create the product
	CMatrixD<DIM, TYPE> mProduct(mRight);

	// use in-place multiplication
	mProduct *= scalar;

	// return the product
	return mProduct;

}	// operator*(const CMatrixD<DIM, TYPE>&, const CMatrixD<DIM, TYPE>&)


#ifdef __AFX_H__

//////////////////////////////////////////////////////////////////////
// operator<<
//
// matrix serialization
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CArchive& operator<<(CArchive &ar, CMatrixD<DIM, TYPE> m)
{
	// serialize the individual row vectors
	for (int nAt = 0; nAt < DIM; nAt++)
	{
		ar << m[nAt];
	}

	// return the archive object
	return ar;

}	// operator<<


//////////////////////////////////////////////////////////////////////
// operator>>
//
// matrix serialization
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CArchive& operator>>(CArchive &ar, CMatrixD<DIM, TYPE>& m)
{
	// serialize the individual row vectors
	for (int nAt = 0; nAt < DIM; nAt++)
	{
		ar >> m[nAt];
	}

	// return the archive object
	return ar;

}	// operator>>


#endif 

//////////////////////////////////////////////////////////////////////
// CreateTranslate
//
// creates a 3x3 homogeneous translation matrix from a vector and 
//		a scalar for a 2D translation
//////////////////////////////////////////////////////////////////////
inline CMatrixD<3> CreateTranslate(const double& s, 
								  const CVectorD<2>& vAxis)
{
	// start with an identity matrix
	CMatrixD<3> mTranslate;

	mTranslate[2][0] = s * vAxis[0];
	mTranslate[2][1] = s * vAxis[1];

	return mTranslate;

}	// CreateTranslate


//////////////////////////////////////////////////////////////////////
// CreateTranslate
//
// creates a 4x4 homogeneous translation matrix from a vector and 
//		a scalar for a 3D translation
//////////////////////////////////////////////////////////////////////
inline CMatrixD<4> CreateTranslate(const double& s, 
								  const CVectorD<3>& vAxis)
{
	// start with an identity matrix
	CMatrixD<4> mTranslate;

	mTranslate[3][0] = s * vAxis[0];
	mTranslate[3][1] = s * vAxis[1];
	mTranslate[3][2] = s * vAxis[2];

	return mTranslate;

}	// CreateTranslate


//////////////////////////////////////////////////////////////////////
// CreateTranslate
//
// creates a translation matrix from an offset vector
//////////////////////////////////////////////////////////////////////
inline CMatrixD<4> CreateTranslate(const CVectorD<3>& vAxis)
{
	return CreateTranslate(1.0, vAxis);

}	// CreateTranslate


//////////////////////////////////////////////////////////////////////
// CreateRotate
//
// creates a 2D rotation matrix given an angle
//////////////////////////////////////////////////////////////////////
inline CMatrixD<2> CreateRotate(const double& theta)
{
	// start with an identity matrix
	CMatrixD<2> mRotate;

	// now compute the rotation matrix

	// angle helper values
	double st = sin(theta);
	double ct = cos(theta);

	mRotate[0][0] = ct;
	mRotate[1][0] = - st;

	mRotate[0][1] = st;
	mRotate[1][1] = ct;

	return mRotate;

}	// CreateRotate


//////////////////////////////////////////////////////////////////////
// CreateRotate
//
// creates a 3D rotation matrix given an angle and an axis of rotation
//////////////////////////////////////////////////////////////////////
inline CMatrixD<3> CreateRotate(const double& theta, 
							   const CVectorD<3>& vAxis)
{
	// start with an identity matrix
	CMatrixD<3> mRotate;

	// normalize the axis
	CVectorD<3> vNormAxis = vAxis;
	vNormAxis.Normalize();

	// now compute the rotation matrix

	// angle helper values
	double st = sin(theta);
	double ct = cos(theta);
	double vt = 1.0 - cos(theta);

	mRotate[0][0] = vNormAxis[0] * vNormAxis[0] * vt + ct;
	mRotate[1][0] = vNormAxis[0] * vNormAxis[1] * vt - vNormAxis[2] * st;
	mRotate[2][0] = vNormAxis[0] * vNormAxis[2] * vt + vNormAxis[1] * st;

	mRotate[0][1] = vNormAxis[0] * vNormAxis[1] * vt + vNormAxis[2] * st;
	mRotate[1][1] = vNormAxis[1] * vNormAxis[1] * vt + ct;
	mRotate[2][1] = vNormAxis[1] * vNormAxis[2] * vt - vNormAxis[0] * st;

	mRotate[0][2] = vNormAxis[0] * vNormAxis[2] * vt - vNormAxis[1] * st;
	mRotate[1][2] = vNormAxis[1] * vNormAxis[2] * vt + vNormAxis[0] * st;
	mRotate[2][2] = vNormAxis[2] * vNormAxis[2] * vt + ct;

	return mRotate;

}	// CreateRotate


//////////////////////////////////////////////////////////////////////
// CreateRotate
//
// creates a 3D rotation matrix given two positions: a pre-rotated
//		position and a post-rotated position. scale is a scale factor
//		for the resulting rotation.
//////////////////////////////////////////////////////////////////////
inline CMatrixD<3> CreateRotate(const CVectorD<3>& vInitPt,
	const CVectorD<3>& vFinalPt, double scale)
{
	// compute the length of the two legs of the triangle (a & b)
	double a = vInitPt.GetLength(); 
	double b = vFinalPt.GetLength();
	
	// compute the length of the segment connecting the initial and final
	//		point (c)
	double c = (vFinalPt - vInitPt).GetLength();

	// compute the angle theta using the law of cosines
	double theta = 0.0;
	if (a * b != 0.0)
	{
		theta = (2.0 * PI - acos((a * a + b * b - c * c) 
			/ (2.0 * a * b)));
	}

	// compute the axis of rotation = normalized cross product of 
	//		initial and current drag points
	CVectorD<3> u = Cross(vFinalPt, vInitPt);
	u.Normalize();

	// now compute the rotation matrix
	return CreateRotate(scale * theta, u);

}	// CreateRotate


//////////////////////////////////////////////////////////////////////
// CreateScale
//
// creates a scaling matrix from a vector whose element's lengths 
//		are scale factors
//////////////////////////////////////////////////////////////////////
inline CMatrixD<2> CreateScale(const CVectorD<2>& vScale)
{
	// start with an identity matrix
	CMatrixD<2> mScale;

	mScale[0][0] = vScale[0];
	mScale[1][1] = vScale[1];

	return mScale;

}	// CreateScale


//////////////////////////////////////////////////////////////////////
// CreateScale
//
// creates a scaling matrix from a vector whose element's lengths 
//		are scale factors
//////////////////////////////////////////////////////////////////////
inline CMatrixD<3> CreateScale(const CVectorD<3>& vScale)
{
	// start with an identity matrix
	CMatrixD<3> mScale;

	mScale[0][0] = vScale[0];
	mScale[1][1] = vScale[1];
	mScale[2][2] = vScale[2];

	return mScale;

}	// CreateScale


//////////////////////////////////////////////////////////////////////
// CreateProjection
//
// creates a projection matrix from a near-plane and a far-plane
//////////////////////////////////////////////////////////////////////
inline CMatrixD<4> CreateProjection(const double& n, const double& f)
{
	CMatrixD<4> mProj;

	// set the near plane elements
	mProj[0][0] = n;
	mProj[1][1] = n;

	// set the ratio elements
	mProj[2][2] = -(f + n)	  / (f - n);
	mProj[3][2] = -2.0 * f * n / (f - n);

	// set the projection elements
	mProj[2][3] = -1.0;
	mProj[3][3] =  0.0;

	// return the created matrix
	return mProj;

}	// CreateProjection


#ifdef DIRECT3D_VERSION

//////////////////////////////////////////////////////////////////////
// CMatrixD<DIM, TYPE>::CMatrixD(const D3DMATRIX& matr)
//
// constructs from a D3DMATRIX
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CMatrixD<DIM, TYPE>::CMatrixD(const D3DMATRIX& matr)
{
	// we have to assign the columns before calling 
	//		CMatrixBase::SetElements, because otherwise
	//		SetElements will allocate the column vectors
	//		(and ours are part of the CMatrixD)
	m_pColumns = &m_arrColumns[0];
	m_nCols = DIM;

	// allocate the elements
	SetElements(DIM, DIM, &m_arrElements[0], FALSE);

	// assign matrix elements
	(*this)[0][0] = matr._11;

	if (DIM > 1)
	{
		(*this)[0][1] = matr._12;

		(*this)[1][0] = matr._21;
		(*this)[1][1] = matr._22;
	}

	if (DIM > 2)
	{
		(*this)[0][2] = matr._13;
		(*this)[1][2] = matr._23;

		(*this)[2][0] = matr._31;
		(*this)[2][1] = matr._32;
		(*this)[2][2] = matr._33;
	}

	if (DIM > 3)
	{
		(*this)[0][3] = matr._14;
		(*this)[1][3] = matr._24;
		(*this)[2][3] = matr._34;

		(*this)[3][0] = matr._41;
		(*this)[3][1] = matr._42;
		(*this)[3][2] = matr._43;
		(*this)[3][3] = matr._44;
	}

}	// CMatrixD<DIM, TYPE>::CMatrixD(const D3DMATRIX& matr)


//////////////////////////////////////////////////////////////////////
// CMatrixD<DIM, TYPE>::operator D3DMATRIX()
//
// converts to a D3DMATRIX
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CMatrixD<DIM, TYPE>::operator D3DMATRIX() const
{
	D3DMATRIX matr;

	matr._11 = (*this)[0][0];

	if (DIM > 1)
	{
		matr._12 = (*this)[0][1];

		matr._21 = (*this)[1][0];
		matr._22 = (*this)[1][1];
	}

	if (DIM > 2)
	{
		matr._13 = (*this)[0][2];
		matr._23 = (*this)[1][2];

		matr._31 = (*this)[2][0];
		matr._32 = (*this)[2][1];
		matr._33 = (*this)[2][2];
	}

	if (DIM > 3)
	{
		matr._14 = (*this)[0][3];
		matr._24 = (*this)[1][3];
		matr._34 = (*this)[2][3];

		matr._41 = (*this)[3][0];
		matr._42 = (*this)[3][1];
		matr._43 = (*this)[3][2];
		matr._44 = (*this)[3][3];
	}

	return matr;

}	// CMatrixD<DIM, TYPE>::operator D3DMATRIX()

#endif


//////////////////////////////////////////////////////////////////////
// Eigenvector
//
// computes the eigenvector of a 2x2 matrix, or returns <0.0, 0.0> if
//		no real-valued eigenvectors exist
// order is either 1 or 2, depending on whether the principle or
//		remaining eigenvector is wanted
//////////////////////////////////////////////////////////////////////
inline REAL Eigen(CMatrixD<2> m, int nOrder = 1, 
				  CVectorD<2> *pVector = NULL)
{
	// compute factors of the characteristic quadratic equation
	REAL b = -(m[0][0] + m[1][1]);
	REAL c = m[0][0] * m[1][1] - m[0][1] * m[1][0];

	// check to see if characteristic equation has real roots
	if (b * b < 4.0 * c)
	{
		// if not, return 0.0 to indicate no eigenvalue computed
		return 0.0;
	}

	// and find the roots of the equation
	REAL r1 = -(b + sqrt(b * b - 4.0 * c)) / 2.0;
	REAL r2 = -(b - sqrt(b * b - 4.0 * c)) / 2.0;

	// choose root which is either largest or smallest, depending on order
	REAL r = ((fabs(r1) > fabs(r2)) && (nOrder <= 1)) 
		? r1 : r2;

	// find the larger off-diagonal element
	if (fabs(m[1][0]) > fabs(m[0][1]))
	{
		// and solve for the vector 
		(*pVector)[0] = 1.0;
		(*pVector)[1] = (r - m[0][0]) / m[1][0];
	}
	// otherwise use the other element if it is non-zero
	else if (fabs(m[0][1]) > 0.0)
	{
		// and solve for the vector 
		(*pVector)[0] = (r - m[1][1]) / m[0][1];
		(*pVector)[1] = 1.0;
	}
	// otherwise,
	else
	{
		// if not, return 0.0 to indicate no eigenvalue computed
		return 0.0;
	}

	// normalize the computed eigenvector
	pVector->Normalize();

#ifdef _DEBUG
	// assert that our eigenvector is really an eigenvector
	CVectorD<2> vLeft = m * (*pVector);
	CVectorD<2> vRight = r * (*pVector);

	ASSERT(vLeft.IsApproxEqual(vRight));
#endif

	// return the computed eigenvalue
	return r;

}	// Eigenvector

#endif