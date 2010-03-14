#pragma once
#include <DataSeries.h>
#ifdef USE_RTOPT
#include <KLDivTerm.h>
#endif

namespace dH { class KLDivTerm; }

class CTargetDVHSeries :
	public dH::DataSeries
{
public:
	CTargetDVHSeries(dH::KLDivTerm *pKLDT);
	virtual ~CTargetDVHSeries(void);

// 	virtual void SetDataMatrix(const CMatrixNxM<>& mData);

#ifdef USE_RTOPT
	// stores pointer to term
	dH::KLDivTerm *m_pKLDivTerm;
#endif
	void OnKLDTChanged(void * pEv, void * pVoid);
};
