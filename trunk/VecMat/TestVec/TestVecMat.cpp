// TestVecMat.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


#include <stdlib.h>
#include <assert.h>

#include <iostream>
using namespace std;


#include <VectorN.h>
#include <VectorD.h>
#include <MatrixNxM.h>
#include <MatrixD.h>

// include for MatrixBase implementation
#include <MatrixBase.inl>

//////////////////////////////////////////////////////////////////////
// template<class VECTOR_CLASS>
// void InitVector(VECTOR_CLASS& m, int nDim)
//
// initializes the vector with the given dimension
//////////////////////////////////////////////////////////////////////
template<class VECTOR_CLASS>
void InitVector(VECTOR_CLASS& m, int nDim)
{
}

template<>
void InitVector< CVectorN<> >(CVectorN<>& v, int nDim)
{
	v.SetDim(nDim);
}

template<class TYPE>
TYPE GenRand(TYPE range)
{
	return range - 2.0 * range * (REAL) rand() 
			/ (REAL) RAND_MAX;
}

#ifdef USE_COMPLEX
template<>
complex<double> GenRand(complex<double> range)
{
	return complex<double>(
		range.real() - 2.0 * range.real() * (REAL) rand() 
			/ (REAL) RAND_MAX,
		range.imag() - 2.0 * range.imag() * (REAL) rand() 
			/ (REAL) RAND_MAX );
}
#endif

//////////////////////////////////////////////////////////////////////
// template<class VECTOR_CLASS, int DIM>
// TestVectorClass(REAL scale)
//
// tests the specified vector class, creating random vectors with
//		elements scaled by scale
//////////////////////////////////////////////////////////////////////
template<class VECTOR_CLASS, class ELEM_TYPE>
void TestVectorClass(int nDim, ELEM_TYPE range)
{
	// construction
	VECTOR_CLASS v1;
	InitVector<VECTOR_CLASS>(v1, nDim);

	// element accessors
	for (int nAt = 0; nAt < v1.GetDim(); nAt++)
	{
		v1[nAt] = GenRand<ELEM_TYPE>(range);
	}
	TRACE_VECTOR("v1 = ", v1);

	// copy construction
	//	-> ensure dimensions are replicated
	VECTOR_CLASS v2(v1);
	TRACE("VECTOR_CLASS v2(v1);\n");
	TRACE_VECTOR("v2 = ", v2);

	// assignment
	//	-> ensure dimensions are replicated
	VECTOR_CLASS v3 = v2;
	TRACE("VECTOR_CLASS v3 = v2;\n");
	TRACE_VECTOR("v3 = ", v3);

	// length
	TRACE("v3 length = %lf\n", v3.GetLength());

	// normalization
	v3.Normalize();
	TRACE_VECTOR("v3 Normalized = ", v3);

	// comparison (== , !=, IsApproxEqual)
	TRACE("ASSERT(v2 == v1);\n");
	ASSERT(v2 == v1);

	TRACE("ASSERT(v3 != v1);\n");
	ASSERT(v3 != v1);

	TRACE("ASSERT(v1.IsApproxEqual(v2));\n");
	ASSERT(v1.IsApproxEqual(v2));

	TRACE("ASSERT(!v1.IsApproxEqual(v3));\n");
	ASSERT(!v1.IsApproxEqual(v3));

	// in-place arithmetic
	v1 += v2;
	TRACE("v1 += v2;\n");
	TRACE_VECTOR("v1 = ", v1);

	v1 -= v2;
	TRACE("v1 -= v2;\n");
	TRACE_VECTOR("v1 = ", v1);

	v1 *= 2.0;
	TRACE("v1 *= 2.0;\n");
	TRACE_VECTOR("v1 = ", v1);

	// dyadic arithmetic
	TRACE_VECTOR("v1 + v2 = ", v1 + v2);

	TRACE_VECTOR("v1 - v3 = ", v1 - v3);

	TRACE("v1 * v3 = %lf\n", v1 * v3);
}


//////////////////////////////////////////////////////////////////////
// template<class MATRIX_CLASS>
// InitMatrix(REAL scale)
//
// initializes the matrix with the given dimension
//////////////////////////////////////////////////////////////////////
template<class MATRIX_CLASS>
void InitMatrix(MATRIX_CLASS& m, int nDim)
{
}

template<>
void InitMatrix< CMatrixNxM<> >(CMatrixNxM<>& m, int nDim)
{
	m.Reshape(nDim, nDim);
}

