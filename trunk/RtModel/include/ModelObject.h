// Copyright (C) 2nd Messenger Systems
// $Id: ModelObject.h 637 2009-06-11 04:33:15Z dglane001 $
#if !defined(_MODELOBJECT_H__INCLUDED_)
#define _MODELOBJECT_H__INCLUDED_

#include <ItkUtils.h>
using namespace itk;

#include "Observer.h"

namespace dH
{

/**
 * ModelObject can fire change events observable,
 * and is serializable
 */
class ModelObject : public DataObject
{
public:
	ModelObject();
	virtual ~ModelObject() { }

public:
	/** itk typedefs */
	typedef ModelObject Self;
	typedef DataObject Superclass;
	typedef SmartPointer<Self> Pointer;
	typedef SmartPointer<const Self> ConstPointer;

	/** returns a reference to this object's change event */ 
	ObservableEvent& GetChangeEvent();

protected:
	/** the change event for this object */
	ObservableEvent m_eventChange;
};

//////////////////////////////////////////////////////////////////////
inline 
ObservableEvent& 
	ModelObject::GetChangeEvent()
{
	return m_eventChange;
}

}	// namespace dH

typedef dH::ModelObject CModelObject;


#endif //  !defined(_MODELOBJECT_H__INCLUDED_)
