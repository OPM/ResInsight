Simple introduction to Python
=============================

.. contents::
   :depth: 2
   :local: 

Python is a object oriented scripting language. The language is used
in a wide range of a applications. Beeing a interpreted scripting
language it is not blazingly fast in itself, but it is quite easy to
extend with compiled code written in e.g. C, that is how we are making
large parts of the ERT functionality available in Python. The Python
language has good documentation, including tutorials on
http://www.python.org, however some language concepts which will be
much used in the documentation will be briefly explained here. The
rest of this section will focus on the following little example which
contains the class defintion of a circle object.

.. code-block:: python
   :linenos:          

    import math
      
    class Circle(object):
                                                                   
       def __init__( self , x0 , y0 , r):                          
          "Create a new circle with radius r centered at (x0,y0)." 
          self.x0 = x0                                             
          self.y0 = y0                                             
          self.r = r                                               
                                                                   
       def move(self, new_x , new_y):
          "Move the position of the circle to (new_x,new_y)."              
          self.x0 = new_x
          self.y0 = new_y

       
       # Observe that here we have three different ways to get the area
       # of the circle: The normal method getArea( ), the class method
       # calculateArea( ) and finally the area property. Having three
       # different methods in this way should *not* be interpreted as a
       # best practice in any way - it is mainly to illustrate the
       # concepts of class methods and properties. 

       def getArea( self ):     
          "Calculate the area of the circle."
          return math.pi*self.r * self.r

       def calculateArea(cls , radius):
           "Calculate the area of *a* circle with radius 'radius'."
           return math.pi * radius * radius

       @property
       def area( self ):     
          "Calculate the area of the circle."
          return math.pi*self.r * self.r

       def distance( self , other ):
          "Calculate the distance between two circles."
          if isinstance( other , Circle ):
             dx = self.x0 - other.x0
             dy = self.y0 - other.y0      
             return math.sqrt( dx*dx + dy*dy)  
          else:
             raise TypeError("Second argument must be Circle instance.")   


	     

    # Create two circles 
    circle1 = Circle( 2,2,3 )
    circle2 = Circle( 5,5,7 )

    # Calculate the area of the circles, and the distance between them
    print "Area of circle1:%10.3f" % circle1.getArea( )
    print "Area of circle2:%10.3f" % circle2.getArea( )
    print "Distance between circles: %10.3f" % circle1.distance( circle2 )

    # Move one of the circles and calculate new distance 
    circle1.move( 3,3 )
    print "Distance between circles: %10.3f" % circle1.distance( circle2 )


Notation
--------

Whitespace matters
..................

In Python whitespace, or more precisely indentation level, matters -
the logical codeblocks are defined by the indentation level. Observe
for instance the if test on line 39. If the if test evaluates to True
the codeblock in lines 40-42 is executed, otherwise the statement on
line 44 is executed. Aport from the indentation level additional
whitespace in the code, including blank lines, is ignored.


Comments
........

Comments in Python are prefixed with :code:`#`; everything from a
:code:`#` to the end of the current line is disregarded as a comment:

.. code:: python

   # The following function will add to numbers and return the summary
   def add_numbers(a,b):
       return a+b


Documentation strings
.....................


It is possible to add documentation strings right after the
:code:`def` statement, in the example above we have documentation
strings on lines 6,12,25,29,34 and 38. These documentation strings
will be picked up by the Python documentation tool :code:`pydoc`. In
the example below we use :code:`pydoc` to print the documentation of
the area method:

.. code:: bash
          
    bash% pydoc XXX.Circle.getArea
    Calculate the area of the circle.

Where XXX should be the name of the module containing the Circle
class. Most of the modules and classes in the ert package have
reasonably good documentation strings.



Functions
---------

Functions are declared with the keyword :code:`def` and the name of
the function. Functions take ordinary named arguments, but can in
addition have optional arguments. If an argument is optional the
default value must be specified in the statement defining the
function. In the function below the a gross price is calculated based
on a net price and a vat rate, the vat rate is given as an optional
variable:

.. code:: python

    def gross_price( net_price , vat_rate = 0.25):
        return net_price * (1 + vat_rate)

    total = gross_price( 100 )
    total_taxfree = gross_price( 100 , vat_rate = 0.0 )

Optional arguments with suitable defaults is used quite a lot in the
ert. The ERT python code is mainly based on classes, it is not
required to use classes in the the code you write yourself - but ample
use of functions is highly recommended to aid readiblity and
maintainence.



Object oriented programming - classes
-------------------------------------

Python is an *object oriented* language. An object is a composite
variable which consists of normal variables (often called members) and
functions to operate on the members (often called) methods. When
programming we *implement a Class*, and the class can viewed as a
recipee for how to create an object. The process of creating an object
is often called instantiation, and an object of a particular type is
called an instance. The normal way to create a new object is just to
"call" the class name with the required arguments, i.e.

.. code:: python

   circle = Circle(0,0,5)

Will create a new :code:`Circle` object located in position (0,0) with
radius 5.

Members
.......

In Python the class members are not declared anywhere, but
automagically comes into life when assigned to. In the
:code:`__init__()` method on line 5 we create the member variables
:code:`x0,y0` and :code:`r` by assignment. The Python language does
not have any built in notion of public and private, all members and
methods are public by construction, and can be accessed outside the
class, if we have a :code:`Circle` instance we can both read and the
radius attribute directly:

.. code:: python

   circle = Circle(0,0,5)
   print("The radius of the circle is %g " % circle.r)
   
   # Enlarge the circle
   circle.r *= 2.0
   print("The radius of the circle is %g " % circle.r)

Directly accessing a member variable in this form is considered bad
form - see the discussion of the :code:`move()` implementation
further down for a better alternative.

Methods
.......

Methods are functions defined as part of a class definition; the
methods represent the "handles" to the outside world - which can be
used to query and manipulate the state of the object. The method
:code:`move()` in the :code:`Circle` class is used to move the circle
to a new location:

.. code:: python

   circle = Circle(0,0,5)
   circle.move(2 , 2)

   # Since the members are not protected, you can move the circle
   # without going through the move() method:

   circle.x0 = 2
   circle.y0 = 2

   # Directly assigning to the member variable in this way is
   # considered *very bad form*.

In addition to the :code:`move` method The :code:`Circle` class has
the ordinary methods :code:`distance()` and :code:`getArea()`. The
:code:`distance()` and :code:`getArea()` methods perform a
calculation, but do not change the state of the object. The
:code:`__init__()` method is quite ordinary, but it is invoked
automatically by the Python runtime system when a new object is
instantiated, and normally not invoked explicitly.

In the implementation all normal methods should have the *class
instance* as the first argument, the variable name *self* is usually
used for this class instance variable. 



Properties
..........

Properties are methods without arguments which can be called without
the empty :code:`( )`. Consider the :code:`area` property in
:code:`Circle` example:

.. code:: python
 
   circle = Circle( 10 )

   print("method call: The area of the circle is: %g" % circle.getArea( ))
   print("property   : The area of the circle is: %g" % circle.area)

The example in :code:`Circle` class is a *read/get* property - it is
also possible to implement *write/set* properties. In the :code:`ert`
Python package we have deprecated the use of properties in the favor
of traditional methods with :code:`( )`- but there are still some
properties, in particular in the :code:`ert.ecl` package.


Classmethods
............

A normal method requires that you first create an instance of an
object, and then afterwards you can can use the methods of the
object. For instance you must have a :code:`Circle` instance available
before you can call the :code:`getArea()` method. However there are
situations where you would like to make functionality available with a
particular class, without really requring a class instance - then a
*class method* can be used. Se for instance the :code:`calculateArea`
method of :code:`Circle` class. Since a class method is implemented
without a class instance, the implementation *can not make use of
members in the class*. The class methods are called with the class
name as prefix, or *if* you have a circle instance you can use that
prefix:


