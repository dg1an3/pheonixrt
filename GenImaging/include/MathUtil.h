//////////////////////////////////////////////////////////////////////
// MathUtil.h: declarations of standard math types, macros and 
//		functions
//
// Copyright (C) 1999-2003 Derek G Lane
// $Id: MathUtil.h,v 1.1 2007-10-29 01:43:52 Derek Lane Exp $
//////////////////////////////////////////////////////////////////////

#if !defined(MATHUTIL_H)
#define MATHUTIL_H

// standard math libraries
#include <math.h>
#include <float.h>

#include <complex>
using namespace std;

//////////////////////////////////////////////////////////////////////
// standard real representation
//////////////////////////////////////////////////////////////////////
#ifndef REAL_DEFINED
#define REAL_DEFINED
#ifdef REAL_FLOAT
typedef float REAL;
#define REAL_FMT "%f"
#else
typedef double REAL;
#define REAL_FMT "%lf"
#endif
#endif

// macro to wrap real values
#define R(x) ((REAL)(x))


//////////////////////////////////////////////////////////////////////
// declare PI for later use
//////////////////////////////////////////////////////////////////////
const REAL PI = (REAL) (atan(1.0) * 4.0);
const float PIf = (float) atan(1.0f) * 4.0f;

//////////////////////////////////////////////////////////////////////
// macros to assist in approximate evaluation of vectors
//////////////////////////////////////////////////////////////////////
const REAL DEFAULT_EPSILON = (REAL) 1e-5;

//////////////////////////////////////////////////////////////////////
// macro to check IPP operations
//////////////////////////////////////////////////////////////////////
#ifdef USE_IPP
#ifdef _DEBUG
#define CK_IPP(x) { IppStatus stat = (x); ASSERT(stat == ippStsOk); }
#else
#define CK_IPP(x) (x);
#endif
#endif

//////////////////////////////////////////////////////////////////////
// template function to assist in approximate evaluations
//////////////////////////////////////////////////////////////////////
template<class TYPE>
bool IsApproxEqual(TYPE r1, TYPE r2, TYPE epsilon =  DEFAULT_EPSILON)
{
	return (fabs(r1 - r2) < epsilon);
}


//////////////////////////////////////////////////////////////////////
// Round
//
// template function for rounding (type conversion)
//////////////////////////////////////////////////////////////////////
template<class RETURN_TYPE>
RETURN_TYPE Round(double value)
{
	return (RETURN_TYPE) floor((double) value + 0.5);

}	// Round


//////////////////////////////////////////////////////////////////////
// template function to randomly perturb a value by delta
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void Perturb(TYPE *pVal, TYPE delta)
{
	(*pVal) *= 1.0 + delta * (0.5 - (TYPE) rand() / (TYPE) RAND_MAX);
}

//////////////////////////////////////////////////////////////////////
// declare functions to represent gaussian distributions
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline TYPE Gauss(TYPE x, TYPE s)
{
	// compute the exponent
	TYPE d = (x * x) / ((TYPE) 2.0 * s * s); 

	// return the exponential
	return (TYPE) exp(-d)
		/ (TYPE) (sqrt((TYPE) 2.0 * PI) * s);
}

template<class TYPE>
inline TYPE dGauss(TYPE x, TYPE s)
{
	// compute the factor
	TYPE dx = -(x) / (s * s);

	return dx * Gauss(x, s);
}

template<class TYPE>
inline TYPE Gauss2D(TYPE x, TYPE y, TYPE sx, TYPE sy)
{
	// compute the exponent
	TYPE d = (x * x) / ((TYPE) 2.0 * sx * sx) 
		+ (y * y) / ((TYPE) 2.0 * sy * sy);

	// return the exponential
	return (TYPE) exp(-d)
		/ (TYPE) (sqrt(2.0 * PI * sx * sy));
}

template<class TYPE>
inline TYPE dGauss2D_dx(TYPE x, TYPE y, TYPE sx, TYPE sy)
{
	// compute the exponent
	TYPE dx = -((TYPE) 2.0 * x) / ((TYPE) 2.0 * sx * sx);

	// return the exponential
	return dx * Gauss2D(x, y, sx, sy);
}

template<class TYPE>
inline TYPE dGauss2D_dy(TYPE x, TYPE y, TYPE sx, TYPE sy)
{
	// compute the exponent
	TYPE dy = -((TYPE) 2.0 * y) / ((TYPE) 2.0 * sy * sy);

	// return the exponential
	return dy * Gauss2D(x, y, sx, sy);
}

