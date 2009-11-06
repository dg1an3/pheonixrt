//
//
#include "StdAfx.h"
#include "ContoursToRegionFilter.h"

BeginNamespace(dH)

///////////////////////////////////////////////////////////////////////////////
ContoursToRegionFilter::ContoursToRegionFilter(void)
{
}

///////////////////////////////////////////////////////////////////////////////
ContoursToRegionFilter::~ContoursToRegionFilter(void)
{
}

///////////////////////////////////////////////////////////////////////////////
void 
	ContoursToRegionFilter::AddContour(ContourType::Pointer pContour)
	// adds a contour to the filter
{
	this->AddInput(pContour);
}

///////////////////////////////////////////////////////////////////////////////
void 
	ContoursToRegionFilter::SetOutputParametersFromImage ( OutputImageType::Pointer Image )
	// Helper method to set the output parameters based on this image 
{
	for (int nN = 0; nN < OutputImageDimension; nN++)
	{
		m_Origin[nN] = Image->GetOrigin()[nN];
		m_Spacing[nN] = Image->GetSpacing()[nN];
		m_Size[nN] = Image->GetLargestPossibleRegion().GetSize()[nN];
	}
}


///////////////////////////////////////////////////////////////////////////////
void 
	ContoursToRegionFilter::GenerateOutputInformation()
{
	OutputImagePointer OutputImage = this->GetOutput();

	VolumeReal::IndexType index;
	index.Fill(0);
	VolumeReal::RegionType region;

	// If the size of the output has been explicitly specified, the filter
	// will set the output size to the explicit size, otherwise the size from the spatial
	// paths's bounding box will be used as default.
	bool specified = false;
	for (int i = 0; i < OutputImageDimension; i++)
	{
		if (m_Size[i] != 0)
		{
			specified = true;
			break;
		}
	}
	if (!specified)
	{
		itkExceptionMacro( << "Currently, the user MUST specify an image size" )
	}

	region.SetSize( m_Size );
	region.SetIndex( index );

	OutputImage->SetLargestPossibleRegion( region);     // 
	OutputImage->SetBufferedRegion( region );           // set the region 
	OutputImage->SetRequestedRegion( region );          //                                                                       

	// If the spacing has been explicitly specified, the filter
	// will set the output spacing to that explicit spacing, otherwise the spacing from
	// the spatial object is used as default.
	specified = false;
	for (int i = 0; i < OutputImageDimension; i++)
	{
		if (m_Spacing[i] != 0)
		{
			specified = true;
			break;
		}
	}
	if (!specified)
	{
		itkExceptionMacro( << "Currently, the user MUST specify an image spacing" )
	}

	OutputImage->SetSpacing(this->m_Spacing);   // set spacing
	OutputImage->SetOrigin(this->m_Origin);			//   and origin

	/// TODO: does superclass do this?
	OutputImage->Allocate();    // allocate the image                            
}


///////////////////////////////////////////////////////////////////////////////
void 
	ContoursToRegionFilter::GenerateData()
{
	itkDebugMacro(<< "ContoursToRegionFilter::GenerateData() called");

	// Get the input and output pointers 
	const InputPathType * InputPath   = this->GetInput();
	OutputImagePointer OutputImage = this->GetOutput();
	OutputImage->FillBuffer(m_BackgroundValue);

	// region generation
	for (int nContour = 0; nContour < this->GetNumberOfInputs(); nContour++)
	{
		GeneratePlaneRegion(GetInput(nContour));
	}

	itkDebugMacro(<< "PathToImageFilter::GenerateData() finished");

} // end update function  


///////////////////////////////////////////////////////////////////////////////
void 
	ContoursToRegionFilter::GeneratePlaneRegion(ContourType::ConstPointer pContour)
{
	OutputImagePointer pOutputImage = this->GetOutput();

	CDC dc;
	BOOL bRes = dc.CreateCompatibleDC(NULL);

	CBitmap bitmap;
	bRes = bitmap.CreateBitmap(pOutputImage->GetBufferedRegion().GetSize()[0], 
		pOutputImage->GetBufferedRegion().GetSize()[1], 1, 1, NULL);

	CBitmap *pOldBitmap = (CBitmap *) dc.SelectObject(&bitmap);
	dc.SelectStockObject(WHITE_PEN);
	dc.SelectStockObject(WHITE_BRUSH);

	Point<REAL,3> vOrigin = pOutputImage->GetOrigin();
	Vector<REAL,3> vSpacing = pOutputImage->GetSpacing();

	static CArray<CPoint, CPoint&> arrPoints;
	arrPoints.SetSize(pContour->GetVertexList()->size());
	REAL zPlane;
	for (int nAt = 0; nAt < pContour->GetVertexList()->size(); nAt++)
	{
		dH::VertexType vVert = pContour->GetVertexList()->at(nAt);
		vVert[0] = (vVert[0] - vOrigin[0]) / vSpacing[0];
		vVert[1] = (vVert[1] - vOrigin[1]) / vSpacing[1];

		arrPoints[nAt].x = vVert[0];
		arrPoints[nAt].y = vVert[1];

		zPlane = vVert[2];
	}
	dc.Polygon(arrPoints.GetData(), arrPoints.GetSize());

	// finished with DC
	dc.SelectObject(pOldBitmap);
	dc.DeleteDC();

	// now get the bitmap descriptor (for scan width
	BITMAP bm;
	bitmap.GetBitmap(&bm);

	// resize the buffer
	static CArray<BYTE, BYTE> arrBuffer;
	arrBuffer.SetSize(pOutputImage->GetBufferedRegion().GetSize()[1] * bm.bmWidthBytes);
	int nByteCount = bitmap.GetBitmapBits((DWORD) arrBuffer.GetSize(), arrBuffer.GetData());

	// set up the iteration
	VolumeReal::IndexType sliceIndex = pOutputImage->GetBufferedRegion().GetIndex();
	sliceIndex[2] = Round<int>((zPlane - vOrigin[2]) / vSpacing[2]);
	if (pOutputImage->GetBufferedRegion().IsInside(sliceIndex))
	{
		VolumeReal::RegionType sliceRegion = pOutputImage->GetBufferedRegion();
		sliceRegion.SetIndex(sliceIndex);
		VolumeReal::SizeType sliceSize = sliceRegion.GetSize();
		sliceSize[2] = 1;
		sliceRegion.SetSize(sliceSize);
		ImageRegionIterator<VolumeReal> iter(pOutputImage, sliceRegion);
		for (; !iter.IsAtEnd(); ++iter)
		{
			VolumeReal::IndexType index = iter.GetIndex();
			BYTE maskByte = arrBuffer[index[1] * bm.bmWidthBytes + index[0] / 8];
			maskByte >>= (7 - index[0] % 8);
			maskByte &= 0x01;
			if (maskByte != 0)
			{
				iter.Set(1.0);
			}
		}
	}

	// done with bitmap
	bitmap.DeleteObject();

}

EndNamespace(dH)