.. code:: python
 
   # Call the calculateArea class method:
   print("Area of circle with radius:10 m = %g m*m " % Circle.calculateArea( 10 ))


   # If we have class instance we can use that to call a class method; since
   # the class method does not have access to the members of the class we must 
   # pass the radius explicitly:
   c = Circle( 10 )
   print("Area of circle : %g" % c.calculateArea( c.r ))


Special functions / operators
.............................

When implementing a Python class you can implement various *special
methods* which will integrate your class into Python language, the
special methods all have names like :code:`__xxx__`. Let us assume we
are creating a class for a mathematical vector, for a vector you
typically want to query how long it is, set and get individual
elements and you might be interested in adding two vectors:

.. code-block:: python
   :linenos:          

   class Vector(object):

      def __init__(self, size, initial_value = 0):
         self.data = [ initial_value ] * size


      def __len__(self):
         return len(self.data)

      
      def __iadd__(self , other):
          if isinstance(other , Vector):
             if len(self) == len(other):
                 for index,value in enumerate(other):
                     self[index] += value
             else:
                 raise ValueError("The two vectors must be equally long")                 
          else:
              raise TypeError("__iadd__() function requires two vectors")

          return self          


      def __add__(self , other):
          copy = Vector( len(self) )
          for index,value in enumerate(self):
              copy[index] = value
          copy += other  
          return copy 


      def __getitem__(self, index):
         if 0 <= index < len(self):
            return self.data[index]
         else:
            raise IndexError("Invalid index:%d" % index)


      def __setitem__(self, index,value):
         if 0 <= index < len(self):
            self.data[index] = value
         else:
            raise IndexError("Invalid index:%d" % index)


The important point with this example is the methods :code:`__len__`,
:code:`__getitem__` and :code:`__setitem__`. The :code:`__len__`
method is bound to the :code:`len( )` operator and :code:`__getitem__`
and :code:`__setitem__` are bound to the :code:`[]` operator [1]_: 


.. code:: python

   # Create a new vector of length 10; initialized to zero.
   v = Vector(10)

   # Check the length
   print("The length of the vector is:%d" % len(v))

   # Set the elements to 0,1,2,3,...
   for i in range(len(v)):
       v[i] = i

   # Check that the elements are correctly set
   for i in range(len(v)):
       if not v[i] == i:
          raise Exception("operator[] not corectly implemented")

   # When getitem is implemented we can loop over the whole vector 
   sum = 0
   for elm in v:
       sum += elm

   # Create a new vector
   v2 = Vector(10)
   for i in range(len(v2)):
       v2[i] = i

   # This will call the __add__() method
   v3 = v + v2 

   # This will call the __iadd__() method
   v3 += v2


For this example we have only implemented the :code:`__add__` and
:code:`__iadd__` methods to add to vectors, in addition there are
similar functions :code:`__sub__`, :code:`__mul__` and :code:`__div__`
for subtraction, multiplaction and divition. Observe the difference
between the :code:`__add__` and :code:`__iadd__` - the first creates a
new copy whereas the second updates a vector *in place*. 

For the ert classes we have tried to implement the special methods
where it makes sense, depending on the class that typically includes
:code:`__len__`, :code:`__getitem__`, :code:`__setitem__`, 
:code:`__contains__` and the arithmetic operators.


      
Error handling - exceptipons 
-----------------------------

Python uses a mechanism called *exceptions* to signal error
conditions. When an exception is raised the "current location" in the
program will return back (stack unwind) all the way back to a
statement handling the exception, or the program will exit if the
exception is not handled. The advantage of exceptions as a way to
handle errors is that it is not necessary to clutter the code with
error handling conditions "all over the place". Exceptions is the way
to handle error conditions in many programming languages.

.. code-block:: python
   :linenos:          

    #!/usr/bin/env python
    import math

    def call_sqrt(x):
        y = math.sqrt(x)
        print "The square root of %g is %g" % (x , y)

        
    y = call_sqrt( -1 )

In this example we try to calculate the square root of -1, that is
invalid and Python will raise an exception:

.. code:: bash

    Traceback (most recent call last):
      File "/path/to/file.py" line 9 in <module>
         call_sqrt(-1)
      File "/path/to/file.py" line 5 in call_sqrt
          y = math.sqrt(x)
    ValueError: math domain error

