// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: DataSeries.h 607 2008-09-14 18:32:17Z dglane001 $
#pragma once

//#include <VectorD.h>
#include <MatrixNxM.h>

// #include <ModelObject.h>
// #include <Attributes.h>

class CGraph;

class CDataSeries : public itk::DataObject
{
public:
	CDataSeries(void);
	virtual ~CDataSeries(void);

	// standard class typedefs
	typedef CDataSeries Self;
	typedef itk::DataObject Superclass;
	typedef itk::SmartPointer< Self > Pointer;
	typedef itk::SmartPointer< const Self > ConstPointer;

	itkNewMacro(CDataSeries);

	// the object of the data series
	DECLARE_ATTRIBUTE_PTR(Object, CObject);

	// curve attributes
	DECLARE_ATTRIBUTE_GI(Color, COLORREF);
	DECLARE_ATTRIBUTE(PenStyle, int);

	// accessors for the data series data
	virtual const CMatrixNxM<>& GetDataMatrix();
	virtual void SetDataMatrix(const CMatrixNxM<>& mData);
	void AddDataPoint(const itk::Vector<REAL,2>& vDataPt);

	// flag to indicate whether the data series should have handles
	//		for interaction
	DECLARE_ATTRIBUTE(HasHandles, bool);

	// returns index of handle, if point hits it
	int GetHandleHit(const CPoint& point, int nSize, CPoint *pOfs = NULL);

	// accessors to indicate the data series monotonic direction: 
	//		-1 for decreasing, 
	//		1 for increasing,
	//		0 for not monotonic
	DECLARE_ATTRIBUTE(MonotonicDirection, int);

	bool UseForAutoScale;

protected:
	friend CGraph;

public:
	// my graph (if part of graph
	CGraph *m_pGraph;

	// use a std vector instead of CArray, because CArray is bad
	mutable CMatrixNxM<> m_mData;	

};	// class CDataSeries
