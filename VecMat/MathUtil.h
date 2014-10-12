//////////////////////////////////////////////////////////////////////
// MathUtil.h: declarations of standard math types, macros and 
//		functions
//
// Copyright (C) 1999-2003 Derek G Lane
// $Id: MathUtil.h,v 1.4 2003/02/25 06:16:52 default Exp $
//////////////////////////////////////////////////////////////////////

#if !defined(MATHUTIL_H)
#define MATHUTIL_H

// standard math library
#include <math.h>

#include <complex>
using namespace std;

//////////////////////////////////////////////////////////////////////
// standard real representation
//////////////////////////////////////////////////////////////////////
typedef double REAL;


//////////////////////////////////////////////////////////////////////
// declare PI for later use
//////////////////////////////////////////////////////////////////////
const REAL PI = (atan(1.0) * 4.0);


//////////////////////////////////////////////////////////////////////
// macros to assist in approximate evaluation of vectors
//////////////////////////////////////////////////////////////////////
const REAL DEFAULT_EPSILON = 1e-5;


//////////////////////////////////////////////////////////////////////
// template function to assist in approximate evaluations
//////////////////////////////////////////////////////////////////////
template<class TYPE>
BOOL IsApproxEqual(TYPE r1, TYPE r2, TYPE epsilon =  DEFAULT_EPSILON)
{
	return (fabs(r1 - r2) < epsilon);
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
		/ (TYPE) (sqrt((TYPE) 2.0 * PI * s));
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

#endif // !defined(MATHUTIL_H)
