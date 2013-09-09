from ert.cwrap import BaseCClass, CWrapper
from ert.ecl import ECL_LIB


class EclSMSPECNode(BaseCClass):
    """
    Small class with some meta information about a summary variable.

    The summary variables have different attributes, like if they
    represent a total quantity, a rate or a historical quantity. These
    quantities, in addition to the underlying values like WGNAMES,
    KEYWORD and NUMS taken from the the SMSPEC file are stored in this
    structure.
    """

    def __init__(self):
        super(EclSMSPECNode, self).__init__(0) # null pointer
        raise NotImplementedError("Class can not be instantiated directly!")

    @property
    def is_total(self):
        """
        Will check if the node corresponds to a total quantity.

        The question of whether a variable corresponds to a 'total'
        quantity or not can be interesting for e.g. interpolation
        purposes. The actual question whether a quantity is total or
        not is based on a hardcoded list in smspec_node_set_flags() in
        smspec_node.c; this list again is based on the tables 2.7 -
        2.11 in the ECLIPSE fileformat documentation.
        """
        return EclSMSPECNode.cNamespace().node_is_total(self)

    @property
    def is_rate(self):
        """
        Will check if the variable in question is a rate variable.

        The conecpt of rate variabel is important (internally) when
        interpolation values to arbitrary times.
        """
        return EclSMSPECNode.cNamespace().node_is_rate(self)


    @property
    def is_historical(self):
        """
        Checks if the key corresponds to a historical variable.

        The check is only based on the last character; all variables
        ending with 'H' are considered historical.
        """
        return EclSMSPECNode.cNamespace().node_is_historical(self)


    @property
    def unit(self):
        """
        Returns the unit of this node as a string.
        """
        return EclSMSPECNode.cNamespace().node_unit(self)

    @property
    def wgname(self):
        """
        Returns the WGNAME property for this node.

        Many variables do not have the WGNAME property, i.e. the field
        related variables like FOPT and the block properties like
        BPR:10,10,10. For these variables the function will return
        None, and not the ECLIPSE dummy value: ":+:+:+:+".
        """
        return EclSMSPECNode.cNamespace().node_wgname(self)


    @property
    def keyword(self):
        """
        Returns the KEYWORD property for this node.

        The KEYWORD property is the main classification property in
        the ECLIPSE SMSPEC file. The properties of a variable can be
        read from the KEWYORD value; see table 3.4 in the ECLIPSE file
        format reference manual.
        """
        return EclSMSPECNode.cNamespace().node_keyword(self)

    @property
    def num(self):
        """
        Returns the NUMS value for this keyword; or None.

        Many of the summary keywords have an integer stored in the
        vector NUMS as an attribute, i.e. the block properties have
        the global index of the cell in the nums vector. If the
        variable in question makes use of the NUMS value this property
        will return the value, otherwise it will return None:

           sum.smspec_node("FOPT").num     => None
           sum.smspec_node("BPR:1000").num => 1000
        """
        if EclSMSPECNode.cNamespace().node_need_num(self):
            return EclSMSPECNode.cNamespace().node_num(self)
        else:
            return None



cwrapper = CWrapper(ECL_LIB)

cwrapper.registerType("smspec_node", EclSMSPECNode)
cwrapper.registerType("smspec_node_obj", EclSMSPECNode.createPythonObject)
cwrapper.registerType("smspec_node_ref", EclSMSPECNode.createCReference)

EclSMSPECNode.cNamespace().node_is_total = cwrapper.prototype("bool smspec_node_is_total( smspec_node )")
EclSMSPECNode.cNamespace().node_is_historical = cwrapper.prototype("bool smspec_node_is_historical( smspec_node )")
EclSMSPECNode.cNamespace().node_is_rate = cwrapper.prototype("bool smspec_node_is_rate( smspec_node )")
EclSMSPECNode.cNamespace().node_unit = cwrapper.prototype("char* smspec_node_get_unit( smspec_node )")
EclSMSPECNode.cNamespace().node_wgname = cwrapper.prototype("char* smspec_node_get_wgname( smspec_node )")
EclSMSPECNode.cNamespace().node_keyword = cwrapper.prototype("char* smspec_node_get_keyword( smspec_node )")
EclSMSPECNode.cNamespace().node_num = cwrapper.prototype("int   smspec_node_get_num( smspec_node )")
EclSMSPECNode.cNamespace().node_need_num = cwrapper.prototype("bool  smspec_node_need_nums( smspec_node )")