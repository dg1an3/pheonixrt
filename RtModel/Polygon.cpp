// Copyright (C) 2nd Messenger Systems
// $Id: Polygon.cpp 602 2008-09-14 16:54:49Z dglane001 $
#include "stdafx.h"

#include <float.h>

// utility macros
#include <UtilMacros.h>

// class declaration
#include "Polygon.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CPolygon::CPolygon
// 
// constructs an empty polygon
//////////////////////////////////////////////////////////////////////
CPolygon::CPolygon()
{
}	// CPolygon::CPolygon


//////////////////////////////////////////////////////////////////////
// CPolygon::CPolygon(const CPolygon& fromPoly)
// 
// constructs a copy of the polygon
//////////////////////////////////////////////////////////////////////
CPolygon::CPolygon(const CPolygon& fromPoly)
{
	(*this) = fromPoly;

}	// CPolygon::CPolygon(const CPolygon& fromPoly)


//////////////////////////////////////////////////////////////////////
// declares CPolygon as a serializable class
//////////////////////////////////////////////////////////////////////
#define POLYGON_SCHEMA 2
	// Schema 1: geometry description, blocks
#ifdef USE_MFC_SERIALIZATION
IMPLEMENT_SERIAL(CPolygon, CObject, VERSIONABLE_SCHEMA | POLYGON_SCHEMA)
#endif

//////////////////////////////////////////////////////////////////////
// CPolygon::~CPolygon
// 
// destroys the polygon
//////////////////////////////////////////////////////////////////////
CPolygon::~CPolygon()
{
}	// CPolygon::~CPolygon


//////////////////////////////////////////////////////////////////////
// CPolygon::operator=
// 
// assignment operator
//////////////////////////////////////////////////////////////////////
CPolygon& CPolygon::operator=(const CPolygon& fromPoly)
{
	// copy the vertices
	// m_arrVertex.Copy(fromPoly.m_arrVertex);
	m_mVertex = fromPoly.m_mVertex;

	// return a reference to this
	return (*this);

}	// CPolygon::operator=


//////////////////////////////////////////////////////////////////////
// CPolygon::GetVertexCount
// 
// returns the number of vertices in the polygon
//////////////////////////////////////////////////////////////////////
int CPolygon::GetVertexCount() const
{
	return m_mVertex.GetCols();

}	// CPolygon::GetVertexCount


//////////////////////////////////////////////////////////////////////
// CPolygon::GetVertexAt
// 
// returns the vertex at the given index.  forms the modulus index
//////////////////////////////////////////////////////////////////////
const itk::Vector<REAL,2>& CPolygon::GetVertexAt(int nIndex) const
{
	// form the modulus index
	int nModIndex = nIndex;
	while (nModIndex < 0)
	{
		nModIndex += GetVertexCount();
	}

	nModIndex %= GetVertexCount();

	// return the desired vertex
	const REAL *pElem = &m_mVertex[nModIndex][0];
	return reinterpret_cast< const itk::Vector<REAL,2>& >(*pElem);

}	// CPolygon::GetVertexAt


//////////////////////////////////////////////////////////////////////
// CPolygon::AddVertex
// 
// adds a new vertex to the end of the polygon
//////////////////////////////////////////////////////////////////////
int CPolygon::AddVertex(const itk::Vector<REAL,2>& v)
{
	// vertex index = current column count
	int nIndex = m_mVertex.GetCols();

	// reshape the matrix
	m_mVertex.Reshape(m_mVertex.GetCols()+1, 2, TRUE);
	m_mVertex[nIndex][0] = v[0];
	m_mVertex[nIndex][1] = v[1];

	// fire a change
	GetChangeEvent().Fire();

	// return the index of the new vertex
	return nIndex;

}	// CPolygon::AddVertex


//////////////////////////////////////////////////////////////////////
// CPolygon::RemoveVertex
// 
// removes a vertex from the polygon
//////////////////////////////////////////////////////////////////////
void CPolygon::RemoveVertex(int nIndex)
{
	// form the modulus index
	int nModIndex = nIndex;
	while (nModIndex < 0)
		nModIndex += GetVertexCount();

	nModIndex %= GetVertexCount();

	// remove the vertex at the modulus index
	// m_arrVertex.RemoveAt(nModIndex);
	ASSERT(FALSE);

	// fire a change event
	GetChangeEvent().Fire();

}	// CPolygon::RemoveVertex


//////////////////////////////////////////////////////////////////////
// CPolygon::LockVertexMatrix
// 
// returns a CHANGEABLE reference to the polygon vertices -- must
//		be paired with an UnlockVertexMatrix call
//////////////////////////////////////////////////////////////////////
CMatrixNxM<REAL>& CPolygon::LockVertexMatrix()
{
	return m_mVertex;

}	// CPolygon::LockVertexMatrix


//////////////////////////////////////////////////////////////////////
// CPolygon::UnlockVertexMatrix
// 
// returns a CHANGEABLE reference to the polygon vertices -- must
//		be paired with an UnlockVertexMatrix call
//////////////////////////////////////////////////////////////////////
void CPolygon::UnlockVertexMatrix(BOOL bChanged)
{
	if (bChanged)
	{
		GetChangeEvent().Fire();
	}

}	// CPolygon::UnlockVertexMatrix


#ifdef USE_MFC_SERIALIZATION
//////////////////////////////////////////////////////////////////////
// CPolygon::Serialize
// 
// serializes the polygon
//////////////////////////////////////////////////////////////////////
void CPolygon::Serialize(CArchive &ar)
{
	// store the schema for the beam object
	UINT nSchema = POLYGON_SCHEMA;
	SERIALIZE_VALUE(ar, nSchema);

	SERIALIZE_VALUE(ar, m_mVertex);

}	// CPolygon::Serialize
#endif
