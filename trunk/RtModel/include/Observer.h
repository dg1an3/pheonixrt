// Copyright (C) 2nd Messenger Systems
// $Id: Observer.h 600 2008-09-14 16:46:15Z dglane001 $
#if !defined(_OBSERVER_H__INCLUDED_)
#define _OBSERVER_H__INCLUDED_

#include <map>

// forward declaration of the CObservableEvent class
class CObservableEvent;
class CModelObject;

// defines the ChangeFunction which is called when a change occurs
typedef void (CModelObject::*ListenerFunction)(CObservableEvent *, void *);

/**
 * a CObservableEvent fires change events that can be processed by 
 * an observer
 */
class CObservableEvent
{
public:
	/**
	 * creates an event for the parent object
	 */
	CObservableEvent(CModelObject *pParent = NULL);

	/**
	 * returns the parent of this event
	 */
	CModelObject *GetParent();

	/**
	 * accessors for the observer list
	 */ 
	void AddObserver(CModelObject *pObserver, ListenerFunction func) const;
	void RemoveObserver(CModelObject *pObserver, ListenerFunction func) const;

	/**
	 * called to fire a change
	 */
	void Fire(void *pValue = NULL);

private:
	/**
	 * the parent object of this event
	 */
	CModelObject *m_pParent;

	/**
	 * the array of observers
	 */
	typedef std::multimap<CModelObject*,ListenerFunction> ListenerMapType;
	typedef std::pair<CModelObject* const,ListenerFunction> MapEntryType;
	mutable ListenerMapType m_arrObservers;
};

#endif // !defined(_OBSERVER_H__INCLUDED_)
