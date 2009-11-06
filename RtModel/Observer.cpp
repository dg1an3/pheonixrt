// Copyright (C) 2nd Messenger Systems
// $Id: Observer.cpp 602 2008-09-14 16:54:49Z dglane001 $
#include "stdafx.h"

#include <algorithm>

// the main include for the class
#include "Observer.h"

// debug new statement
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CObservableEvent::CObservableEvent
// 
// creates an event for the parent object
//////////////////////////////////////////////////////////////////////
CObservableEvent::CObservableEvent(CObject *pParent)
	: m_pParent(pParent)
{
}

//////////////////////////////////////////////////////////////////////
// declares CObservableEvent as a dynamically creatable class
//////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC(CObservableEvent, CObject)

//////////////////////////////////////////////////////////////////////
// CObservableEvent::GetParent
// 
// returns the parent of this event
//////////////////////////////////////////////////////////////////////
CObject *CObservableEvent::GetParent()
{
	return m_pParent;
}

//////////////////////////////////////////////////////////////////////
// CObservableEvent::AddObserver
// 
// member function to an observer to the CEvent
//////////////////////////////////////////////////////////////////////
void CObservableEvent::AddObserver(CObject *pObserver, ChangeFunction func) const
{
	// see if the pair is already present
	std::multimap<CObject*,ChangeFunction>::iterator iter;
	iter = find(m_arrObservers.begin(), m_arrObservers.end(), 
		std::pair<CObject* const,ChangeFunction>(pObserver, func));

	// if not found,
	if (iter == m_arrObservers.end())

		// add to the list of observers
		m_arrObservers.insert(std::make_pair(pObserver, func));
}

//////////////////////////////////////////////////////////////////////
// CObservableEvent::RemoveObserver
// 
// member function to an observer to the CEvent
//////////////////////////////////////////////////////////////////////
void CObservableEvent::RemoveObserver(CObject *pObserver, ChangeFunction func) const
{
	// see if the pair is present
	std::multimap<CObject*,ChangeFunction>::iterator iter;
	iter = find(m_arrObservers.begin(), m_arrObservers.end(), 
		std::pair<CObject* const,ChangeFunction>(pObserver, func));

	// if so,
	if (iter != m_arrObservers.end())

		// erase it
		m_arrObservers.erase(iter);
}

//////////////////////////////////////////////////////////////////////
// CObservableEvent::Fire
// 
// fires the event, notifying all observers that the object has 
// changed.
//////////////////////////////////////////////////////////////////////
void CObservableEvent::Fire(void *pValue)
{
	// iterate over listeners
	std::multimap<CObject*,ChangeFunction>::iterator iter;
	for (iter = m_arrObservers.begin(); iter != m_arrObservers.end(); iter++)

		// firing event handler
		((iter->first)->*(iter->second))(this, pValue);
}
