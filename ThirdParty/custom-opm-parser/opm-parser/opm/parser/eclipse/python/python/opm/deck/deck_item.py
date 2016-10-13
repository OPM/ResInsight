from cwrap import BaseCClass
from opm import OPMPrototype
from opm.deck import ItemType


class DeckItem(BaseCClass):
    TYPE_NAME = "deck_item"

    _size        = OPMPrototype("int            deck_item_get_size( deck_item )")
    _get_type    = OPMPrototype("item_type_enum deck_item_get_type( deck_item )")
    _iget_int    = OPMPrototype("int            deck_item_iget_int( deck_item , int )")
    _iget_double = OPMPrototype("double         deck_item_iget_double( deck_item , int )")
    _iget_string = OPMPrototype("char*          deck_item_iget_string( deck_item , int )")

    
    def __init__(self):
        raise NotImplementedError("Can not create instantiate record directly")

    
    def __len__(self):
        """
        Returns the number of values in the item.
        """
        return self._size( )
    

    def assertType(self):
        if not hasattr(self,"value_type"):
            self.value_type = self._get_type( )

            if self.value_type == ItemType.INTEGER:
                self.iget = self._iget_int
            elif self.value_type == ItemType.DOUBLE:
                self.iget = self._iget_double
            elif self.value_type == ItemType.STRING:
                self.iget = self._iget_string
            else:
                raise Exception("What the ??")
                

            
    def __getitem__(self , index):
        """
        Will return value corresponding to @index.
        """
        self.assertType()
        if isinstance(index , int):
            if index < 0:
                index += len(self)
            if 0 <= index < len(self):
                return self.iget( index )
            else:
                raise IndexError("Invalid index:%d  -valid range [0,%d)" % (index , len(self)))
        else:
            raise TypeError("Expected integer index")

        

