from ert.ecl import EclFileEnum, EclFileFlagEnum, EclPhaseEnum, EclUnitTypeEnum , EclUtil
from ert.ecl import EclTypeEnum, EclDataType
from ert.ecl import EclSumVarType
from ert.ecl import EclSumTStep
from ert.ecl import EclSum #, EclSumVector, EclSumNode, EclSMSPECNode
from ert.ecl import EclSumKeyWordVector
from ert.ecl import EclPLTCell, EclRFTCell
from ert.ecl import EclRFT, EclRFTFile
from ert.ecl import FortIO, openFortIO
from ert.ecl import EclKW
from ert.ecl import Ecl3DKW
from ert.ecl import EclFileView
from ert.ecl import EclFile , openEclFile
from ert.ecl import Ecl3DFile
from ert.ecl import EclInitFile
from ert.ecl import EclRestartFile
from ert.ecl import EclGrid
from ert.ecl import EclRegion
from ert.ecl import EclSubsidence
from ert.ecl import phase_deltag, deltag
from ert.ecl import EclGrav
from ert.ecl import EclSumNode
from ert.ecl import EclSumVector
from ert.ecl import EclNPV , NPVPriceVector
from ert.ecl import EclCmp
from ert.ecl import EclGridGenerator

from ert.ecl.faults import Layer
from ert.ecl.faults import FaultCollection
from ert.ecl.faults import Fault
from ert.ecl.faults import FaultLine
from ert.ecl.faults import FaultSegment , SegmentMap
from ert.ecl.faults import FaultBlock , FaultBlockCell
from ert.ecl.faults import FaultBlockLayer



from tests import EclTest


class ErtLegacyEclTest(EclTest):
    pass
