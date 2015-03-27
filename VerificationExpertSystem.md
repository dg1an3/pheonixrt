# Introduction #

Way back when I was working at Siemens I had the need to verify the behavior of an RT application (a CT simulator); this involved various computational geometry calculations, as well as a number of image processing operations.

I had considered setting up some MATLAB programs that would implement algorithms parallel to what had been implemented (though presumably less efficient), but then I started considering the possibility that, when verifying an algorithm its not always necessary to compare it to another implementation.  In some cases (in fact in most cases) its possible to describe the 'correct' result of the algorithm in a declarative fashion, without actually specifying how the outputs are produced from the inputs.  In other words, what is described is a set of relationships that need to hold between the inputs and the outputs, but not necessarily the operations to be performed on the inputs in order to produce the outputs.

At the time I had begun playing around with an open-source Prolog environment, [SWI-Prolog](http://www.swi-prolog.org/), that provided a fairly complete set of tools for writing and querying Prolog databases.


# Details #

A verification expert system is kinda like [executable requirements](http://agilemodeling.com/essays/agileRequirementsBestPractices.htm#ExecutableRequirements) on steroids.

The verification predicates included checks on the accuracy and correctness of:
  * Image geometry
  * Contour placement
  * Contour meshing and intersection
  * 2D and 3D margin operations
  * Threshold extraction
  * Beam geometry

More detailed explanation of the verification predicates can be found at this link:

[Expert System for Geometric RTP Algorithms](http://pheonixrt.googlecode.com/files/Expert%20System%20for%20Geom%20RTP%20Algorithms.pdf)

<a href='Hidden comment: 
Generate the equation from the codecogs server:
<img src="http://latex.codecogs.com/gif.latex?%5Cforall%20P_n%20%3A%20%5Cforall%20%5Cvec%7B%5Cmathbf%7Bv%7D%7D%20%5Cin%20P_n%20%3A%20%5Cvec%7B%5Cmathbf%7Bv%7D%7D%20%5Cin%20%7BM%7D%27" />
'></a>