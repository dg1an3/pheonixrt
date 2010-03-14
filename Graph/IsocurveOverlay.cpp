//
//
#include "StdAfx.h"

#include <itkConstShapedNeighborhoodIterator.h>

#include <IsocurveOverlay.h>


//////////////////////////////////////////////////////////////////////////////
IsocurveOverlay::IsocurveOverlay(void)
{
	SetMinIsocurve(0.3);
	SetMaxIsocurve(1.0);
	SetIsocurveStep(0.05);
}

//////////////////////////////////////////////////////////////////////////////
IsocurveOverlay::~IsocurveOverlay(void)
{
	SetDoseVolume(NULL);
	SetIsocurveColorTable(NULL);
}

//////////////////////////////////////////////////////////////////////////////
void 
IsocurveOverlay::Draw(CDC *pDC)
{
	if (GetDoseVolume() == NULL)
		return;

	ColorTable<unsigned char>::Pointer pColormap = GetIsocurveColorTable();
	for (VoxelReal c = GetMinIsocurve(); c <= GetMaxIsocurve(); c += GetIsocurveStep())
	{
		int colorIndex = Round<int>(c * (pColormap->GetNumberOfColors()-1));
		RGBPixel<unsigned char> *pColor = pColormap->GetColor(colorIndex);
		CPen pen(PS_SOLID, 1, RGB(pColor->GetBlue(), pColor->GetGreen(), pColor->GetRed()));
		pDC->SelectObject(&pen);
#ifdef NDEBUG
		GenerateContour(pDC, c);
#endif
	}
}


//////////////////////////////////////////////////////////////////////////////
inline 
dH::VertexType
	InterpolateContourPosition(VoxelReal c, 
																VoxelReal fromValue, VoxelReal toValue,
																const VolumeReal::IndexType& fromIndex, 
																const VolumeReal::OffsetType& toOffset)
{
  // Now calculate the fraction of the way from 'from' to 'to' that the contour
  // crosses. Interpolate linearly: y = v0 + (v1 - v0) * x, and solve for the
  // x that gives y = m_ContourValue: x = (m_ContourValue - v0) / (v1 - v0).
  // This assumes that v0 and v1 are separated by exactly ONE unit. So the to
  // Offset. value must have exactly one component 1 and the other component 0.
  // Also this assumes that fromValue and toValue are different. Otherwise we
  // can't interpolate anything!
  assert(fromValue != toValue);

  assert( (toOffset[0] == 0 && toOffset[1] == 1) || 
          (toOffset[0] == 1 && toOffset[1] == 0)     );

  double x = (c - fromValue) / (toValue - fromValue);

	dH::VertexType output;
  output[0] = fromIndex[0] + x * toOffset[0];
  output[1] = fromIndex[1] + x * toOffset[1];

  return output;
}

