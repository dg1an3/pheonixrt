# Introduction #

Let's start with the typical product manager's job description:

  * Ability to write vague, grammatically incorrect clinical requirements
  * Ability to refuse to review detailed software requirements documents, or even try out new features, until after the release
  * Ability to act surprised that the new software that one has never touched acts differently than one's expectations
  * Ability to blame engineers for not being able to read minds

Wouldn't an open process of submitting feature enhancement requests, voting on the most valuable ones, and then have an expandable pool of resources available to work on them would do just as well at getting software developed?

# Details #

Of course, one of the tricks is to ensure that an open source development process produces software of the same high quality as proprietary processes.  Because proprietary processes _always_ produce high quality software, correct?  Then again, you never really know for sure because you can't see the source code.  Does this make sense:  Acme Medical Software produces very high-quality code, and you know this because Acme Medical Software told you so.  Is this a scientific approach to software development?

Being able to see the code may at least help you see how poorly written it is; but once the source code is out there in the public eye then (the belief is that) it will improve as more eyeballs look at it.  But for very critical software, just having a lot of eyeballs looking at it isn't enough.  It needs to be well-tested, and the tests need to have a low cost to run (as multiple iterations of the software are rapidly produced).

Which suggests that automated testing is useful, but in the context of verifying software correctness this leads to a bit of a chicken-and-egg problem:  if the tests are comprehensive then their complexity is going to tend toward the larger size, and then how can you be sure that the test themselves are correct?

This is where the verification 'expert system' comes in; using declarative logic its possible to write relatively concise descriptions of correct program behavior.  These descriptions, in this case written in Prolog, are then available to directly verify the open implementations.  They take a while to run, but these days CPU cycles are fairly plentiful.

For the ITK point of view, see the chapter [in The Architecture of Open Source](http://www.aosabook.org/en/itk.html) book.

# New BSD License #
Copyright (c) 2007-2014, Derek G. Lane
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.