// Copyright (C) 2nd Messenger Systems
// $Id$
#pragma once

#include <itkDataObject.h>
using namespace itk;

BeginNamespace(dH)

// forward definition
class Graph;

//////////////////////////////////////////////////////////////////////////////
class DataSeries :
	public DataObject 
{
public:
	DataSeries(void);
	virtual ~DataSeries(void);

	// itk typedefs
	typedef DataSeries Self;
	typedef DataObject Superclass;
	typedef SmartPointer<Self> Pointer;
	typedef SmartPointer<const Self> ConstPointer;

	// defines itk's New and CreateAnother static functions
	itkNewMacro(Self);

	// the object of the data series (should be DataObject)
	DeclareMemberSPtr(Object, DataObject);

	// curve attributes
	DeclareMember(Color, COLORREF);
	DeclareMember(PenStyle, int);

	// type of curves displayed as data series
	typedef PolyLineParametricPath<2> CurveType;

	// accessors for the data series data
	DeclareMemberSPtr(Curve, CurveType);

	// called to update the curve (if needed)
	virtual void UpdateCurve() { }

	// flag to indicate whether the data series should have handles
	//		for interaction
	DeclareMember(HasHandles, bool);

	// returns index of handle, if point hits it
	int GetHandleHit(const CPoint& point, int nSize, CPoint *pOfs = NULL);

	// accessors to indicate the data series monotonic direction: 
	//		-1 for decreasing, 
	//		1 for increasing,
	//		0 for not monotonic
	DeclareMember(MonotonicDirection, int);

protected:
	friend dH::Graph;

public:
	// my graph (if part of graph)
	dH::Graph *m_pGraph;

};	// class DataSeries

EndNamespace(dH)
