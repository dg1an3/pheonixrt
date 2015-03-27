The main component of pheonixrt is an implementation of an optimization algorithm that can optimize various parameters for a given treatment, including
  * the dose intensity map (analogous to inverse planning)
  * a rigid or affine registration
  * a treatment offset
  * DVH curves

In addition there is a component that can be used to analyze data imported from other systems.  This analysis can be used to determine:
  * consistency with DICOM RT or IHE-RO
  * verified algorithm implementations from other systems

pheonixrt uses the [Insight Toolkit](http://itk.org/) for its basic image processing pipeline.

[Why the New BSD License](https://code.google.com/p/pheonixrt/wiki/WhyOpenSource)