//////////////////////////////////////////////////////////////////////
// template<class VECTOR_CLASS, int DIM>
// TestMatrixClass(REAL scale)
//
// tests the specified matrix class, creating random matrices with
//		elements scaled by scale
//////////////////////////////////////////////////////////////////////
template<class MATRIX_CLASS>
void TestMatrixClass(int nDim, REAL scale)
{
	// construction
	MATRIX_CLASS m1;
	InitMatrix<MATRIX_CLASS>(m1, nDim);

	// element accessors
	for (int nAtCol = 0; nAtCol < m1.GetCols(); nAtCol++)
	{
		for (int nAtRow = 0; nAtRow < m1.GetRows(); nAtRow++)
		{
			m1[nAtCol][nAtRow] = scale - 2.0 * scale * (REAL) rand() 
				/ (REAL) RAND_MAX;
		}
	}
	TRACE_MATRIX("m1", m1);

	// copy construction
	//	-> ensure dimensions are replicated
	MATRIX_CLASS m2(m1);
	TRACE("MATRIX_CLASS m2(m1);\n");
	TRACE_MATRIX("m2", m2);

	// assignment
	//	-> ensure dimensions are replicated
	MATRIX_CLASS m3 = m2;
	TRACE("MATRIX_CLASS m3 = m2;\n");
	TRACE_MATRIX("m3", m3);

	// determinant
	// TRACE("m3 determinant = %lf\n", m3.GetDeterminant());

	// orthogonalization
	m3.Orthogonalize();
	TRACE_MATRIX("m3 Orthogonalize = ", m3);
	ASSERT(m3.IsOrthogonal());

	// comparison (== , !=, IsApproxEqual)
	TRACE("ASSERT(m2 == m1);\n");
	ASSERT(m2 == m1);

	TRACE("ASSERT(m3 != m1);\n");
	ASSERT(m3 != m1);

	TRACE("ASSERT(m1.IsApproxEqual(m2));\n");
	ASSERT(m1.IsApproxEqual(m2));

	TRACE("ASSERT(!m1.IsApproxEqual(m3));\n");
	ASSERT(!m1.IsApproxEqual(m3));

	// in-place arithmetic
	m1 += m2;
	TRACE("m1 += m2;\n");
	TRACE_MATRIX("m1", m1);

	m1 -= m2;
	TRACE("m1 -= m2;\n");
	TRACE_MATRIX("m1", m1);

	m1 *= 2.0;
	TRACE("m1 *= 2.0;\n");
	TRACE_MATRIX("m1", m1);

	// dyadic arithmetic
	TRACE_MATRIX("m1", m1);
	TRACE_MATRIX("m2", m2);
	TRACE_MATRIX("m3", m3);
	TRACE_MATRIX("m1 + m2 = ", m1 + m2);
	TRACE_MATRIX("m1 - m3 = ", m1 - m3);
	TRACE_MATRIX("m1 * m3 = ", m1 * m3);

	// transpose
	m1.Transpose();
	TRACE_MATRIX("m1.Transpose()", m1);
	m1.Transpose();

	// inverse
	m2 = m1;
	m1.Invert();
	TRACE_MATRIX("m1.Invert()", m1);

	// create an identity matrix for comparison
	MATRIX_CLASS mI;
	InitMatrix<MATRIX_CLASS>(mI, nDim);
	mI.SetIdentity();

	// assert approximate equality
	TRACE("ASSERT(mI.IsApproxEqual(m1 * m2));\n");
	ASSERT(mI.IsApproxEqual(m1 * m2));
}


