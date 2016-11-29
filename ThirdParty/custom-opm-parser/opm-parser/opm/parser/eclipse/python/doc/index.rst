Parser
======

The class :code:`Parser` will create a parser object which can be used
to parse an Eclipse formatted file. To create a new parser object is
quite simple:

.. code:: python

   from opm.parser import Parser

   parser = Parser()


   
Parsing a file
--------------

The parser class has a method :code:`parseFile()` which is used
to parse a file. The :code:`parseFile()` method can be invoked as an
ordinary method on a parser instance, or alternatively as a class
method directly on the :code:`Parser` class:

.. code:: python

   from opm.parser import Parser
   import sys


   input_file1 = sys.argv[1]

   # Alternative 1 - parse directly from the class:
   deck = Parser.parseFile( input_file1 )

   # Alternative 2 - create a Parser instance and use that
   # multiple times
   parser = Parser( )
   for file in sys.argv[1:]:
       deck = parser.parseFile( file )


Keywords in the parser
----------------------

The :code:`__contains__()` method is implemented on the parser and can
be used to check if a certain keyword is recognized by the parser:

.. code:: python

   from opm.parser import Parser
   import sys

   parser = Parser()
   for kw in sys.argv[1:]:
       if kw in parser:
           print("Parser has keyword:%s " % kw)
       else:
           print("Parser does not recognize keyword: %s" % kw)
   
Using the method :code:`addKeyword( )` you can add keywords to the
parser runtime. Adding keywords dynamcally is a slightly advanced
topic, but if you look at the json strings for the built in keywords
you should get a good hint.

In this example we add the keyword :code:`SALINITY` to the parser:

.. code:: python

   from opm.parser import Parser

   salinity = {"name" : "SALINITY" , "sections" : ["PROPS"], size : 1,
               "items" : [{"name" : "SALINITY"
                           "value_type" : "DOUBLE",
                           "dimension" : "Density"}]}

   parser = Parser( )
   parser.addKeyword( salinity )

Adding keywords this way requires some knowledge of the schma used to
define keywords, but it is possible.


ParseContext
============

By default the parser will parse the input files in the strictest mode
possible, this is quite strict and there are plenty of things which
can go wrong. Using the :code:`ParseContext` class you can optionally
configure how strict the parser should be when encountering various
common error situations. In the example below we configure the
:code:`ParseContext` object to ignore unknown keywords:

.. code:: python

          
   from opm.parser import Parser, ParseContext, ErrorAction

   pc = ParseContext()
   # Ignore unknown keywords found during parsing
   pc.update( "PARSE_UNKNOWN_KEYWORD" , ErrorAction.IGNORE )

   deck = Parser.parseFile( input_file , parse_mode = pc )

   
The :code:`ParseContext.update( )` method supports wildcards in the
variable name, so this code will instruct the parser in the most
lenient way possible:

.. code:: python

          
   from opm.parser import Parser, ParseContext, ErrorAction

   pc = ParseContext()

   # Ignore all possible error encountered during the parsing 
   pc.update( "*" , ErrorAction.IGNORE )

   deck = Parser.parseFile( input_file , parse_mode = pc )

Observe that using :code:`ParseContext` to ignore parse errors is a
slippery slope, when a parser has detected and ignored an error the
internal state will be more fragile, and the chance of a complete
explosion - and also incorrect results, certainly increase this way.


Deck
====

The return value from the :code:`Parser::parseFile()` method is 
an instance of the class :code:`Deck`. The :code:`Deck` class is a
faithful represantation of all the keywords in the input
file. Specifically the following transformations have been applied:

1. All :code:`INCLUDE` files have been resolved and read.
2. Default values and multipliers have been handled.
3. All items have been converted to correct type; i.e. real, int and
   string and all numeric values have been converted to SI units.

Keyword keyword interactions like applying box modifiers to grid
properties have not been applied, that is in the :code:`EclipseState`
class which is not fully exposed in Python yet.
 

The Deck hierarchy
------------------

The information in a deck is organized in hierarchy going as
:code:`Deck`, :code:`DeckKeyword`, :code:`DeckRecord` and
:code:`DeckItem`. Consider the following (incomplete) input deck
consisting of only the :code:`EQLDIMS` and :code:`EQUIL` keywords:

.. code:: 

   EQLDIMS                                                      
     3  100  20  1  1  /                                        
                                                                 
                                                                
   EQUIL
        2460   382.4   1705.0  0.0   500   0.0  1  1  20 / 
        2470   383.4   1000.0  0.0   600   0.0  1  1  20 /       
        2480   384.4   3000.0  0.0   400   0.0  1  1  20 /       

