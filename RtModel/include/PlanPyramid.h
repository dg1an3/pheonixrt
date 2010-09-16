// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: PlanPyramid.h 647 2009-11-05 21:52:59Z dglane001 $
#pragma once

#include <Plan.h>

namespace dH
{

///////////////////////////////////////////////////////////////////////////////
// class PlanPyramid
// 
// object that manages pyramid of CPlans
///////////////////////////////////////////////////////////////////////////////
class PlanPyramid
{
public:
	PlanPyramid(CPlan *pPlan);
	~PlanPyramid(void);

	// constant represent number of sub-scales
	static const int MAX_SCALES = 4; // Structure::MAX_SCALES-1;

	// accessor to the Plan object
	DECLARE_ATTRIBUTE_PTR_GI(Plan, CPlan);

	// getter for sub-plans
	CPlan *GetPlan(int nLevel);

	// helper to calculate sub beamlets
	void CalcPencilSubBeamlets(int nBeam = -1);

	// helper function to transfer intensity maps from one level
	//		to next
	void InvFiltIntensityMap(int nLevel, const CBeam::IntensityMap * vWeights,
								CBeam::IntensityMap * vFiltWeights);

protected:
	// array of plans (= plan pyramid)
	vector<CPlan*> m_arrPlans;

	//// helper function to set up filter matrix
	//const CMatrixNxM<>& GetFilterMat(int nLevel);

	// filter for intensity maps
	CVectorN<> m_vWeightFilter;

	//// corresponding filter matrix 
	//CMatrixNxM<> m_mFilter[MAX_SCALES - 1];
};

}	// namespace dH
