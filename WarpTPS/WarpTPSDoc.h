// WarpTPSDoc.h : interface of the CWarpTPSDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_WARPTPSDOC_H__A6F7D611_CDE3_48D6_97C7_E1FF9BBA326B__INCLUDED_)
#define AFX_WARPTPSDOC_H__A6F7D611_CDE3_48D6_97C7_E1FF9BBA326B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Dib.h"
#include "TPSTransform.h"	// Added by ClassView

// enum used to access images on the doc
enum ImageRole
{ 
	DestinationImage = 0, 
	WarpedDestinationImage = 3,
	SourceImage = 2, 
	WarpedSourceImage = 1, 
	BlendedImage = 4,
};


class CWarpTPSDoc : public CDocument
{
protected: // create from serialization only
	CWarpTPSDoc();
	DECLARE_DYNCREATE(CWarpTPSDoc)

// Attributes
public:

	// get one of the images: 0 for first, 
	//		2 for second, 
	//		1 for warped second
	CDib * GetImage(int nIndex);

	// get the transform object
	CTPSTransform * GetTransform();

	// get the transform object
	CTPSTransform * GetInverseTransform();

// Operations
public:
	// loads the two specified images
	BOOL LoadImages(LPCTSTR pszFilename1, LPCTSTR pszFilename2);
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWarpTPSDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual void DeleteContents();
	//}}AFX_VIRTUAL

// Implementation
public:
	void InitCornerLandmarks();
	// destroys the document
	virtual ~CWarpTPSDoc();

	CTPSTransform m_transform;
	CTPSTransform m_inversetransform;

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CWarpTPSDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	// stores the images as:
	//		0 : first image (destination)
	//		1 : warped image (warped source image)
	//		2 : second image (source)
	//		3 : inverse warped image (warped destination image)
	//		4 : blended image
	CDib *m_pImage[5];
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WARPTPSDOC_H__A6F7D611_CDE3_48D6_97C7_E1FF9BBA326B__INCLUDED_)
