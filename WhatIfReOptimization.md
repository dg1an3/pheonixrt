# Introduction #
Assessing a large quantity of cone beam CTs has become a significant challenge in the current RT environment.  A method is needed to quickly filter images revealing information requiring in-depth analysis, as opposed to the majority of images that don't require in-depth analysis.

If a method were available to quickly re-optimize a plan based on new information, this could provide an efficient filter based on the magnitude of improvement indicated by the re-optimization.

# Details #

"What-if" re-optimization asks the question: Given some additional information, how much better could the treatment be?  This may not be the same as re-optimizing as part of a replanning step, because the replanning may involve significantly more effort.  But if there is a means to quickly and easily perform a re-optimization based on additional data, then this could serve to aid the image assessment process.

The workflow for reviewing a CBCT with what-if re-optimization:
  1. An initial registration is imported as an SRO or done automatically
  1. A second landmark-based TPS registration is based on a set of pre-defined landmarks specific to the anatomical site
  1. Further manual adjustment of the landmarks fine-tune the fit
  1. As the landmarks are adjusted, the pheonixrt algorithm dynamically updates the resulting re-optimization result
  1. Decision:
    * Stable DVHs >> landmark shifts do not significantly affect the treatment
    * Significant variation in DVHs w.r.t. landmarks >> further investigation is warranted

# The phoenixrt algorithm #

The algorithm is a multi-resolution beamlet weight (influence matrix) optimization, based on matching of target DVH curve shapes using relative entropy.  More details can be found here: [US Patent 7369645](http://www.google.com/patents/US7369645).

A video of the optimization for a simple "2.5D" case:

<a href='http://www.youtube.com/watch?feature=player_embedded&v=eITwG8UhxOs' target='_blank'><img src='http://img.youtube.com/vi/eITwG8UhxOs/0.jpg' width='425' height=344 /></a>

The "2.5D" case is a shallow volume of 5 slices with contours on the central slice, being optimized by a single row of beamlets on each beam.