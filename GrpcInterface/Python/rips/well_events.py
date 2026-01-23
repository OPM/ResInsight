"""
Well Events module for managing time-dependent well events.

This module provides functionality for creating and managing well events
in a timeline-based event system. Events can be perforation events, valve events,
tubing changes, well state changes, and production/injection control changes.
"""

from typing import Any, Dict
from datetime import date, datetime

from .pdmobject import add_method
from .resinsight_classes import Case
from .generated.generated_classes import (
    KeywordEvent,
    WellEventKeyword,
    WellEventTimeline,
)


def _format_date(event_date: str | date | datetime) -> str:
    """Convert date to ISO format string (YYYY-MM-DD)."""
    if isinstance(event_date, str):
        return event_date
    elif isinstance(event_date, datetime):
        return event_date.strftime("%Y-%m-%d")
    elif isinstance(event_date, date):
        return event_date.strftime("%Y-%m-%d")
    else:
        raise TypeError(
            f"event_date must be a string, date, or datetime, not {type(event_date)}"
        )


@add_method(WellEventTimeline)
def add_well_keyword_event(
    self: WellEventTimeline,
    event_date: str | date | datetime,
    well_path: Any,
    keyword_name: str,
    keyword_data: Dict[str, Any],
) -> WellEventKeyword:
    """Add a well keyword event with arbitrary keyword data.

    This is a convenience method that automatically infers types from Python
    values and calls the underlying GRPC method with parallel arrays.

    Type inference rules:
    - str → STRING
    - int → INT
    - float → DOUBLE
    - bool → INT (1 or 0)

    Arguments:
        event_date: Date string in YYYY-MM-DD format, date, or datetime object
        well_path (WellPath): The well path object
        keyword_name (str): Keyword name (e.g., "WCONHIST", "WELTARG", "WRFTPLT")
        keyword_data (dict): Dictionary mapping keyword item names to values

    Returns:
        WellEventKeyword: The created keyword event object

    Raises:
        TypeError: If keyword_data contains unsupported value types

    Example:
        ```python
        # Get the timeline
        well_path = project.well_paths()[0]
        timeline = well_path.event_timeline()

        # Add WCONHIST - Historical production data
        timeline.add_well_keyword_event(
            event_date="2018-04-01",
            well_path=well_path,
            keyword_name="WCONHIST",
            keyword_data={
                "WELL": well_path.name,
                "STATUS": "OPEN",
                "CMODE": "RESV",
                "ORAT": 3999.98999,
                "WRAT": 0.01,
                "GRAT": 550678.438,
                "VFP_TABLE": 1
            }
        )

        # Add WELTARG - Change target
        timeline.add_well_keyword_event(
            event_date="2018-05-01",
            well_path=well_path,
            keyword_name="WELTARG",
            keyword_data={
                "WELL": well_path.name,
                "CMODE": "ORAT",
                "NEW_VALUE": 5000.0
            }
        )

        # Add WRFTPLT - Enable RFT output
        timeline.add_well_keyword_event(
            event_date="2018-06-01",
            well_path=well_path,
            keyword_name="WRFTPLT",
            keyword_data={
                "WELL": well_path.name,
                "OUTPUT_RFT": "YES",
                "OUTPUT_PLT": "NO"
            }
        )

        # Generate schedule
        case = project.cases()[0]
        schedule_text = timeline.generate_schedule_text(eclipse_case=case)
        print(schedule_text)
        ```
    """
    # Type inference and conversion
    item_names = []
    item_types = []
    item_values = []

    for name, value in keyword_data.items():
        item_names.append(name)

        if isinstance(value, str):
            item_types.append("STRING")
            item_values.append(value)
        elif isinstance(value, bool):
            # Handle bool before int (bool is subclass of int in Python)
            item_types.append("INT")
            item_values.append("1" if value else "0")
        elif isinstance(value, int):
            item_types.append("INT")
            item_values.append(str(value))
        elif isinstance(value, float):
            item_types.append("DOUBLE")
            item_values.append(str(value))
        else:
            raise TypeError(
                f"Unsupported type for keyword item '{name}': {type(value).__name__}. "
                "Supported types: str, int, float, bool"
            )

    # Format date
    date_str = _format_date(event_date)

    # Call internal GRPC method with parallel arrays
    return self.add_well_keyword_event_internal(
        event_date=date_str,
        well_path=well_path,
        keyword_name=keyword_name,
        item_names=item_names,
        item_types=item_types,
        item_values=item_values,
    )


