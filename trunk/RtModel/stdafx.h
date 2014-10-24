// Copyright (C) 2nd Messenger Systems
// $Id: stdafx.h 635 2009-06-11 04:23:03Z dglane001 $
#if !defined(AFX_STDAFX_H__C1C1F8E2_EE56_11D4_AF87_00C0F05A20CE__INCLUDED_)
#define AFX_STDAFX_H__C1C1F8E2_EE56_11D4_AF87_00C0F05A20CE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#ifndef WINVER				// Allow use of features specific to Windows 95 and Windows NT 4 or later.
#define WINVER 0x0501		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

// MFC includes
#include <afx.h>
#include <afxwin.h>
#include <afxdisp.h>
#include <afxtempl.h>
#include <atlcoll.h>

// math include
#include <math.h>

// utility macros
#include <UtilMacros.h>

// MTL includes
#include <MatrixNxM.h>

// for VectorD conversions
#include <itkVector.h>

// geom includes
#include <ItkUtils.h>

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__C1C1F8E2_EE56_11D4_AF87_00C0F05A20CE__INCLUDED_)
