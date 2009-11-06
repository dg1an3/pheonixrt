// Copyright (C) 2nd Messenger Systems
// $Id: ModelObject.h 637 2009-06-11 04:33:15Z dglane001 $
#if !defined(AFX_MODELOBJECT_H__5BF91A87_C623_11D4_BE42_005004D16DAA__INCLUDED_)
#define AFX_MODELOBJECT_H__5BF91A87_C623_11D4_BE42_005004D16DAA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>

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
class CModelObject : public CObject
{
public:
	// constructors/destructors
	CModelObject(const CString& strName = _T(""));
	virtual ~CModelObject();

	// makes the model object serializable
	DECLARE_SERIAL(CModelObject)

	// the given name for this model object
	const CString& GetName() const;
	void SetName(const CString& strName);

	// returns a reference to this object's change event
	CObservableEvent& GetChangeEvent();

	// serialization
	virtual void Serialize( CArchive& ar );

protected:
	// the name of the object
	CString m_strName;

	// the change event for this object
	CObservableEvent m_eventChange;
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
