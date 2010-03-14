#pragma once

#include <itkLightObject.h>

//////////////////////////////////////////////////////////////////////////////
class PlanarTracker : 
	public itk::LightObject
{
public:
	virtual void OnButtonDown(UINT nFlags, CPoint point) { }
	virtual void OnMouseMove(UINT nFlags, CPoint point) { }
	virtual void OnButtonUp(UINT nFlags, CPoint point) { }
	virtual void OnButtonDblClk(UINT nFlags, CPoint point) { }
};
