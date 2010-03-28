// TestRayTrace.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "TermaCalculator.h"

int _tmain(int argc, _TCHAR* argv[])
{
	TermaCalculator::Pointer pTermaCalc = TermaCalculator::New();

	TermaCalculator::FluenceMapType::Pointer pFluenceMap = 
		TermaCalculator::FluenceMapType::New();
	TermaCalculator::FluenceMapType::SizeType sizeFM = {{ 20, 20 }};
	pFluenceMap->SetRegions(sizeFM);
	pFluenceMap->Allocate();
	pFluenceMap->FillBuffer(1.0);

	// this is the physical spacing on the fluence plane
	TermaCalculator::FluenceMapType::SpacingType spacingFM = MakeVector<2>(0.2, 0.2);
	pFluenceMap->SetSpacing(spacingFM);

	pTermaCalc->SetFluenceMap(pFluenceMap);

	VolumeReal::Pointer pDensity = VolumeReal::New();
	VolumeReal::SizeType size = {{ 100, 100, 100 }};
	pDensity->SetRegions(size);
	pDensity->Allocate();
	pTermaCalc->SetDensity(pDensity);

	pTermaCalc->Update();
	VolumeReal * pTerma = pTermaCalc->GetOutput();

	return 0;
}

