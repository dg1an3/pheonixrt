// Copyright (C) 2nd Messenger Systems
// $Id: Series.h 640 2009-06-13 05:06:50Z dglane001 $
#if !defined(AFX_SERIES_H__731877C4_EE46_11D4_9E36_00B0D0609AB0__INCLUDED_)
#define AFX_SERIES_H__731877C4_EE46_11D4_9E36_00B0D0609AB0__INCLUDED_

#pragma once

#include <Structure.h>

///////////////////////////////////////////////////////////////////////////////
// class CSeries
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
class CSeries : public CModelObject
{
public:
	CSeries();          
	virtual ~CSeries();

#ifdef USE_MFC_SERIALIZATION
	DECLARE_SERIAL(CSeries)
	virtual void Serialize(CArchive& ar);
#endif

	// Structures for the series
	int GetStructureCount() const;
	dH::Structure *GetStructureAt(int nAt);
	dH::Structure * GetStructureFromName(const CString& strName);
	void AddStructure(dH::Structure *pStruct);

	// Volume data for the series
	DECLARE_ATTRIBUTE_SPTR(Density, VolumeReal);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

private:
	//CTypedPtrArray<CObArray, CStructure*> m_arrStructures;
	// the structure array
	std::vector< dH::Structure::Pointer > m_arrStructures;
};


//{{AFX_INSERT_LOCATION}	// CSeries}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SERIES_H__731877C4_EE46_11D4_9E36_00B0D0609AB0__INCLUDED_)
