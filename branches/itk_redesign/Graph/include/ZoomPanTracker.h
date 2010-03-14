#pragma once

#include <PlanarTracker.h>
#include <ContourOverlay.h>

namespace dH {

class PlanarView;

//////////////////////////////////////////////////////////////////////////////
class ZoomPanTracker :
	public PlanarTracker
{
public:
	ZoomPanTracker(void);

	// itk typedefs
	typedef ZoomPanTracker Self;
	typedef PlanarTracker Superclass;
  typedef SmartPointer<Self> Pointer;
  typedef SmartPointer<const Self> ConstPointer;

	// defines itk's New and CreateAnother static functions
	itkNewMacro(Self);

	// associated overlay
	DeclareMemberPtr(View, dH::PlanarView);

	// editing modes
	DeclareMember(LockToSlice, bool);

	// tracker over-rides
	virtual void OnButtonDown(UINT nFlags, CPoint point);
	virtual void OnMouseMove(UINT nFlags, CPoint point);
	virtual void OnButtonUp(UINT nFlags, CPoint point);

protected:
	// zoom mode
	bool m_bZooming;
	REAL m_zoomStart;
	CPoint m_ptZoomStart;

	// panning mode
	bool m_bPanning;
	bool m_bPanningZ;
	Vector<REAL> m_vPanStart;
	Vector<REAL> m_vCenterStart;
	Matrix<REAL, 4, 4> m_mBasisStart;
};

}