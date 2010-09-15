// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: PlanOptimizer.h 603 2008-09-14 16:58:43Z dglane001 $
#pragma once

#include <Prescription.h>
#include <PlanPyramid.h>

#include <ConjGradOptimizer.h>

namespace dH
{

///////////////////////////////////////////////////////////////////////////////
// class PlanOptimizer
// 
// object that manages optimization of a CPlan
///////////////////////////////////////////////////////////////////////////////
class PlanOptimizer
{
public:
	PlanOptimizer(CPlan *pPlan);
	~PlanOptimizer(void);

	// reference to my plan
	DECLARE_ATTRIBUTE_PTR(Plan, CPlan);

	// reference to the plan pyramid manager
	DECLARE_ATTRIBUTE_PTR(Pyramid, PlanPyramid);

	// accessors for the prescription objects
	Prescription *GetPrescription(int nLevel);
	DynamicCovarianceOptimizer *GetOptimizer(int nLevel);

	// handles cloning to separate layers
	void AddStructureTerm(VOITerm *pST);

	// performs the optimization (calls sub-levels first)
	bool Optimize(CVectorN<>& vInit, OptimizerCallback *pFunc, void *pParam);

	// transfers state vector from plan
	void GetStateVectorFromPlan(CVectorN<>& vState);
	void SetStateVectorToPlan(const CVectorN<>& vState);

	// transfers state vector from level n+1 to level n
	void InvFilterStateVector(int nScale, const CVectorN<>& vIn, CVectorN<>& vOut);

protected:
	// helper to set up the prescription
	void SetupPrescription();

	// initial state vector
	void GetInitStateVector(CVectorN<>& vInit);

	// helper functions for state vector
	void StateVectorToIntensityMap(int nScale, int nBeam,
		const CVectorN<>& vState, 
		CBeam::IntensityMap *pIntensityMap); // CMatrixNxM<>& mBeamletWeights)

	void IntensityMapToStateVector(int nScale, int nBeam,
			const CBeam::IntensityMap *pIntensityMap, CVectorN<>& vState);

private:
	// pointers to the other prescription objects
	vector< std::pair<dH::Prescription*, DynamicCovarianceOptimizer*> > m_arrPrescriptions;
};

}	// namespace dH