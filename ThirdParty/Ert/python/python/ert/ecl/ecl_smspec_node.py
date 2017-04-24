#  Copyright (C) 2016  Statoil ASA, Norway.
#
#  This file is part of ERT - Ensemble based Reservoir Tool.
#
#  ERT is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  ERT is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or
#  FITNESS FOR A PARTICULAR PURPOSE.
#
#  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
#  for more details.

from cwrap import BaseCClass
from ert.ecl import EclPrototype


class EclSMSPECNode(BaseCClass):
    """
    Small class with some meta information about a summary variable.

    The summary variables have different attributes, like if they
    represent a total quantity, a rate or a historical quantity. These
    quantities, in addition to the underlying values like WGNAMES,
    KEYWORD and NUMS taken from the the SMSPEC file are stored in this
    structure.
    """
    TYPE_NAME = "smspec_node"
    _node_is_total      = EclPrototype("bool smspec_node_is_total( smspec_node )")
    _node_is_historical = EclPrototype("bool smspec_node_is_historical( smspec_node )")
    _node_is_rate       = EclPrototype("bool smspec_node_is_rate( smspec_node )")
    _node_unit          = EclPrototype("char* smspec_node_get_unit( smspec_node )")
    _node_wgname        = EclPrototype("char* smspec_node_get_wgname( smspec_node )")
    _node_keyword       = EclPrototype("char* smspec_node_get_keyword( smspec_node )")
    _node_num           = EclPrototype("int   smspec_node_get_num( smspec_node )")
    _node_need_num      = EclPrototype("bool  smspec_node_need_nums( smspec_node )")
    _gen_key1           = EclPrototype("char* smspec_node_get_gen_key1( smspec_node )")
    _gen_key2           = EclPrototype("char* smspec_node_get_gen_key2( smspec_node )")
    _var_type           = EclPrototype("ecl_sum_var_type smspec_node_get_var_type( smspec_node )")
    
    
    def __init__(self):
        super(EclSMSPECNode, self).__init__(0) # null pointer
        raise NotImplementedError("Class can not be instantiated directly!")

    @property
    def unit(self):
        """
        Returns the unit of this node as a string.
        """
        return self._node_unit( )

    @property
    def wgname(self):
        """
        Returns the WGNAME property for this node.

        Many variables do not have the WGNAME property, i.e. the field
        related variables like FOPT and the block properties like
        BPR:10,10,10. For these variables the function will return
        None, and not the ECLIPSE dummy value: ":+:+:+:+".
        """
        return self._node_wgname( )


    @property
    def keyword(self):
        """
        Returns the KEYWORD property for this node.

        The KEYWORD property is the main classification property in
        the ECLIPSE SMSPEC file. The properties of a variable can be
        read from the KEWYORD value; see table 3.4 in the ECLIPSE file
        format reference manual.
        """
        return self._node_keyword( )

    @property
    def num(self):
        return self.getNum( )

    def getKey1(self):
        """
        Returns the primary composite key, i.e. like 'WOPR:OPX' for this
        node.
        """
        return self._gen_key1( )


    def getKey2(self):
        """Returns the secondary composite key for this node.

        Most variables have only one composite key, but in particular
        nodes which involve (i,j,k) coordinates will contain two
        forms:

            getKey1()  =>  "BPR:10,11,6"
            getKey2()  =>  "BPR:52423"

        Where the '52423' in getKey2() corresponds to i + j*nx +
        k*nx*ny.
        """
        return self._gen_key2( )


    def varType(self):
        return self._var_type( )


    def getNum(self):
        """
        Returns the NUMS value for this keyword; or None.

        Many of the summary keywords have an integer stored in the
        vector NUMS as an attribute, i.e. the block properties have
        the global index of the cell in the nums vector. If the
        variable in question makes use of the NUMS value this property
        will return the value, otherwise it will return None:

           sum.smspec_node("FOPT").num     => None
           sum.smspec_node("BPR:1000").num => 1000

        """
        if self._node_need_num( ):
            return self._node_num( )
        else:
            return None

    def isRate(self):
        """
        Will check if the variable in question is a rate variable.

        The conecpt of rate variabel is important (internally) when
        interpolation values to arbitrary times.
        """
        return self._node_is_rate()

        
    def isTotal(self):
        """
        Will check if the node corresponds to a total quantity.

        The question of whether a variable corresponds to a 'total'
        quantity or not can be interesting for e.g. interpolation
        purposes. The actual question whether a quantity is total or
        not is based on a hardcoded list in smspec_node_set_flags() in
        smspec_node.c; this list again is based on the tables 2.7 -
        2.11 in the ECLIPSE fileformat documentation.
        """
        return self._node_is_total( )


    def isHistorical(self):
        """
        Checks if the key corresponds to a historical variable.

        The check is only based on the last character; all variables
        ending with 'H' are considered historical.
        """
        return self._node_is_historical( )

