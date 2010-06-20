// Copyright (C) 2nd Messenger Systems
// $Id: Observer.h 600 2008-09-14 16:46:15Z dglane001 $
#if !defined(_OBSERVER_H__INCLUDED_)
#define _OBSERVER_H__INCLUDED_

#include <map>

namespace dH
{

// forward declaration of the CObservableEvent class
class ObservableEvent;
class ModelObject;

// defines the ChangeFunction which is called when a change occurs
typedef void (ModelObject::*ListenerFunction)(ObservableEvent *, void *);

/**
 * a CObservableEvent fires change events that can be processed by 
 * an observer
 */
class ObservableEvent
{
public:
	/** creates an event for the parent object */
	ObservableEvent(ModelObject *pParent = NULL);

	/** returns the parent of this event */
	ModelObject *GetParent();

	/** accessors for the observer list */ 
	void AddObserver(ModelObject *pObserver, ListenerFunction func) const;
	void RemoveObserver(ModelObject *pObserver, ListenerFunction func) const;

	/** called to fire a change */
	void Fire(void *pValue = NULL);

private:
	/** the parent object of this event */
	ModelObject *m_pParent;

	/** the array of observers */
	typedef std::multimap<ModelObject*,ListenerFunction> ListenerMapType;
	typedef std::pair<ModelObject* const,ListenerFunction> MapEntryType;
	mutable ListenerMapType m_arrObservers;
};

}

typedef dH::ObservableEvent CObservableEvent;

#endif // !defined(_OBSERVER_H__INCLUDED_)
