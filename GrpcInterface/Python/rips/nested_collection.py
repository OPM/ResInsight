from .pdmobject import add_method
from .resinsight_classes import PdmNestedCollectionBase


@add_method(PdmNestedCollectionBase)
def collection_name(self: PdmNestedCollectionBase) -> str:
    """Get the name of this folder

    The same for every folder class. The class specific attributes
    (surface_user_description, polygon_collection_name) remain for existing scripts.

    Returns:
        The folder name
    """
    return self.collection_name_internal().values[0]
