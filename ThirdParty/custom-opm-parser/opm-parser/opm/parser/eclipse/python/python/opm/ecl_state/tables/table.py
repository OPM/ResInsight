import os.path

from cwrap import BaseCClass
from opm import OPMPrototype

from .table_index import TableIndex


class Table(BaseCClass):
    TYPE_NAME   = "table"
    _has_column          = OPMPrototype("bool            table_has_column( table , char* )")
    _num_rows            = OPMPrototype("int             table_get_num_rows( table )")
    _get_value           = OPMPrototype("double          table_get_value( table , char* , int)")
    _evaluate            = OPMPrototype("double          table_evaluate( table , char* , int)")
    _evaluate_with_index = OPMPrototype("double          table_evaluate_from_index( table , char* , table_index)")
    _lookup              = OPMPrototype("table_index_obj table_lookup( table , double)")
    
    def __init__(self , *args, **kwargs):
        raise NotImplementedError("Can not create instantiate tables directly")

    
    def __contains__(self , column):
        return self._has_column( column )
    

    def hasColumn(self , column):
        return self._has_column( column )


    def numRows(self):
        return self._num_rows( )
    

    def getValue(self , column , row):
        """Will return a value from the table.

        @column should be the name of a column, and @row should be an
        integer value in the range [0,num_rows).
        """
        if not column in self:
            raise KeyError("No such column: %s" % column)

        if not 0 <= row < self.numRows( ):
            raise IndexError("Invalid row index:%d Valid range: [0,%d)" % (row , self.numRows()))

        return self._get_value( column , row )


    def evaluate(self , column , arg_value):
        """Will evaluate the column @column using linear interpolation of the
        argument value @arg_value. The @arg_value will be looked up in
        the first column:

           ARG_COLUM   COL1
           0           2.50
                                <----- Here!
           1           7.50 
           2          10.00
        
           table.evaluate("COL1" , 0.50)

        This will use linear interpolation of the value 0.50 in the
        first column, and find that the answer should be just between
        the two first elements in the "COL1" column - i.e. 5.00.

        Alternatively the @arg_value can be a TableIndex instance
        which is an intermediate result from a previous linear
        interpolation - typically inferred from another table.
        """


        if not column in self:
            raise KeyError("No such column: %s" % column)

        if isinstance(arg_value , TableIndex):
            return self._evaluate_with_index(  column , arg_value  )
        else:
            return self._evaluate( column , arg_value  )


    def lookup(self , arg_value):
        """
        Will do linear interpolation of the @arg_value in the argument
        column of the table. The return value is a TableIndex instance
        which can be used as arg_value argument in the
        Table.evaluate() method.
        """
        
        return self._lookup(arg_value)
