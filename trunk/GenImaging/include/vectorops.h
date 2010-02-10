#if !defined(VECTOROPS_H)
#define VECTOROPS_H

#ifdef USE_IPP
#include <ipps.h>
#include <ippm.h>
#endif


#ifndef INLINE_DEFINED
// subst for forcing inline of function expansions
#define INLINE __forceinline
#define INLINE_DEFINED
#endif

///////////////////////////////////////////////////////////////////////////////////////////
// Allocate / Free values
///////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
void 
	AllocValues(int nCount, TYPE*& pValues)
{
	pValues = new TYPE[nCount];

}	// AllocValues

///////////////////////////////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
void 
	FreeValues(TYPE*& pValues)
{
	delete pValues;
	pValues = NULL;

}	// FreeValues

#ifdef USE_IPP

// alloc / free macros
#define IPP_ALLOC_FREE(TYPE_IPP)				\
template<> INLINE												\
void AllocValues(int nCount, Ipp##TYPE_IPP*& pValues)			\
{																\
	pValues = ippsMalloc_##TYPE_IPP(nCount);					\
}																\
template<> INLINE												\
void FreeValues(Ipp##TYPE_IPP*& pValues)						\
{																\
	ippsFree(pValues); pValues = NULL;							\
}

// declarations for different types
IPP_ALLOC_FREE(8u);
IPP_ALLOC_FREE(16u);
IPP_ALLOC_FREE(16s);
IPP_ALLOC_FREE(32u);
IPP_ALLOC_FREE(32s);
IPP_ALLOC_FREE(32f);
IPP_ALLOC_FREE(64f);

#endif	// USE_IPP


///////////////////////////////////////////////////////////////////////////////////////////
// Monadic op base macros
///////////////////////////////////////////////////////////////////////////////////////////

#define BASE_MONADIC_OP(NAME, BODY)				\
template<class TYPE> INLINE													\
void																		\
	NAME(TYPE *pDst, const TYPE *pSrc, int nLength)							\
{																			\
	for (int nAt = 0; nAt < nLength; nAt++)									\
	{																		\
		BODY;																\
	}																		\
}

#define IPP_MONADIC_OP(NAME, TYPE, FUNC)		\
template<> INLINE															\
void																		\
	NAME(TYPE *pDst, const TYPE *pSrc, int nLength)							\
{																			\
	FUNC(pSrc, pDst, nLength);												\
}


#define BASE_MONADIC_OP_I(NAME, BODY)			\
template<class TYPE> INLINE													\
void																		\
	NAME(TYPE *pSrcDst, int nLength)										\
{																			\
	for (int nAt = 0; nAt < nLength; nAt++)									\
	{																		\
		BODY;																\
	}																		\
}

#define IPP_MONADIC_OP_I(NAME, TYPE, FUNC)		\
template<> INLINE															\
void																		\
	NAME(TYPE *pSrcDst, int nLength)										\
{																			\
	FUNC(pSrcDst, nLength);													\
}


///////////////////////////////////////////////////////////////////////////////////////////
// Dyadic op base macros
///////////////////////////////////////////////////////////////////////////////////////////

#define BASE_DYADIC_OP(NAME, BODY)												\
template<class TYPE> INLINE														\
void NAME(TYPE *pDst, const TYPE *pSrcL, const TYPE *pSrcR, int nLength)		\
{	for (int nAt = 0; nAt < nLength; nAt++) BODY; }

#define IPP_DYADIC_OP(NAME, TYPE, FUNC)											\
template<> INLINE																\
void NAME(TYPE *pDst, const TYPE *pSrcL, const TYPE *pSrcR, int nLength)		\
{	FUNC(pSrcL, pSrcR, pDst, nLength); }


#define BASE_DYADIC_OP_C(NAME, BODY)											\
template<class TYPE> INLINE														\
void NAME(TYPE *pDst, const TYPE *pSrcL, const TYPE& valueR, int nLength)		\
{	for (int nAt = 0; nAt < nLength; nAt++) BODY; }

#define IPP_DYADIC_OP_C(NAME, TYPE, FUNC)										\
template<> INLINE																\
void NAME(TYPE *pDst, const TYPE *pSrcL, const TYPE& valueR, int nLength)		\
{	FUNC(pSrcL, valueR, pDst, nLength); }


#define BASE_DYADIC_OP_I(NAME, BODY)											\
template<class TYPE> INLINE														\
void NAME(TYPE *pSrcLDst, const TYPE *pSrcR, int nLength)						\
{	for (int nAt = 0; nAt < nLength; nAt++) { BODY; } }

#define IPP_DYADIC_OP_I(NAME, TYPE, FUNC)										\
template<> INLINE																\
void NAME(TYPE *pSrcLDst, const TYPE *pSrcR, int nLength)						\
{	FUNC(pSrcR, pSrcLDst, nLength); }


#define BASE_DYADIC_OP_C_I(NAME, BODY)											\
template<class TYPE> INLINE														\
void NAME(TYPE *pSrcLDst, const TYPE& valueR, int nLength)						\
{	for (int nAt = 0; nAt < nLength; nAt++) { BODY; } }

#define IPP_DYADIC_OP_C_I(NAME, TYPE, FUNC)										\
template<> INLINE																\
void NAME(TYPE *pSrcLDst, const TYPE& valueR, int nLength)						\
{	FUNC(valueR, pSrcLDst, nLength); }



///////////////////////////////////////////////////////////////////////////////////////////
// Copying
///////////////////////////////////////////////////////////////////////////////////////////

BASE_MONADIC_OP(CopyValues, pDst[nAt] = pSrc[nAt]);
#ifdef USE_IPP
IPP_MONADIC_OP(CopyValues, Ipp16s, ippsCopy_16s);
IPP_MONADIC_OP(CopyValues, Ipp32f, ippsCopy_32f);
IPP_MONADIC_OP(CopyValues, Ipp64f, ippsCopy_64f);
#endif

///////////////////////////////////////////////////////////////////////////////////////////
// Init to zero
///////////////////////////////////////////////////////////////////////////////////////////

BASE_MONADIC_OP_I(ZeroValues, pSrcDst[nAt] = (TYPE) 0.0);
#ifdef USE_IPP
IPP_MONADIC_OP_I(ZeroValues, Ipp8u, ippsZero_8u);
IPP_MONADIC_OP_I(ZeroValues, Ipp16s, ippsZero_16s);
IPP_MONADIC_OP_I(ZeroValues, Ipp32f, ippsZero_32f);
IPP_MONADIC_OP_I(ZeroValues, Ipp64f, ippsZero_64f);
#endif



///////////////////////////////////////////////////////////////////////////////////////////
// Sum
///////////////////////////////////////////////////////////////////////////////////////////

BASE_DYADIC_OP(SumValues, pDst[nAt] = pSrcL[nAt] + pSrcR[nAt]);
#ifdef USE_IPP
IPP_DYADIC_OP(SumValues, Ipp16s, ippsAdd_16s);
IPP_DYADIC_OP(SumValues, Ipp32f, ippsAdd_32f);
IPP_DYADIC_OP(SumValues, Ipp64f, ippsAdd_64f);
#endif

BASE_DYADIC_OP_C(SumValues, (pDst[nAt] = pSrcL[nAt] + valueR));
#ifdef USE_IPP
// IPP_DYADIC_OP_C(SumValues, Ipp16s, ippsAddC_16s);
IPP_DYADIC_OP_C(SumValues, Ipp32f, ippsAddC_32f);
IPP_DYADIC_OP_C(SumValues, Ipp64f, ippsAddC_64f);
#endif

BASE_DYADIC_OP_I(SumValues, pSrcLDst[nAt] += pSrcR[nAt]);
#ifdef USE_IPP
IPP_DYADIC_OP_I(SumValues, Ipp16s, ippsAdd_16s_I);
IPP_DYADIC_OP_I(SumValues, Ipp32f, ippsAdd_32f_I);
IPP_DYADIC_OP_I(SumValues, Ipp64f, ippsAdd_64f_I);
#endif

BASE_DYADIC_OP_C_I(SumValues, pSrcLDst[nAt] += valueR);
#ifdef USE_IPP
IPP_DYADIC_OP_C_I(SumValues, Ipp16s, ippsAddC_16s_I);
IPP_DYADIC_OP_C_I(SumValues, Ipp32f, ippsAddC_32f_I);
IPP_DYADIC_OP_C_I(SumValues, Ipp64f, ippsAddC_64f_I);
#endif


///////////////////////////////////////////////////////////////////////////////////////////
// Difference
///////////////////////////////////////////////////////////////////////////////////////////

BASE_DYADIC_OP(DiffValues, pDst[nAt] = pSrcL[nAt] - pSrcR[nAt]);
#ifdef USE_IPP
IPP_DYADIC_OP(DiffValues, Ipp16s, ippsSub_16s);
IPP_DYADIC_OP(DiffValues, Ipp32f, ippsSub_32f);
IPP_DYADIC_OP(DiffValues, Ipp64f, ippsSub_64f);
#endif

BASE_DYADIC_OP_C(DiffValues, pDst[nAt] = pSrcL[nAt] - valueR);
#ifdef USE_IPP
// IPP_DYADIC_OP_C(DiffValues, Ipp16s, ippsSubC_16s);
IPP_DYADIC_OP_C(DiffValues, Ipp32f, ippsSubC_32f);
IPP_DYADIC_OP_C(DiffValues, Ipp64f, ippsSubC_64f);
#endif

BASE_DYADIC_OP_I(DiffValues, pSrcLDst[nAt] -= pSrcR[nAt]);
#ifdef USE_IPP
IPP_DYADIC_OP_I(DiffValues, Ipp16s, ippsSub_16s_I);
IPP_DYADIC_OP_I(DiffValues, Ipp32f, ippsSub_32f_I);
IPP_DYADIC_OP_I(DiffValues, Ipp64f, ippsSub_64f_I);
#endif

BASE_DYADIC_OP_C_I(DiffValues, pSrcLDst[nAt] -= valueR);
#ifdef USE_IPP
IPP_DYADIC_OP_C_I(DiffValues, Ipp16s, ippsSubC_16s_I);
IPP_DYADIC_OP_C_I(DiffValues, Ipp32f, ippsSubC_32f_I);
IPP_DYADIC_OP_C_I(DiffValues, Ipp64f, ippsSubC_64f_I);
#endif


///////////////////////////////////////////////////////////////////////////////////////////
// Multiplication
///////////////////////////////////////////////////////////////////////////////////////////

BASE_DYADIC_OP(MultValues, pDst[nAt] = pSrcL[nAt] * pSrcR[nAt]);
#ifdef USE_IPP
IPP_DYADIC_OP(MultValues, Ipp16s, ippsMul_16s);
IPP_DYADIC_OP(MultValues, Ipp32f, ippsMul_32f);
IPP_DYADIC_OP(MultValues, Ipp64f, ippsMul_64f);
#endif

BASE_DYADIC_OP_C(MultValues, pDst[nAt] = pSrcL[nAt] * valueR);
#ifdef USE_IPP
// IPP_DYADIC_OP_C(MultValues, Ipp16s, ippsMulC_16s);
IPP_DYADIC_OP_C(MultValues, Ipp32f, ippsMulC_32f);
IPP_DYADIC_OP_C(MultValues, Ipp64f, ippsMulC_64f);
#endif

BASE_DYADIC_OP_I(MultValues, pSrcLDst[nAt] *= pSrcR[nAt]);
#ifdef USE_IPP
IPP_DYADIC_OP_I(MultValues, Ipp16s, ippsMul_16s_I);
IPP_DYADIC_OP_I(MultValues, Ipp32f, ippsMul_32f_I);
IPP_DYADIC_OP_I(MultValues, Ipp64f, ippsMul_64f_I);
#endif

BASE_DYADIC_OP_C_I(MultValues, pSrcLDst[nAt] *= valueR);
#ifdef USE_IPP
IPP_DYADIC_OP_C_I(MultValues, Ipp16s, ippsMulC_16s_I);
IPP_DYADIC_OP_C_I(MultValues, Ipp32f, ippsMulC_32f_I);
IPP_DYADIC_OP_C_I(MultValues, Ipp64f, ippsMulC_64f_I);
#endif


///////////////////////////////////////////////////////////////////////////////////////////
// Division
///////////////////////////////////////////////////////////////////////////////////////////

// TODO: check this out (IPP uses strange order)
BASE_DYADIC_OP(DivValues, pDst[nAt] = pSrcR[nAt] / pSrcL[nAt]);
#ifdef USE_IPP
// IPP_DYADIC_OP(DivValues, Ipp16s, ippsDiv_16s);
IPP_DYADIC_OP(DivValues, Ipp32f, ippsDiv_32f);
IPP_DYADIC_OP(DivValues, Ipp64f, ippsDiv_64f);
#endif

BASE_DYADIC_OP_C(DivValues, pDst[nAt] = pSrcL[nAt] / valueR);
#ifdef USE_IPP
// IPP_DYADIC_OP_C(DivValues, Ipp16s, ippsDivC_16s);
IPP_DYADIC_OP_C(DivValues, Ipp32f, ippsDivC_32f);
IPP_DYADIC_OP_C(DivValues, Ipp64f, ippsDivC_64f);
#endif

BASE_DYADIC_OP_I(DivValues, pSrcLDst[nAt] /= pSrcR[nAt]);
#ifdef USE_IPP
// IPP_DYADIC_OP_I(DivValues, Ipp16s, ippsDiv_16s_I);
IPP_DYADIC_OP_I(DivValues, Ipp32f, ippsDiv_32f_I);
IPP_DYADIC_OP_I(DivValues, Ipp64f, ippsDiv_64f_I);
#endif

BASE_DYADIC_OP_C_I(DivValues, pSrcLDst[nAt] /= valueR);
#ifdef USE_IPP
// IPP_DYADIC_OP_C_I(DivValues, Ipp16s, ippsDivC_16s_I);
IPP_DYADIC_OP_C_I(DivValues, Ipp32f, ippsDivC_32f_I);
IPP_DYADIC_OP_C_I(DivValues, Ipp64f, ippsDivC_64f_I);
#endif


///////////////////////////////////////////////////////////////////////////////////////////
// Square / root
///////////////////////////////////////////////////////////////////////////////////////////

BASE_MONADIC_OP(SqrValues, pDst[nAt] = pSrc[nAt] * pSrc[nAt]);
#ifdef USE_IPP
IPP_MONADIC_OP(SqrValues, Ipp32f, ippsSqr_32f);
IPP_MONADIC_OP(SqrValues, Ipp64f, ippsSqr_64f);
#endif

BASE_MONADIC_OP_I(SqrValues, pSrcDst[nAt] *= pSrcDst[nAt]);
#ifdef USE_IPP
IPP_MONADIC_OP_I(SqrValues, Ipp32f, ippsSqr_32f_I);
IPP_MONADIC_OP_I(SqrValues, Ipp64f, ippsSqr_64f_I);
#endif


BASE_MONADIC_OP(SqrtValues, pDst[nAt] = sqrt(pSrc[nAt]));
#ifdef USE_IPP
IPP_MONADIC_OP(SqrtValues, Ipp32f, ippsSqrt_32f);
IPP_MONADIC_OP(SqrtValues, Ipp64f, ippsSqrt_64f);
#endif

BASE_MONADIC_OP_I(SqrtValues, pSrcDst[nAt] = sqrt(pSrcDst[nAt]));
#ifdef USE_IPP
IPP_MONADIC_OP_I(SqrtValues, Ipp32f, ippsSqrt_32f_I);
IPP_MONADIC_OP_I(SqrtValues, Ipp64f, ippsSqrt_64f_I);
#endif


///////////////////////////////////////////////////////////////////////////////////////////
template<class ELEM_TYPE> INLINE
ELEM_TYPE 
	VectorLength(const ELEM_TYPE *pV, int nLength)
	// compute vector length
{
	// form the sum of the square of each element
	ELEM_TYPE lenSq = (ELEM_TYPE) 0.0;
	for (int nAt = 0; nAt < nLength; nAt++)
	{
		lenSq += pV[nAt] * pV[nAt];
	}

	return (ELEM_TYPE) sqrt(lenSq);

}	// VectorLength


#ifdef NEVER // USE_IPP

///////////////////////////////////////////////////////////////////////////////////////////
template<> INLINE
float 
	VectorLength(const float *pV, int nLength) 
{													
	float length;									
	CK_IPP(ippmL2Norm_v_32f(pV, sizeof(float), &length, nLength));

	return length;								

}	// VectorLength(const float *pV, int nLength) 


///////////////////////////////////////////////////////////////////////////////////////////
template<> INLINE
double 
	VectorLength(const double *pV, int nLength) 
{													
	double length;									
	CK_IPP(ippmL2Norm_v_64f(pV, sizeof(double), &length, nLength));

	return length;								

}	// VectorLength(const double *pV, int nLength) 

#endif



///////////////////////////////////////////////////////////////////////////////////////////
template<class ELEM_TYPE> INLINE
ELEM_TYPE 
	DotProduct(const ELEM_TYPE *pLeft, 
			const ELEM_TYPE *pRight, int nLength)
	// compute vector dot product
{
	// stores the dot product
	ELEM_TYPE prod = (ELEM_TYPE) 0.0;

	// sum the element-wise multiplication
	for (int nAt = 0; nAt < nLength; nAt++)
	{
		prod += pLeft[nAt] * pRight[nAt];
	}
	
	return prod;

}	// DotProduct


#ifdef NEVER // USE_IPP

///////////////////////////////////////////////////////////////////////////////////////////
template<> INLINE								
float 
	DotProduct(const float *vLeft,					
			const float *vRight, int nLength)
{															
	float prod;
	CK_IPP(ippmDotProduct_vv_32f(vLeft, sizeof(float), vRight, sizeof(float),
		&prod, nLength));

	return prod;											

}	// DotProduct(const float *vLeft,					
	//		const float *vRight, int nLength)


///////////////////////////////////////////////////////////////////////////////////////////
template<> INLINE								
double 
	DotProduct(const double *vLeft,					
			const double *vRight, int nLength)
{															
	double prod;
	CK_IPP(ippmDotProduct_vv_64f(vLeft, sizeof(double), vRight, sizeof(double), 
		&prod, nLength));

	return prod;											

}	// DotProduct(const double *vLeft,					
	//		const double *vRight, int nLength)

#endif



//////////////////////////////////////////////////////////////////////
template<class ELEM_TYPE>
void 
	RandomVector(ELEM_TYPE range, ELEM_TYPE *pV, int nLength)
	// initializes a random vector
{
	for (int nAt = 0; nAt < nLength; nAt++)
	{
		pV[nAt] = range - (ELEM_TYPE) 2.0 * range 
			* (ELEM_TYPE) rand() 
				/ (ELEM_TYPE) RAND_MAX;
	}

}	// RandomVector



#ifdef USE_XMLLOGGING

//////////////////////////////////////////////////////////////////////
INLINE 
void 
	LogExprExt(const CRect& rect, 
			const char *pszName, const char *pszModule)
	// helper function for XML logging of vectors
{
	// get the global log file
	CXMLLogFile *pLog = CXMLLogFile::GetLogFile();

	// only if we are logging --
	if (pLog->IsLogging())
	{
		// create a new expression element
		CXMLElement *pVarElem = pLog->NewElement("lx", pszModule);

		// if there is a name,
		if (strlen(pszName) > 0)
		{
			// set it.
			pVarElem->Attribute("name", pszName);
		}

		// set type to generice "CVector"
		pVarElem->Attribute("type", "CRect");
		
		// get the current format for the element type
		const char *pszFormat = pLog->GetFormat(rect.bottom);

		// format each element
		pLog->Format(pszFormat, rect.top);
		pLog->Format(pszFormat, rect.left);
		pLog->Format(pszFormat, rect.bottom);
		pLog->Format(pszFormat, rect.right);

		// done.
		pLog->GetLogFile()->CloseElement();
	}

}	// LogExprExt

#endif	// USE_XMLLOGGING

#endif	// #if !defined(VECTOROPS_H)