//////////////////////////////////////////////////////////////////////
// template<class TYPE>
// TestSVD(REAL scale)
//
// tests the singular-valued decomposition
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void TestSVD(int nCols, int nRows, BOOL bHomogeneous)
{
	// generate a matrix for SVD test
	CMatrixNxM<TYPE> m(nCols, nRows);
	for (int nAtRow = 0; nAtRow < nRows; nAtRow++)
	{
		for (int nAtCol = 0; nAtCol < nCols; nAtCol++)
		{
			m[nAtCol][nAtRow] = 
				10.0 - 20.0 * (double) rand() / (double) RAND_MAX;

			if (bHomogeneous && nCols > nRows)
				m[nAtCol][nRows-1] = 1.0;
		}
		if (bHomogeneous && nCols <= nRows)
			m[nCols-1][nAtRow] = 1.0;
	}

	// store the original
	CMatrixNxM<TYPE> mOrig = m;

	// trace out the original
	TRACE_MATRIX("M_orig", m);

	// create the singular value vector
	CVectorN<TYPE> w(nCols);

	// create the orthogonal matrix
	CMatrixNxM<TYPE> v(nCols,nCols);

	// perform SVD
	m.SVD(w, v);

	// output the U matrix (stored in m)
	TRACE_MATRIX("U", m);

	// check orthogonality of U
	m.Transpose();
	CMatrixNxM<TYPE> uIdent = m;
	m.Transpose();
	uIdent *= m;
	TRACE_MATRIX("U * U^T", uIdent);

	CMatrixNxM<TYPE> mI(nCols,nCols);
	mI.SetIdentity();

	if (nCols < nRows)
	{
		TRACE("ASSERT(mI.IsApproxEqual(U * U^T));\n");
		ASSERT(mI.IsApproxEqual(uIdent));
	}
	else
	{
		uIdent.Reshape(nRows, nRows);
		mI.Reshape(nRows, nRows);
		TRACE("ASSERT(mI.IsApproxEqual(U * U^T));\n");
		ASSERT(mI.IsApproxEqual(uIdent));
	}

	// form the S matrix
	CMatrixNxM<TYPE> s(w.GetDim(), w.GetDim());
	for (int nAt = 0; nAt < w.GetDim(); nAt++)
	{
		s[nAt][nAt] = w[nAt];
	}
	TRACE_MATRIX("S", s);

	// output V matrix
	TRACE("\n");
	TRACE_MATRIX("V", v);

	// check orthogonality of V
	CMatrixNxM<TYPE> vIdent = v;
	v.Transpose();
	vIdent *= v;
	TRACE_MATRIX("V * V^T", vIdent);

	mI.Reshape(nCols,nCols);
	mI.SetIdentity();
	TRACE("ASSERT(mI.IsApproxEqual(V * V^T));\n");
	ASSERT(mI.IsApproxEqual(vIdent));

	// reform M
	CMatrixNxM<TYPE> mNew = m * s * v;
	TRACE_MATRIX("M_new", mNew);

	// assert equality with original
	TRACE("ASSERT(mNew.IsApproxEqual(mOrig));\n");
	ASSERT(mNew.IsApproxEqual(mOrig));

	if (nRows >= nCols)
	{
		// test pseudo-inverse
		CMatrixNxM<> mPsinv = mOrig;
		mPsinv.Pseudoinvert();
		ASSERT(mOrig.IsApproxEqual(mOrig * mPsinv * mOrig));
		ASSERT(mPsinv.IsApproxEqual(mPsinv * mOrig * mPsinv));
	}
}


