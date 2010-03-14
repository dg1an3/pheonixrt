// Copyright (C) 2nd Messenger Systems
// $Id: Polygon.h 601 2008-09-14 16:48:05Z dglane001 $
#if !defined(AFX_POLYGON_H__AAA9A385_F0B7_11D4_9E39_00B0D0609AB0__INCLUDED_)
#define AFX_POLYGON_H__AAA9A385_F0B7_11D4_9E39_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <MatrixNxM.h>

#include <ModelObject.h>
#include <ItkUtils.h>

//////////////////////////////////////////////////////////////////////
// class CPolygon
//
// represents a polygon on a plane; includes change notification
// and some computational geometry
//////////////////////////////////////////////////////////////////////
class CPolygon : public CModelObject
{
public:
	// constructors / destructors
	CPolygon();
	CPolygon(const CPolygon& fromPoly);
	virtual ~CPolygon();

	// serialization support for the polygon
	DECLARE_SERIAL(CPolygon)

	// assignment operator
	CPolygon& operator=(const CPolygon& fromPoly);

	//// vertex accessors
	int GetVertexCount() const;

	const itk::Vector<REAL,2>& GetVertexAt(int nIndex) const;
	void SetVertexAt(int nIndex, const itk::Vector<REAL,2>& v);

	int AddVertex(const itk::Vector<REAL,2>& v);
	void RemoveVertex(int nIndex);

	// direct access to vertex array 
	// WARNING: Lock calls must be matched by unlock calls
	CMatrixNxM<>& LockVertexMatrix();
	void UnlockVertexMatrix(BOOL bChanged = TRUE);

	// serializes the polygon
	void Serialize(CArchive &ar);

public:
	// the polygon's vertex array
	CMatrixNxM<REAL> m_mVertex;
};




///////////////////////////////////////////////////////////////////////////////
// CreateRegion
// 
// Creates a region (bit mask) from a polygon
///////////////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
void CreateRegion(const CArray<CPolygon *, CPolygon *>& arrPolygons,
				  itk::Image<VOXEL_TYPE,3> *pRegion, int nSlice)
{
	CDC dc;
	BOOL bRes = dc.CreateCompatibleDC(NULL);

	CBitmap bitmap;
	bRes = bitmap.CreateBitmap(pRegion->GetBufferedRegion().GetSize()[0], 
		pRegion->GetBufferedRegion().GetSize()[1], 1, 1, NULL);

	CBitmap *pOldBitmap = (CBitmap *) dc.SelectObject(&bitmap);
	dc.SelectStockObject(WHITE_PEN);
	dc.SelectStockObject(WHITE_BRUSH);

	itk::Point<REAL,3> vOrigin = pRegion->GetOrigin();
	itk::Vector<REAL,3> vSpacing = pRegion->GetSpacing();
	for (int nAtPoly = 0; nAtPoly < arrPolygons.GetSize(); nAtPoly++)
	{
		// static 
			CArray<CPoint, CPoint&> arrPoints;
		arrPoints.SetSize(arrPolygons[nAtPoly]->GetVertexCount());
		for (int nAt = 0; nAt < arrPolygons[nAtPoly]->GetVertexCount(); nAt++)
		{
			itk::Vector<REAL,2> vVert = arrPolygons[nAtPoly]->GetVertexAt(nAt);
			vVert[0] = (vVert[0] - vOrigin[0]) / vSpacing[0];
			vVert[1] = (vVert[1] - vOrigin[1]) / vSpacing[1];

			arrPoints[nAt].x = vVert[0];
			arrPoints[nAt].y = vVert[1];
		}
		dc.Polygon(arrPoints.GetData(), arrPolygons[nAtPoly]->GetVertexCount());
	}

	// finished with DC
	dc.SelectObject(pOldBitmap);
	dc.DeleteDC();

	// now get the bitmap descriptor (for scan width
	BITMAP bm;
	bitmap.GetBitmap(&bm);

	// resize the buffer
	// static 
		CArray<BYTE, BYTE> arrBuffer;
	arrBuffer.SetSize(pRegion->GetBufferedRegion().GetSize()[1] * bm.bmWidthBytes);
	int nByteCount = bitmap.GetBitmapBits((DWORD) arrBuffer.GetSize(), arrBuffer.GetData());

	// now populate region
	// DON'T CLEAR REGION HERE -- this is called multiple times
	// pRegion->FillBuffer(0.0); 

	int nStride = pRegion->GetBufferedRegion().GetSize()[0];
	int nStrideZ = pRegion->GetBufferedRegion().GetSize()[1] * nStride;
	// now populate the region
	for (int nY = 0; nY < pRegion->GetBufferedRegion().GetSize()[1]; nY++)
	{
		for (int nX = 0; nX < pRegion->GetBufferedRegion().GetSize()[0]; nX++)
		{
			if ((arrBuffer[nY * bm.bmWidthBytes + nX / 8] >> (7 - nX % 8)) & 0x01)
			{
				// (*pRegion)[0][nY][nX] 
				pRegion->GetBufferPointer()[nSlice * nStrideZ + nY * nStride + nX]
					= (VOXEL_TYPE) 1.0;
			}
		}
	}

	// done with bitmap
	bitmap.DeleteObject();

}	// CreateRegion


#endif // !defined(AFX_POLYGON_H__AAA9A385_F0B7_11D4_9E39_00B0D0609AB0__INCLUDED_)
