from ert.cwrap import BaseCClass
from opm import OPMPrototype

class DeckRecord(BaseCClass):
    TYPE_NAME = "deck_record"

    _size      = OPMPrototype("int           deck_record_get_size( deck_record )")
    _has_item  = OPMPrototype("bool          deck_record_has_item( deck_record , char*)")
    _get_item  = OPMPrototype("deck_item_ref deck_record_get_item( deck_record , char*)")
    _iget_item = OPMPrototype("deck_item_ref deck_record_iget_item( deck_record , int)")

    
    def __init__(self):
        raise NotImplementedError("Can not create instantiate record directly")

    
    def __len__(self):
        """Returns the number of items in the record; observe that an item
        can consist of several input values. Observe that the count
        will include items with a default, which are not specified in
        the deck, i.e. for a input item like:

          DATES
             10 'MAY' 2012 /
          /
        
          dates_kw = deck["DATES"][0]
          record = dates_kw[0]
          length = len(record)

        The length variable will have the value 4 - because the DATES
        item has an optional fourth item 'TIME' with a default value.
        """
        return self._size( )


    def __contains__(self , item):
        return self._has_item( item )
    
    
    def __getitem__(self , index):
        """
        Will return DeckItem corresponding to @index.
        """
        if isinstance(index , int):
            if index < 0:
                index += len(self)

            if 0 <= index < len(self):
                item = self._iget_item( index )
                item.setParent( self )
                return item
            else:
                raise IndexError("Invalid index:%d  -valid range [0,%d)" % (index , len(self)))
        elif isinstance(index,str):
            if index in self:
                item = self._get_item(  index )
                item.setParent( self )
                return item
            else:
                raise KeyError("Not recognized item key:%s" % index)

