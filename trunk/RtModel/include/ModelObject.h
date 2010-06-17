// Copyright (C) 2nd Messenger Systems
// $Id: ModelObject.h 637 2009-06-11 04:33:15Z dglane001 $
#if !defined(_MODELOBJECT_H__INCLUDED_)
#define _MODELOBJECT_H__INCLUDED_

#include <ItkUtils.h>
using namespace itk;

#include "Observer.h"

/**
 * CModelObject can fire change events observable,
 * and is serializable
 */
class CModelObject : public DataObject
{
public:
	CModelObject();
	virtual ~CModelObject();

	/**
	 * returns a reference to this object's change event
	 */ 
	CObservableEvent& GetChangeEvent();

protected:
	/**
	 * the change event for this object
	 */
	CObservableEvent m_eventChange;
};

//////////////////////////////////////////////////////////////////////
inline CObservableEvent& CModelObject::GetChangeEvent()
{
	return m_eventChange;
}

#endif //  !defined(_MODELOBJECT_H__INCLUDED_)
