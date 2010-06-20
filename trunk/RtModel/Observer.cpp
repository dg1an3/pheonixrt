// Copyright (C) 2nd Messenger Systems
// $Id: Observer.cpp 602 2008-09-14 16:54:49Z dglane001 $
#include "stdafx.h"

#include <algorithm>

// the main include for the class
#include "Observer.h"
#include "ModelObject.h"

namespace dH
{

//////////////////////////////////////////////////////////////////////
ObservableEvent::ObservableEvent(ModelObject *pParent)
	: m_pParent(pParent)
{
}

//////////////////////////////////////////////////////////////////////
ModelObject *
	ObservableEvent::GetParent()
{
	return m_pParent;
}

//////////////////////////////////////////////////////////////////////
void 
	ObservableEvent::AddObserver(ModelObject *pObserver, ListenerFunction func) const
{
	// remove if present
	RemoveObserver(pObserver, func);

	// add to the list of observers
	m_arrObservers.insert(std::make_pair(pObserver, func));
}

//////////////////////////////////////////////////////////////////////
void 
	ObservableEvent::RemoveObserver(ModelObject *pObserver, ListenerFunction func) const
{
	// see if the pair is present
	ListenerMapType::iterator iter 
		= find(m_arrObservers.begin(), m_arrObservers.end(), MapEntryType(pObserver, func));
	if (iter != m_arrObservers.end())
	{
		// erase it
		m_arrObservers.erase(iter);
	}
}

//////////////////////////////////////////////////////////////////////
void 
	ObservableEvent::Fire(void *pValue)
{
	// iterate over listeners
	ListenerMapType::iterator iter = m_arrObservers.begin();
	for (; iter != m_arrObservers.end(); iter++)
	{
		// firing event handler
		((iter->first)->*(iter->second))(this, pValue);
	}
}

}	// namespace dH
