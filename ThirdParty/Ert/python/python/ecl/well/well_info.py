from cwrap import BaseCClass
from ecl.ecl import EclGrid
from ecl.ecl.ecl_file import EclFile
from ecl.well import WellTimeLine, WellPrototype


class WellInfo(BaseCClass):
    TYPE_NAME = "well_info"

    _alloc            = WellPrototype("void* well_info_alloc(ecl_grid)", bind = False)
    _free             = WellPrototype("void  well_info_free(well_info)")
    _load_rstfile     = WellPrototype("void  well_info_load_rstfile(well_info, char*, bool)")
    _load_rst_eclfile = WellPrototype("void  well_info_load_rst_eclfile(well_info, ecl_file, bool)")
    _get_well_count   = WellPrototype("int   well_info_get_num_wells(well_info)")
    _iget_well_name   = WellPrototype("char* well_info_iget_well_name(well_info, int)")
    _has_well         = WellPrototype("bool  well_info_has_well(well_info, char*)")
    _get_ts           = WellPrototype("well_time_line_ref well_info_get_ts(well_info, char*)")


    def __init__(self, grid, rst_file=None, load_segment_information=True):
        """
        @type grid: EclGrid
        @type rst_file: str or EclFile or list of str or list of EclFile
        """
        c_ptr = self._alloc(grid)
        super(WellInfo, self).__init__(c_ptr)
        if not c_ptr:
            raise ValueError('Unable to construct WellInfo from grid %s.' % str(grid))

        if rst_file is not None:
            if isinstance(rst_file, list):
                for item in rst_file:
                    self.addWellFile(item, load_segment_information)
            else:
                self.addWellFile(rst_file, load_segment_information)


    def __repr__(self):
        return 'WellInfo(well_count = %d) at 0x%x' % (len(self), self._address())

    def __len__(self):
        """ @rtype: int """
        return self._get_well_count( )


    def __getitem__(self, item):
        """
         @type item: int or str
         @rtype: WellTimeLine
        """

        if isinstance(item, str):
            if not item in self:
                raise KeyError("The well '%s' is not in this set." % item)
            well_name = item

        elif isinstance(item, int):
            if not 0 <= item < len(self):
                raise IndexError("Index must be in range 0 <= %d < %d" % (item, len(self)))
            well_name = self._iget_well_name( item )

        return self._get_ts(well_name).setParent(self)

    def __iter__(self):
        """ @rtype: iterator of WellTimeLine """
        index = 0

        while index < len(self):
            yield self[index]
            index += 1


    def allWellNames(self):
        """ @rtype: list of str """
        return [self._iget_well_name(index) for index in range(0, len(self))]


    def __contains__(self, item):
        """
         @type item: str
         @rtype: bool
        """
        return self._has_well( item )

    def addWellFile(self, rst_file, load_segment_information):
        """ @type rstfile: str or EclFile """
        if isinstance(rst_file, str):
            self._load_rstfile(rst_file, load_segment_information)
        elif isinstance(rst_file, EclFile):
            self._load_rst_eclfile(rst_file, load_segment_information)
        else:
            raise TypeError("Expected the RST file to be a filename or an EclFile instance.")


    def hasWell(self , well_name):
        return well_name in self


    def free(self):
        self._free( )
