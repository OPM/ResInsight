The ert.ecl package
===================

.. contents::
   :depth: 2
   :local: 

The :code:`ert.ecl` package for working with Eclipse files is quite
extensive. There is some quite limited support for reading
:code:`grdecl` formatted input files, but mainly the package is
devoted to loading and interpreting the binary output files from
Eclipse. This includes :code:`INIT, GRID/EGRID, RFT`, summary and
restart files.

.. _structure_binary_files
Structure of Eclipse binary files
---------------------------------

The binary files from ECLIPSE are built up with a common structure.
They consist of a collection of keywords [1]_, where each keyword has a
header and a block of data. The header consist of three fields:

1. Name - this is a string with the name of the keyword. The name is
   not unique in one file.

2. The size of the keyword, i.e. the number of data elements.

3. The data-type of the keyword - the different datatypes are 'INTE'
   for integer date, 'REAL' for floating point numbers in single
   precision (i.e. float), 'DOUB' for floating point numbers in double
   precision, 'LOGI' for boolean variables [2]_ and 'CHAR' for character variables.

Following the header comes blocks of data. Normally the files are
unformatted, so it is impossible to inspect them directly. But by
converting them, e.g. with the ert utility :code:`convert.x`, to
formatted one can take a look:

.. code:: bash

   bash% convert.x ECLIPSE.INIT
   converting ECLIPSE.INIT -> ECLIPSE.FINIT

Looking at the formatted file ECLIPSE.FINIT, we can e.g. look at the
PORV keyword from the INIT file:

.. code:: bash

  'PORV    '      30000    'REAL'
  0.334572E+5     0.334561E+5    0.337613E+5    0.329863E+5
  0.3377042+5     0.330056E+5    0.319002E+5    0.323340E+5
  0.340970E+5     0.3410972+5    0.342097E+5    0.338722E+5
  ....

Here we see that the name of keyword is 'PORV', and it consists of
30000 elements of type REAL. At least all the file types summary
(BASE.SMSPEC / BASE.UNSMRY / BASE.Snnnn), restart (BASE.UNRST /
BASE.Xnnnn), grid (BASE.EGRID / BASE.GRID), rft (BASE.RFT) and init
(BASE.INIT) consist of collections of such keywords. In the
:code:`ert.ecl` code the classes :code:`EclKW` and :code:`Ecl3DKW` are
designed to work with *one* such keyword.


FortIO - work with binary Eclipse IO
------------------------------------

The binary files created by ECLIPSE are created according to a Fortran
IO convention, which is a bit special, when writing e.g. a block of
1000 bytes of data to disk the Fortran IO system will surround the
block with a header and a tail denoting the size of the
block. I.e. for the Fortran code:

.. code:: bash

   integer array(100)
   write(unit) array

What actually hits the disk looks like this:

.. code:: bash

   | 400 | array ...... | 400 |

The header and tail is a 4 byte integer, which value is the number of
bytes in the immediately following record. In addition the ECLIPSE
files are ususally(?) written in the "the other" endianness. The
:code:`FortIO` class should handle these matters transparently. For
normal use of the library it should not be necessary to explicitly use
the :code:`FortIO` class.

The :code:`FortIO` class has quite good embedded documentation, and
you are advised to use :code:`pydoc ert.ecl.FortIO` or browse the API
documentation at :ref:`python_documentation` for further details.


EclKW/Ecl3DKW - work with one keyword
-------------------------------------

The :code:`EclKW` class represents one keyword [1]_ from an Eclipse
result file. The :code:`EclKW` class is essentially a vector of data,
along with header with the size and type of data, and the name of the
vector. Mostly you will get :code:`EclKW` instances by querying a
:code:`EclFile` instance - but you can also instantiate
:code:`EclKW` instances manually. The :code:`Ecl3DKW` class is for
keywords which represent 3D properties like e.g. PRESSURE and PORO,
this class requires a grid instance, and is documented with the
:code:`EclGrid` documentation.


Special methods - container
...........................

The :code:`EclKW` class implements the :code:`__getitem__()` and
:code:`__setitem__()` methods which are used to implement access using
the :code:`[ ]` notation, and the :code:`__len__()` method which gives
the size of the :code:`EclKW` instance. In the example below we load
an INIT file and extract the PERMX and PORO keywords, we then
forcefully set permeability to zero for all elements where the
porosity is below a limit:

