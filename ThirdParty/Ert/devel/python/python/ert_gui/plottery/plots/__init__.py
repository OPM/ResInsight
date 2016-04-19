import os
import matplotlib

def headless():
    return "DISPLAY" not in os.environ

if headless():
    matplotlib.use("Agg")
else:
    matplotlib.use("Qt4Agg")

from .histogram import plotHistogram
from .gaussian_kde import plotGaussianKDE

from .refcase import plotRefcase
from .history import plotHistory
from .observations import plotObservations

from .ensemble import plotEnsemble
from .statistics import plotStatistics
from .distribution import plotDistribution
from .ccsp import plotCrossCaseStatistics