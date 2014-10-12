//////////////////////////////////////////////////////////////////////
// Observer.cpp: implementation of the CObservableEvent class
//
// Copyright (C) 1999-2001
// $Id: Observer.cpp,v 1.2 2002/06/03 05:39:14 default Exp $
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

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
	// check to ensure the observer is not already in the list
	for (int nAt = m_arrObservers.GetSize()-1; nAt >= 0; nAt--)
	{
		if (m_arrObservers[nAt] == pObserver && m_arrFunctions[nAt] == func)
		{
			return;
		}
	}

	// add to the list of observers
	m_arrObservers.Add(pObserver);
	m_arrFunctions.Add(func);
}

//////////////////////////////////////////////////////////////////////
// CObservableEvent::RemoveObserver
// 
// member function to an observer to the CEvent
//////////////////////////////////////////////////////////////////////
void CObservableEvent::RemoveObserver(CObject *pObserver, ChangeFunction func) const
{
	for (int nAt = m_arrObservers.GetSize()-1; nAt >= 0; nAt--)
	{
		if (m_arrObservers[nAt] == pObserver && m_arrFunctions[nAt] == func)
		{
			m_arrObservers.RemoveAt(nAt);
			m_arrFunctions.RemoveAt(nAt);
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CObservableEvent::Fire
// 
// fires the event, notifying all observers that the object has 
// changed.
//////////////////////////////////////////////////////////////////////
void CObservableEvent::Fire(void *pValue)
{
	for (int nAt = 0; nAt < m_arrObservers.GetSize(); nAt++)
	{
		(m_arrObservers[nAt]->*m_arrFunctions[nAt])(this, pValue);
	}
}

//////////////////////////////////////////////////////////////////////
// CObservableEvent::FireChange
// 
// fires a change, notifying all observers that the object has 
// changed.
//////////////////////////////////////////////////////////////////////
/* void CObservableEvent::FireChange(void *pOldValue)
{
	Fire(pOldValue);
}
*/