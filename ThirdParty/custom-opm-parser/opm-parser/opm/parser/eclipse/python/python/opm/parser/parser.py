import os.path
import json
import inspect
from functools import partial

from cwrap import BaseCClass
from opm import OPMPrototype
from opm.deck import Deck
from opm.parser import ParseContext



class class_and_instance_method(object):
    def __init__(self , func):
        self.func = func

    def __get__(self , obj , type = None):
        if obj is None:
            first_arg = type
        else:
            first_arg = obj

        return partial(self.func , first_arg)

    

class Parser(BaseCClass):
    TYPE_NAME = "parser"
    _alloc            = OPMPrototype("void*     parser_alloc()" , bind = False)
    _free             = OPMPrototype("void      parser_free(parser)")
    _has_keyword      = OPMPrototype("bool      parser_has_keyword(parser, char*)")
    _parse_file       = OPMPrototype("deck_obj  parser_parse_file(parser, char*, parse_context)")
    _add_json_keyword = OPMPrototype("void      parser_add_json_keyword(parser, char*)")

    
    def __init__(self):
        c_ptr = self._alloc()
        super(Parser, self).__init__(c_ptr)

        
    def __contains__(self , kw):
        return self._has_keyword(kw)
        
        
    def free(self):
        self._free( )


    @class_and_instance_method
    def parseFile(arg , filename , parse_mode = None):
        """The parseFile() method will parse a filename and return a Deck
        instance. The parseFile method can be called both as a bound
        method on a Parser instance, and as a classmethod without first
        creating a parser intance:
    
        Using a parser instance:

            parser = Parser( )
            parser.addKeyword({"name" : "MY_KEYWORD" , "sections" : ["SCHEDULE"] , "items" : [...]})
            deck = parser.parseFile("FILE.DATA")

        As illustrated the main purpose of the instance based method
        is the ability to add additional keywords before actually
        parsing.
        Using the classmethod:

            deck = Parser.parseFile("FILE.DATA")

        """

        if isinstance(arg , type):
            parser = Parser()
        else:
            parser = arg

        if os.path.isfile( filename ):
            if parse_mode is None:
                parse_mode = ParseContext( )
            return parser._parse_file( filename, parse_mode)
        else:
            raise IOError("No such file:%s" % filename)


    def addKeyword(self , schema):
        """
        schema should be an ordinary Python dictionary describing the
        keyword structure.
        """
        json_string = json.dumps( schema )
        self._add_json_keyword( json_string )
