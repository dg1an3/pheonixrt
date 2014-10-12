========================================================================
       CONSOLE APPLICATION : TestVec
========================================================================


AppWizard has created this TestVec application for you.  

This file contains a summary of what you will find in each of the files that
make up your TestVec application.

TestVec.dsp
    This file (the project file) contains information at the project level and
    is used to build a single project or subproject. Other users can share the
    project (.dsp) file, but they should export the makefiles locally.

TestVec.cpp
    This is the main application source file.


/////////////////////////////////////////////////////////////////////////////
Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named TestVec.pch and a precompiled types file named StdAfx.obj.


/////////////////////////////////////////////////////////////////////////////
Other notes:

AppWizard uses "TODO:" to indicate parts of the source code you
should add to or customize.

/////////////////////////////////////////////////////////////////////////////

Tests:

	CVectorN construction
	CVectorN element accessors
	CVectorN copy construction
		-> ensure dimensions are replicated
	CVectorN assignment
		-> ensure dimensions are replicated
	CVectorN length
	CVectorN normalization
	CVectorN comparison (== , !=, IsApproxEqual)
	CVectorN in-place arithmetic
	CVectorN dyadic arithmetic

	CVector<D> construction
	CVector<D> element accessors
	CVector<D> copy construction
	CVector<D> assignment
	CVector<D> length
	CVector<D> normalization
	CVector<D> comparison (== , !=, IsApproxEqual)
	CVector<D> in-place arithmetic
	CVector<D> dyadic arithmetic

	CVector<D> casting
	CVectorN casting

	