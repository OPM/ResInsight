.. index::
   single: standards conformance, ANSI C
   single: ANSI C, use of
   single: C extensions, compatible use of
   single: compatibility

*****************
Using the Library
*****************

.. include:: include.rst

This chapter describes how to compile programs that use GSL, and
introduces its conventions.  

An Example Program
==================

The following short program demonstrates the use of the library by
computing the value of the Bessel function :math:`J_0(x)` for :math:`x=5`:

.. include:: examples/intro.c
   :code:

The output is shown below, and should be correct to double-precision
accuracy [#f1]_,

.. include:: examples/intro.txt
   :code:

The steps needed to compile this program are described
in the following sections.

.. index::
   single: compiling programs, include paths
   single: including GSL header files
   single: header files, including

Compiling and Linking
=====================

The library header files are installed in their own :file:`gsl`
directory.  You should write any preprocessor include statements with a
:file:`gsl/` directory prefix thus::

    #include <gsl/gsl_math.h>

If the directory is not installed on the standard search path of your
compiler you will also need to provide its location to the preprocessor
as a command line flag.  The default location of the :file:`gsl`
directory is :file:`/usr/local/include/gsl`.  A typical compilation
command for a source file :file:`example.c` with the GNU C compiler
:code:`gcc` is::

    $ gcc -Wall -I/usr/local/include -c example.c

This results in an object file :file:`example.o`. The default
include path for :code:`gcc` searches :file:`/usr/local/include` automatically so
the :code:`-I` option can actually be omitted when GSL is installed 
in its default location.

.. index::
   single: compiling programs, library paths
   single: linking with GSL libraries
   single: libraries, linking with

Linking programs with the library
---------------------------------

The library is installed as a single file, :file:`libgsl.a`.  A shared
version of the library :file:`libgsl.so` is also installed on systems
that support shared libraries.  The default location of these files is
:file:`/usr/local/lib`.  If this directory is not on the standard search
path of your linker you will also need to provide its location as a
command line flag.

To link against the library you need to specify
both the main library and a supporting |cblas| library, which
provides standard basic linear algebra subroutines.  A suitable
|cblas| implementation is provided in the library
:file:`libgslcblas.a` if your system does not provide one.  The following
example shows how to link an application with the library::

    $ gcc -L/usr/local/lib example.o -lgsl -lgslcblas -lm

The default library path for :code:`gcc` searches :file:`/usr/local/lib`
automatically so the :code:`-L` option can be omitted when GSL is
installed in its default location.  

The option :code:`-lm` links with the system math library. On some
systems it is not needed. [#f2]_

For a tutorial introduction to the GNU C Compiler and related programs,
see "An Introduction to GCC" (ISBN 0954161793). [#f3]_

Linking with an alternative BLAS library
----------------------------------------

The following command line shows how you would link the same application
with an alternative |cblas| library :file:`libcblas.a`::

    $ gcc example.o -lgsl -lcblas -lm

For the best performance an optimized platform-specific |cblas|
library should be used for :code:`-lcblas`.  The library must conform to
the |cblas| standard.  The |atlas| package provides a portable
high-performance |blas| library with a |cblas| interface.  It is
free software and should be installed for any work requiring fast vector
and matrix operations.  The following command line will link with the
|atlas| library and its |cblas| interface::

    $ gcc example.o -lgsl -lcblas -latlas -lm

If the |atlas| library is installed in a non-standard directory use
the :code:`-L` option to add it to the search path, as described above.  

For more information about |blas| functions see :ref:`chap_blas-support`.

.. index::
   single: shared libraries
   single: libraries, shared
   single: LD_LIBRARY_PATH

Shared Libraries
================

To run a program linked with the shared version of the library the
operating system must be able to locate the corresponding :file:`.so`
file at runtime.  If the library cannot be found, the following error
will occur::

    $ ./a.out 
    ./a.out: error while loading shared libraries: 
    libgsl.so.0: cannot open shared object file: No such file or directory

To avoid this error, either modify the system dynamic linker
configuration [#f4]_ or
define the shell variable :code:`LD_LIBRARY_PATH` to include the
directory where the library is installed.

For example, in the Bourne shell (:code:`/bin/sh` or :code:`/bin/bash`),
the library search path can be set with the following commands::

    $ LD_LIBRARY_PATH=/usr/local/lib
    $ export LD_LIBRARY_PATH
    $ ./example

In the C-shell (:code:`/bin/csh` or :code:`/bin/tcsh`) the equivalent
command is::

    % setenv LD_LIBRARY_PATH /usr/local/lib

The standard prompt for the C-shell in the example above is the percent
character %, and should not be typed as part of the command.

To save retyping these commands each session they can be placed in an
individual or system-wide login file.

To compile a statically linked version of the program, use the
:code:`-static` flag in :code:`gcc`::

    $ gcc -static example.o -lgsl -lgslcblas -lm

ANSI C Compliance
=================

The library is written in ANSI C and is intended to conform to the ANSI
C standard (C89).  It should be portable to any system with a working
ANSI C compiler.

The library does not rely on any non-ANSI extensions in the interface it
exports to the user.  Programs you write using GSL can be ANSI
compliant.  Extensions which can be used in a way compatible with pure
ANSI C are supported, however, via conditional compilation.  This allows
the library to take advantage of compiler extensions on those platforms
which support them.

When an ANSI C feature is known to be broken on a particular system the
library will exclude any related functions at compile-time.  This should
make it impossible to link a program that would use these functions and
give incorrect results.

To avoid namespace conflicts all exported function names and variables
have the prefix :code:`gsl_`, while exported macros have the prefix
:code:`GSL_`.

.. index::
   single: inline functions
   single: HAVE_INLINE
   single: GSL_C99_INLINE
   single: C99, inline keyword
   single: extern inline

.. _sec_inline-functions:

Inline functions
================

The :code:`inline` keyword is not part of the original ANSI C standard
(C89) so the library does not export any inline function definitions
by default.  Inline functions were introduced officially in the newer
C99 standard but most C89 compilers have also included :code:`inline` as
an extension for a long time.

To allow the use of inline functions, the library provides optional
inline versions of performance-critical routines by conditional
compilation in the exported header files.  The inline versions of these
functions can be included by defining the macro :code:`HAVE_INLINE`
when compiling an application::

    $ gcc -Wall -c -DHAVE_INLINE example.c

If you use :code:`autoconf` this macro can be defined automatically.  If
you do not define the macro :code:`HAVE_INLINE` then the slower
non-inlined versions of the functions will be used instead.

By default, the actual form of the inline keyword is :code:`extern
inline`, which is a :code:`gcc` extension that eliminates unnecessary
function definitions.  If the form :code:`extern inline` causes
problems with other compilers a stricter autoconf test can be used,
see :ref:`chap_autoconf-macros`.

When compiling with :code:`gcc` in C99 mode (:code:`gcc -std=c99`) the
header files automatically switch to C99-compatible inline function
declarations instead of :code:`extern inline`.  With other C99
compilers, define the macro :code:`GSL_C99_INLINE` to use these
declarations.

Long double
===========
.. index:
   long double

In general, the algorithms in the library are written for double
precision only.  The :code:`long double` type is not supported for
actual computation.

One reason for this choice is that the precision of :code:`long double`
is platform dependent.  The IEEE standard only specifies the minimum
precision of extended precision numbers, while the precision of
:code:`double` is the same on all platforms.

However, it is sometimes necessary to interact with external data in
long-double format, so the vector and matrix datatypes include
long-double versions.

It should be noted that in some system libraries the :code:`stdio.h`
formatted input/output functions :code:`printf` and :code:`scanf` are
not implemented correctly for :code:`long double`.  Undefined or
incorrect results are avoided by testing these functions during the
:code:`configure` stage of library compilation and eliminating certain
GSL functions which depend on them if necessary.  The corresponding
line in the :code:`configure` output looks like this::

    checking whether printf works with long double... no

Consequently when :code:`long double` formatted input/output does not
work on a given system it should be impossible to link a program which
uses GSL functions dependent on this.

If it is necessary to work on a system which does not support formatted
:code:`long double` input/output then the options are to use binary
formats or to convert :code:`long double` results into :code:`double` for
reading and writing.

.. _portability-functions:

Portability functions
=====================

To help in writing portable applications GSL provides some
implementations of functions that are found in other libraries, such as
the BSD math library.  You can write your application to use the native
versions of these functions, and substitute the GSL versions via a
preprocessor macro if they are unavailable on another platform. 

For example, after determining whether the BSD function :func:`hypot` is
available you can include the following macro definitions in a file
:file:`config.h` with your application::

    /* Substitute gsl_hypot for missing system hypot */

    #ifndef HAVE_HYPOT
    #define hypot gsl_hypot
    #endif

The application source files can then use the include command
:code:`#include <config.h>` to replace each occurrence of :func:`hypot` by
:func:`gsl_hypot` when :func:`hypot` is not available.  This substitution
can be made automatically if you use :code:`autoconf`, see :ref:`chap_autoconf-macros`.

In most circumstances the best strategy is to use the native versions of
these functions when available, and fall back to GSL versions otherwise,
since this allows your application to take advantage of any
platform-specific optimizations in the system library.  This is the
strategy used within GSL itself.

.. index::
   single: alternative optimized functions
   single: optimized functions, alternatives

Alternative optimized functions
===============================

The main implementation of some functions in the library will not be
optimal on all architectures.  For example, there are several ways to
compute a Gaussian random variate and their relative speeds are
platform-dependent.  In cases like this the library provides alternative
implementations of these functions with the same interface.  If you
write your application using calls to the standard implementation you
can select an alternative version later via a preprocessor definition.
It is also possible to introduce your own optimized functions this way
while retaining portability.  The following lines demonstrate the use of
a platform-dependent choice of methods for sampling from the Gaussian
distribution::

    #ifdef SPARC
    #define gsl_ran_gaussian gsl_ran_gaussian_ratio_method
    #endif
    #ifdef INTEL
    #define gsl_ran_gaussian my_gaussian
    #endif

These lines would be placed in the configuration header file
:file:`config.h` of the application, which should then be included by all
the source files.  Note that the alternative implementations will not
produce bit-for-bit identical results, and in the case of random number
distributions will produce an entirely different stream of random
variates.

Support for different numeric types
===================================

Many functions in the library are defined for different numeric types.
This feature is implemented by varying the name of the function with a
type-related modifier---a primitive form of C++ templates.  The
modifier is inserted into the function name after the initial module
prefix.  The following table shows the function names defined for all
the numeric types of an imaginary module :code:`gsl_foo` with function
:func:`fn`::

    gsl_foo_fn               double        
    gsl_foo_long_double_fn   long double   
    gsl_foo_float_fn         float         
    gsl_foo_long_fn          long          
    gsl_foo_ulong_fn         unsigned long 
    gsl_foo_int_fn           int           
    gsl_foo_uint_fn          unsigned int  
    gsl_foo_short_fn         short         
    gsl_foo_ushort_fn        unsigned short
    gsl_foo_char_fn          char          
    gsl_foo_uchar_fn         unsigned char 

The normal numeric precision :code:`double` is considered the default and
does not require a suffix.  For example, the function
:func:`gsl_stats_mean` computes the mean of double precision numbers,
while the function :func:`gsl_stats_int_mean` computes the mean of
integers.

A corresponding scheme is used for library defined types, such as
:code:`gsl_vector` and :code:`gsl_matrix`.  In this case the modifier is
appended to the type name.  For example, if a module defines a new
type-dependent struct or typedef :code:`gsl_foo` it is modified for other
types in the following way::

    gsl_foo                  double        
    gsl_foo_long_double      long double   
    gsl_foo_float            float         
    gsl_foo_long             long          
    gsl_foo_ulong            unsigned long 
    gsl_foo_int              int           
    gsl_foo_uint             unsigned int  
    gsl_foo_short            short         
    gsl_foo_ushort           unsigned short
    gsl_foo_char             char          
    gsl_foo_uchar            unsigned char 

When a module contains type-dependent definitions the library provides
individual header files for each type.  The filenames are modified as
shown in the below.  For convenience the default header includes the
definitions for all the types.  To include only the double precision
header file, or any other specific type, use its individual filename::

    #include <gsl/gsl_foo.h>               All types
    #include <gsl/gsl_foo_double.h>        double        
    #include <gsl/gsl_foo_long_double.h>   long double   
    #include <gsl/gsl_foo_float.h>         float         
    #include <gsl/gsl_foo_long.h>          long          
    #include <gsl/gsl_foo_ulong.h>         unsigned long 
    #include <gsl/gsl_foo_int.h>           int           
    #include <gsl/gsl_foo_uint.h>          unsigned int  
    #include <gsl/gsl_foo_short.h>         short         
    #include <gsl/gsl_foo_ushort.h>        unsigned short
    #include <gsl/gsl_foo_char.h>          char          
    #include <gsl/gsl_foo_uchar.h>         unsigned char 

.. index::
   single: C++, compatibility
   single: exceptions, C++

Compatibility with C++
======================

The library header files automatically define functions to have
:code:`extern "C"` linkage when included in C++ programs.  This allows
the functions to be called directly from C++.

To use C++ exception handling within user-defined functions passed to
the library as parameters, the library must be built with the
additional :code:`CFLAGS` compilation option :code:`-fexceptions`.

.. index:: aliasing of arrays

.. _aliasing-of-arrays:

Aliasing of arrays
==================

The library assumes that arrays, vectors and matrices passed as
modifiable arguments are not aliased and do not overlap with each other.
This removes the need for the library to handle overlapping memory
regions as a special case, and allows additional optimizations to be
used.  If overlapping memory regions are passed as modifiable arguments
then the results of such functions will be undefined.  If the arguments
will not be modified (for example, if a function prototype declares them
as :code:`const` arguments) then overlapping or aliased memory regions
can be safely used.

Thread-safety
=============

The library can be used in multi-threaded programs.  All the functions
are thread-safe, in the sense that they do not use static variables.
Memory is always associated with objects and not with functions.  For
functions which use *workspace* objects as temporary storage the
workspaces should be allocated on a per-thread basis.  For functions
which use *table* objects as read-only memory the tables can be used
by multiple threads simultaneously.  Table arguments are always declared
:code:`const` in function prototypes, to indicate that they may be
safely accessed by different threads.

There are a small number of static global variables which are used to
control the overall behavior of the library (e.g. whether to use
range-checking, the function to call on fatal error, etc).  These
variables are set directly by the user, so they should be initialized
once at program startup and not modified by different threads.

.. index:: deprecated functions

Deprecated Functions
====================

From time to time, it may be necessary for the definitions of some
functions to be altered or removed from the library.  In these
circumstances the functions will first be declared *deprecated* and
then removed from subsequent versions of the library.  Functions that
are deprecated can be disabled in the current release by setting the
preprocessor definition :code:`GSL_DISABLE_DEPRECATED`.  This allows
existing code to be tested for forwards compatibility.

.. index::
   single: code reuse in applications
   single: source code, reuse in applications

Code Reuse
==========

Where possible the routines in the library have been written to avoid
dependencies between modules and files.  This should make it possible to
extract individual functions for use in your own applications, without
needing to have the whole library installed.  You may need to define
certain macros such as :code:`GSL_ERROR` and remove some :code:`#include`
statements in order to compile the files as standalone units. Reuse of
the library code in this way is encouraged, subject to the terms of the
GNU General Public License.

.. rubric:: Footnotes

.. [#f1] The last few digits may vary slightly depending on the compiler and platform used---this is normal
.. [#f2] It is not needed on MacOS X
.. [#f3] http://www.network-theory.co.uk/gcc/intro/
.. [#f4] :file:`/etc/ld.so.conf` on GNU/Linux systems