What happens here is the following:

1. In the internal function :code:`sqrt` which is called on line 5
   the invalid input argument is detected an exception is *raised*.

2. The exception will propagate backwards in the call stack, first to
   to the line 5, and then back to call on line 9 and finally the
   program will print an error message and terminate.

The error messages from an exception can be a bit challenging to read,
but by looking at it we can see where the error happens, and we get a
*hint* of what is wrong - in this case the hint "math domain error"
should give us a clue of what the problem is. There is some logic to
the type of the exception, in this case the exception raised will be
:code:`ValueError`- the value -1 is an invalid argument to the
:code:`sqrt` function.

If we don't *handle* the exception the program will eventuelly exit
with an error message, but if this error does not signal a fatal
error, but rather something which might very well happen, we might
wish to *catch* the exception with a :code:`try` :code:`except`
construction:

.. code-block:: python
   :linenos:          

    #!/usr/bin/env python
    import math

    def call_sqrt(x):
        try:
           y = math.sqrt(x)
           print "The square root of %g is %g" % (x , y)
        except ValueError:
           print "Oh no - you can not do sqrt( %g )" % x


    call_sqrt( 16 )
    call_sqrt( -1 )
    call_sqrt(  9 )

When we run this program the :code:`try: except:` block will *catch*
the :code:`ValueError` exception and instead print a friendly error
message:

.. code:: bash

    The square root of 16 is 4
    Oh no - you can not do sqrt( -1 )
    The square root of 9 is 3

The ert code uses exceptions to signal error conditions, if you get an
exception in your ert based code you can *assume* that there is a
something wrong in your code. On the other hand you might get a really
*hard crash* with an error message looking like[2]_:


.. code:: bash
          
    Abort called from: float_vector_iset (/home/joaho/jenkins/workspace/ERT-deploy-upstream-branch/build/libert_util/src/float_vector.c:615) 
    float_vector_iset: Sorry - can NOT set negative indices. called with index:-1 


    --------------------------------------------------------------------------------
     #00 ???(..)                         in /path/devel/libert_util/src/util_abort_gnu.c:154
     #01 util_abort__(..)                in /path/devel/libert_util/src/util_abort_gnu.c:263
     #02 float_vector_iset(..)           in /path/build/libert_util/src/float_vector.c:615
     #03 ecl_smspec_fread_alloc(..)      in /path/devel/libecl/src/ecl_smspec.c:1078
     #04 ???(..)                         in /path/devel/libecl/src/ecl_sum.c:198
     #05 ecl_sum_fread_alloc_case__(..)  in /path/devel/libecl/src/ecl_sum.c:227
     #06 ffi_call_unix64(..)             in /tmp/Python-2.7.6/Modules/_ctypes/libffi/src/x86/unix64.S:79
     #07 ffi_call(..)                    in /tmp/Python-2.7.6/Modules/_ctypes/libffi/src/x86/ffi64.c:524
     #08 _ctypes_callproc(..)            in /tmp/Python-2.7.6/Modules/_ctypes/callproc.c:857
     #09 ???(..)                         in /tmp/Python-2.7.6/Modules/_ctypes/_ctypes.c:3940
     #10 PyObject_Call(..)               in ???
     #11 PyEval_EvalFrameEx(..)          in ???
     #12 PyEval_EvalCodeEx(..)           in ???
    --------------------------------------------------------------------------------

These hard crashes can unfortunately *not* be handled by the
:code:`try: except:` construction in Python.  There might be a bug in
your code; but that it results in such a brutal traceback is certainly
a bug in the ert code - please report it!
    


Organizing python code in modules and packages
----------------------------------------------

Python code is written in files and stored in directories; this is the
foundation for modules and packages.


Modules
.......

Python code is written in files, one file of Python code is called a
module, and can be imported into other Python code. In the small
example below we create a module which contains a class :code:`C`, a
function :code:`add` and a scalar variable :code:`var`. The whole
thing is saved in a file called :code:`module.py`:

