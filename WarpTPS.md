# Introduction #
One of the problems to be addressed for adaptive treatment paradigms is the need to visualize and interact with deformable vector fields (DVF).  While a number of techniques exist for visualizing vector fields, such as heat maps and hedgehog plots, a simple technique is to allow interactive morphing to examine how the vector field is altering the target image to match the source.

# Details #
This is a very old MFC program that allows loading two different PNG images, and then provides a slider to morph back and forth between them.  The source code is self contained in the [WarpTPS](http://code.google.com/p/pheonixrt/source/browse/#svn%2Ftrunk%2FWarpTPS) source repository directory.

Note that currently the two images that can be loaded through ` File > Open Images...` must be have the same width/height, and are both required to be in .BMP format.

First is a video of Grumpy to Hedgy:

<a href='http://www.youtube.com/watch?feature=player_embedded&v=ggDMs3GozSU' target='_blank'><img src='http://img.youtube.com/vi/ggDMs3GozSU/0.jpg' width='425' height=344 /></a>

This video shows one MR slice being morphed on to another one:

<a href='http://www.youtube.com/watch?feature=player_embedded&v=1w0Gk1YRcuI' target='_blank'><img src='http://img.youtube.com/vi/1w0Gk1YRcuI/0.jpg' width='425' height=344 /></a>