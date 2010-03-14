// Copyright (C) 2008 DGLane
// $Id$
#pragma once

#include <itkColorTable.h>
using namespace itk;

#include <PlanarOverlay.h>
#include <Series.h>

//////////////////////////////////////////////////////////////////////////////
class IsocurveOverlay :
	public PlanarOverlay
{
public:
	IsocurveOverlay(void);
	~IsocurveOverlay(void);

	// itk typedefs
	typedef IsocurveOverlay Self;
	typedef PlanarOverlay Superclass;
  typedef SmartPointer<Self> Pointer;
  typedef SmartPointer<const Self> ConstPointer;

	// defines itk's New and CreateAnother static functions
	itkNewMacro(Self);

	DeclareMemberSPtr(DoseVolume, VolumeReal);

	DeclareMember(MinIsocurve, VoxelReal);
	DeclareMember(MaxIsocurve, VoxelReal);
	DeclareMember(IsocurveStep, VoxelReal);

	DeclareMemberSPtr(IsocurveColorTable, ColorTable<unsigned char>);

	// drawing helper
	void Draw(CDC *pDC);

protected:

	// helper to draw contours for a given voxel value
	void GenerateContour(CDC *pDC, VoxelReal c);

	// draws a single segment of the isocurve
	void DrawSegment(CDC *pDC, const dH::VertexType& from, const dH::VertexType& to);

};
