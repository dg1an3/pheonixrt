//////////////////////////////////////////////////////////////////////
// ModelObject.h: interface for the CModelObject class.
//
// Copyright (C) 1999-2001
// $Id: ModelObject.h,v 1.3 2003/04/26 20:51:38 default Exp $
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MODELOBJECT_H__5BF91A87_C623_11D4_BE42_005004D16DAA__INCLUDED_)
#define AFX_MODELOBJECT_H__5BF91A87_C623_11D4_BE42_005004D16DAA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Observer.h"

//////////////////////////////////////////////////////////////////////
// class CModelObject
// 
// a model object:
//		1) fires change events observable
//		2) has a name
//		3) possibly has children
//		4) can be serialized
//////////////////////////////////////////////////////////////////////
class CModelObject : public CObject,
		public IUnknown
{
public:
	// constructors/destructors
	CModelObject(const CString& strName = "");
	virtual ~CModelObject();

	// makes the model object serializable
	DECLARE_SERIAL(CModelObject)

	// the given name for this model object
	const CString& GetName() const;
	void SetName(const CString& strName);

	// collection of the children of this model object
	int GetChildCount() const;
	CModelObject *GetChildAt(int nIndex);
	int AddChild(CModelObject *pObject);

	// returns a reference to this object's change event
	CObservableEvent& GetChangeEvent();

	// serialization
	virtual void Serialize( CArchive& ar );

// IUnknown interface methods
	STDMETHOD(QueryInterface)(REFIID, void **);   
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	// static class function to dispose of objects
	static void DisposeObjects();

protected:
	// the name of the object
	CString m_strName;

	// the model object's children
	CObArray m_arrChildren;

	// the change event for this object
	CObservableEvent m_eventChange;

	// reference count for the object
	DWORD m_dwRefCount;

	// static array of objects to be disposed of
	static CObArray m_arrDispose;
};

//////////////////////////////////////////////////////////////////////
// CModelObject::GetChangeEvent
// 
// returns a reference to this object's change event
//////////////////////////////////////////////////////////////////////
inline CObservableEvent& CModelObject::GetChangeEvent()
{
	return m_eventChange;
}

#endif // !defined(AFX_MODELOBJECT_H__5BF91A87_C623_11D4_BE42_005004D16DAA__INCLUDED_)