//////////////////////////////////////////////////////////////////////
// function to find the angle from a sine/cosine value pair
//////////////////////////////////////////////////////////////////////
inline REAL AngleFromSinCos(REAL sin_angle, REAL cos_angle)
{
	REAL angle = 0.0;

	if (sin_angle >= 0.0)
	{
		angle = (REAL) acos(cos_angle);	// range of [0..PI]	
	}
	else if (cos_angle >= 0.0)
	{
		angle = (REAL) asin(sin_angle);	// range of [-PI/2..PI/2]
	}
	else
	{
		angle = (REAL) (2.0 * PI) - (REAL) acos(cos_angle);
	}

	// ensure the angle is in range [0..2*PI];
	if (angle < (REAL) 0.0)
	{
		angle += (REAL) (2.0 * PI);
	}

	// now check
#ifdef _DEBUG_ACCURACY
	ASSERT(fabs(sin(angle) - sin_angle) < 1e-6);
	ASSERT(fabs(cos(angle) - cos_angle) < 1e-6);
#endif

	return angle;
}

inline REAL Deg2Rad(const REAL& deg)
{
	return deg * (atan(1.0) / 4.0) / 180.0;
}

///////////////////////////////////////////////////////////////////////////////
// Sigmoid
// 
// Compute Sigmoid function, with scale parameter
///////////////////////////////////////////////////////////////////////////////
template<class TYPE>
inline TYPE Sigmoid(TYPE x, TYPE scale /* = 1.0 */) 
{
	TYPE res = (TYPE) (1.0 / (1.0 + exp(-scale * x)));

	if (_finite(res))
	{
		return res;
	}

	return (x > 0.0) ? (TYPE) 1.0 : (TYPE) 0.0;

}	// Sigmoid


///////////////////////////////////////////////////////////////////////////////
// dSigmoid
// 
// Derivative of Sigmoid
///////////////////////////////////////////////////////////////////////////////
template<class TYPE>
inline TYPE dSigmoid(TYPE x, TYPE scale /* = 1.0 */) 
{
	TYPE exp_val = exp(-scale * x);
	TYPE denom = (TYPE) (1.0 + exp_val);
	TYPE d_denom = -scale * exp_val;
	TYPE res = -d_denom / (denom * denom);

	if (_finite(res))
	{
		return res;
	}

	return 0.0;

}	// dSigmoid


///////////////////////////////////////////////////////////////////////////////
// InvSigmoid
// 
// inverse of sigmoid function
///////////////////////////////////////////////////////////////////////////////
template<class TYPE>
inline TYPE InvSigmoid(TYPE y, TYPE scale /* = 1.0 */)
{
	// test for bounds
	if (y >= 1.0) 
	{
		y = (TYPE) 1.0 - (TYPE) 1e-6;
	}

	// compute value
	TYPE value = (TYPE) (-log(1.0 / y - 1.0) 
		/ scale);
	ASSERT(_finite(value));

	// test result
	ASSERT(IsApproxEqual(Sigmoid(value, scale), y));

	// return
	return value;

}	// InvSigmoid


//////////////////////////////////////////////////////////////////////
// ITERATE_VECTOR
//
// macro for quick iterative loops over vectors
//////////////////////////////////////////////////////////////////////
#define ITERATE_VECTOR(coll, index, statement) \
{ for (int index = 0; index < coll.GetDim(); index++) { statement; } }


// only use if we have included necessary AFX calls
#ifdef __AFXWIN_H__
//////////////////////////////////////////////////////////////////////
// GetProfileReal
//
// function for real-valued registry values
//////////////////////////////////////////////////////////////////////
inline REAL GetProfileReal(const char *pszSection, const char *pszName, double default_value)
{
	USES_CONVERSION;

	CString strValue;
	strValue.Format(_T("%lf"), default_value);
	
	// get profile string, or default value
	strValue = ::AfxGetApp()->GetProfileString(A2T(pszSection), A2T(pszName), strValue);

	// turn to REAL
	REAL value;
	_stscanf_s(strValue.GetBuffer(), _T(REAL_FMT), &value);

	// make sure parameter is written, so it can be modified through RegEdit
	::AfxGetApp()->WriteProfileString(A2T(pszSection), A2T(pszName), strValue);

	return value;

}	// GetProfileReal
#endif

//////////////////////////////////////////////////////////////////////
// functions for complex values
//////////////////////////////////////////////////////////////////////
inline REAL arg(const complex<REAL>& c)
{
	return atan2(c.real(), c.imag());
}

inline REAL abs(const complex<REAL>& c)
{
	return sqrt(c.real() * c.real() + c.imag() * c.imag());
}

inline complex<REAL> conjg(const complex<REAL>& c)
{
	return complex<REAL>(c.real(), -c.imag());
}

inline REAL conjg(const REAL& c)
{
	return c;
}


#define CK_BOOL(x) if (!(x)) TRACE("CK_BOOL failed\n");

#endif // !defined(MATHUTIL_H)