//////////////////////////////////////////////////////////////////////
// main
//
// runs tests on
//		static vectors of dimension 1..9, 
//			with elements scaled to 0.01
//		static vectors of dimension 1..9, 
//			with elements scaled to 100.0
//		dynamic vectors of dimension 1..9, 
//			with elements scaled to 0.01
//		dynamic vectors of dimension 1..9, 
//			with elements scaled to 100.0
//
//		static matrices of dimension 1..9, 
//			with elements scaled to 0.01
//		static matrices of dimension 1..9, 
//			with elements scaled to 100.0
//		dynamic matrices of dimension 1..9, 
//			with elements scaled to 0.01
//		dynamic matrices of dimension 1..9, 
//			with elements scaled to 100.0
//////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
	//	static vectors of dimension 1..9, 
	//		with elements scaled to 0.01
	TestVectorClass< CVectorD<1> >(1, 0.01);
	TestVectorClass< CVectorD<2> >(2, 0.01);
	TestVectorClass< CVectorD<3> >(3, 0.01);
	TestVectorClass< CVectorD<4> >(4, 0.01);
	TestVectorClass< CVectorD<5> >(5, 0.01);
	TestVectorClass< CVectorD<6> >(6, 0.01);
	TestVectorClass< CVectorD<7> >(7, 0.01);
	TestVectorClass< CVectorD<8> >(8, 0.01);
	TestVectorClass< CVectorD<9> >(9, 0.01);

	//	static vectors of dimension 1..9, 
	//		with elements scaled to 100.0
	TestVectorClass< CVectorD<1> >(1, 100.0);
	TestVectorClass< CVectorD<2> >(2, 100.0);
	TestVectorClass< CVectorD<3> >(3, 100.0);
	TestVectorClass< CVectorD<4> >(4, 100.0);
	TestVectorClass< CVectorD<5> >(5, 100.0);
	TestVectorClass< CVectorD<6> >(6, 100.0);
	TestVectorClass< CVectorD<7> >(7, 100.0);
	TestVectorClass< CVectorD<8> >(8, 100.0);
	TestVectorClass< CVectorD<9> >(9, 100.0);

	//	dynamic vectors of dimension 1..9, 
	//		with elements scaled to 0.01
	TestVectorClass< CVectorN<> >(1, 0.01);
	TestVectorClass< CVectorN<> >(2, 0.01);
	TestVectorClass< CVectorN<> >(3, 0.01);
	TestVectorClass< CVectorN<> >(4, 0.01);
	TestVectorClass< CVectorN<> >(5, 0.01);
	TestVectorClass< CVectorN<> >(6, 0.01);
	TestVectorClass< CVectorN<> >(7, 0.01);
	TestVectorClass< CVectorN<> >(8, 0.01);
	TestVectorClass< CVectorN<> >(9, 0.01);

	//	dynamic vectors of dimension 1..9, 
	//		with elements scaled to 100.0
	TestVectorClass< CVectorN<> >(1, 100.0);
	TestVectorClass< CVectorN<> >(2, 100.0);
	TestVectorClass< CVectorN<> >(3, 100.0);
	TestVectorClass< CVectorN<> >(4, 100.0);
	TestVectorClass< CVectorN<> >(5, 100.0);
	TestVectorClass< CVectorN<> >(6, 100.0);
	TestVectorClass< CVectorN<> >(7, 100.0);
	TestVectorClass< CVectorN<> >(8, 100.0);
	TestVectorClass< CVectorN<> >(9, 100.0);

	//	static matrices of dimension 1..9, 
	//		with elements scaled to 0.01
	TestMatrixClass< CMatrixD<1> >(1, 0.01);
	TestMatrixClass< CMatrixD<2> >(2, 0.01);
	TestMatrixClass< CMatrixD<3> >(3, 0.01);
	TestMatrixClass< CMatrixD<4> >(4, 0.01);
	TestMatrixClass< CMatrixD<5> >(5, 0.01);
	TestMatrixClass< CMatrixD<6> >(6, 0.01);
	TestMatrixClass< CMatrixD<7> >(7, 0.01);
	TestMatrixClass< CMatrixD<8> >(8, 0.01);
	TestMatrixClass< CMatrixD<9> >(9, 0.01);

	//	static matrices of dimension 1..9, 
	//		with elements scaled to 0.01
	TestMatrixClass< CMatrixD<1> >(1, 100.0);
	TestMatrixClass< CMatrixD<2> >(2, 100.0);
	TestMatrixClass< CMatrixD<3> >(3, 100.0);
	TestMatrixClass< CMatrixD<4> >(4, 100.0);
	TestMatrixClass< CMatrixD<5> >(5, 100.0);
	TestMatrixClass< CMatrixD<6> >(6, 100.0);
	TestMatrixClass< CMatrixD<7> >(7, 100.0);
	TestMatrixClass< CMatrixD<8> >(8, 100.0);
	TestMatrixClass< CMatrixD<9> >(9, 100.0);

	//	dynamic matrices of dimension 1..9, 
	//		with elements scaled to 0.01
	TestMatrixClass< CMatrixNxM<> >(1, 0.01);
	TestMatrixClass< CMatrixNxM<> >(2, 0.01);
	TestMatrixClass< CMatrixNxM<> >(3, 0.01);
	TestMatrixClass< CMatrixNxM<> >(4, 0.01);
	TestMatrixClass< CMatrixNxM<> >(5, 0.01);
	TestMatrixClass< CMatrixNxM<> >(6, 0.01);
	TestMatrixClass< CMatrixNxM<> >(7, 0.01);
	TestMatrixClass< CMatrixNxM<> >(8, 0.01);
	TestMatrixClass< CMatrixNxM<> >(9, 0.01);

	//	dynamic matrices of dimension 1..9, 
	//		with elements scaled to 100.0
	TestMatrixClass< CMatrixNxM<> >(1, 100.0);
	TestMatrixClass< CMatrixNxM<> >(2, 100.0);
	TestMatrixClass< CMatrixNxM<> >(3, 100.0);
	TestMatrixClass< CMatrixNxM<> >(4, 100.0);
	TestMatrixClass< CMatrixNxM<> >(5, 100.0);
	TestMatrixClass< CMatrixNxM<> >(6, 100.0);
	TestMatrixClass< CMatrixNxM<> >(7, 100.0);
	TestMatrixClass< CMatrixNxM<> >(8, 100.0);
	TestMatrixClass< CMatrixNxM<> >(9, 100.0);

	// test the SVD
	for (int nRows = 2; nRows < 20; nRows++)
	{
		for (int nCols = 2; nCols < 20; nCols++)
		{
			TestSVD<REAL>(nCols, nRows, FALSE);
			TestSVD<REAL>(nCols, nRows, TRUE);
		}
	}

	return 0;
}
