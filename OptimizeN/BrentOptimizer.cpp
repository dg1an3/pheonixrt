// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: BrentOptimizer.cpp 606 2008-09-14 18:06:57Z dglane001 $
#include "stdafx.h"

// the class definition
#include "BrentOptimizer.h"

#include <vnl/algo/vnl_bracket_minimum.h>

///////////////////////////////////////////////////////////////////////////////
// macros used to manipulate parameters
///////////////////////////////////////////////////////////////////////////////

// shifts three parameters
#define SHFT(a,b,c,d) (a)=(b); (b)=(c); (c)=(d);

// moves three parameters at a time
#define MOV3(a, b, c, d, e, f) (a)=(d); (b)=(e); (c)=(f);

// SIGN returns the argument a with the sign of the argument b 
#define SIGN(a,b) ((b) >= (REAL) 0.0 ? (REAL) fabs(a) : (REAL) -fabs(a))  
	

///////////////////////////////////////////////////////////////////////////////
// constants used to optimize
///////////////////////////////////////////////////////////////////////////////

const REAL GOLD = (REAL) 1.618034;	// golden mean

const REAL CGOLD = (REAL) 0.3819660;	// golden section ratio
								
const REAL ZEPS = (REAL) 1.0e-1;	// z-epsilon -- small number to protect against 
									// fractional accuracy for a minimum that
									// happens to be exactly zero;  used in
									//    function FindMinimum


const REAL TINY = (REAL) 1.0e-20;	// used in function BracketMinimum 

/*
const REAL BRACKET = (REAL) 10.0;	// 10.0	// initial bracket size

const REAL GLIMIT = (REAL) 100.0;		// 100.0;	// parameter needed by function BracketMinimum  
*/

const int ITER_MAX = 1000;		// maximum iteration


// holds the initial value for the brent optimization
const CVectorN<> CBrentOptimizer::m_vBrentInit(1);


//////////////////////////////////////////////////////////////////////
// CBrentOptimizer::CBrentOptimizer
// 
// constructs a new Brent optimizer
//////////////////////////////////////////////////////////////////////
CBrentOptimizer::CBrentOptimizer(CObjectiveFunction *pFunc)
	: COptimizer(pFunc),
		m_vAx(1), 
		m_vBx(1), 
		m_vCx(1),
		m_vX(1), 
		m_vU(1),
		m_vGrad(1),
		m_Bracket(4.0),
		m_GLimit(100.0),
		m_brentMinimizer(*pFunc)
{
	// make sure the brent optimizer's starting value is correct
	ASSERT(0.0 == m_vBrentInit[0]);
}

//////////////////////////////////////////////////////////////////////
// CBrentOptimizer::Optimize
// 
// performs the optimization given the initial value vector
//////////////////////////////////////////////////////////////////////
const CVectorN<>& CBrentOptimizer::Optimize(const CVectorN<>& vInit)
{
	BEGIN_LOG_SECTION(CBrentOptimizer::Optimize);

	// find three values the bracket a minimum
	REAL ax = vInit[0] - m_Bracket;
	REAL bx = vInit[0] ;
	REAL cx = vInit[0] + m_Bracket;
	REAL fa, fb, fc;
	// 	BracketMinimum(ax, bx, cx);
	vnl_bracket_minimum(*m_pFunc,ax,bx,cx,fa,fb,fc);
	LOG_EXPR(ax);  LOG_EXPR(bx);  LOG_EXPR(cx);

	// find the actual minimum
	REAL finalx;

#define USE_THIS_IMPL
#ifdef USE_THIS_IMPL

	// which version of FindMinimum we use depends on whether
	//		gradient information is available and should be used
	finalx = FindMinimum(ax, bx, cx);
	LOG_EXPR(finalx);

#else
	//m_brentMinimizer.set_x_tolerance(GetTolerance());
	finalx = m_brentMinimizer.minimize_given_bounds_and_all_f(ax,bx,cx,fa,fb,fc);
#endif

	// set the member variable that holds the final value
	m_vFinalParam.SetDim(1);	// 1-d for a brent optimizer
	m_vFinalParam[0] = finalx;

	END_LOG_SECTION();	// CBrentOptimizer::Optimize

	// and return it
	return m_vFinalParam;
}

