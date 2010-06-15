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
class CModelObject 
#ifdef USE_MFC_SERIALIZATION
	: public CObject
#endif
{
public:
	// constructors/destructors
	CModelObject(
#ifdef USE_MODEL_NAME
		const CString& strName = _T("")
#endif
		);
	virtual ~CModelObject();

#ifdef USE_MFC_SERIALIZATION
	// makes the model object serializable
	DECLARE_SERIAL(CModelObject)
#endif

#ifdef USE_MODEL_NAME
	// the given name for this model object
	const CString& GetName() const;
	void SetName(const CString& strName);
#endif

	// returns a reference to this object's change event
	CObservableEvent& GetChangeEvent();

#ifdef USE_MFC_SERIALIZATION
	// serialization
	virtual void Serialize( CArchive& ar );
#endif

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
