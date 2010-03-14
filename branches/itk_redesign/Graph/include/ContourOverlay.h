#pragma once

#include "PlanarOverlay.h"

#include <Series.h>

//////////////////////////////////////////////////////////////////////////////
class ContourOverlay :
	public PlanarOverlay
{
public:
	ContourOverlay(void);

	// itk typedefs
	typedef ContourOverlay Self;
	typedef PlanarOverlay Superclass;
  typedef SmartPointer<Self> Pointer;
  typedef SmartPointer<const Self> ConstPointer;

	// defines itk's New and CreateAnother static functions
	itkNewMacro(Self);

	DeclareMemberSPtr(Series, dH::Series);

	DeclareMemberSPtr(SelectedStructure, dH::Structure);
	DeclareMemberSPtr(SelectedContour, dH::ContourType);

	// drawing helper
	void Draw(CDC *pDC);

	// helper for selecting contours
	bool ContourHitTest(CPoint& point, dH::ContourType *pContour, 
		dH::VertexType*& pVertex);
	CRgn *GetRgnForContour(dH::ContourType *pContour);
};
