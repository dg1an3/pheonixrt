#pragma once

#include "PlanarTracker.h"
#include "ContourOverlay.h"

namespace dH {
class PlanarView;
}

//////////////////////////////////////////////////////////////////////////////
class WindowLevelTracker :
	public PlanarTracker
{
public:
	WindowLevelTracker(void) { }

	// itk typedefs
	typedef WindowLevelTracker Self;
	typedef PlanarTracker Superclass;
	typedef SmartPointer<Self> Pointer;
	typedef SmartPointer<const Self> ConstPointer;

	// defines itk's New and CreateAnother static functions
	itkNewMacro(Self);

	// associated overlay
	DeclareMemberPtr(View, dH::PlanarView);

	// tracker over-rides
	virtual void OnButtonDown(UINT nFlags, CPoint point);
	virtual void OnMouseMove(UINT nFlags, CPoint point);

protected:
	// window / level mode 
	REAL m_windowStart;
	REAL m_levelStart;
	REAL m_windowMax;

	CPoint m_ptWLStart;
};
