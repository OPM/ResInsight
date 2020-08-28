from opm._common import DeckKeyword
from opm._common import DeckItem


# in prinsiple it should be possible to use the  has_value(int) function
# on not only the first element. However, in 99% of the use cases it is the
# first element which is of interesst. Hence for python bindings this is
# hardcoded to the first element

@property
def defaulted_deckitem(self):
    return self.__defaulted(0)


@property
def has_value_deckitem(self):
    return self.__has_value(0)


@property
def get_item_deckitem(self):

    if self.is_int():
        return self.get_int(0)
    elif self.is_string():
        return self.get_str(0)
    elif self.is_double():
        return self.get_raw(0)
    elif self.is_uda():
        if self.__is_numberic():
            return self.__uda_double()
        else:
            return self.__uda_str()
    else:
        raise ValueError("Deck Item, unknown type")


setattr(DeckItem, "defaulted", defaulted_deckitem)
setattr(DeckItem, "valid", has_value_deckitem)
setattr(DeckItem, "value", get_item_deckitem)
