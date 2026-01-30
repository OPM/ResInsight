"""
Well Events module for managing time-dependent well events.

This module provides functionality for creating and managing well events
in a timeline-based event system. Events can be perforation events, valve events,
tubing changes, well state changes, and production/injection control changes.
"""

from typing import Dict
from datetime import date, datetime

from .pdmobject import add_method
from .resinsight_classes import Case
from .generated.generated_classes import (
    WellEventKeyword,
    WellEventTimeline,
)


def _format_date(event_date) -> str:
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


def parse_well_events_config(config_path: str) -> Dict[str, Dict[str, dict]]:
    """Parse a YAML configuration file with well events.

    The YAML format is organized by well name, then by date:

    ```yaml
    A-1H:
      2024-01-01:
        PERF:
          - START_MD: 1000
            END_MD: 1500
            DIAMETER: 0.1
            STATE: OPEN
        WSTATE: OPEN
        WCONTROL:
          MODE: ORAT
          VALUE: 1000
      2024-06-01:
        PERF:
          - START_MD: 1500
            END_MD: 2000
            STATE: OPEN
    ```

    Arguments:
        config_path (str): Path to YAML configuration file.

    Returns:
        Dict mapping well names to dicts mapping dates to event data.
    """
    try:
        import yaml
    except ImportError:
        raise ImportError(
            "PyYAML is required for parsing config files. Install with: pip install pyyaml"
        )

    with open(config_path, "r") as f:
        config = yaml.safe_load(f)

    return config


def load_events_from_config(
    project,
    config_path: str,
) -> None:
    """Load well events from a YAML configuration file.

    Arguments:
        project: ResInsight project object.
        config_path (str): Path to YAML configuration file.
    """
    config = parse_well_events_config(config_path)

    for well_name, well_events in config.items():
        well = project.well_path_by_name(well_name)
        if well is None:
            print(f"Warning: Well '{well_name}' not found in project, skipping")
            continue

        timeline = well.event_timeline()
        if timeline is None:
            print(f"Warning: Could not get timeline for well '{well_name}', skipping")
            continue

        for event_date, events in well_events.items():
            date_str = str(event_date)

            # Process PERF events
            if "PERF" in events:
                perf_events = events["PERF"]
                if isinstance(perf_events, list):
                    for perf in perf_events:
                        timeline.add_perf_event(
                            event_date=date_str,
                            well_path=well,
                            start_md=perf.get("START_MD", 0),
                            end_md=perf.get("END_MD", 0),
                            diameter=perf.get("DIAMETER", 0.216),
                            skin_factor=perf.get("SKIN_FACTOR", 0.0),
                            state=perf.get("STATE", "OPEN"),
                        )

            # Process VALVE events
            if "VALVE" in events:
                valve_events = events["VALVE"]
                if isinstance(valve_events, list):
                    for valve in valve_events:
                        timeline.add_valve_event(
                            event_date=date_str,
                            well_path=well,
                            measured_depth=valve.get("MD", 0),
                            valve_type=valve.get("TYPE", "ICV"),
                            state=valve.get("STATE", "OPEN"),
                            flow_coefficient=valve.get("FLOW_COEFFICIENT", 0.7),
                            area=valve.get("AREA", 0.0001),
                        )

            # Process WSTATE event
            if "WSTATE" in events:
                state = events["WSTATE"]
                if isinstance(state, str):
                    timeline.add_state_event(
                        event_date=date_str,
                        well_path=well,
                        well_state=state,
                    )

            # Process WCONTROL event
            if "WCONTROL" in events:
                ctrl = events["WCONTROL"]
                if isinstance(ctrl, dict):
                    timeline.add_control_event(
                        event_date=date_str,
                        well_path=well,
                        control_mode=ctrl.get("MODE", "ORAT"),
                        control_value=ctrl.get("VALUE", 0.0),
                        bhp_limit=ctrl.get("BHP_LIMIT", 0.0),
                        oil_rate=ctrl.get("OIL_RATE", 0.0),
                        water_rate=ctrl.get("WATER_RATE", 0.0),
                        gas_rate=ctrl.get("GAS_RATE", 0.0),
                        is_producer=ctrl.get("IS_PRODUCER", True),
                    )

            # Process TUBING events
            if "TUBING" in events:
                tubing_events = events["TUBING"]
                if isinstance(tubing_events, list):
                    for tubing in tubing_events:
                        timeline.add_tubing_event(
                            event_date=date_str,
                            well_path=well,
                            start_md=tubing.get("START_MD", 0),
                            end_md=tubing.get("END_MD", 0),
                            inner_diameter=tubing.get("INNER_DIAMETER", 0.15),
                            roughness=tubing.get("ROUGHNESS", 1.0e-5),
                        )


# Store the original GRPC add_keyword_event method before wrapping
_original_add_keyword_event = WellEventTimeline.add_keyword_event


@add_method(WellEventTimeline)
def add_keyword_event_with_dict(
    self,
    event_date,
    well_path,
    keyword_name: str,
    keyword_data: dict,
) -> "WellEventKeyword":
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
        timeline.add_keyword_event(
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
        timeline.add_keyword_event(
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
        timeline.add_keyword_event(
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

    # Call original GRPC method with parallel arrays
    return _original_add_keyword_event(
        self,
        event_date=date_str,
        well_path=well_path,
        keyword_name=keyword_name,
        item_names=item_names,
        item_types=item_types,
        item_values=item_values,
    )


# Replace the add_keyword_event method with our enhanced version
WellEventTimeline.add_keyword_event = add_keyword_event_with_dict


@add_method(WellEventTimeline)
def generate_schedule_text(self, eclipse_case: Case) -> str:
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
