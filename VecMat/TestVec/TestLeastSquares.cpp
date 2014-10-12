/*	CMatrixNxM<> a(5, 3);

	a[0][0] = 22.0;
	a[0][1] = 31.0;
	a[0][2] = 1.0;

	a[1][0] = 63.0;
	a[1][1] = 21.0;
	a[1][2] = 1.0;

	a[2][0] = 36.0;
	a[2][1] = 60.0;
	a[2][2] = 1.0;

	a[3][0] = 76.0;
	a[3][1] = 65.0;
	a[3][2] = 1.0;

	a[4][0] = 34.0;
	a[4][1] = 88.0;
	a[4][2] = 1.0;

	TraceMatrix("a", a);

	CMatrixNxM<> b(5, 3);

/* B1
	b[0][0] = 26.0;
	b[0][1] = 26.0;
	b[0][2] = 1.0;

	b[1][0] = 74.0;
	b[1][1] = 17.0;
	b[1][2] = 1.0;

	b[2][0] = 40.0;
	b[2][1] = 54.0;
	b[2][2] = 1.0;

	b[3][0] = 85.0;
	b[3][1] = 64.0;
	b[3][2] = 1.0;

	b[4][0] = 42.0;
	b[4][1] = 93.0;
	b[4][2] = 1.0;
* /
	b[0][0] = 33.0;
	b[0][1] = 23.0;
	b[0][2] = 1.0;

	b[1][0] = 78.0;
	b[1][1] = 10.0;
	b[1][2] = 1.0;

	b[2][0] = 48.0;
	b[2][1] = 51.0;
	b[2][2] = 1.0;

	b[3][0] = 89.0;
	b[3][1] = 60.0;
	b[3][2] = 1.0;

	b[4][0] = 51.0;
	b[4][1] = 88.0;
	b[4][2] = 1.0;

	TraceMatrix("b", b);
*/
	// work with the transpose of the problem, so that columns < rows
	mA.Transpose();
	mB.Transpose();

	// form the pseudo-inverse of the a coordinates
	CMatrixNxM<> mA_ps = mA;
	if (!mA_ps.Pseudoinvert())
		return FALSE;

	// form the transform matrix
	mT = mB * mA_ps;

	// un-transpose the matrices
	mA.Tranpose();
	mB.Transpose();
	mT.Transpose();

/*	// test transform
	t.Invert();		// invert for transform from b -> a
	CMatrixNxM<> bTrans = t * b;
	TraceMatrix("bTrans", bTrans); */

	return TRUE;
