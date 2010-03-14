// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: TargetDVHSeries.h 607 2008-09-14 18:32:17Z dglane001 $
#pragma once
#include <DataSeries.h>
#ifdef USE_RTOPT
#include <KLDivTerm.h>
#endif

namespace dH { class KLDivTerm; }

class CTargetDVHSeries :
	public CDataSeries
{
public:
	CTargetDVHSeries(dH::KLDivTerm *pKLDT);
	virtual ~CTargetDVHSeries(void);

	virtual void SetDataMatrix(const CMatrixNxM<>& mData);

#ifdef USE_RTOPT
	// stores pointer to term
	dH::KLDivTerm *m_pKLDivTerm;
#endif
	void OnKLDTChanged(CObservableEvent * pEv, void * pVoid);
};
