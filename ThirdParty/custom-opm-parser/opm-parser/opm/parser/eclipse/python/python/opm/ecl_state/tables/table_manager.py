import os.path

from cwrap import BaseCClass
from opm import OPMPrototype


class TableManager(BaseCClass):
    TYPE_NAME   = "table_manager"
    _alloc      = OPMPrototype("void*     table_manager_alloc(deck)" , bind = False)
    _free       = OPMPrototype("void*     table_manager_free(table_manager)")
    _has_table  = OPMPrototype("bool      table_manager_has_table(table_manager , char*) ")
    _num_tables = OPMPrototype("int       table_manager_num_tables(table_manager , char*) ")    
    _get_table  = OPMPrototype("table_ref table_manager_get_table(table_manager , char*, int) ")    

    
    def __init__(self , deck):
        c_ptr = self._alloc(deck)
        super(TableManager, self).__init__(c_ptr)

        
    def __contains__(self,table_name):
        return self._has_table( table_name )

        
    def free(self):
        self._free( )
        

    def hasTable(self, name ):
        """Will check if the table manager has table @name. Also available as
        the __contains__() operator:

        if "SWOF" in table_manager:
           print("Have SWOF")

        """
        return self._has_table(  name )


    def numTables(self, name):
        """Will return the number of @name tables. 

        Observe that the number of tables will always be zero, or the
        appropriate item (NTSFUN, NTPVT, NSSFUN, ...) from the TABDIMS
        keyword; i.e. the manager does not differentiate between
        tables explicitly added to the deck, and thoe which are
        implicitly available through default behaviour.
        """
        return self._num_tables( name )
    

    def getTable(self , name , num = 0):
        if not name in self:
            raise KeyError("No such table:%s" % name)

        if num >= self.numTables( name ):
            raise IndexError("For table:%s allocwd num range is [0,%d)" % (name , self.numTables(name) - 1))

        table = self._get_table(name , num )
        table.setParent( self )
        return table
