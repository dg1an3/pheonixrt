#pragma once

#include <itkLightObject.h>

//////////////////////////////////////////////////////////////////////////////
class PlanarOverlay 
	: public itk::LightObject
{
public:
	void SetOriginSpacing(const VolumeReal::PointType& origin,
			const VolumeReal::SpacingType& spacing)
	{
		m_pOrigin = &origin;
		m_pSpacing = &spacing;
	}

	template<class VertexType>
	void MoveTo(CDC *pDC, const VertexType& vVert)
	{
		CPoint pt;
		pt.x = Round<LONG>((vVert[0] - (*m_pOrigin)[0]) / (*m_pSpacing)[0]);
		pt.y = Round<LONG>((vVert[1] - (*m_pOrigin)[1]) / (*m_pSpacing)[1]);

		pDC->MoveTo(pt);
	}

	template<class VertexType>
	void LineTo(CDC *pDC, const VertexType& vVert)
	{
		CPoint pt;
		pt.x = Round<LONG>((vVert[0] - (*m_pOrigin)[0]) / (*m_pSpacing)[0]);
		pt.y = Round<LONG>((vVert[1] - (*m_pOrigin)[1]) / (*m_pSpacing)[1]);

		pDC->LineTo(pt);
	}

	template<class VertexType>
	CPoint ToPoint(const VertexType& vVert)
	{
		CPoint pt;
		pt.x = Round<LONG>((vVert[0] - (*m_pOrigin)[0]) / (*m_pSpacing)[0]);
		pt.y = Round<LONG>((vVert[1] - (*m_pOrigin)[1]) / (*m_pSpacing)[1]);

		return pt;
	}

	template<class VType>
	VType ToVertex(const CPoint& pt)
	{
		VType vVert;
		vVert[0] = pt.x * (*m_pSpacing)[0] + (*m_pOrigin)[0];
		vVert[1] = pt.y * (*m_pSpacing)[1] + (*m_pOrigin)[1];
		vVert[2] = (*m_pOrigin)[2];

		return vVert;
	}

protected:
	const VolumeReal::PointType *m_pOrigin;
	const VolumeReal::SpacingType *m_pSpacing;
};

