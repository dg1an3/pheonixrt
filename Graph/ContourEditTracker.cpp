#include "StdAfx.h"

#include <ContourEditTracker.h>

//////////////////////////////////////////////////////////////////////////////
ContourEditTracker::ContourEditTracker(void)
	: m_EditContourMode(true)
	, m_AddContourMode(false)
{
}

//////////////////////////////////////////////////////////////////////////////
void 
	ContourEditTracker::OnButtonDown(UINT nFlags, CPoint point)
{
	if (GetOverlay()->GetSelectedStructure() == NULL)
		return;

	if (GetAddContourMode())
	{
		if (GetOverlay()->GetSelectedContour() == NULL)
		{
			// start a new polygon
			GetOverlay()->SetSelectedContour( dH::ContourType::New() );
		}

		dH::VertexType vVert = GetOverlay()->ToVertex<dH::VertexType>(point);
		GetOverlay()->GetSelectedContour()->AddVertex(vVert);
		return;
	}

	else if (GetEditContourMode())
	{
		dH::Structure *pStruct = GetOverlay()->GetSelectedStructure();
		GetOverlay()->SetSelectedContour(NULL);
		m_pSelectedVertex = NULL;

		// see if we hit one of the selected structures contours
		for (int nAtContour = 0; nAtContour < pStruct->GetContourCount(); nAtContour++)
		{
			dH::ContourType *pContour = pStruct->GetContourPoly(nAtContour);			
			if (GetOverlay()->ContourHitTest(point, pContour, m_pSelectedVertex))
			{
				GetOverlay()->SetSelectedContour(pContour);
				if (m_pSelectedVertex)
				{					
					m_vVertexEditStart = *m_pSelectedVertex;
					m_vMouseEditStart = GetOverlay()->ToVertex<dH::VertexType>(point);
				}
				return;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
void 
	ContourEditTracker::OnMouseMove(UINT nFlags, CPoint point)
{
	if (GetEditContourMode()
		&& m_pSelectedVertex != NULL)
	{
		dH::VertexType vMouseEditCurr = GetOverlay()->ToVertex<dH::VertexType>(point);
		for (int nN = 0; nN < 3; nN++)
			(*m_pSelectedVertex)[nN] = m_vVertexEditStart[nN] + (vMouseEditCurr[nN] - m_vMouseEditStart[nN]);
	}
}

//////////////////////////////////////////////////////////////////////////////
void 
	ContourEditTracker::OnButtonUp(UINT nFlags, CPoint point)
{
	if (GetEditContourMode()
		&& m_pSelectedVertex != NULL)
	{
		m_pSelectedVertex = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////////
void 
	ContourEditTracker::OnButtonDblClk(UINT nFlags, CPoint point)
{
	if (GetAddContourMode()
		&& GetOverlay()->GetSelectedContour() != NULL)
	{
		dH::ContourType::Pointer pNewContour = GetOverlay()->GetSelectedContour();
		GetOverlay()->GetSelectedStructure()->AddContourPoly(pNewContour);
		GetOverlay()->SetSelectedContour(NULL);

		SetAddContourMode(false);
		SetEditContourMode(true);
	}
}