// Copyright (C) 2nd Messenger Systems
// $Id: ModelObject.cpp 602 2008-09-14 16:54:49Z dglane001 $
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


// CObArray CModelObject::m_arrDispose;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CModelObject::CModelObject(
#ifdef USE_MODEL_NAME
						   const CString& strName
#endif
						   )
#pragma warning(disable: 4355)
	: m_eventChange(this)
#pragma warning(default: 4355)
#ifdef USE_MODEL_NAME
		, m_strName(strName)
#endif
	// constructs a model object with the given name
{
}	// CModelObject::CModelObject


//////////////////////////////////////////////////////////////////////
CModelObject::~CModelObject()
	// destroys the model objects
{
}	// CModelObject::~CModelObject


//////////////////////////////////////////////////////////////////////
// declares CModelObject as a serializable class
//////////////////////////////////////////////////////////////////////
#ifdef USE_MFC_SERIALIZATION
IMPLEMENT_SERIAL(CModelObject, CObject, 1)
#endif

#ifdef USE_MODEL_NAME
//////////////////////////////////////////////////////////////////////
const CString& 
	CModelObject::GetName() const
	// returns the given name for this model object
{
	return m_strName;

}	// CModelObject::GetName


//////////////////////////////////////////////////////////////////////
void 
	CModelObject::SetName(const CString& strName)
	// sets the given name for this model object, firing a change in
	//		the process
{
	// set the name
	m_strName = strName;

	// and fire the change
	GetChangeEvent().Fire();

}	// CModelObject::SetName
#endif

#ifdef USE_MFC_SERIALIZATION
//////////////////////////////////////////////////////////////////////
void 
	CModelObject::Serialize( CArchive& ar )
	// serialization
{
#ifdef USE_MFC_SERIALIZATION
	// serialize the base class
	CObject::Serialize(ar);
#endif

	// serialize the object's name
	SERIALIZE_VALUE(ar, m_strName);

	// serialize the children
	CObArray arrChildren;
	arrChildren.Serialize(ar);

}	// CModelObject::Serialize

#endif