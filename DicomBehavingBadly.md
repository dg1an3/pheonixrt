# Introduction #

After having spent a number of years dealing with very creative DICOM encodings, I thought maybe others might be amused by some of these.

# Details #
  * _NaN is Not A Number_ - so it shouldn't show up inside Decimal Strings, correct?  Maybe Varian's OBI and the Mirada planning system don't agree with this.  Of course, the fact that XiO and Mosaiq both pass the bad values through without flagging doesn't help anything...
  * _Is an orthogonal grid supposed to be orthogonal_ - an old bug in the OBI used to produce (under some conditions) CBCTs with non-orthogonal volume grids.  This was specifically because the direction cosines were rotated (Varian rotates the direction cosines if the couch has been rotated), but the image positions were not adjusted to lie along the rotated Z-axis.