The :code:`Deck` is the complete set of keywords in the input and a
:code:`DekcKeyword` is one block of data like :code:`EQLDIMS` and
:code:`EQUIL`. A :code:`DeckRecord` is one *slash terminated* block of
data, and a :code:`DeckItem` is one - or several related - pieces of
data. In the example above the :code:`EQLDIMS` keyword contains one
record, which has five items, and the :code:`EQUIL` keyword has three
records, each consisting of 9 items.

The concept of :code:`Deck` and :code:`DeckKeyword` are generally
quite clear. For the :code:`EQUIL` and :code:`EQLDIMS` example above
the :code:`DeckRecord` and :code:`DeckItem` are also easily
understable, but in the case of large numerical fields like
:code:`ACTNUM, PERMX` and :code:`PORO`, and also the tables like
:code:`SGOF3` and :code:`PVTG`, :code:`DeckRecord` and :code:`DeckItem`
are not so tightly bound to the input structure. Consider for example
the :code:`PORO` keyword which could look like this for a model with
25 cells:

.. code::


   PORO
      0.15 0.16 0.17 0.62 0.62
      0.15 0.16 0.42 0.62 0.51
      0.18 0.12 0.17 0.65 0.52
      0.12 0.26 0.27 0.65 0.62
      0.11 0.09 9.15 0.62 0.62 /

When this is parsed the :code:`PORO` :code:`DeckKeyword` will consists
of only *one* :code:`DeckRecord`, and that record consists of *one*
:code:`DeckItem` - with 25 numerical values. When working
with these keywords you should preferably work through the
:code:`EclipseState` API where these details have been abstracted away.


Iterating through the Deck
--------------------------

The :code:`Deck` object support iteration and random access with the
:code:`[..]` operator, it also implements the :code:`in`
operator whcih can be used to check if the deck has a certain keyword:

.. code:: python

          
   from opm.parser import Parser

   deck = Parser.parseFile( input_file )

   # Check of a certain keyword is in the deck:
   if "NNC" in deck:
       print("The deck has the NNC keyword")
   else:
       print("No NNC keyword found in deck")


   # Go through all the keywords in the deck I:
   for kw in deck:
       print kw
   
   # Go through all the keywors in the deck II:
   for index in range(len(deck)):
       kw = deck[index]

   # Get a list of all the WCONHIST keywords:
   wconhist_list = deck["WCONHIST"]


   
Iterating through a DeckKeyword
-------------------------------
When you have a :code:`DeckKewyord` you can get the records using the
:code:`[..]` operator:


.. code:: python

          
   from opm.parser import Parser

   deck = Parser.parseFile( input_file )
   equil = deck["EQUIL"][0]

   print("EQUIL keyword has %d records" % len(equil))
   for index in range(len(equil)):
       record = equil[index]
      

      
Iterating through a DeckRecord
------------------------------

From a :code:`DeckRecord` you can iterate over the items in the
:code:`DeckRecord`. The items in the record can be accessed either
through integer indices or item names; the item names are loosely
inspired by the Eclipse documentation, but the authorative source is
the set of Json files used to configure the keywords.

.. code:: python

          
   from opm.parser import Parser

   deck = Parser.parseFile( input_file )
   equil = deck["EQUIL"][0]
   rec0 = equil[0]

   # Go through all the items:
   for item in rec0:
       print item

   # Fetch named items:
   depth = rec0["DATUM_DEPTH"]
   owc = rec0["OWC"]
   goc = rec0["GOC"]
   
   


Getting values from a DeckItem
------------------------------
   
When you have finally gotten all the way down to a :code:`DeckItem` it
is time to get an actual value, so to actually get the numerical
values for reference depth, goc and owc from the :code:`EQUIL` keyword
you would do:

.. code:: python

          
   from opm.parser import Parser

   deck = Parser.parseFile( input_file )
   equil = deck["EQUIL"][0]

   for region in range(len(equil)):
       depth_item = region["DATUM_DEPTH"]
       owc = region["OWC"]
       goc = region["GOC"]
       print("Equilibration region: %d  datum depth:%g   OWC:%g GOC:%g" %
                 (depth_item[0] , owc[0] , goc[0]))
   

Since the :code:`DeckItem` instances can *in principle* be multivalued we must use
:code:`[0]` to access the first element. It is mostly for the large numerical
keywords like :code:`PORO` that the :code:`DeckRecord` / :code:`DeckItem` is not
entirely evident from the input structure. Consider this code to iterate through all
the elements in a :code:`PORO` keyword:


.. code:: python

          
   from opm.parser import Parser

   deck = Parser.parseFile( input_file )
   equil = deck["PORO"][0]
   rec0 = equil[0]
   item0 = rec[0]

   min_value = 1
   max_value = 0
   sum = 0
   for value in item0:
       min_value = min( min_value , value )
       max_value = max( max_value , value )
       sum += value
   avg = sum / len(item0) 
       
   print("PORO min:%g  avg:%g   max:%g" % (min_value , avg , max_value))

    
    