.. code:: python

   from ert.ecl import EclFile,EclGrid
   poro_limit = 0.05

   grid = EclGrid("CASE.EGRID")
   init_file = EclFile("CASE.INIT")
   permx = init_file["PERMX][0]
   poro = init_file["PORO"][0]

   for index,value in enumerate(poro):
       if value < poro_limit:
           permx[index] = 0

   # Save the updated permx to a new grdecl input file:
   with open("permx.grdecl" , "w") as fileH:
      grid.write_grdecl( permx , fileH )

In addition to the :code:`EclKW` class this example uses the classes
:code:`EclGrid` and :code:`EclFile` - see the documentation of them
below. Furthermore this example demonstrates an important point: even
though the :code:`EclKW` class is an important "workhorse" - we mostly
get it from an :code:`EclFile` instance and do not instantiate it
directly.

For more details type :code:`pydoc ert.ecl.EclKW` or browse the API
documentation at :ref:`python_documentation`.


Special methods - arithmetic
............................

The :code:`EclKW` class implements all the arithmetic operators,
meaning that :code:`EclKW` instances can be added, multiplied and
shifted. In the example below we load all INIT files with a matching
filename pattern and then calculate the mean and standard deviation of
the permeability:

.. code:: python

   import glob
   import math
   from ert.ecl import EclFile, EclTypeEnum

   initfile_pattern = "/path/to/files/real*/CASE-*.INIT"
   
   kw_list = []
   for init_file in glob.glob(initfile_pattern):
       ecl_file = EclFile(init_file)
       kw_list.append( ecl_file["PERMX"][0] )

   mean = EclKW.create("AVG-PERMX" , len(kw_list[0]) , EclTypeEnum.ECL_FLOAT_TYPE)
   std  = EclKW.create("STD-PERMX" , len(kw_list[0]) , EclTypeEnum.ECL_FLOAT_TYPE)

   # Here we do normal arithmetic calculations with the EclKW instances   
   for kw in kw_list:
       mean += kw
       std += kw * kw

   mean /= len(kw_list)
   std /= len(kw_list)
   std -= mean * mean 

   # The sqrt() function can not be implemented in the object, so here
   # we must do it more explicitly.
   std.apply( math.sqrt ) 
	
Observe that for the arithmetic operations you can also call the
inplace methods (*without* leading 'i') :code:`add()`, :code:`mul()`,
:code:`sub` and :code:`div()` directly - in this form the methods also
accept a *mask* parameter as an :code:`EclRegion` instance which can
limit the operation to only a subset of the elements.


EclFile - an arbitrary binary Eclipse file
------------------------------------------

The :code:`EclFile` class loads an arbitrary binary Eclipse file, and
creates an index of all the keywords in the file. The main reason for
opening an Eclipse file with an :code:`EclFile` instance is to look up
keywords in the file as :code:`EclKW` instances. The :code:`EclFile`
is general and can be used to open any file, but in addition there are
specialized classes :code:`EclInitFile` and :code:`EclRestartFile`
which can be used to open :code:`INIT` and restart files respectively;
these are documented along with the :code:`Ecl3DKW` class after the
:code:`EclGrid` documentation.


Special method __contains__()
.............................

The :code:`EclFile` class implements the :code:`__contains__` method
which is typically used to check if a file contains a certain
keyword. The following example is a small script which will load an
Eclipse binary file given as a command line argument, and check if the
file contains keywords also given on the command line:

.. code:: bash

   bash% scan_file.py ECLIPSE.UNRST  SWAT SGAS SOIL

Will open the file :code:`ECLIPSE.UNRST` and check if it contains the
keywords :code:`SWAT`, :code:`SGAS` and :code:`SOIL`:

.. code:: python

   #!/usr/bin/env python
   import sys
   from ert.ecl import EclFile


   # Open the file, EclFile will raise the IOError exception
   # if the open fails.
   try:
      file = EclFile( sys.argv[1] )
   except IOError:
      sys.exit("Could not open file: %s" % sys.argv[1])

   # Go through the keywords from the command line
   # and check if they are in the file
   for kw in sys.argv[2:]:
       if kw in file:
           print("Found %s in %s" % (kw , file.getFilename()))
       else:
           print("Missing %s in %s" % (kw , file.getFilename()))


Special method __getitem__()
............................

The :code:`__getitem__()` method is used to get an :code:`EclKW`
instance from a file through the :code:`[]` operator. The argument to
the :code:`[]` operator can either be an integer to get a keywords by
plain index order, or a keyword name.

.. code:: python

   from ert.ecl import EclFile

   file = EclFile("CASE.UNRST")
   first_kw = file[0]

   swat_kw = file["SWAT"]

Observe that when :code:`[]` is used with a keyword *name* the return
value is a *list* of keywords - there can potentially be *many*
keywords with the same name in a file.

Restart specials
................

The :code:`EclFile` class has many specialized methods for to perform
queries on the time direction of restart files. These methods should
be moved to :code:`EclRestartFile` class, and are documented there.


For more details type :code:`pydoc ert.ecl.EclFile` or browse the API
documentation at :ref:`python_documentation`.


EclGrid - load a grid
---------------------

The :code:`EclGrid` class is used to load an Eclipse Grid, the main
way to load a grid is from a :code:`EGRID` or :code:`GRID` file, but
as an alternative it is also possible to create a grid from a .grdecl
formatted *input* file [3]_, or a simple rectangualar grid can be
*created* without an input file. In most cases the :code:`EclGrid`
instance will be created as simple as:

.. code:: python

   from ert.ecl import EclGrid

   grid = EclGrid("ECLIPSE.EGRID")



Active/inactive cells - index types
...................................

For a typical reservoir model a large fraction of the cells are not
active; i.e. they are ignored in the flow calculations. The
bookkeeping of active/inactive cells is managed by grid. As a user of
the :code:`ert.ecl` Python package you must have some understanding of
these issues. Consider a 2D 3x3 grid model where only five of the
cells are active:

.. code::


      +---------+---------+---------+
      |1  (0,2) |1  (1,2) |0  (2,2) |
      |      6g |      7g |	 8g |
      |      3a |      4a |	  - |
      +---------+---------+---------+
      |0  (0,1)	|1  (1,1) |1  (2,1) |
      |	     3g	|      4g |      5g |
      |	     -	|      1a |      2a |
      +---------+---------+---------+
      |0  (0,0)	|1  (1,0) |0  (2,0) |
      |	     0g	|      1g |	 2g |
      |	     - 	|      0a |	 -  |
      +---------+---------+---------+

The 0 or 1 in the upper left corner of each cell indicates whether the
cell is active(1) or inactive(0) [4]_. When working with the
:code:`EclGrid` there are generally *three* different ways to refer to
a specific cell - these are:

 1. A triplet of :code:`(i,j,k)` values - in the example above
    indicated with the :code:`(i,j)`. 

 2. A *global index* in the range :code:`[0..nx*ny*nz)` which uniquely
    identifies a cell. This is indicated as the number with a trailing
    'g' in the example above.

 3. An *active index* in the range :code:`[0..nactive)` which
    uniquely identifies an *active* cell. In the figure above the
    active indices are the integers with a trailing 'a'.

All the methods on the :code:`EclGrid` object which evaluate
properties for a particular cell can take the cell coordinate in any
of the three formats above. In addition there are conversion functions
between the three. Observe that all the indexing methods assume that
the indices are zero offset, i.e. starting at 0, this is in contrast
to Eclipse itself and many other post processing applications which
assume that indices start at 1.

In the restart and init files most of the properties are stored as
vectors of length *nactive*, i.e. the indexing with active indices is
the most natural. In the example below we load a grid and a init file,
and then we print the (i,j,k) values for all the cells with
:code:`PERMX` below a limit:

.. code:: python

   from ert.ecl import EclGrid,EclFile

   permx_limit = 1e-2
   grid = EclGrid( "CASE.EGRID" )
   init = EclFile( "CASE.INIT" )

   permx_kw = init["PERMX"][0]
   
   for ai in range(len(permx_kw)):
       if permx_kw[ai] < permx_limit:
          ijk = grid.get_ijk( active_index = ai)
	  print("permx[%d,%d,%d] < %g" % (ijk[0], ijk[1], ijk[2], permx_limit))

In the case of dynamic properties like :code:`PRESSURE` and
:code:`SWAT` it does not make sense to ask what the value is in the
inactive cells - it has not been calculated. For properties the input
files typically have :code:`nx*ny*nz` elements - so here it is/might
be possible to get hold of a valid value also for the inactive
cells. When working interchangebly with properties defined over all
cells or only the active cells it is very important to think straight.


Ecl3DKW - a grid aware EclKW class
..................................

The :code:`Ecl3DKW` class is derived from the :code:`EclKW` class, but
the instance has a :code:`EclGrid` and optionally a default value
associated to it. The purpose of this is to be able to use
:code:`(i,j,k)` as index when looking up values. When :code:`(i,j,k)`
is used to identify the cell the, :code:`Ecl3DKW` class can
transparently handle the active/inactive cells issue - returning a
default value in the case of undefined inactive cells. 

When the :code:`[]` argument is a single integer the :code:`Ecl3DKW`
class can not know whether the index supplied is an active or a global
index, and it will be a simple index lookup - which properties are
determined by the length of the underlying data. 

The :code:`Ecl3DKW` class is mainly convenience compared to the pure
:code:`EclKW` class - for performance reasons it should probably not
be used if you wish to run through all the cells.


Using the grid object
.....................

The :code:`EclGrid` object has a long range of methods for extracting
grid properties:

  1. Many different methods for working with cell data like position,
     depth, size and location of cell corners.
  
  2. Methods doing the reverse mapping :code:`(x,y,z) -> (i,j,k)`.

  3. *Some* functionality for working with LGRs, coarse groups and
     fractured grid. 

  4. Methods for exporting a :code:`EclKW` defined over
     :code:`nactive` elements to a :code:`grdecl` formatted file with
     :code:`nx*ny*nz` elements.

In addition the :code:`EclGrid` is used as an input for the
:code:`Ecl3DKW` properties and also for the :code:`EclRegion`
class. For further details please type :code:`pydoc ert.ecl.EclGrid`
or browse the API documentation at :ref:`python_documentation`.

EclInitFile and EclRestartFile - grid aware files
-------------------------------------------------

For restart and init files you can optionally choose to use
:code:`EclInitFile` and :code:`EclRestartFile` classes instead of the
basic :code:`EclFile` class. These two derived classes have a grid
attached, and will return a :code:`Ecl3DKW` instance instead of a
:code:`EclKW` instance for keywords with either :code:`nx*ny*nz` or
:code:`nactive` elements.

In the example below we have a list of :code:`(i,j,k)` triplets and we
look up the permeability values for these cells without going through
the :code:`(i,j,k) -> active_index` transformation:

.. code:: python

   from ert.ecl import EclGrid,EclInitFile

   grid = EclGrid( "CASE.EGRID" )
   init = EclInitFile( "CASE.INIT" )

   cell_list = [(1,2,3), (1,4,5), (2,2,7)]
   # The permx_kw will now be a Ecl3DKW instance

   permx_kw = init["PERMX"][0]
   for ijk in cell_list:
       print("permx : %g" % permx_kw[ijk])       
   

Time queries in EclRestartFile
..............................

The :code:`EclRestartFile` class has many methods for queries on the
temporal content of a restart file [5]_. 

Classmethods
,,,,,,,,,,,,

Several of the methods giving temporal information on restart files
are *classmethods* - which means that an be invoked *without* creating
the :code:`EclRestartFile` instance first:


EclRestartFile.file_report_list
*******************************

The classmethod :code:`file_report_list` will scan through a file and
identify all the report steps in the file. In the example below we
print a list of all the report steps which can be found in a restart
file.

.. code:: python

   from ert.ecl import EclRestartFile

   report_list = EclRestartFile.file_report_list("ECLIPSE.UNRST")

   print("The file: %s contains the following report steps: ") 
   print( ", ".join(report_list))
 

EclRestartFile.contains_report_step
***********************************

The classmethod :code:`contains_report_step` will check if the file
*filename* contains the report_step *report_step*:

   
   if EclRestartFile.contains_report_step( "ECLIPSE.UNRST" , 100):
       print("The file has a section for report step=100")
   else:
       print("No - the file does not have report_step = 100")


EclRestartFile.contains_sim_time
********************************

The classmethod :code:`contains_report_step` will check if the file
*filename* has a result block for a particular date, the date should
be given as a normal Python :code:`datetime`:

.. code:: python

   from ert.ecl import EclRestartFile
   import datetime   

   sim_time = datetime.datetime( 2010 , 6 , 15 )
   if EclRestartFile.contains_sim_time( "ECLIPSE.UNRST" , sim_time ):
       print("The file has a section date: %s" % sim_time)
   else:
       print("No - the file does not have data at: %s" % sim_time)



EclRegion - a method to select cells
------------------------------------

The purpose of the :code:`EclRegion` class is to build up a set of set
cells in a *region* [6]_ based on various selection criteria. That
selection is then typically used to update a set of cells in a
:code:`EclKW` instance, either as a :code:`mask=region` parameter in
one of the arithmetic operators or by directly looping through the
index set.

In the example below we load the PORO and PERMX fields from
:code:`grdecl` input files, select different regions based on the
values and create a SATNUM keyword. The rather arbitrary rule we
apply is:

 1. For cells with PORO < 0.01 we assign SATNUM = 1
 2. For cells with PORO > 0.01 and PERMX < 200 we assign SATNUM = 2
 3. For cells with PORO > 0.01 and PERMX > 200 we assign SATNUM = 3


.. code:: python

   from ert.ecl import EclGrid, EclRegion, EclKW, EclTypeEnum

   grid = EclGrid( "CASE.EGRID" )
   
   with open("poro.grdecl") as f:
       poro = EclKW.read_grdecl( f , "PORO")

   with open("permx.grdecl") as f:
       permx = EclKW.read_grdecl( f , "PERMX")

   # Create an initially empty region, and select all the cells where
   # PORO is below 0.01
   reg1 = EclRegion( grid , False )
   reg1.select_less( poro , 0.01 )


   # Create an initially empty region, and select all the cells where
   # PORO is above 0.01. Then we select all the cells where PERMX is
   # above 200. Since the flag intersect is True this second selection
   # is only among the already selected cells.
   reg2 = EclRegion( grid , False )
   reg2.select_more( poro , 0.01 )
   reg2.select_more( permx , 200 , intersect = True )


   # Create a region where all cells are initially selected,
   # then subtract the regions reg1 and reg2.
   reg3 = EclRegion( grid , True )
   reg3 -= (reg1 + reg2)


   # Create a new satnum keyword and use the assign() method with a
   # mask parameter.
   satnum = EclKW.create( "SATNUM" , grid.getGlobalSize() , EclTypeEnum.ECL_INT_TYPE)
   satnum.assign( 1, mask = reg1 )
   satnum.assign( 2, mask = reg2 )
   satnum.assign( 3, mask = reg3 )

   with open("satnum.grdecl" , "w") as f:
       satnum.write_grdecl( f )


EclRegion - selectors
.....................

A region can be constructed in many different ways:
  
 1. Based on slices of :code:`i,j,k` values.
 2. Inside or outside a polygon; or alternatively "above" or "below" a
    line.
 3. Based on comparing a :code:`EclKW` instance with a scalar value.
 4. Based on comparing two :code:`EclKW` instances.
 5. Based on cell geometry - i.e. size, depth or thickness.

Observe the following:

 1. For each :code:`select_xxx` method there is a corresponding
    :code:`deselect_xxx` method.

 2. By default all cells are eligible for selection, but if you pass
    the :code:`intersect = True` flag to the :code:`select_xxx` method
    the selection algorithm will only consider the already selected
    cells.


EclRegion - special methods
...........................

The :code:`EclRegion` class implements the special methods required to
view the regions as set; i.e. you can add and subtract regions and
form the union and intersection of regions.

.. code:: python

   reg1 = ...
   reg2 = ...


   # Region reg3 will be the union reg1 and reg2, i.e. the cells
   # selected in reg3 is the set of all cells selected in either reg1
   # or reg2.
   reg3 = reg1 | reg2 
   reg3 = reg1 + reg2

   # Region reg3 will be set of cells which are *only* selected in reg1.
   reg3 = reg1 - reg2

   # Region reg3 will be the set of cells which are selected in *both*
   # reg1 and reg2.
   reg3 = reg1 & reg2
 
For further details, specially of the various select methods, please
type :code:`pydoc ert.ecl.EclRegion` or browse the API documentation
at :ref:`python_documentation`.

RFT
---

The support for RFT files in :code:`ert.ecl` is split among the three
classes :code:`EclRFTFile`, :code:`EclRFT` and :code:`EclRFTCell`. The
:code:`EclRFTFile` class is used to load an ECLIPSE RFT file. The RFT
files will generally contain results for several wells, and several
times, the :code:`EclRFTFile` class will load them all - and then
supplies an interface to query for individual RFT results based on
wellname and/or date; the individual RFT results will be in the form
of :code:`EclRFT` instances. 

.. code:: python
  
   from ert.ecl import EclRFTFile

   # Load the RFT file
   rft_file = EclRFTFile("ECLIPSE.RFT")
   
   # Extract the RFT results for well 'OP-X' at date 2010-01-15; 
   # will return None if no such RFT exists - should probably raise an
   # exception.
   rft = rft_file.get("OP-X" , datetime.date(2010,1,15))
   
In addition to the main method: :code:`EclRFTFile.get()` the
:code:`EclRFTFile` class has utility methods to list all the well and
date values present in the RFT file, the number of wells and so on.


EclRFT
......
From the :code:`EclRFTFile.get()` method we get a :code:`EclRFT`
instance. Observe that one RFT file can contain a lump of different
data RFT types:
   
   RFT: This is old-fashioned RFT which contains measurements of
        saturations for each of the completed cells.
   
   PLT: This contains production and flow rates for each phase in
        each cell.

   SEGMENT: Not implemented.

The :code:`EclRFT` object has some metadata describing which type
of data it represents, and there is some special functionality related
to MSW wells; but the main purpose of the :code:`EclRFT` class is to
serve as container holding a list of :code:`EclRFTCell` instances -
one for each perforated cell in the RFT. The :code:`EclRFT` class has
implemented the :code:`__getitem__()` method, so the following code
will loop identify an RFT from a file and then loop through all the
cells for that RFT.

.. code:: python
  
   from ert.ecl import EclRFTFile
   rft_file = EclRFTFile("ECLIPSE.RFT")
   rft = rft_file.get("OP-X" , datetime.date(2010,1,15))

   for cell in rft:
       print("Looking at cell: (%d,%d,%d)  depth:%g   pressure:%g" % (
             cell.get_i() , cell.get_j() , cell.get_k() , cell.depth , cell.pressure))

Depending on whether this is RFT or a PLT the exact type of the cell
object will be either :code:`EclRFTCell` or :code:`EclPLTCell`, the
:code:`EclPLTCell` has many extra properties not in the
:code:`EclRFTCell` class. For more detail use :code:`pydoc` to look at
the classes :code:`ert.ecl.EclRFTFile`, :code:`ert.ecl.EclRFT`,
:code:`ert.EclRFTCell` or :code:`ert.ecl.EclPLTCell` - or the API
documentation at :ref:`python_documentation`.

EclSum - working with summary files
-----------------------------------

Summary files are loaded with the :code:`EclSum` class. The
:code:`EclSum` class is a quite complete implementation for working
with Eclipse summary data, but it should also be said the
:code:`EclSum` class is one of the oldest classes in the
:code:`ert.ecl` package and the api could have been cleaner. 


Creating a :code:`EclSum` instance
..................................

In more than 99% of the cases the assumption is that we want to create
a :code:`EclSum` instance by loading read-only summary results from
disk, however it is also possible to assemble a :code:`EclSum`
instance using the api - that is not covered in this documentation.

The summary results from ECLIPSE come in two different types of files;
the :code:`ECLIPSE.SMSEPEC` file is a *header file* with all the
properties of the variables, and the :code:`ECLIPSE.UNSMRY` (or
alternatively :code:`ECLIPSE.S0000, ECLIPSE.S0001, ECLIPSE.S0002,
...`) file contains the actual values. Creating a :code:`EclSum`
instance from this is as simple as:

.. code:: python

   from ert.ecl import EclSum

   ecl_sum = EclSum("ECLIPSE")
  
As is clear from the example the :code:`EclSum` instance is created
based only on the basename of the simulation, you can optionally have
an extension like :code:`ecl_sum = EclSum("ECLIPSE.UNSMRY")` - but
that is *mostly* [7]_ ignored. 

If your case is restarted from an another case the :code:`EclSum`
cconstructor will by default try to locate the historical case, and
load the summary results from that as well. Alternatively you can pass
the argument :code:`include_restart = False` to the :code:`EclSum`
constructor. The loading of historical case will fail with an error
message if:

 1. The case can not be found in the filesystem.

 2. The :code:`SMSPEC` is not 100% identical to the current
    :code:`SMSPEC` setion; this will typically fail if you have modified
    the :code:`SUMMARY` section of the ECLIPSE deck between the two
    simulations.


About summary keys 
..................

The header file :code:`CASE.SMSPEC` has all the information *about*
the summary data. The :code:`CASE.SMSPEC` file consists of several
:code:`EclKW` instances, where the three most important one are:
:code:`KEYWORDS` which contains the variable names like
:code:`FOPT`, :code:`WGOR` and :code:`BPR`, the :code:`WGNAMES` vector
which contains names of groups and wells, and :code:`NUMS` which
contain extra numbers to characterize the variables. A small
:code:`SMSPEC` file could look like this:

.. code:: 

  KEYWORDS       WGNAMES        NUMS              |   PARAM index   Corresponding ERT key              
  ------------------------------------------------+--------------------------------------------------
  WGOR           OP_1           0                 |        0        WGOR:OP_1                          
  FOPT           +-+-+-+-       0                 |        1        FOPT                               
  WWCT           OP_1           0                 |        2        WWCT:OP_1                          
  WIR            OP_1           0                 |        3        WIR:OP_1                           
  WGOR           WI_1           0                 |        4        WWCT:OP_1                          
  WWCT           W1_1           0                 |        5        WWCT:WI_1                          
  BPR            +-+-+-         12675             |        6        BPR:12675, BPR:i,j,k              
  RPR            +-+-+-         1                 |        7        RPR:1                              
  FOPT           +-+-+-         0                 |        8        FOPT                               
  GGPR           NORTH          0                 |        9        GGPR:NORTH                         
  COPR           OP_1           5628              |       10        COPR:OP_1:56286, COPR:OP_1:i,j,k   
  RXF            +-+-+-         32768*R1(R2 + 10) |       11        RXF:2-3                                 
  SOFX           OP_1           12675             |       12        SOFX:OP_1:12675, SOFX:OP_1:i,j,jk 
  ------------------------------------------------+--------------------------------------------------

As indicated above the ERT library combines elements from the
:code:`KEYWORDS`, :code:`WGNAMES` and :code:`NUMS` vectors to create a
unique combined key. When referring to a 'key' in the rest of the
documentation, we mean one of these combined keys. Observe the
following about the smspec index:


  - For LGR's even more vectors are needed; ERT supports the LGR
    information contained in the ordinary summary files, but not the
    high temporal frequency results which are in a separate file.

  - The KEYWORDS array is always relevant; which of the other vectors
    is consulted depends on the type of variable, for e.g. WWCT the
    well name is fetched from the WGNAMES vector, whereas the NUMS
    vector is ignored. On the other hand the WGNAMES vector is ignored
    (explictly by using the dummy well +-+-+-) for BPR but the cell
    coordinate is read off from the NUMS vector.

  - For the properties defined in the grid like BPR and COPR both the
    key based directly on the NUMS value and the key based on
    tranforming the NUMS value to i,j,k are present. This is not the
    case for local grid properties, where only the i,j,k variety is
    used. For these keys the offset of :code:`(i,j,k)` is *one-based*
    , which is slightly untypical for the :code:`ert` code.

  - All well variables are present for all wells - that means the
    summary file contains oil production rate :code:`WOPR` for an
    injector, and injection rate :code:`WIR` for an oil producer.

  - The column *PARAM index* denotes the index this key will have in
    the :code:`PARAMS` *storage* vectors in the :code:`UNSMRY` or
    :code:`.Snnnn` files.

  - Nearly all variable types are supported by ERT - those which are
    missing are: *Network variables* and *Aquifer variables*.



About the time direction 
........................

As we can see from the table in "About summary keys" section the
variables in the SMSPEC file have a unique index, i.e. for the example
above we can see that the water cut in well 'OP_1' -
i.e. :code:`WWCT:OP_1` is stored as element nr 2 in the :code:`PARAMS`
vectors; so to actually get the water cut in well 'OP_1' we look up
the value of the PARAMS keyword at index 2.  By default the SUMMARY
data will be created and stored for every timestep of the simulator,
i.e. the raw time resolution is directly given by the simulators
performance, in ECLIPSE parlance these are called ministeps. When the
:code:`EclSum` class loads a summary all the ECLIPSE ministeps are
stacked together in one long vector, observe that e.g. when the
keyword RPTONLY is used in the ECLIPSE data file there can be "holes"
in the ministep sequence. 

The ert.ecl summary implementation works with four different concepts
of time:

time_index 
,,,,,,,,,,,

This is a plain index in the range :code:`[0,..num_timestep)`. Observe
that :code:`num_timesteps` is the number of timesteps loaded, and not
the total number of timesteps simulated. There can be holes in the
ministep sequence, but there will never be holes in this range. It is
closely coupled to the simulator timestep and what the user of ECLIPSE
has chosen to store, so no further meaning should be attached to these
indices. Ultimately all lookups will be based on :code:`time_index`; in the C
code it is therefore often denoted :code:`internal_index`.

ministep
,,,,,,,,,

Each simulator timestep corresponds to one ministep, but an arbitrary
summary dataset need not contain all ministeps. In the case of a
restarted simulation the first ministeps might be missing completely,
and there can also be holes in the series. Each block of summary data
is tagged with a MINISTEP number by ECLIPSE. The ministep indices are
arbitrary properties of the simulation, and are not exported by the
:code:`ert.ecl` API.  


report_step 
,,,,,,,,,,,,

This is the ECLIPSE report step, there are functions to convert
between report step and index, and you can use report step as time
value when querying for values.  


True time 
,,,,,,,,,,

It is possible to query the summary object for values interpolated to
"true" time; the true time can eiether be specified in days since
simulation start, or as python datetime.date() instance.  

Some methods
.............


__contains__
,,,,,,,,,,,,

The :code:`__contains__` method implements :code:`in` support. If you
are uncertain whether the summary contains a key or not, you should
use this function to check. In the example below a list of keys is
read from the commandline, and we check whether they are in the
summary or not: 

.. code:: python

   import sys
   from ert.ecl import EclSum
   sum = ecl.EclSum("ECLIPSE")

   for key in sys.argv[1:]
       if key in sum:
          print "Key:%s exists" % key
       else:
          print "Key:%s does NOT exist" % key


keys( pattern = None )
,,,,,,,,,,,,,,,,,,,,,,

This method will generate a list of keys witch match :code:`pattern`,
or all keys if :code:`pattern == None`. The pattern is a shell-type
wildcard expression, and the final matching is done with the stdlib
function fnmatch(), and not regular expressions. So to get a list of
all keys corresponding to block pressures, and all historical group
variables for the group "NORTH" we can use the :code:`keys()` function as:

.. code:: python

   from ert.ecl import EclSum
   sum = ecl.EclSum("ECLIPSE")

   matching = sum.keys( "BPR:*" ) + sum.keys( "G*H:NORTH" ) 


iget_report( index )
,,,,,,,,,,,,,,,,,,,,

The iget_report() method will convert a time index (as one gets from
e.g. first_gt()) and return the report step which contains this
index. For instance if you have loaded a unified summary file with the
following keywords:

.. code:: bash

   SEQHDR    <--------.
   MINISTEP 0         |
   PARAMS             |  
   MINISTEP 1         |   Report step 1
   PARAMS             |
   MINISTEP 2         |
   PARAMS             |
   SEQHDR   <---------+
   MINISTEP 3         |      
   PARAMS             |   Report step 2
   MINISTEP 4         |
   PARAMS             |


The plain index will be counting ministeps, as given by the numbering
0..4. If we call iget_report(2) we will get one, because it is Report
step 1 which contains ministep 2.

get_unit( key )
,,,,,,,,,,,,,,,

Will return the unit, i.e. :code:`SM3` for the summary variable key.  Methods
to get the value at one point in time

iget( key , index )
,,,,,,,,,,,,,,,,,,,


This function will return the value corresponding to key at 'time'
given by index.


get_interp( key , days = None , date = None )
,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,


This method will return the summary value correponding to key
(e.g. :code:`WWCT:A6-H`) interpolated to simulation date or simulation
days days - one-and-only-one of the two optional parameters must be
supplied. The date parameter is a normal python
:code:`datetime.date()` or :code:`datetime.datetime()` instance:

.. code:: python

  import datetime
  import ert.ecl.ecl as ecl
  sum = ecl.EclSum( case )

  print "FWCT after 1000 days: %g" % sum.get_interp( "FWCT" , days = 1000 )
  print "Total oil production at 10.10.2010: %g" % sum.get_interp( "FOPT" , date = datetime.date( 2010 , 10 , 10) )


If the supplied time argument falls outside the time range where you
have simulation data the function will return :code:`None`. Observe
that for rate-like variables the :code:`get_interp()` will return
step-like results if you have finer time-resolution than the
simulation results. It might be tempting to interpolate the values,
but that would be wrong. If you want interpolated values at many
points in time you can use the method :code:`get_interp_vector()`
below:


get_interp_vector( key , days_list = None , date_list = None )
,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,

This method will return a Python list of summary values corresponding
to :code:`key`, interpolated to the time points given by either days_list or
date_list. In the example below we fetch the total oil production
every 6 months:

.. code:: python

   import datetime
   import ert.ecl.ecl as ecl
   sum = EclSum( "ECLIPSE" )

   # Building up the list of dates:
   date = sum.start_date
   date_list = []
   while date < sum.end_date:
       date_list.append( date )   
       if date.month < 7:
          date = datetime.date( date.year , 7 , 1)
       else:
          date = datetime.date( date.year + 1 , 1 , 1)

    # Get the values 
    FOPT_vector = sum.get_interp_vector( "FOPT" , date_list = date_list )

    # Print the results
    for (date , FOPT) in zip(date_list , FOPT_vector):
        print "%s   %g" % (date , FOPT)


Methods to get a vector of the complete time series
...................................................


get_vector( key , report_only = False)
,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,

The get_vector function will return a EclSumVector instance
corresponding to <code>key. The __getitem__ function is also
implemented in terms of the get_vector function, i.e. the same
behaviour can be achieved with the [] operator: import ert.ecl as ecl

.. code:: python

   sum = ecl.EclSum( "/path/simuluation/BASE" )
   wwct = ecl.get_vector("WWCT:C-1")
   wopr = ecl["WOPR:C-1"]


If the summary variable key does not exist in the case the exception
KeyError will be raised. By default EclSumVector will have full
temporal resolution; but if the optional argument report_only is set
to True the vector will only contain data from the report times. This
option is not available when the [] operator is used.  Properties

length
,,,,,,

The length property is the number of data-points in the summary instance; this corresponds to the number of timesteps in the ECLIPSE simulation; alternatively you can use Python builtin function len().
start_date / end_date

The start_date and end_date properties are the starting date and
ending date of the simulation respectively. The return value is an
instance of Python datetime.date(): ...  start_date = sum.start_date
end_date = sum.end_date

print "Simulation started.............: %s" % start_date
print "Simulation ended...............: %s" % end_date
print "The simulation spans %s days...: %s" % (end_date - start_date).days 

start_time / end_time
,,,,,,,,,,,,,,,,,,,,,

This is similar to the start_date, end_date properties, but the return
value is a Python datetime.datetime() instance instead. Only relevant
if you need sub-days time resolution. (Sub-days time resolution is
probably quite bug infested.)  first_report / last_report The report
steps are the numbering ECLIPSE gives to the DATES keywords. The
properties first_report and last_report return the first and last
report steps in the current summary instance.

first_gt(key , limit) / first_lt(key , limit)
,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,

These two functions will return the index of the first "time" where
the summary vector corresponding to key goes above (first_gt()) or
falls below a limit (first_lt).  If the limit is not reached the
function will return -1; if the -1 is used as index in a subsequent
call to an EclSum method, it will fail hard.

.. code:: python

   gt_index = sum.first_gt( "WWCT:OP_3" , 0.30)
   if gt_index < 0:
      print "The water cut in well OP_3 never exceeds 0.30"
   else: 
      print "WWCT:OP_3 exceeds 0.30 after %g days." % sum.iget_days( gt_index )

   lt_index = sum.first_lt("RPR:2" , 210)
   if lt_index < 0:
      print "The pressure in region 2 never falls below 210 BARS"
   else:
      print "RPR:2 below 210 bars at : %s" % sum.iget_time(lt_index).date()

numpy_value( key )
,,,,,,,,,,,,,,,,,,

This method will return a numpy vector with all the values for the key
key. The numpy vector type can then be used e.g. to plot or do other
manipulations.

      
numpy_days
,,,,,,,,,,

This property will return a numpy vector with the number of simulation
days.  mpl_dates This property is a numpy vector of "time-values" in
matplotlib format. Suitable when plotting with matplotlib. In the
example below we fetch two vectors, and the simulation days from a
summary, and then print it all to the screen: 

.. code:: python
   from ert.ecl import EclSum

   sum = EclSum( "/path/ECLIPSE.DATA" )
   days = sum.days
   wwct = sum.numpy_value("WWCT:OP_1")
   wopr = sum.numpy_value("WOPR:OP_1")

   for (d , w , o) in zip( days , wwct , wopr):
       print "%5.0f   %5.3f  %8.1f" % (d,w,o)


Special functions/operators

[] : get The [] operator is mapped to the get_vector() method and will
return the EclSumVector corresponding to the key entered, i.e. the
EclSum instance will behave much like a dictionary. I.e. to get a
EclSumVector corresponding to field oil production rate you can do:
sum = ecl.EclSum( "ECLIPSE" ) fopr = sum["FOPR"] If you give a key
which does not exist in the summary the KeyError exception will be
raised.

EclSumVector
............

The EclSumVector class is a small convenience class to work with only
one summary vector. It is not necessary to use the EclSumVector
class - everything can be accessed directly from the EclSum
class. Many of the methods and properties of the EclSumVector are very
similar or actually identical to methods in the EclSum class:

The methods/properties which access the timeseries are the same.  Many
of the methods which take key input argument in the EclSum case
obviously take no key argument in the case of EclSumVector; if there
are no arguments left these methods will typically be converted to
read-only properties.

Constructor

EclSumVector( parent , key , report_only ) The default constructor
EclSumVector() will create a new EclSumVector instance, but the
intentition is that the EclSumVector instances should be constructed
via the EclSum parent instance, and not explicitly by the user.
Methods

The EclSumVector class has all the same methods as the EclSum class;
with the obvious exceptions of has_key() and keys(). The key argument
present in the methods ... is removed in the EclSumVector incarnation.
Properties

Special functions/operators

[] : get

The EclSumVector implements the __getitem__() method to support
iteration and the [] operator. The __getitem__ method supports
negative indices and partly slicing. When slicing the returned value
will not be reduced EclSumVector instance, but rather a plain Python
list with the correct set of elements. The elements returned from the
__getitem__ method are EclSumNode instances.

.. code:: python

   from ert.ecl import EclSum

   eclsum = EclSum( "ECLIPSE" )
   wwct = eclsum["WWCT:OP-5"]

   print "First value element: " wwct[0].value
   print "Last value         : " wwct[-1].value
   print "List of every third: " wwct[0::3]


EclSumNode
..........

The EclSumNode class is very small, more like a C struct. It contains
11the value, along with time in different units for one summary vector
at one point in time; the content of the EclSumNode is plain field
variables; no properties or anything fancy. The EclSumNode has the
following fields:

  value       : The actual value
  report_step : The report step
  mini_step   : The ministep
  days        : Days since simulation start
  date        : The simulation date
  mpl_date    : A date format suitable for matplotlib 

When iterating over a EclSumVector the return values will be in the
form EclSumNode instances. The EclSumNode instances are created on
demand by a EclSumVector instance. The example below show how the
EclSumNode instances arise when iterating over the content of a
EclSumVector:

.. code:: python

  wwct = ecl_sum["WWCT:C-1A"]
  for node in wwct:
      print "Days:%g  value:%g" % (node.days , node.value)


.. [1] Observe that the word *keyword* here means one block of
       information, as the :code:`PORV` from the
       :ref:`structure_binary_files`, which is generally *not* the
       same as one keyword from the input file.


.. [2] In binary files Boolean True and False is represented by the
       integer values -1 and 0 respectively; whereas the characters
       'T' and 'F' are used in ASCII formatted files.


.. [3] Observe that the reservoir simulator will typically
       *deactivate* cells. The :code:`EGRID/GRID` output files are
       created *after* cells have been deactivated, hence the
       distribution of active/inactive cells in a grid created from
       the input files will generally *not* agree with the result
       arrays found in the restart files.

.. [4] This corresponds to the :code:`ACTNUM` property used internally
       by Eclipse to denote active/inactive status.

.. [5] The methods are currently implemented in the base class
       :code:`EclFile` - but they should be moved to the
       :code:`EclRestartFile` class.

.. [6] Observe that the set of cells need *not* form a singly
       connected set.

.. [7] Since the summary files can be both formatted and unformatted,
       and also both unified and non-unified there can potentially be
       several datasets with the same basename present in the
       directory. The :code:`EclSum` loader will by default use the
       latest version, but by supplying an extension you can control
       which files should be loaded; i.e. when calling as
       :code:`EclSum("ECLIPSE.A0056")` the loader will *only* look for
       multiple formatted files.

