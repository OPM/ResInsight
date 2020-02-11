"""
ResInsight Well
"""
import rips.generated.Commands_pb2 as Cmd

from rips.pdmobject import PdmObject

class Well(PdmObject):
    """ResInsight well class

    Attributes:
        name(string): Name of the well.

    """
    def __init__(self, name, pdm_object):
        PdmObject.__init__(self, pdm_object.pb2_object(), pdm_object.channel(), pdm_object.project())
        self.name = name
