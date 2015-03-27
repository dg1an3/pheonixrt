# Introduction #

RtModel contains the data model for plans, structures, and images, as well as logic for persistence of the data.  It also contains algorithms for
  * Optimization from a prescription
  * Calculating Dose-volume histograms
  * Calculating Dose (in the case that dose hasn't been imported)

# Details #

Attached is a [class diagram](https://lh4.googleusercontent.com/-KhcqEnnSe_o/U_KNJY_1b1I/AAAAAAAAlOg/q3OTA0TIuUI/w724-h762-no/phx.png) showing the main class relationships within the RtModel package.

![https://lh4.googleusercontent.com/-KhcqEnnSe_o/U_KNJY_1b1I/AAAAAAAAlOg/q3OTA0TIuUI/w724-h762-no/phx.png](https://lh4.googleusercontent.com/-KhcqEnnSe_o/U_KNJY_1b1I/AAAAAAAAlOg/q3OTA0TIuUI/w724-h762-no/phx.png)

_In the process of converting the optimizier to use the vnl optimization base.  Unfortunately the vnl conjugate gradient optimizer has no "hook" to be able to compute the dynamic covariance, so a completely separate DynamicCovarianceOptimizer is being used.  Also a DynamicCovarianceCostFunction implements those parts of the cost function that are specific to the DynamicCovariance technique._