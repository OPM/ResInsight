.. toctree::
   :maxdepth: 1

.. _prior_distributions:

Prior distributions avaliable in ERT
====================================

The :ref:`GEN_KW <gen_kw>` keyword is typically used in sensitivy
studies and as parameters which are updated with the Ensemble Smoother
in a model updating project. In your configuration file the
:ref:`GEN_KW <gen_kw>` keyword is configured as: 

::
   
    GEN_KW  ID  my_template.txt  my_eclipse_include.txt  my_priors.txt

The file ``my_priors.txt`` contains the names of the variables
you are considering, and specifies the distribution which should be
used for the initial sampling. 


NORMAL
------
To set a normal (Gaussian) prior, use the keyword NORMAL. It takes two
arguments, a mean value and a standard deviation. Thus, the following
example will assign a normal prior with mean 0 and standard deviation
1 to the variable VAR1:

::
   
   VAR1   NORMAL    0   1

LOGNORMAL
---------
A stochastic variable is log normally distributed if the logarithm of
the variable is normally distributed. In other words, if X is normally
distributed, then Y = exp(X) is log normally distributed.

A log normal prior is suited to model positive quanties with a heavy
tail (tendency to take large values). To set a log normal prior, use
the keyword LOGNORMAL. It takes two arguments, the mean and standard
deviation of the *logarithm* of the variable:

::
   
   VAR2   LOGNORMAL  0

TRUNCATED_NORMAL 
-----------------

This *TRUNCATED_NORMAL* distribution works as follows:

   1. Draw random variable X ~ N(mu,std)
   2. Clamp X to the interval [min, max]

This is **not** a proper truncated normal distribution; hence the
clamping to ``[min,max]` should be an exceptional event. To configure
this distribution for a situation with mean 1, standard deviation 0.25
and hard limits 0 and 10:

::

   VAR3  TRUNCATED_NORMAL  1  0.25   0  10

   
UNIFORM
-------

A stochastic variable is uniformly distributed if has a constant
probability density on a closed interval. Thus, the uniform
distribution is completely characterized by it's minimum and maximum
value. To assign a uniform distribution to a variable, use the keyword
UNIFORM, which takes a minimum and a maximum value for a the
variable. Here is an example, which assigns a uniform distribution
between 0 and 1 to a variable ``VAR4``:

::

   VAR4 UNIFORM 0 1

It can be shown that among all distributions bounded below by a and
above by b, the uniform distribution with parameters a and b has the
maximal entropy (contains the least information). Thus, the uniform
distribution should be your preferred prior distribution for robust
modeling of bounded variables.


LOGUNIF
-------

A stochastic variable is log uniformly distributed if it's logarithm
is uniformly distributed on the interval [a,b]. To assign a log
uniform distribution to a a variable, use the keyword LOGUNIF, which
takes a minimum and a maximum value for the output variable as
arguments. The example

::  

   VAR5  LOGUNIF 0.00001 1 

will give values in the range [0.00001,1] - with considerably more
weight towards the lower limit. The log uniform distribution is useful
when modeling a bounded positive variable who has most of it's
probability weight towards one of the bounds.

CONST
-----

The keyword CONST is used to assign a Dirac distribution to a
variable, i.e. set it to a constant value. Here is an example of use:

::

    CONST 1.0



Priors and transformations
==========================

The Ensemble Smoother method, which ERT uses for updating of
parameters, works with normally distributed variables. So internally
in ERT the interplay between ``GEN_KW`` variables and updates is as
follows:

  1. ERT samples a random variable ``x ~ N(0,1)`` - before outputing
     to the forward model this is *transformed* to ``y ~ F(Y)`` where
     the the distribution ``F(Y)`` is the correct prior distribution.

  2. When the prior simulations are complete ERT calculates misfits
     between simulated and observed values and *updates* the
     parameters; hence the variables ``x`` now represent samples from
     a posterior distribution which is Normal with mean and standard
     deviation *different from (0,1)*.

The transformation prescribed by ``F(y)`` still "works" - but it no
longer maps to a distribution in the same family as initially
specified by the prior. A consequence of this is that the update
process can *not* give you a posterior with updated parameters in the
same distribution family as the Prior.
