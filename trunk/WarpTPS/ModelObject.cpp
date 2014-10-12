//////////////////////////////////////////////////////////////////////
// ModelObject.cpp: implementation of the CModelObject class.
//
// Copyright (C) 1999-2001
// $Id: ModelObject.cpp,v 1.3 2003/04/26 20:51:50 default Exp $
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// utility macros
#include "UtilMacros.h"

// the main include for the class
#include "ModelObject.h"

// debug new statement
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CObArray CModelObject::m_arrDispose;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CModelObject::CModelObject
// 
// constructs a model object with the given name
//////////////////////////////////////////////////////////////////////
CModelObject::CModelObject(const CString& strName)
#pragma warning(disable: 4355)
	: m_eventChange(this),
#pragma warning(default: 4355)
		m_strName(strName),
		m_dwRefCount(0)
{
}

//////////////////////////////////////////////////////////////////////
// CModelObject::~CModelObject
// 
// destroys the model objects
//////////////////////////////////////////////////////////////////////
CModelObject::~CModelObject()
{
}

//////////////////////////////////////////////////////////////////////
// declares CModelObject as a serializable class
//////////////////////////////////////////////////////////////////////
IMPLEMENT_SERIAL(CModelObject, CObject, 1)

//////////////////////////////////////////////////////////////////////
// CModelObject::GetName
// 
// returns the given name for this model object
//////////////////////////////////////////////////////////////////////
const CString& CModelObject::GetName() const
{
	return m_strName;
}

//////////////////////////////////////////////////////////////////////
// CModelObject::SetName
// 
// sets the given name for this model object, firing a change in
//		the process
//////////////////////////////////////////////////////////////////////
void CModelObject::SetName(const CString& strName)
{
	// set the name
	m_strName = strName;

	// and fire the change
	GetChangeEvent().Fire();
}

//////////////////////////////////////////////////////////////////////
// CModelObject::GetChildCount
// 
// returns the number of children of this model object
//////////////////////////////////////////////////////////////////////
int CModelObject::GetChildCount() const
{
	return m_arrChildren.GetSize();
}

//////////////////////////////////////////////////////////////////////
// CModelObject::GetChildAt
// 
// returns the child at the index
//////////////////////////////////////////////////////////////////////
CModelObject *CModelObject::GetChildAt(int nIndex)
{
	return (CModelObject *) m_arrChildren[nIndex];
}

//////////////////////////////////////////////////////////////////////
// CModelObject::AddChild
// 
// adds a child object to this object
//////////////////////////////////////////////////////////////////////
int CModelObject::AddChild(CModelObject *pObject)
{
	// store the index
	int nIndex = m_arrChildren.Add(pObject);

	// fire a change event
	GetChangeEvent().Fire();

	// and return the index
	return nIndex;
}

//////////////////////////////////////////////////////////////////////
// CModelObject::Serialize
// 
// serialization
//////////////////////////////////////////////////////////////////////
void CModelObject::Serialize( CArchive& ar )
{
	// serialize the base class
	CObject::Serialize(ar);

	// serialize the object's name
	SERIALIZE_VALUE(ar, m_strName);

	// serialize the children
	m_arrChildren.Serialize(ar);
}


// IUnknown interface methods
STDMETHODIMP CModelObject::QueryInterface(REFIID, void **)
{
	return E_NOTIMPL;
}


STDMETHODIMP_(ULONG) CModelObject::AddRef()
{
	return ++m_dwRefCount;
}

STDMETHODIMP_(ULONG) CModelObject::Release()
{
	m_dwRefCount--;

	if (0 == m_dwRefCount)
	{
		m_arrDispose.Add(this);
	}

	return m_dwRefCount;
}
