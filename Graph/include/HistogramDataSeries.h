// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: HistogramDataSeries.h 607 2008-09-14 18:32:17Z dglane001 $
#pragma once

#ifdef USE_RTOPT
#include <Histogram.h>
#endif
#include <DataSeries.h>

class CHistogram;

class CHistogramDataSeries : public CDataSeries
{
public:
	CHistogramDataSeries(CHistogram *pHisto = NULL);
	virtual ~CHistogramDataSeries(void);

	// standard class typedefs
	typedef CHistogramDataSeries Self;
	typedef CDataSeries Superclass;
	typedef itk::SmartPointer< Self > Pointer;
	typedef itk::SmartPointer< const Self > ConstPointer;

	itkNewMacro(CHistogramDataSeries);

	DECLARE_ATTRIBUTE_PTR(Histogram, CHistogram);

	virtual const CMatrixNxM<>& GetDataMatrix();

	void OnHistogramChanged(); // CObservableEvent *, void *);

private:
//	CHistogram *m_pHisto;
// public:
//	CHistogram * GetHistogram(void);

	mutable bool m_bRecalcCurve;
};

