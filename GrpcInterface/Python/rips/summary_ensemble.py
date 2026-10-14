"""Convenience methods for working with summary ensembles from Python.

The methods are added to the ``SummaryEnsemble`` class, which is used
for both summary ensembles and summary groups. Use the ``is_ensemble`` flag to
pick the ensembles.
"""

from typing import Any, Dict, List

from .pdmobject import add_method
from .resinsight_classes import SummaryEnsemble


@add_method(SummaryEnsemble)
def vector_values(
    self: SummaryEnsemble, address: str, time_resolution: str = "NONE"
) -> Dict[int, Dict[str, List[Any]]]:
    """Read one summary vector for all realizations in the ensemble.

    Arguments:
        address (str): Summary vector address, for example "FOPT" or "WOPT:A1"
        time_resolution (str): "NONE" returns the original time steps of each
            realization. Use "DAY", "WEEK", "MONTH", "QUARTER", "HALFYEAR" or
            "YEAR" to resample all realizations to a common time axis.

    Returns:
        dict: Realization (summary case) id mapped to a dictionary with the
        keys "time_steps" (seconds since epoch) and "values".
    """
    result: Dict[int, Dict[str, List[Any]]] = {}
    for summary_case in self.summary_cases():
        data = summary_case.resample_values(address, time_resolution)
        result[summary_case.id] = {
            "time_steps": list(data.time_steps),
            "values": list(data.values),
        }
    return result
