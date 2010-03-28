// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#include <stdio.h>
#include <tchar.h>

#include <vector>
#include <map>
#include <algorithm>

#include <iostream>
#include <iomanip>
using namespace std;

#include "UtilMacros.h"
#include "MathUtil.h"
#include "ItkUtils.h"


#include <itkOrientedImage.h>
#include <itkImageToImageFilter.h>
#include <itkImageRegionIterator.h>
#include <itkContinuousIndex.h>
#include <itkLinearInterpolateImageFunction.h>
using namespace itk;

//#define DeclareMember(Name, Type) \
//	private: Type m_##Name; \
//	public: const Type& Get##Name() { return m_##Name; } \
//	public: void Set##Name(const Type& value) { m_##Name = value; }
//
//typedef OrientedImage<double,3> VolumeReal;


////////////////////////////////////////////////////////////////////////////////
//template<typename TCoordRep, unsigned int VDim>
//Index<VDim> 
//	MakeIndex(ContinuousIndex<TCoordRep,VDim> cIndex)
//{
//	Index<VDim> index;
//	for (int nD = 0; nD < VDim; nD++)
//	{
//		index[nD] = ceil(cIndex[nD]-0.5);
//	}
//	return index;
//}
//
////////////////////////////////////////////////////////////////////////////////
//inline Vector<double,3> 
//	MakeVector(double x, double y, double z)
//{
//	Vector<double,3> v;
//	v[0] = x;
//	v[1] = y; 
//	v[2] = z;
//
//	return v;
//}
//
////////////////////////////////////////////////////////////////////////////////
//inline Vector<double,2> 
//	MakeVector(double x, double y)
//{
//	Vector<double,2> v;
//	v[0] = x;
//	v[1] = y; 
//
//	return v;
//}