@add_method(WellEventTimeline)
def add_keyword_event(
    self: WellEventTimeline,
    event_date: str | date | datetime,
    keyword_name: str,
    keyword_data: Dict[str, Any],
) -> KeywordEvent:
    """Add a schedule-level keyword event (not tied to a specific well path).

    This is for global schedule keywords like RPTRST, GRUPTREE, RPTSCHED, etc.
    that apply to the entire simulation rather than a specific well.

    Type inference rules:
    - str → STRING
    - int → INT
    - float → DOUBLE
    - bool → INT (1 or 0)

    Arguments:
        event_date: Date string in YYYY-MM-DD format, date, or datetime object
        keyword_name (str): Keyword name (e.g., "RPTRST", "GRUPTREE", "RPTSCHED")
        keyword_data (dict): Dictionary mapping keyword item names to values

    Returns:
        KeywordEvent: The created keyword event object

    Raises:
        TypeError: If keyword_data contains unsupported value types

    Example:
        ```python
        # Get the timeline
        well_path_coll = project.descendants(rips.WellPathCollection)[0]
        timeline = well_path_coll.event_timeline()

        # Add RPTRST - Report restart settings (schedule-level, not well-specific)
        timeline.add_keyword_event(
            event_date="2024-01-01",
            keyword_name="RPTRST",
            keyword_data={
                "BASIC": 2,
                "FREQ": 1,
            }
        )

        # Add GRUPTREE - Group tree definition
        timeline.add_keyword_event(
            event_date="2024-01-01",
            keyword_name="GRUPTREE",
            keyword_data={
                "CHILD": "OP",
                "PARENT": "FIELD",
            }
        )

        # Add RPTSCHED - Report schedule settings
        timeline.add_keyword_event(
            event_date="2024-01-01",
            keyword_name="RPTSCHED",
            keyword_data={
                "FIP": 1,
                "WELLS": 2,
            }
        )
        ```
    """
    # Type inference and conversion
    item_names = []
    item_types = []
    item_values = []

    for name, value in keyword_data.items():
        item_names.append(name)

        if isinstance(value, str):
            item_types.append("STRING")
            item_values.append(value)
        elif isinstance(value, bool):
            # Handle bool before int (bool is subclass of int in Python)
            item_types.append("INT")
            item_values.append("1" if value else "0")
        elif isinstance(value, int):
            item_types.append("INT")
            item_values.append(str(value))
        elif isinstance(value, float):
            item_types.append("DOUBLE")
            item_values.append(str(value))
        else:
            raise TypeError(
                f"Unsupported type for keyword item '{name}': {type(value).__name__}. "
                "Supported types: str, int, float, bool"
            )

    # Format date
    date_str = _format_date(event_date)

    # Call internal GRPC method with parallel arrays
    return self.add_keyword_event_internal(
        event_date=date_str,
        keyword_name=keyword_name,
        item_names=item_names,
        item_types=item_types,
        item_values=item_values,
    )


@add_method(WellEventTimeline)
def generate_schedule_text(self: WellEventTimeline, eclipse_case: Case) -> str:
    """Generate Eclipse schedule text for all wells in the collection.

    The timeline is shared across all wells in the well path collection.
    This method generates schedule data for all wells that have events in
    the timeline.

    This is a convenience wrapper around generate_schedule() that returns the
    text directly instead of a DataContainerString.

    Arguments:
        eclipse_case (Case): Eclipse case to use for schedule generation.

    Returns:
        str: Eclipse schedule text containing DATES, COMPDAT, WELSEGS, WCONPROD, etc.
             for all wells in the collection.

    Example:
        ```python
        # Get the timeline (shared across all wells)
        well_path = project.well_paths()[0]
        timeline = well_path.event_timeline()

        # Add events for multiple wells
        timeline.add_perf_event(
            event_date="2024-01-01",
            well_name="WELL-1",
            start_md=1000,
            end_md=1500,
            diameter=0.1,
            skin_factor=0.5,
            state="OPEN"
        )

        timeline.add_perf_event(
            event_date="2024-02-01",
            well_name="WELL-2",
            start_md=2000,
            end_md=2500,
            diameter=0.1,
            state="OPEN"
        )

        # Generate schedule text for all wells
        case = project.cases()[0]
        schedule_text = timeline.generate_schedule_text(eclipse_case=case)
        print(schedule_text)
        ```
    """
    container = self.generate_schedule(eclipse_case_id=eclipse_case.id)
    if container and container.values:
        # Workaround: Concatenate all values in case the schedule text
        # was split by comma parsing in the gRPC layer
        return "".join(container.values)
    return ""