//////////////////////////////////////////////////////////////////////////////
void
	IsocurveOverlay::GenerateContour(CDC *pDC, VoxelReal c)
{
	VolumeReal *pVolume = GetDoseVolume();

  // Set up an iterator to "march the squares" across the image.
  // We associate each 2px-by-2px square with the pixel in the upper left of
  // that square. We then iterate across the image, examining these 2x2 squares
  // and building the contour. By iterating the upper-left pixel of our 
  // "current square" across every pixel in the image except those on the 
  // bottom row and rightmost column, we have visited every valid square in the
  // image.
  
	VolumeReal::RegionType region = pVolume->GetRequestedRegion();
  VolumeReal::SizeType shrunkSize = region.GetSize();
  shrunkSize[0] -= 1;
  shrunkSize[1] -= 1;
	shrunkSize[2] = 1;

	VolumeReal::IndexType index = region.GetIndex();

	// compute the slice position
	const itk::Point<REAL>& srcOrigin = pVolume->GetOrigin();
	const itk::Vector<REAL>& srcSpacing = pVolume->GetSpacing();
	int nStrideZ = Round<int>(((*m_pOrigin)[2] - srcOrigin[2])/srcSpacing[2]);
	if (nStrideZ < 0 || nStrideZ >= region.GetSize()[2])
		return;
	index[2] = nStrideZ;

	// and the total region for the iteration
  VolumeReal::RegionType shrunkRegion(index, shrunkSize);

  // A 1-pixel radius sets up a neighborhood with the following indices:
  // 0 1 2
  // 3 4 5
  // 6 7 8
  // We are interested only in the square of 4,5,7,8 which is the 2x2 square 
  // with the center pixel at the top-left. So we only activate the 
  // coresponding offsets, and only query pixels 4, 5, 7, and 8 with the 
  // iterator's GetPixel method.
  typedef ConstShapedNeighborhoodIterator<VolumeReal> SquareIterator;
  SquareIterator::RadiusType radius = {{1,1,0}};
  SquareIterator it(radius, pVolume, shrunkRegion);
  VolumeReal::OffsetType none  = {{0,0,0}};
  VolumeReal::OffsetType right = {{1,0,0}};
  VolumeReal::OffsetType down  = {{0,1,0}};
  VolumeReal::OffsetType diag  = {{1,1,0}};
  it.ActivateOffset(none);
  it.ActivateOffset(right);
  it.ActivateOffset(down);
  it.ActivateOffset(diag);

	for(it.GoToBegin(); !it.IsAtEnd(); ++it)
	{
		// There are sixteen different possible square types, diagramed below.
		// A + indicates that the vertex is above the contour value, and a -
		// indicates that the vertex is below or equal to the contour value.
		// The vertices of each square are here numbered:
		// 01
		// 23
		// and treated as a binary value with the bits in that order. Thus each
		// square can be so numbered:
		//  0--   1+-   2-+   3++   4--   5+-   6-+   7++
		//   --    --    --    --    +-    +-    +-    +-
		//
		//  8--   9+-  10-+  11++  12--  13+-  14-+  15++
		//   -+    -+    -+    -+    ++    ++    ++    ++
		//
		// The position of the line segment that cuts through (or doesn't, in case
		// 0 and 15) each square is clear, except in cases  6 and 9. In this case, 
		// where the segments are placed is determined by 
		// m_VertexConnectHighPixels. If m_VertexConnectHighPixels is false, then
		// lines like are drawn through square 6, and lines like are drawn through
		// square 9. Otherwise, the situation is reversed.
		// Finally, recall that we draw the lines so that (moving from tail to 
		// head) the lower-valued pixels are on the left of the line. So, for 
		// example, case 1 entails a line slanting from the middle of the top of 
		// the square to the middle of the left side of the square.

		// (1) Determine what number square we are currently inspecting. Remember 
		// that as far as the neighborhood iterator is concerned, our square
		// 01    is numbered as    45
		// 23                      78

		VoxelReal v0, v1, v2, v3;
		v0 = it.GetPixel(4);
		v1 = it.GetPixel(5);
		v2 = it.GetPixel(7);
		v3 = it.GetPixel(8);
		VolumeReal::IndexType index = it.GetIndex();
		unsigned char squareCase = 0;
		if (v0 > c) squareCase += 1;
		if (v1 > c) squareCase += 2;
		if (v2 > c) squareCase += 4;
		if (v3 > c) squareCase += 8;

		const bool m_VertexConnectHighPixels = true;

		// Set up macros to find the ContinuousIndex where the contour intersects
		// one of the sides of the square.  Normally macros should, of course, be
		// eschewed, but since this is an inner loop not calling the function four
		// times when two would do is probably worth while. Plus, copy-pasting
		// these into the switch below is even worse.  InterpolateContourPosition
		// takes the values at two vertices, the index of the first vertex, and the
		// offset between the two vertices.
		#define TOP_     InterpolateContourPosition(c,v0,v1,index,       right)
		#define BOTTOM_  InterpolateContourPosition(c,v2,v3,index + down,right)
		#define LEFT_    InterpolateContourPosition(c,v0,v2,index,       down)
		#define RIGHT_   InterpolateContourPosition(c,v1,v3,index + right,down)

		// (2) Add line segments to the growing contours as defined by the cases.
		// AddSegment takes a "from" vertex and a "to" vertex, and adds it to the
		// a growing contour, creates a new contour, or merges two together.
		switch(squareCase)
		{
		case 0: // no line
			break;
		case 1:  // top to left
			this->DrawSegment(pDC, TOP_, LEFT_);
			break;
		case 2: // right to top
			this->DrawSegment(pDC, RIGHT_, TOP_);
			break;
		case 3: // right to left
			this->DrawSegment(pDC, RIGHT_, LEFT_);
			break;
		case 4: // left to bottom
			this->DrawSegment(pDC, LEFT_, BOTTOM_);
			break;
		case 5: // top to bottom
			this->DrawSegment(pDC, TOP_, BOTTOM_);
			break;
		case 6:
			if (m_VertexConnectHighPixels)
			{
				// left to top
				this->DrawSegment(pDC, LEFT_, TOP_);
				// right to bottom
				this->DrawSegment(pDC, RIGHT_, BOTTOM_);
			}
			else
			{
				// right to top
				this->DrawSegment(pDC, RIGHT_, TOP_);
				// left to bottom
				this->DrawSegment(pDC, LEFT_, BOTTOM_);
			}
			break;
		case 7: // right to bottom
			this->DrawSegment(pDC, RIGHT_, BOTTOM_);
			break;
		case 8: // bottom to right
			this->DrawSegment(pDC, BOTTOM_, RIGHT_);
			break;
		case 9:
			if (m_VertexConnectHighPixels)
			{
				// top to right
				this->DrawSegment(pDC, TOP_, RIGHT_);
				// bottom to left
				this->DrawSegment(pDC, BOTTOM_, LEFT_);
			}
			else
			{
				// top to left
				this->DrawSegment(pDC, TOP_, LEFT_);
				// bottom to right
				this->DrawSegment(pDC, BOTTOM_, RIGHT_);
			}
			break;
		case 10: // bottom to top
			this->DrawSegment(pDC, BOTTOM_, TOP_);
			break;
		case 11: // bottom to left
			this->DrawSegment(pDC, BOTTOM_, LEFT_);
			break;
		case 12: // left to right
			this->DrawSegment(pDC, LEFT_, RIGHT_);
			break;
		case 13: // top to right
			this->DrawSegment(pDC, TOP_, RIGHT_);
			break;
		case 14: // left to top
			this->DrawSegment(pDC, LEFT_, TOP_);
			break;
		case 15: // no line
			break;
		} // switch squareCase
	} // iteration
}

//////////////////////////////////////////////////////////////////////////////
void 
	IsocurveOverlay::DrawSegment(CDC *pDC, const dH::VertexType& from, 
																 const dH::VertexType& to)																
{
	VolumeReal *pVolume = GetDoseVolume();

	VolumeReal::PointType fromPoint;
	pVolume->TransformContinuousIndexToPhysicalPoint(from, fromPoint);
	MoveTo(pDC, fromPoint);

	VolumeReal::PointType toPoint;
	pVolume->TransformContinuousIndexToPhysicalPoint(to, toPoint);
	LineTo(pDC, toPoint);
}
