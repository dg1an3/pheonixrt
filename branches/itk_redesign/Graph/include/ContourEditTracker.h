#pragma once

#include <PlanarTracker.h>
#include <ContourOverlay.h>

//////////////////////////////////////////////////////////////////////////////
class ContourEditTracker :
	public PlanarTracker
{
public:
	ContourEditTracker(void);

	// itk typedefs
	typedef ContourEditTracker Self;
	typedef PlanarTracker Superclass;
  typedef SmartPointer<Self> Pointer;
  typedef SmartPointer<const Self> ConstPointer;

	// defines itk's New and CreateAnother static functions
	itkNewMacro(Self);

	// associated overlay
	DeclareMemberSPtr(Overlay, ContourOverlay);

	// editing modes
	DeclareMember(AddContourMode, bool);
	DeclareMember(EditContourMode, bool);

	// tracker over-rides
	virtual void OnButtonDown(UINT nFlags, CPoint point);
	virtual void OnMouseMove(UINT nFlags, CPoint point);
	virtual void OnButtonUp(UINT nFlags, CPoint point);
	virtual void OnButtonDblClk(UINT nFlags, CPoint point);

protected:
	// helpers for vertex edit operation
	dH::VertexType *m_pSelectedVertex;
	dH::VertexType m_vVertexEditStart;
	dH::VertexType m_vMouseEditStart;
};
