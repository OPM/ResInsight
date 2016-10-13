from cwrap import BaseCEnum


class ItemType(BaseCEnum):
    TYPE_NAME="item_type_enum"
    INTEGER = None
    STRING = None
    DOUBLE = None


ItemType.addEnum("INTEGER", 1)
ItemType.addEnum("STRING", 2)
ItemType.addEnum("DOUBLE", 3)

