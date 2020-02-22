from ert.util import Version
from ert.util import RngAlgTypeEnum, RngInitModeEnum
from ert.util import CTime
from ert.util import PermutationVector
from ert.util import VectorTemplate
from ert.util import DoubleVector
from ert.util import IntVector
from ert.util import BoolVector
from ert.util import TimeVector
from ert.util import StringList
from ert.util import RandomNumberGenerator
from ert.util import LookupTable
from ert.util import Hash, StringHash, DoubleHash, IntegerHash
from ert.util import ThreadPool
from ert.util import installAbortSignals, updateAbortSignals

from tests import EclTest

try:
    from res.util import SubstitutionList
except ImportError:
    pass

try:
    from res.util import CThreadPool, startCThreadPool
except ImportError:
    pass

class ErtLegacyUtilTest(EclTest):
    pass