.. code:: python

    class C(object):

       def __init__(self , arg ):
          self.arg = arg

       def calc( self , v ):
           return ...


    def add(a, b):
        return a+b


    var = "Bjarne"


This module now contains three *symbols*: :code:`C`, :code:`add` and
:code:`var` which can be reused from another context. To be able to
use the symbols from the module we must *import* the module, there are
several minor variations over the import statement, the exact import
statement used determines which name the symbols should be given in
the importing scope.


Import all symbols under the module namespace 
,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,

.. code:: python

     import module

     c = module.C( 10 )
     print "Variable is:%s" % module.var
     print "Adding 10,20:%g" % module.add(10,20)

I.e. all the symbols from module.py have become available, but we must
prefix them with :code:`module` to use them.


Import all the symbols from module into the current namespace
,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,

.. code:: python

    from module import *

    c = C( 10 )
    print "Variable is:%s" % var
    print "Adding 10,20:%g" % add(10,20)

Again all the symbols from :code:`module.py` are available, but now
they have been imported all the way into the current namespace and can
be accessed without the module prefix.


Selectively importing some symbols
,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,

.. code:: python

    from module import C
    import module.add

    c = C( 10 )
    print "Adding 10,20:%g" % module.add(10,20)

We have imported the :code:`C` and :code:`add` symbols; the :code:`C`
symbol has been imported all the way into the current namespace,
whereas the :code:`add` symbol is available under the module
namespace. The :code:`var` symbol has not been imported, and if we try
to access that, using either :code:`var` or :code:`module.var` will we
get an error.


Import and rename
,,,,,,,,,,,,,,,,,

.. code:: python
          
    import module as myModule

    c = myModule.C( 10 )
    print "Adding 10,20:%g" % myModule.add(10,20)


Here we have imported all of module, but we access it internally as
myModule.


Packages
........

Modules are just ordinary Python files, in the same way packages are
just ordinary directories, with the *special file* :code:`__init__.py`
in them; the :code:`__init__.py` file can be empty - but it must exist
in the directory for Python to treat the directory as a *package*. The
files and directories contained in the package directory will then be
modules and subpackages. In the ert distribution the package,
subpackage and module looks like this:

.. code::

   ert/                      <-- Top level package / directory.
   ert/__init__.py           <-- Magic file signalling that 'ert' is a package.
   ert/ecl/                  <-- ecl is subpackage.
   ert/ecl/__init__.py       <-- Magic file signalling that 'ecl' is a package.
   ert/ecl/ecl_grid.py       <-- Normal module in the ecl package.
   ert/ecl/ecl_sum.py        <-- Normal module in the ecl package.

All of ert Python is organized as a Python package. The top level
package ert has nearly no content, but contains subpackages ert.ecl,
ert.job_queue and so on ert.util. Each of the subpackages contain many
modules, as the module ert.ecl.ecl_grid which implements functionality
to work with ECLIPSE grids. Each of the modules typically implement
one or a few related classes, like the class EclGrid which is
implemented in the ert.ecl.ecl_grid module.

The :code:`__init__.py` file must be present for Python to realize
that a certainl directory should be interpreted as a *package*; the
file can be empty, but it can also be used for e.g. initialization. In
the case of :code:`ert` we import symbols from modules to the
subpackage, so that we can import class symbols as:

.. code:: python
          
   from ert.ecl import EclGrid, EclSum



Getting started with ert Python
-------------------------------

If the ert python package and the necessary shared libraries have all
been correctly installed at your site, the following script should
work:

.. code:: python

   #!/usr/bin/env python
   import ert

   print "Python: the ert package has been sucessfuly loaded"


If this works as intended you are ready to actually start working on
your actual problem. Hopefully the example section will have something
you can start from.



.. [1] A vector designed for high performance numerical operations
       would never be implemented this way; performance aside a class
       implementing this functionality should also have much more
       error checking.

.. [2] Observe that the error message will typically point to a file
       like :code:`/tmp/ert_abort_dump_xxx_20160101-026343.log` - the
       content of that file will resemble the backtrace listed here.