//////////////////////////////////////////////////////////////////////
// CBrentOptimizer<REAL>::BracketMinimum
// 
// Given two distinct initial points ax and bx, this routine searches downhill 
// (defined by the function as evaluated at the initial points) and returns new 
// points ax, bx, cx that bracket a minimum of the function.  Also returned are 
// the function values at the three points fa, fb, fc.
//
// GOLD is the default ratio by which successive intervals are magnified;
// GLIMIT is the maximum magnification allowed for a parabolic-fit step
//
// This function was adapted from function mnbrak, Press et al., Numerical
// Recipes in C, 2nd Ed. 1992.
//
//////////////////////////////////////////////////////////////////////
void CBrentOptimizer::BracketMinimum(REAL& ax, REAL& bx, REAL& cx)
{
	USES_CONVERSION;

	BEGIN_LOG_SECTION(CBrentOptimizer::BracketMinimum);

	REAL fa, fb, fc;

	m_vAx[0] = ax;
	fa = (*m_pFunc)(m_vAx);

	m_vBx[0] = bx;
	fb = (*m_pFunc)(m_vBx);

	// Switch roles of a and b so that we can go downhill in the direction 
	// from a to b. 
	if (fb > fa)
	{
		REAL temp;
		SHFT(temp, ax, bx, temp)
		SHFT(temp, fa, fb, temp)
	}

	// First guess for c. 
	cx = bx + (REAL) GOLD * (bx - ax);

	m_vCx[0] = cx;
	fc = (*m_pFunc)(m_vCx);

	LOG(_T("f(%lf) = %lf"), ax, fa);
	LOG(_T("f(%lf) = %lf"), bx, fc);
	LOG(_T("f(%lf) = %lf"), cx, fc);

	// Keep returning here until we bracket.
	m_nIteration = 0;
	while (fb > fc)
	{
		BEGIN_LOG_SECTION("CBrentOptimizer::BracketMinimum!Iteration");
		LOG(_T("Iteration %i"), m_nIteration);

		// Compute u by parabolic extrapolation from a,b,c.  TINY is used to 
		// prevent any possible division by zero. 
		REAL r = (bx - ax) * (fb - fc);
		REAL q = (bx - cx) * (fb - fa);
		REAL u = (REAL) (bx - ((bx - cx) * q - (bx - ax) * r)
			 / (2.0 * SIGN(__max(fabs(q - r), TINY),q - r)));
		REAL fu; // function value at u
		REAL ulim = bx + (m_GLimit * (cx - bx));

		// We won't go farther than this.  Test various possibilities 
		if ((bx - u) * (u - cx) > 0.0)
		{
			// Parabolic u is between b and c: try it. 
			m_vU[0] = u;
			fu = (*m_pFunc)(m_vU);

			if (fu < fc)
			{
				// Got a minimum between b and c. 
				ax = bx;
				bx = u;
				fa = fb;
				fb = fu;

				EXIT_LOG_SECTION();
				goto cleanup;
			}
			else if (fu > fb)
			{
				// Got a minimum between a and u. 
				cx = u;
				fc = fu;

				EXIT_LOG_SECTION();
				goto cleanup;
			}

			// Parabolic fit was no use.  Use default magnification. 
			u = cx + (REAL) GOLD * (cx - bx);

			m_vU[0] = u;
			fu = (*m_pFunc)(m_vU);
		}
		else if ((cx - u) * (u - ulim) > 0.0)
		{
			// Parabolic fit is between c and its allowed limit. 
			m_vU[0] = u;
			fu = (*m_pFunc)(m_vU);

			if (fu < fc)
			{
				SHFT(bx,cx,u,cx + (REAL) GOLD * (cx - bx))

				m_vU[0] = u;
				SHFT(fb,fc,fu,(*m_pFunc)(m_vU))
			}
		}
		else if ((u - ulim) * (ulim - cx) >= 0.0)
		{
			// Limit parabolic u to maximum allowed value. 
			u = ulim;

			m_vU[0] = u;
			fu = (*m_pFunc)(m_vU);
		}
		else
		{
			// Reject parabolic u, use default magnification. 
			u=(cx) + (REAL) GOLD*(cx-bx);

			m_vU[0] = u;
			fu = (*m_pFunc)(m_vU);
		}

		// Eliminate oldest point and continue. 
		SHFT(ax,bx,cx,u)
		SHFT(fa,fb,fc,fu)

		m_nIteration++;
		END_LOG_SECTION();		// Iteration
	}

cleanup:
	END_LOG_SECTION();		// CBrentOptimizer::BracketMinimum
}


