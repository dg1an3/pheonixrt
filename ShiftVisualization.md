# Introduction #
Interpretation of IGRT shift information is not the easiest thing in the world.  Elekta's Mosaiq, for one, has had a terrible time defining a coherent interpretation of shifts that is robust irrespective of online device conventions, regional settings, and general user entry errors.

# Details #
Maybe the problem is that we are expecting people to be more adept at spatial reasoning than the actually are.  Just copying values from a TrueBeam, and expecting no errors to be made is probably an unreasonable burden for most people, because they may not be able to tell easily if the values were copied wrong.  A simple substitution of a dot for a comma on an installation in another country could easily change the entered value by several orders of magnitude, especially if it were not noticed.

But wouldn't a several order of magnitude shift be easily noticed?  For a pair of coregistered images, changing a shift of 0.1 cm to 100.0 cm would be easily noticed (and would inevitably create values that are outside the range of normal machine tolerance).  But what is the likelihood that a dot/comma confusion would cause a smaller error?

I have produced a simulation of possible transpositions of dot/commas, and my results seem to indicate that, with proper bounds checking, the likelihood of a mis-entered value being silently accepted is limited to a fairly specific set of input ranges.  In particular, a value would need to be formatted as 9,999.999 or 99,999.999 in order to be susceptible to dot/comma transposition.  A number formatted 999.99 or smaller would be rejected: an incorrect thousand separator would cause the value to be larger than the maximum, and an incorrect decimal separator would be flagged as being below the allowed precision.

At any rate, after entry has been made, typically a comparison is needed to another reference.  As we have learned with the TrueBeam offsets, this interpretation may involve comparing other coordinate systems or even other correction models.  An appropriate verification (see VerificationExpertSystem) would help to ensure that this is error-free.

Finally, display of a visual indication of the shift would be great.  Here is a schematic I made to show one possible way of indicating 6 DoF shifts on a mannequin:

![http://i.imgur.com/YA0ffcD.jpg](http://i.imgur.com/YA0ffcD.jpg)