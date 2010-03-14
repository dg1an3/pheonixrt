#include "StdAfx.h"
#include "ContourOverlay.h"

//////////////////////////////////////////////////////////////////////////////
ContourOverlay::ContourOverlay(void)
{
}

//////////////////////////////////////////////////////////////////////////////
void 
	ContourOverlay::Draw(CDC *pDC)
{
	if (!GetSeries())
		return;

	for (int nAtStruct = 0; nAtStruct < GetSeries()->GetStructureCount(); nAtStruct++)
	{
		dH::Structure *pStruct = GetSeries()->GetStructureAt(nAtStruct);
		if (!pStruct->GetVisible())
		{
			continue;
		}

		// set up drawing pen for color
		CPen pen(PS_SOLID, 2, pStruct->GetColor());
		CPen *pOldPen = pDC->SelectObject(&pen);
		pDC->SelectStockObject(HOLLOW_BRUSH);

		// draw all contours matching the current plane
		for (int nAtContour = 0; nAtContour < pStruct->GetContourCount(); nAtContour++)
		{
			dH::ContourType *pPoly = pStruct->GetContourPoly(nAtContour);
			REAL zPos = pPoly->GetVertexList()->at(0)[2]; 
			if (IsApproxEqual(zPos, (*m_pOrigin)[2], (*m_pSpacing)[2])) 
			{
				MoveTo(pDC, pPoly->GetVertexList()->at(0));
				for (int nAtVert = 1; nAtVert < pPoly->GetVertexList()->size(); nAtVert++)
				{
					LineTo(pDC, pPoly->GetVertexList()->at(nAtVert));
				}
				LineTo(pDC, pPoly->GetVertexList()->at(0));
			}
		}

		// do we need to display the points?
		if (GetSelectedStructure() == pStruct)
		{
			// display the points for the selected contour
			for (int nAtContour = 0; nAtContour < pStruct->GetContourCount(); nAtContour++)
			{
				dH::ContourType *pContour = pStruct->GetContourPoly(nAtContour);
				REAL zPos = pContour->GetVertexList()->at(0)[2];
				if (GetSelectedContour() == pContour
						&& IsApproxEqual(zPos, (*m_pOrigin)[2], (*m_pSpacing)[2]))
					{
						CPen penHandle(PS_SOLID, 1, RGB(224, 224, 224));
						pDC->SelectObject(&penHandle);
						dH::ContourType *pContour = GetSelectedContour();
						for (int nAtVert = 0; nAtVert < pContour->GetVertexList()->size(); nAtVert++)
						{
							CRect rect(ToPoint(pContour->GetVertexList()->at(nAtVert)), CSize(5, 5));
							rect -= CPoint(2, 2);
							pDC->Rectangle(rect);
						}
					}
				}
		}

		// restore old pen
		pDC->SelectObject(pOldPen);
	}
}


//////////////////////////////////////////////////////////////////////////////
bool 
	ContourOverlay::ContourHitTest(CPoint& point, dH::ContourType *pContour, dH::VertexType*& pVertex)
{	
	// see if contour is on plane
	REAL zPos = pContour->GetVertexList()->at(0)[2];
	if (!IsApproxEqual(zPos, (*m_pOrigin)[2], (*m_pSpacing)[2])) 
	{
		return false;
	}

	// see if we hit a handle
	for (int nVertex = 0; nVertex < pContour->GetVertexList()->size(); nVertex++)
	{
		pVertex = const_cast<dH::VertexType*>(&pContour->GetVertexList()->at(nVertex));
		CRect rectHandle(ToPoint(*pVertex), CSize(4, 4));
		rectHandle -= CPoint(2, 2);
		rectHandle.InflateRect(5, 5);
		if (rectHandle.PtInRect(point))
		{
			return true;
		}
	}

	// no handle, so there is no selected vertex
	pVertex = NULL;

	// see if we hit the region
	CRgn *pRgn = GetRgnForContour(pContour);
	bool ptInRegion = pRgn->PtInRegion(point) ? true : false;
	pRgn->DeleteObject();
	delete pRgn;

	return ptInRegion;
}

//////////////////////////////////////////////////////////////////////////////
CRgn *
	ContourOverlay::GetRgnForContour(dH::ContourType *pContour)
{
	// generate a DC to draw the contour (conforms with current window)
	CDC *pCommonDC = AfxGetMainWnd()->GetDC();
	CDC dc;
	dc.CreateCompatibleDC(pCommonDC);
	AfxGetMainWnd()->ReleaseDC(pCommonDC);

	// draw the path
	dc.BeginPath();

	MoveTo(&dc, pContour->GetVertexList()->at(0));
	for (int nAtVert = 1; nAtVert < pContour->GetVertexList()->size(); nAtVert++)
	{
		LineTo(&dc, pContour->GetVertexList()->at(nAtVert));
	}
	LineTo(&dc, pContour->GetVertexList()->at(0));

	dc.EndPath();

	// create the region
	CRgn *pRgn = new CRgn();
	pRgn->CreateFromPath(&dc);

	return pRgn;
}