//////////////////////////////////////////////////////////////////////
// CBrentOptimizer<REAL>::FindMinimum
// 
// Given a function f, and given a bracketing triplet of abscissas
// ax, bx, cx, (such that bx is between ax and cx, and f(bx) < f(ax)
// and f(bx) < f(cx)), this routine isolates the minimum to a fractional
// precision of about tol using Brent's method.  The abscissa of the
// minimum is returned as xmin, and the minimum function value is returned
// as brent, the returned function value.
//
// This function was adapted from function brent, Press et al., Numerical
// Recipes in C, 2nd Ed. 1992.
// 
// ax,..,cx are function values needed to perform the minimization  
// tol is variable specifing the minimum agreement needed between successive 
// intertions before stopping 
//////////////////////////////////////////////////////////////////////
// template<class REAL>
REAL CBrentOptimizer::FindMinimum (REAL ax, REAL bx, REAL cx)
{
	USES_CONVERSION;

	REAL fx, x;

	BEGIN_LOG_SECTION(CBrentOptimizer::FindMinimum);

	// The following are intermediate computed values. 
	REAL a,b,d,etemp,fu,fv,fw,p,q,r,tol1,tol2,u,v,w,xm;
	REAL e=0.0;            // distance moved on the step before last 

	// a and b must be in ascending order, but input abscissas need not be.
	a=(ax < cx ? ax : cx);
	b=(ax > cx ? ax : cx);
	x=w=v=bx;

	m_vX[0] = x;
	fw=fv=fx=(*m_pFunc)(m_vX);

	// Main function loop. 
	for (m_nIteration = 0; m_nIteration < ITER_MAX; m_nIteration++)
	{
		BEGIN_LOG_SECTION(CBrentOptimizer::FindMinimum!Iteration);
		LOG(_T("Iteration %i"), m_nIteration);

		xm = (REAL) 0.5*(a+b);
		tol1 = (REAL) (GetTolerance() + fabs(x)*1e-8);
		tol2 = (REAL) 2.0 * tol1;

		// Test for done here. 
		if (fabs(x - xm) <= (tol2-0.5*(b-a)))
		{
			EXIT_LOG_SECTION();
			goto cleanup;
		}

		// Construct a trial parabolic fit. 
		if (fabs(e) > tol1)
		{
			r=(x-w)*(fx-fv);
			q=(x-v)*(fx-fw);
			p=(x-v)*q-(x-w)*r;
			q=(REAL) 2.0*(q-r);
			if (q > 0.0)
				p = -p;
			q=(REAL)fabs(q);
			etemp=e;
			e=d;

			// These conditions determine the acceptibility of the parabolic 
			//   fit. 
			if ((fabs(p) >= fabs(0.5*q*etemp)) || (p <= q*(a-x)) || (p >= q*(b-x)))
				// Here we take the golden section step into the larger of the two 
				//   segments 
				d=(REAL) CGOLD*(e=(x >= xm ? a-x : b-x));
			else
			{
				// Take the parabolic step. 
				d=p/q;
				u=x+d;
				if ((u-a < tol2) || (b-u < tol2))
					d=(REAL) SIGN(tol1,xm-x);
			}
		}
		else
		{
			d=(REAL) CGOLD*(e=(x >= xm ? a-x : b-x));
		}

		u=(fabs(d) >= tol1 ? x+d : x+(REAL)SIGN(tol1,d));

		// This is the one function evaluation per iteration. 
		m_vU[0] = u;
		fu=(*m_pFunc)(m_vU);
		LOG(_T("f(%lf) = %lf"), u, fu);

		// Now decide what to do with our function evaluation. 
		// Housekeeping follows: 
		if (fu <= fx)
		{
			if (u >= x) a=x; else b=x;
			SHFT(v,w,x,u)
			SHFT(fv,fw,fx,fu)
		}
		else
		{
			if (u < x) a=u; else b=u;
			if (fu <= fw || w == x)
			{
				v=w;
				w=u;
				fv=fw;
				fw=fu;
			}
			else if (fu <= fv || v == x || v == w)
			{
				v=u;
				fv=fu;
			}
		}

		// Done with housekeeping.  Back for another iteration. 
		END_LOG_SECTION()	// Iteration
	}
	LOG(_T("Too many iterations = %i"), m_nIteration);

cleanup:
	END_LOG_SECTION();	// CBrentOptimizer::BracketMinimum

	m_finalValue = fx;
	return x;
}

