from .pdmobject import add_method
from .resinsight_classes import PdmNestedCollectionBase


@add_method(PdmNestedCollectionBase)
def collection_name(self: PdmNestedCollectionBase) -> str:
    """Get the name of this folder

    The same for every folder class. A folder without a stored name returns the
    name ResInsight falls back to, such as the file name of an imported polygon
    file. The class specific attributes
    (surface_user_description, polygon_collection_name) remain for existing scripts.

    Returns:
        The folder name
    """
    return self.collection_name_internal().values[0]
