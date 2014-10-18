//////////////////////////////////////////////////////////////////////////////
// ItkUtils.h
//
// Various utilities for working with the ITK library
//
// Copyright (C) 2007 DG Lane
//////////////////////////////////////////////////////////////////////////////

#pragma once

#ifdef USE_IPP
//#include <ippi.h>
#endif


#include <itkImageRegionIterator.h>
#include <itkImageRegionIteratorWithIndex.h>

#include <itkPolylineParametricPath.h>

// normative voxel type
typedef float VOXEL_REAL;
typedef float VoxelReal;

// helper typedef for the ITK Volume of pixel types
typedef itk::Image<VOXEL_REAL,3> VolumeReal;
typedef itk::Image<VOXEL_REAL,2> VolumeSliceReal;
typedef itk::Image<short,3> VolumeShort;
typedef itk::Image<unsigned char, 3> VolumeChar;

typedef itk::ImageRegionConstIterator< VolumeReal > ConstVolumeRealIterator;
typedef itk::ImageRegionIterator< VolumeReal > VolumeRealIterator;

// helper to declare a smart-pointer member
#define DECLARE_ATTRIBUTE_SPTR(NAME, TYPE) \
private:													\
	TYPE::Pointer m_p##NAME;				\
public:														\
	TYPE *Get##NAME()								\
	{ return m_p##NAME; }						\
	const TYPE *Get##NAME() const		\
	{ return m_p##NAME; }						\
	void Set##NAME(TYPE *pValue)		\
	{ m_p##NAME = pValue; }

#define DeclareMemberSPtr(NAME, TYPE) \
private:													\
	TYPE::Pointer m_p##NAME;				\
public:														\
	TYPE *Get##NAME()								\
	{ return m_p##NAME; }						\
	const TYPE *Get##NAME() const		\
	{ return m_p##NAME; }						\
	void Set##NAME(TYPE *pValue)		\
	{ m_p##NAME = pValue; }

#define DeclareMemberSPtrGet(NAME, TYPE) \
private:													\
	TYPE::Pointer m_p##NAME;				\
public:														\
	TYPE *Get##NAME()								\
	{ return m_p##NAME; }						\
	const TYPE *Get##NAME() const		\
	{ return m_p##NAME; }	

#define DeclareMemberSPtrGI(NAME, TYPE) \
private:													\
	TYPE::Pointer m_p##NAME;				\
public:														\
	TYPE *Get##NAME()								\
	{ return m_p##NAME; }						\
	const TYPE *Get##NAME() const		\
	{ return m_p##NAME; }						\
	void Set##NAME(TYPE *pValue);

#define CreateMemberSPtr(NAME, TYPE) \
	m_p##NAME = TYPE::New();

#ifndef INLINE_DEFINED
// subst for forcing inline of function expansions
#define INLINE __forceinline
#define INLINE_DEFINED
#endif

//////////////////////////////////////////////////////////////////////
inline 
itk::Size<1> 
	MakeSize(const itk::Size<1>::SizeValueType& v)
{
	itk::Size<1> vSize;
	vSize[0] = v;
	return vSize;
}

//////////////////////////////////////////////////////////////////////
inline 
itk::Size<2> 
	MakeSize(const itk::Size<2>::SizeValueType& v0, 
					 const itk::Size<2>::SizeValueType& v1)
{
	itk::Size<2> vSize;
	vSize[0] = v0;
	vSize[1] = v1;
	return vSize;
}

//////////////////////////////////////////////////////////////////////
inline 
itk::Size<3> 
	MakeSize(const itk::Size<3>::SizeValueType& v0, 
					 const itk::Size<3>::SizeValueType& v1,
					 const itk::Size<3>::SizeValueType& v2)
{
	itk::Size<3> vSize;
	vSize[0] = v0;
	vSize[1] = v1;
	vSize[2] = v2;
	return vSize;
}

//////////////////////////////////////////////////////////////////////
template<int DIM> inline
itk::Vector<REAL,DIM>
	MakeVector(REAL v0, REAL v1)
{
	itk::Vector<REAL,DIM> v;
	v[0] = v0;
	v[1] = v1;
	for (int nC = 2; nC < DIM; nC++) v[nC] = 0.0;
	return v;
}

//////////////////////////////////////////////////////////////////////
template<int DIM> inline
itk::ContinuousIndex<REAL,DIM>
	MakeContinuousIndex(REAL v0, REAL v1)
{
	itk::ContinuousIndex<REAL,DIM> v;
	v[0] = v0;
	v[1] = v1;
	for (int nC = 2; nC < DIM; nC++) v[nC] = 0.0;
	return v;
}

//////////////////////////////////////////////////////////////////////
template<int DIM> inline
itk::ContinuousIndex<REAL,DIM>
	MakeContinuousIndex(REAL v0, REAL v1, REAL v2)
{
	itk::ContinuousIndex<REAL,DIM> v;
	v[0] = v0;
	v[1] = v1;
	v[2] = v2;
	for (int nC = 3; nC < DIM; nC++) v[nC] = 0.0;
	return v;
}

//////////////////////////////////////////////////////////////////////
template<int DIM> inline
itk::Vector<REAL,DIM>
	MakeVector(REAL v0, REAL v1, REAL v2)
{
	itk::Vector<REAL,DIM> v;
	v[0] = v0;
	v[1] = v1;
	v[2] = v2;
	for (int nC = 3; nC < DIM; nC++) v[nC] = 0.0;
	return v;
}

//////////////////////////////////////////////////////////////////////
template<int DIM> inline 
itk::Vector<REAL, DIM> 
	MakeVector(const CPoint& pt)
{
	itk::Vector<REAL, DIM> v;
	v[0] = pt.x;
	v[1] = pt.y;
	for (int nC = 2; nC < DIM; nC++) v[nC] = 0.0;
	return v;
}

//////////////////////////////////////////////////////////////////////
template<int DIM> inline 
itk::Point<REAL, DIM> 
	MakePoint(const itk::Vector<REAL, DIM>& v)
{
	itk::Point<REAL, DIM> pt;
	for (int nC = 0; nC < DIM; nC++)
		pt[nC] = v[nC];
	return pt;
}

#ifdef USE_IPP
//////////////////////////////////////////////////////////////////////
template<int DIM>
IppiSize MakeIppiSize(const itk::ImageRegion<DIM>& forRegion)
{
	IppiSize sz = { forRegion.GetSize()[0], forRegion.GetSize()[1] };
	return sz;
}

//////////////////////////////////////////////////////////////////////
template<int DIM>
IppiRect MakeIppiRect(const itk::ImageRegion<DIM>& forRegion)
{
	IppiRect rect = { forRegion.GetIndex()[0], forRegion.GetIndex()[1],
		forRegion.GetSize()[0], forRegion.GetSize()[1] };
	return rect;
}
#endif

//////////////////////////////////////////////////////////////////////
template<int DIM> inline
bool 
	IsApproxEqual(const itk::FixedArray<REAL, DIM> v1, 
				const itk::FixedArray<REAL, DIM> v2)
{
	for (int nC = 0; nC < DIM; nC++) 
		if (!IsApproxEqual(v1[nC], v2[nC]))
			return false;

	return true;
}

//////////////////////////////////////////////////////////////////////
template<int DIM> inline 
void MultHG(const itk::Matrix<REAL, DIM+1, DIM+1>& mHG, 
		const itk::Vector<REAL, DIM>& vRight,
		itk::Vector<REAL, DIM>& vProd_out)
{
	itk::Vector<REAL, DIM+1> vRightHG;
	for (int nR = 0; nR < DIM; nR++) 
		vRightHG[nR] = vRight[nR];
	vRightHG[DIM] = 1.0;
	
	itk::Vector<REAL, DIM+1> vProdHG = mHG * vRightHG;
	for (int nR = 0; nR < DIM; nR++) 
		vProd_out[nR] = vProdHG[nR] / vProdHG[DIM];
}

//////////////////////////////////////////////////////////////////////
template<int DIM> inline 
void 
	CalcBasis(const itk::ImageBase<DIM> *pVol, itk::Matrix<REAL, DIM+1, DIM+1>& mBasis)
{
	mBasis.SetIdentity();

	const itk::ImageBase<DIM>::PointType& vOrigin = pVol->GetOrigin();
	for (int nR = 0; nR < DIM; nR++) 
		mBasis(nR, DIM) = vOrigin[nR];

	const itk::ImageBase<DIM>::SpacingType& vSpacing = pVol->GetSpacing();
	const itk::ImageBase<DIM>::DirectionType& mDir = pVol->GetDirection();
	for (int nR = 0; nR < DIM; nR++)
		for (int nC = 0; nC < DIM; nC++)
			mBasis(nR, nC) = mDir(nR, nC) * vSpacing[nC];
}


//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> inline
CArchive& 
	operator<<(CArchive &ar, const itk::Point<TYPE, DIM>& v)
	// vector serialization
{
	// serialize the individual row vectors
	for (int nC = 0; nC < DIM; nC++)
	{
		ar << v[nC];
	}

	// return the archive object
	return ar;

}	// operator<<

//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> inline
CArchive& 
	operator>>(CArchive &ar, itk::Point<TYPE, DIM>& v)
	// vector serialization
{
	// serialize the individual row vectors
	for (int nC = 0; nC < DIM; nC++)
	{
		ar >> v[nC];
	}

	// return the archive object
	return ar;

}	// operator>>

//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> inline
CArchive& 
	operator<<(CArchive &ar, const itk::Vector<TYPE, DIM>& v)
	// vector serialization
{
	// serialize the individual row vectors
	for (int nC = 0; nC < DIM; nC++)
	{
		ar << v[nC];
	}

	// return the archive object
	return ar;

}	// operator<<

//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> inline
CArchive& 
	operator>>(CArchive &ar, itk::Vector<TYPE, DIM>& v)
	// vector serialization
{
	// serialize the individual row vectors
	for (int nC = 0; nC < DIM; nC++)
	{
		ar >> v[nC];
	}

	// return the archive object
	return ar;

}	// operator>>

//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> inline
CArchive& 
	operator<<(CArchive &ar, const itk::Matrix<TYPE, DIM, DIM>& m)
	// matrix serialization
{
	// serialize the individual row vectors
	for (int nC = 0; nC < DIM; nC++)
	{
		for (int nR = 0; nR < DIM; nR++)
		{
			ar << m(nR, nC);
		}
	}

	// return the archive object
	return ar;

}	// operator<<


//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> inline
CArchive& 
	operator>>(CArchive &ar, itk::Matrix<TYPE, DIM, DIM>& m)
	// matrix serialization
{
	// serialize the individual row vectors
	for (int nC = 0; nC < DIM; nC++)
	{
		for (int nR = 0; nR < DIM; nR++)
		{
			ar >> m(nR, nC);
		}
	}

	// return the archive object
	return ar;

}	// operator>>


//////////////////////////////////////////////////////////////////////
template<int DIM> inline
CArchive& 
	operator<<(CArchive &ar, const itk::ImageRegion<DIM>& region)
	// region serialization
{
	// serialize the individual elements
	for (int nC = 0; nC < DIM; nC++)
	{
		ar << region.GetIndex()[nC];
		ar << region.GetSize()[nC];
	}
	
	// return the archive object
	return ar;

}	// operator<<


//////////////////////////////////////////////////////////////////////
template<int DIM> inline
CArchive& 
	operator>>(CArchive &ar, itk::ImageRegion<DIM>& region)
	// region serialization
{
	itk::ImageRegion<DIM>::IndexType index;
	itk::ImageRegion<DIM>::SizeType size;
	// serialize the individual elements
	for (int nC = 0; nC < DIM; nC++)
	{
		ar >> index[nC];
		ar >> size[nC];
	}

	region.SetIndex(index);
	region.SetSize(size);

	// return the archive object
	return ar;

}	// operator>>


//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE, int DIM> INLINE
void 
	CopyImage(const itk::Image<VOXEL_TYPE,DIM> *pFrom, itk::Image<VOXEL_TYPE,DIM> *pTo)
{
	ConformTo<VOXEL_TYPE,DIM>(pFrom, pTo);
	CopyValues<VOXEL_TYPE>(pTo->GetBufferPointer(), pFrom->GetBufferPointer(),
		pFrom->GetBufferedRegion().GetNumberOfPixels());
}

//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE, int DIM> INLINE
void 
	ConformTo(const itk::ImageBase<DIM> *pFrom, itk::Image<VOXEL_TYPE,DIM> *pTo)
{
	if (pTo->GetLargestPossibleRegion() != pFrom->GetLargestPossibleRegion()
		|| pTo->GetBufferedRegion() != pFrom->GetBufferedRegion())
	{
		pTo->SetLargestPossibleRegion(pFrom->GetLargestPossibleRegion());
		pTo->SetBufferedRegion(pFrom->GetBufferedRegion());
		pTo->Allocate();
	}

    pTo->SetRequestedRegion(pFrom->GetRequestedRegion());

	pTo->SetOrigin(pFrom->GetOrigin());
	pTo->SetSpacing(pFrom->GetSpacing());
	pTo->SetDirection(pFrom->GetDirection());
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

template<class voxel_type> inline
int 
	StepSize(int nwidth)
{
	return nwidth * sizeof(voxel_type);
}


////////////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE> INLINE
bool 
	DivVoxels(VOXEL_TYPE *pVoxelsLDiv, int nWidthLDiv,
			const VOXEL_TYPE *pVoxelsL, int nWidthL,
			const itk::Size<3>& vSize);
			// const CExtent<3,int>& extent);

template<> INLINE 
bool 
	DivVoxels(float *pVoxelsLDiv, int nWidthLDiv,
			const float *pVoxelsR, int nWidthR,
			const itk::Size<3>& vSize)
{
	int nStrideZ = vSize[0] * vSize[1];
	itk::Vector<int,3> vN;
	for (vN[2] = 0; vN[2] < vSize[2]; vN[2]++)
	{
		for (vN[1] = 0; vN[1] < vSize[1]; vN[1]++)
		{
			for (vN[0] = 0; vN[0] < vSize[0]; vN[0]++)
			{
				if (pVoxelsR[vN[2] * nStrideZ + vN[1] * nWidthR + vN[0]] > 1e-8)
					pVoxelsLDiv[vN[2] * nStrideZ + vN[1] * nWidthR + vN[0]] /= 
						pVoxelsR[vN[2] * nStrideZ + vN[1] * nWidthR + vN[0]];
			}
		}
	}

#ifdef USE_IPP_DIVVOXELS
	IppiSize sz;
	sz.width = vSize[0];
	sz.height = vSize[1];

	IppStatus stat = ippiDiv_32f_C1IR
	(
		&pVoxelsR[0/*UpperLeft(extent, nWidthR)*/], StepSize<Ipp32f>(nWidthR), 
		&pVoxelsLDiv[0/*UpperLeft(extent, nWidthLDiv)*/], StepSize<Ipp32f>(nWidthLDiv), 
		sz // (IppiSize) extent
	);

	if (stat == ippStsDivByZero)
	{
		Ipp32f *pCurrVoxel = &pVoxelsLDiv[0/*UpperLeft(extent, nWidthLDiv)*/];
		for (int nY = 0/*extent.GetMin()[1]*/; nY < vSize/*extent.GetMax()*/[1]; nY++)
		{
			for (int nX = 0/*extent.GetMin()[0]*/; nX < vSize/*extent.GetMax()*/[0]; nX++)
			{
				if (!_finite(pCurrVoxel[nX]))
				{
					pCurrVoxel[nX] = 0.0f;
				}
			}
			pCurrVoxel += nWidthLDiv;
		}
	}
#endif

	return true;

}	// DivVoxels


////////////////////////////////////////////////////////////////////////////
// VoxelSum
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE> INLINE  
bool
	VoxelSum(VOXEL_TYPE *pSum, 
			const VOXEL_TYPE *pVoxels, int nWidth, 
			const itk::Size<3>& vSize)
{
	VOXEL_TYPE sum = (VOXEL_TYPE) 0.0;
	const VOXEL_TYPE *pCurrVoxel = &pVoxels[0];
	int nStrideZ = vSize[0] * vSize[1];
	itk::Vector<int,3> vN;
	for (vN[2] = 0; vN[2] < vSize[2]; vN[2]++)
	{
		for (vN[1] = 0; vN[1] < vSize[1]; vN[1]++)
		{
			for (vN[0] = 0; vN[0] < vSize[0]; vN[0]++)
			{
				sum += pCurrVoxel[vN[2] * nStrideZ + vN[1] * vSize[0] + vN[0]];
			}
		}
	}
#ifdef NEVER
	for (int nY = 0/*extent.GetMin()[1]*/; nY < vSize[1]/*extent.GetMax()[1]*/; nY++)
	{
		for (int nX = 0/*extent.GetMin()[0]*/; nX < vSize[0]/*extent.GetMax()[0]*/; nX++)
		{
			sum += pCurrVoxel[nX];
		}
		pCurrVoxel += nWidth;
	}
#endif

	(*pSum) = (VOXEL_TYPE) sum;

	return true;
}

//template<> INLINE  
//bool
//	VoxelSum(Ipp32f *pSum,
//			const Ipp32f *pVoxels, int nWidth, 
//			const CExtent<3,int>& extent)
//{
//	Ipp64f sum;
//	CK_IPP(		ippiSum_32f_C1R
//	(
//		&pVoxels[UpperLeft(extent, nWidth)], StepSize<Ipp32f>(nWidth),
//		(IppiSize) extent, 
//		&sum, ippAlgHintAccurate /* ippAlgHintFast */
//	));
//
//	(*pSum) = (Ipp32f) sum;
//
//	return true;
//
//}	// VoxelSum


////////////////////////////////////////////////////////////////////////////
// VoxelMin
////////////////////////////////////////////////////////////////////////////

#ifdef NEVER
////////////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE> INLINE  
bool
	VoxelMin(VOXEL_TYPE *pMin, 
			const VOXEL_TYPE *pVoxels, int nWidth, 
			const itk::Size<3>& vSize)
			// const C VectorD<3,int>& vSize)
			//const CExtent<3,int>& extent)
{
	(*pMin) = numeric_limits<VOXEL_TYPE>::max();
	itk::Vector<int,3> vN;
	// for (vN[2] = extent.GetMin()[2]; vN[2] < extent.GetMax()[2]; vN[2]++)
	for (vN[2] = 0/*extent.GetMin()[2]*/; vN[2] < vSize/*extent.GetMax()*/[2]; vN[2]++)
	{
		// for (vN[1] = extent.GetMin()[1]; vN[1] < extent.GetMax()[1]; vN[1]++)
		for (vN[1] = 0/*extent.GetMin()[1]*/; vN[1] < vSize/*extent.GetMax()*/[1]; vN[1]++)
		{
			// for (vN[0] = extent.GetMin()[0]; vN[0] < extent.GetMax()[0]; vN[0]++)
			for (vN[0] = 0/*extent.GetMin()[0]*/; vN[0] < vSize/*extent.GetMax()*/[0]; vN[0]++)
			{
				(*pMin) = __min((*pMin), pVoxels[vN[1] * nWidth + vN[0]]);
			}
		}
	}
	return true;
}
#endif

//template<> INLINE  
//bool
//	VoxelMin(Ipp32f *pMin, 
//			const Ipp32f *pVoxels, int nWidth, 
//			const CExtent<3,int>& extent)
//{
//	CK_IPP( ippiMin_32f_C1R
//	(
//		&pVoxels[UpperLeft(extent, nWidth)], StepSize<Ipp32f>(nWidth),
//		(IppiSize) extent, 
//		pMin
//	));
//
//	return true;
//
//}	// VoxelMin

////////////////////////////////////////////////////////////////////////////
// VoxelMax
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE> INLINE  
bool
	VoxelMax(VOXEL_TYPE *pMax,
			const VOXEL_TYPE *pVoxels, int nWidth, 
			const itk::Size<3>& vSize)
			// const C VectorD<3,int>& vSize)
			//const CExtent<3,int>& extent)
{
	(*pMax) = -numeric_limits<VOXEL_TYPE>::max();
	int nStrideZ = vSize[0] * vSize[1];
	itk::Vector<int,3> vN;
	//for (vN[2] = extent.GetMin()[2]; vN[2] < extent.GetMax()[2]; vN[2]++)
	for (vN[2] = 0/*extent.GetMin()[2]*/; vN[2] < vSize/*extent.GetMax()*/[2]; vN[2]++)
	{
		//for (vN[1] = extent.GetMin()[1]; vN[1] < extent.GetMax()[1]; vN[1]++)
		for (vN[1] = 0/*extent.GetMin()[1]*/; vN[1] < vSize/*extent.GetMax()*/[1]; vN[1]++)
		{
			//for (vN[0] = extent.GetMin()[0]; vN[0] < extent.GetMax()[0]; vN[0]++)
			for (vN[0] = 0/*extent.GetMin()[0]*/; vN[0] < vSize/*extent.GetMax()*/[0]; vN[0]++)
			{
				(*pMax) = __max((*pMax), pVoxels[vN[2] * nStrideZ + vN[1] * nWidth + vN[0]]);
			}
		}
	}

	return true;
}

//template<> INLINE  
//bool
//	VoxelMax(Ipp32f *pMax, 
//			const Ipp32f *pVoxels, int nWidth, 
//			const CExtent<3,int>& extent)
//{
//	CK_IPP( ippiMax_32f_C1R
//	(
//		&pVoxels[UpperLeft(extent, nWidth)], StepSize<Ipp32f>(nWidth),
//		(IppiSize) extent, 
//		pMax
//	));
//
//	return true;
//
//}	// VoxelMax



//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE> INLINE
VOXEL_TYPE 
	GetSum(const itk::Image<VOXEL_TYPE,3> *pVol)
	// forms the sum of the volume
{
	VOXEL_TYPE sum;

	VoxelSum<VOXEL_TYPE>(&sum, 
		pVol->GetBufferPointer(),
		pVol->GetBufferedRegion().GetSize()[0], 
		pVol->GetBufferedRegion().GetSize());

	return sum;

}	// GetSum

#ifdef NEVER
//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE> INLINE 
VOXEL_TYPE 
	GetMin(const itk::Image<VOXEL_TYPE,3> *pVol)
	// forms the min of the volume voxel values
{
	VOXEL_TYPE min;
	VoxelMin<VOXEL_TYPE>(&min, 
		pVol->GetBufferPointer(),
		pVol->GetBufferedRegion().GetSize()[0], 
		pVol->GetBufferedRegion().GetSize());

	return min;

}	// GetMin
#endif

//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE> INLINE
VOXEL_TYPE 
	GetMax(const itk::Image<VOXEL_TYPE,3> *pVol) 
	// forms the max of the volume voxel values
{
	VOXEL_TYPE max;
	VoxelMax<VOXEL_TYPE>(&max, 
		pVol->GetBufferPointer(),
		pVol->GetBufferedRegion().GetSize()[0], 
		pVol->GetBufferedRegion().GetSize()); 

	return max;

}	// GetMax

//////////////////////////////////////////////////////////////////////
INLINE REAL 
	GetSliceMax(VolumeReal *pVol, int nSlice)
{
	typedef itk::ImageRegionConstIterator< VolumeReal > ConstIteratorType;
	typedef itk::ImageRegionIterator< VolumeReal > IteratorType;

	VolumeReal::RegionType inputRegion;
	VolumeReal::RegionType::IndexType inputStart = pVol->GetBufferedRegion().GetIndex();
	inputStart[0] = 0;
	inputStart[1] = 0;
	inputStart[2] = nSlice;

	VolumeReal::RegionType::SizeType size = pVol->GetBufferedRegion().GetSize();
	size[2] = 1;

	inputRegion.SetSize( size );
	inputRegion.SetIndex( inputStart );

	ConstIteratorType inputIt( pVol, inputRegion );

	REAL maxVoxel = -1e+6;
	for ( inputIt.GoToBegin(); !inputIt.IsAtEnd(); ++inputIt)
	{
		maxVoxel = __max(inputIt.Get(), maxVoxel);
	}

	return maxVoxel;
}

//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE, int DIM> INLINE
int 
	Stride(const itk::Image<VOXEL_TYPE,DIM> *pImage)
{
	return pImage->GetBufferedRegion().GetSize()[0] * sizeof(VOXEL_TYPE);
}

//
////////////////////////////////////////////////////////////////////////
//template<class VOXEL_TYPE> INLINE
//void Accumulate(const VolumeReal *pVolume, 
//				double weight,
//				VolumeReal *pSrcDst,
//				VolumeReal *pAccum,
//				int nSlice = 0)
//	// accumulates voxel values -- other volume must be conformant
//{
//	ASSERT(pSrcDst->GetBufferedRegion().GetSize() == pVolume->GetBufferedRegion().GetSize());
//
//	int nStrideZ = pSrcDst->GetBufferedRegion().GetSize()[0]
//		* pSrcDst->GetBufferedRegion().GetSize()[1];
//
//	IppStatus stat = ippiMulC_32f_C1R(
//		&pVolume->GetBufferPointer()[nStrideZ * nSlice], Stride<VOXEL_REAL>(pVolume), 
//		(VOXEL_REAL) weight,
//		&pAccum->GetBufferPointer()[nStrideZ * nSlice], Stride<VOXEL_REAL>(pAccum), 
//		MakeIppiSize(pVolume->GetBufferedRegion())); 
//
//	stat = ippiAdd_32f_C1IR(
//		&pAccum->GetBufferPointer()[nStrideZ * nSlice], Stride<VOXEL_REAL>(pAccum), 
//		&pSrcDst->GetBufferPointer()[nStrideZ * nSlice], Stride<VOXEL_REAL>(pSrcDst), 
//		MakeIppiSize(pVolume->GetBufferedRegion()));
//
//	//IppStatus stat = ippiMulC_32f_C1R(
//	//	pVolume->GetBufferPointer(), Stride<VOXEL_REAL>(pVolume), 
//	//	(VOXEL_REAL) weight,
//	//	pAccum->GetBufferPointer(), Stride<VOXEL_REAL>(pAccum), 
//	//	MakeIppiSize(pVolume->GetBufferedRegion())); 
//
//	//stat = ippiAdd_32f_C1IR(
//	//	pAccum->GetBufferPointer(), Stride<VOXEL_REAL>(pAccum), 
//	//	pSrcDst->GetBufferPointer(), Stride<VOXEL_REAL>(pSrcDst), 
//	//	MakeIppiSize(pVolume->GetBufferedRegion()));
//
//}	// Accumulate

//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE> INLINE
void Accumulate3D(const VolumeReal *pVolume, 
				double weight,
				VolumeReal *pSrcDst,
				VolumeReal *pAccum)
	// accumulates voxel values -- other volume must be conformant
{
	ASSERT(pSrcDst->GetBufferedRegion().GetSize() == pVolume->GetBufferedRegion().GetSize());

	typedef itk::ImageRegionConstIterator< VolumeReal > ConstIteratorType;
	typedef itk::ImageRegionIterator< VolumeReal > IteratorType;

	IteratorType srcDstIt( pSrcDst, pSrcDst->GetBufferedRegion() );
	ConstIteratorType volIt( pVolume, pVolume->GetBufferedRegion() );
	for ( srcDstIt.GoToBegin(), volIt.GoToBegin(); 
		!srcDstIt.IsAtEnd(); ++srcDstIt, ++volIt)
	{
		srcDstIt.Set(srcDstIt.Get() + weight*volIt.Get());
	}

#ifdef USE_IPP_ACCUMULATE
	int nStrideZ = pSrcDst->GetBufferedRegion().GetSize()[0]
		* pSrcDst->GetBufferedRegion().GetSize()[1];
	for (int nPlane = 0; nPlane < pSrcDst->GetBufferedRegion().GetSize()[2]; nPlane++)
	{
		IppStatus stat = ippiMulC_32f_C1R(
			&pVolume->GetBufferPointer()[nStrideZ * nPlane], Stride<VOXEL_REAL>(pVolume), 
			(VOXEL_REAL) weight,
			&pAccum->GetBufferPointer()[nStrideZ * nPlane], Stride<VOXEL_REAL>(pAccum), 
			MakeIppiSize(pVolume->GetBufferedRegion())); 

		stat = ippiAdd_32f_C1IR(
			&pAccum->GetBufferPointer()[nStrideZ * nPlane], Stride<VOXEL_REAL>(pAccum), 
			&pSrcDst->GetBufferPointer()[nStrideZ * nPlane], Stride<VOXEL_REAL>(pSrcDst), 
			MakeIppiSize(pVolume->GetBufferedRegion()));
	}
#endif

}	// Accumulate

#ifdef DEPRECATED
///////////////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
inline void Convolve(const itk::Image<VOXEL_TYPE,3> *pVol, 
										 const itk::Image<VOXEL_TYPE,3> *pKernel, 
										 itk::Image<VOXEL_TYPE,3> *pRes)
{
	int nKernelBase = pKernel->GetBufferedRegion().GetSize()[1] / 2;
	for (int nAtRow = 0; nAtRow < pVol->GetBufferedRegion().GetSize()[1]; nAtRow++)
	{
		for (int nAtCol = 0; nAtCol < pVol->GetBufferedRegion().GetSize()[0]; nAtCol++)
		{
			for (int nAtKernRow = -nKernelBase; nAtKernRow <= nKernelBase; nAtKernRow++)
			{
				for (int nAtKernCol = -nKernelBase; nAtKernCol <= nKernelBase; nAtKernCol++)
				{
					if (nAtRow + nAtKernRow >= 0 
						&& nAtRow + nAtKernRow < pRes->GetBufferedRegion().GetSize()[1]
						&& nAtCol + nAtKernCol >= 0
						&& nAtCol + nAtKernCol < pRes->GetBufferedRegion().GetSize()[0])
					{
						int nAtResPixel = 
							(nAtRow + nAtKernRow) * pRes->GetBufferedRegion().GetSize()[0]
								+ (nAtCol + nAtKernCol);
						int nAtVolPixel = 
							(nAtRow) * pVol->GetBufferedRegion().GetSize()[0]
								+ (nAtCol);
						int nAtKernPixel = 
							(nKernelBase + nAtKernRow) * pKernel->GetBufferedRegion().GetSize()[0]
								+ (nKernelBase + nAtKernCol);

						pRes->GetBufferPointer()[nAtResPixel] +=
							pVol->GetBufferPointer()[nAtVolPixel]
								* pKernel->GetBufferPointer()[nAtKernPixel];
					}
				}
			}
		}
	}    

}	// Convolve
#endif

#ifdef DEPRECATED
///////////////////////////////////////////////////////////////////////////////
template<class TYPE>
inline void CalcBinomialFilter(itk::Image<TYPE,3> *pVol)
{
	// find largest dimension
	int nMaxDim = __max(pVol->GetBufferedRegion().GetSize()[2], 
		pVol->GetBufferedRegion().GetSize()[1]);

	// calc coeffecients
	CVectorN<TYPE> vCoeff;
	vCoeff.SetDim(nMaxDim);
	CalcBinomialCoeff(vCoeff);

	// stores the normalization constant
	TYPE norm = 0.0;

	// populate volume
	pVol->FillBuffer(0.0); 
	int nStride = pVol->GetBufferedRegion().GetSize()[0]; 
	for (int nAtRow = 0; nAtRow < pVol->GetBufferedRegion().GetSize()[1]; nAtRow++)
	{
		for (int nAtCol = 0; nAtCol < vCoeff.GetDim(); nAtCol++)
		{
			pVol->GetBufferPointer()[nAtRow * nStride + nAtCol]
			// (*pVol)[0][nAtRow][nAtCol] 
				= vCoeff[nAtRow] * vCoeff[nAtCol];

			norm += vCoeff[nAtRow] * vCoeff[nAtCol];
		}
	}

	// now normalize
	for (int nAtRow = 0; nAtRow < pVol->GetBufferedRegion().GetSize()[1]; nAtRow++)
	{
		for (int nAtCol = 0; nAtCol < vCoeff.GetDim(); nAtCol++)
		{
			// (*pVol)[0][nAtRow][nAtCol] 
			pVol->GetBufferPointer()[nAtRow * nStride + nAtCol] /= norm;
		}
	}

}	// CalcBinomialFilter
#endif

//#ifdef USE_IPP

//
/////////////////////////////////////////////////////////////////////////////////
//inline void Resample3D_(const VolumeReal *pOrig, 
//										 VolumeReal *pNew, 
//										 BOOL bBilinear = FALSE)
//{
//	itk::Matrix<REAL, 4, 4> mBasisOrig;
//	CalcBasis<3>(pOrig, mBasisOrig);
//
//	itk::Matrix<REAL, 4, 4> mBasisNew;
//	CalcBasis<3>(pNew, mBasisNew);
//
//	itk::Matrix<REAL, 4, 4> mXform = mBasisOrig.GetInverse();
//	mXform *= mBasisNew;
//
//	for (int nPlane = 0; nPlane < pNew->GetBufferedRegion().GetSize()[2]; nPlane++)
//	{
//		// calculate plane
//		REAL planeZ =
//			(pNew->GetOrigin()[2] + pNew->GetSpacing()[2] * nPlane - pOrig->GetOrigin()[2]) / pOrig->GetSpacing()[2];
//		// REAL planeZ = // (pNew->GetOrigin()[2] - pOrig->GetOrigin()[2]) / pOrig->GetSpacing()[2];
//		//	(pNew->GetOrigin()[2] + pNew->GetSpacing()[2] * nPlane - pOrig->GetOrigin()[2]) / pOrig->GetSpacing()[2];
//		// calculate original voxel starting position
//		const VOXEL_REAL *pOrigVoxel = pOrig->GetBufferPointer();
//		VolumeReal::IndexType idx;
//		idx[0] = 0;
//		idx[1] = 0;
//		idx[2] = // nPlane; // 
//			::Round<int>(planeZ);
//		if (!pOrig->GetBufferedRegion().IsInside(idx))
//		{
//			return;
//		}
//		pOrigVoxel += pOrig->ComputeOffset(idx);
//
//		VOXEL_REAL *pNewVoxel = pNew->GetBufferPointer();
//		idx[2] = nPlane;
//		pNewVoxel += pNew->ComputeOffset(idx);
//
//		double coeffs[2][3];
//		coeffs[0][0] = mXform(0, 0);
//		coeffs[0][1] = mXform(0, 1);
//		coeffs[0][2] = mXform(0, 3);
//		coeffs[1][0] = mXform(1, 0);
//		coeffs[1][1] = mXform(1, 1);
//		coeffs[1][2] = mXform(1, 3);
//
//		IppStatus stat = ippiWarpAffineBack_32f_C1R(
//			pOrigVoxel, 
//			MakeIppiSize(pOrig->GetBufferedRegion()),
//			pOrig->GetBufferedRegion().GetSize()[0] * sizeof(VOXEL_REAL), 
//			MakeIppiRect(pOrig->GetBufferedRegion()),
//			pNewVoxel, // pNew->GetBufferPointer(), 
//			pNew->GetBufferedRegion().GetSize()[0] * sizeof(VOXEL_REAL), 
//			MakeIppiRect(pNew->GetBufferedRegion()),
//			coeffs, IPPI_INTER_LINEAR);
//	}
//
//}	// Resample3D
//#endif

#ifdef DEPRECATED
///////////////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
inline void Decimate(const itk::Image<VOXEL_TYPE,3> *pVol, 
										 itk::Image<VOXEL_TYPE,3> *pRes)
{
	// set new dimensions
	//int nBase = pVol->m_VolumePtr->GetBufferedRegion().GetSize()[1] / 2;
	//pRes->SetSize(MakeSize(nBase, nBase, 1)); 
	//pRes->ClearVoxels();

	pRes->SetOrigin(pVol->GetOrigin());

	itk::Image<VOXEL_TYPE,3>::SpacingType spacing = pVol->GetSpacing();
	spacing *= 2.0;
	pRes->SetSpacing(spacing);

	pRes->SetDirection(pVol->GetDirection());

	// now copy voxels
	for (int nAtRow = 0; nAtRow < pVol->GetBufferedRegion().GetSize()[1] / 2; nAtRow++)
	{
		for (int nAtCol = 0; nAtCol < pVol->GetBufferedRegion().GetSize()[0] / 2; nAtCol++)
		{
			int nResPixel = (nAtRow) * pRes->GetBufferedRegion().GetSize()[0]
				+ nAtCol;
			int nVolPixel = (nAtRow*2) * pVol->GetBufferedRegion().GetSize()[0]
				+ (nAtCol*2);
			// (*pRes)[0][nAtRow][nAtCol] = 
			pRes->GetBufferPointer()[nResPixel] = 
				// (*pVol)[0][nAtRow*2][nAtCol*2];
				pVol->GetBufferPointer()[nVolPixel];
		}
	}

}	// Decimate
#endif

//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE> INLINE
void SerializeImage(CArchive& ar, itk::Image<VOXEL_TYPE,2> *pImage)
{
	if (ar.IsStoring())
	{
		// serialize geometry
		ar << pImage->GetOrigin();
		ar << pImage->GetSpacing();
		ar << pImage->GetDirection();

		// serialize region
		ar << pImage->GetBufferedRegion();

		// serialize buffer
		UINT nBytes = pImage->GetPixelContainer()->Size() * sizeof(VOXEL_TYPE);
		ar.Write(pImage->GetBufferPointer(), nBytes);
	}
	else
	{
		typedef itk::Image<VOXEL_TYPE,2> ImageType;

		// serialize geometry

		ImageType::PointType origin;
		ar >> origin;
		pImage->SetOrigin(origin);

		ImageType::SpacingType spacing;
		ar >> spacing;
		pImage->SetSpacing(spacing);

		ImageType::DirectionType direction;
		ar >> direction;
		pImage->SetDirection(direction);

		// serialize region

		ImageType::RegionType region;
		ar >> region;
		pImage->SetRegions(region);
		pImage->Allocate();

		// serialize buffer

		UINT nBytes = pImage->GetPixelContainer()->Size() * sizeof(VOXEL_TYPE);
		UINT nActBytes = ar.Read(pImage->GetBufferPointer(), nBytes);
		ASSERT(nActBytes == nBytes);
	}
}

//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE> INLINE
void SerializeVolume(CArchive& ar, itk::Image<VOXEL_TYPE,3> *pVolume)
{
	if (ar.IsStoring())
	{
		itk::Matrix<REAL, 4, 4> mBasis;
		CalcBasis<3>(pVolume, mBasis);
		ar << mBasis;

		itk::Size<3> sz = pVolume->GetBufferedRegion().GetSize();
		ar << (int) sz[0]; 
		ar << (int) sz[1]; 
		ar << (int) sz[2]; 

		UINT nBytes = sz[2] * sz[1] * sz[0] * sizeof(VOXEL_TYPE);
		ar.Write(pVolume->GetBufferPointer(), nBytes);
	}
	else
	{
		itk::Matrix<REAL, 4, 4> mBasis;
		ar >> mBasis;

		itk::Image<VOXEL_TYPE,3>::PointType origin;
		origin[0] = mBasis(0, 3);
		origin[1] = mBasis(1, 3);
		origin[2] = mBasis(2, 3);
		pVolume->SetOrigin(origin);

		// VolumeType::DirectionType 
		itk::Matrix<double, 3, 3> mDir;
		itk::Image<VOXEL_TYPE,3>::SpacingType spacing;
		for (int nCol = 0; nCol < 3; nCol++)
		{
			itk::Vector<REAL> vDir;
			vDir[0] = mBasis(0,nCol);
			vDir[1] = mBasis(1,nCol);
			vDir[2] = mBasis(2,nCol);
			spacing[nCol] = vDir.GetNorm();

			vDir.Normalize();
			mDir(0,nCol) = vDir[0];
			mDir(1,nCol) = vDir[1];
			mDir(2,nCol) = vDir[2];
		}
		pVolume->SetSpacing(spacing);
		pVolume->SetDirection(mDir);

		// get size as 3 ints
		itk::Vector<int, 3> m_vSize;
		ar >> m_vSize[0];
		ar >> m_vSize[1];
		ar >> m_vSize[2];

		// set size as an ItkSize
		pVolume->SetRegions( MakeSize(m_vSize[0], m_vSize[1], m_vSize[2]) );
		pVolume->Allocate();

		UINT nBytes = m_vSize[2] * m_vSize[1] * m_vSize[0] * sizeof(VOXEL_TYPE);
		UINT nActBytes = ar.Read(pVolume->GetBufferPointer(), nBytes);
		ASSERT(nActBytes == nBytes);
	}
}

//////////////////////////////////////////////////////////////////////
template<int DIM> INLINE
void SerializePolyLine(CArchive& ar,
					   itk::PolyLineParametricPath<DIM> *pPoly)
{
	typedef itk::PolyLineParametricPath<DIM>::VertexType VertexType;

	CMatrixNxM<VertexType::CoordRepType> m_mVertex;

	if (ar.IsStoring())
	{
		const itk::PolyLineParametricPath<DIM>::VertexListType *pVertList = pPoly->GetVertexList();
		m_mVertex.Reshape(pVertList->size(), DIM, FALSE);
		for (int nVert = 0; nVert < pVertList->size(); nVert++)
		{
			for (int nCoord = 0; nCoord < DIM; nCoord++)
				m_mVertex[nVert][nCoord] = pVertList->at(nVert)[nCoord];
		}
	}

	SerializeValue(ar, m_mVertex);

	if (ar.IsLoading())
	{
		// clears any existing vertices
		pPoly->Initialize();
		for (int nVert = 0; nVert < m_mVertex.GetCols(); nVert++)
		{
			VertexType vertex;
			for (int nCoord = 0; nCoord < DIM; nCoord++)
				vertex[nCoord] = m_mVertex[nVert][nCoord];
			pPoly->AddVertex(vertex);
		}
	}
}