#ifdef NEVER
//////////////////////////////////////////////////////////////////////
// CBrentOptimizer<REAL>::FindMinimumGrad
// 
// Given a function f, and given a bracketing triplet of abscissas
// ax, bx, cx, (such that bx is between ax and cx, and f(bx) < f(ax)
// and f(bx) < f(cx)), this routine isolates the minimum to a fractional
// precision of about tol using Brent's method.  The abscissa of the
// minimum is returned as xmin, and the minimum function value is returned
// as brent, the returned function value.
//
// This function was adapted from function brent, Press et al., Numerical
// Recipes in C, 2nd Ed. 1992.
// 
// ax,..,cx are function values needed to perform the minimization  
// tol is variable specifing the minimum agreement needed between successive 
// intertions before stopping 
///////////////////////////////////////////////////////////////////////
REAL CBrentOptimizer::FindMinimumGrad(REAL ax, REAL bx, REAL cx)
{
	// a and b must be in ascending order, but input abscissas need not be.
	REAL a=(ax < cx ? ax : cx);
	REAL b=(ax > cx ? ax : cx);

	REAL x,w,v,u;
	x = w = v = bx;

	REAL fx, fw, fv, fu;
	m_vX[0] = x;
	fw = fv = fx = (*m_pFunc)(m_vX, &m_vGrad);

	REAL dx, dw, dv, du;
	dw = dv = dx = m_vGrad[0];
	
	REAL d = 0.0;			// holds a temporary value for e
	REAL e = 0.0;           // distance moved on the step before last 

	// Main function loop. 
	for (m_nIteration = 0; m_nIteration < ITER_MAX; m_nIteration++)
	{
		// computing mean x-value
		REAL xm = (REAL) 0.5 * (a + b);

		// computing bounding tolerances
		REAL tol1 = GetTolerance() * (REAL) fabs(x) + (REAL) ZEPS;
		REAL tol2 = (REAL) 2.0 * tol1;

		// test for convergence
		if ((REAL) fabs(x - xm) <= (tol2 - (REAL) 0.5 * (b-a)))
		{
			m_finalValue = fx;
			return x;
		}

		if ((REAL) fabs(e) > tol1)
		{
			// compute d1 with secant method
			REAL d1 = (REAL) 2.0 * (b - a);	// initialize with out-of-bracket value
			REAL d2 = d1;					// initialize with out-of-bracket value

			//secant method with one point
			if (dw != dx) 
			{
				d1 = (w - x) * dx / (dx - dw);
			}

			// and with the other
			if (dv != dx) 
			{
				d2 = (v - x) * dx / (dx - dv);
			}	

			// which estimate?  must be within the bracket, and on the side 
			//		pointed to be the derivative at x
			REAL u1 = x + d1;
			REAL u2 = x + d2;

			BOOL bOK1 = ((a - u1) * (u1 - b) > 0.0) && (dx * d1 <= 0.0);
			BOOL bOK2 = ((a - u2) * (u2 - b) > 0.0) && (dx * d2 <= 0.0);

			REAL olde = e;
			e = d;

			if (bOK1 || bOK2)
			{
				if (bOK1 && bOK2)
				{
					d = (fabs(d1) < fabs(d2) ? d1 : d2);
				}
				else if (bOK1)
				{
					d = d1;
				}
				else
				{
					d = d2;
				}

				if (fabs(d) <= fabs(0.5*olde))
				{
					u = x + d;
					if ((u - a < tol2) || (b - u < tol2))
					{
						d = SIGN(tol1, xm - x);
					}
				}
				else
				{
					// decide which segment by the sign of the derivative
					d = (REAL) 0.5 * (e = (dx >= 0.0 ? a - x : b - x));
				}
			}
			else
			{
				// decide which segment by the sign of the derivative
				d = (REAL) 0.5 * (e = (dx >= 0.0 ? a - x : b - x));
			}
		}
		else
		{
			// decide which segment by the sign of the derivative
			d = (REAL) 0.5 * (e = (dx >= 0.0 ? a - x : b - x));
		}

		if (fabs(d) >= tol1)
		{
			u = x + d;
			m_vU[0] = u;
			fu = (*m_pFunc)(m_vU, &m_vGrad);
		}
		else
		{
			u = x + SIGN(tol1, d);
			m_vU[0] = u;
			fu = (*m_pFunc)(m_vU, &m_vGrad);

			// if min step in downhill dir. takes us uphill, we are done
			if (fu > fx)
			{
				m_finalValue = fx;
				return x;
			}
		}

		du = m_vGrad[0];
		if (fu <= fx)
		{
			if (u >= x) 
			{
				a = x;
			}
			else
			{
				b = x;
			}

			MOV3(v, fv, dv, w, fw, dw);
			MOV3(w, fw, dw, x, fx, dx);
			MOV3(x, fx, fx, u, fu, du);
		}
		else
		{
			if (u < x) 
			{
				a = u;
			}
			else
			{
				b = u;
			}

			if (fu <= fw || w == x)
			{
				MOV3(v, fv, dv, w, fw, dw);
				MOV3(w, fw, dw, u, fu, du);
			}
			else if (fu < fv || v == x || v == w)
			{
				MOV3(v, fv, dv, u, fu, du);
			}
		}
	}

	ASSERT("Too many iterations\n");

	return 0.0;
}
#endif

const CVectorN<>& CBrentOptimizer::GetInitZero()
{
	return m_vBrentInit;
}

void CBrentOptimizer::SetParams(REAL Bracket, REAL GLimit)
{
	m_Bracket = Bracket;
	m_GLimit = GLimit;
	m_brentMinimizer.set_g_tolerance(m_GLimit);
	m_brentMinimizer.set_x_tolerance(GetTolerance());
}
