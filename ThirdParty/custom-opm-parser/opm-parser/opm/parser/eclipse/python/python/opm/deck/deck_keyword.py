from ert.cwrap import BaseCClass
from opm import OPMPrototype

class DeckKeyword(BaseCClass):
    TYPE_NAME = "deck_keyword"

    _alloc       = OPMPrototype("void* deck_keyword_alloc( char* )" , bind = False)
    _get_name    = OPMPrototype("char* deck_keyword_get_name( deck_keyword )")
    _size        = OPMPrototype("int deck_keyword_get_size( deck_keyword )")
    _free        = OPMPrototype("void deck_keyword_free( deck_keyword )")
    _iget_record = OPMPrototype("deck_record_ref deck_keyword_iget_record( deck_keyword , int)")
    
    def __init__(self , kw):
        c_ptr = self._alloc( kw )
        super(DeckKeyword, self).__init__(c_ptr)

    def __len__(self):
        """
        Returns the number of records in the keyword.
        """
        return self._size( )

    def __getitem__(self , index):
        """
        Will return DeckRecord corresponding to @index.
        """
        if index < 0:
            index += len(self)
            
        if 0 <= index < len(self):
            record = self._iget_record( index )
            record.setParent( self )
            return record
        else:
            raise IndexError("Invalid index:%d  -valid range [0,%d)" % (index , len(self)))

    def __str__(self):
        return self.name()

    def name(self):
        return self._get_name( )

    
    def free(self):
        self._free( )
    

