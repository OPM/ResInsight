"""
Category name/color binding for discrete (INTEGER) grid cell results.

Provides a convenience API on Case for labeling integer result values with
human-readable names and optional colors. Internally, this reuses the existing
ColorLegend infrastructure: a new custom ColorLegend is created and registered
as the default legend for the (case, resultName) pair. When the property is
shown in a 3D view, the legend's item names are used as the category labels.
"""

from typing import Dict, List, Optional

from .pdmobject import add_method
from .project import Project
from .resinsight_classes import Case, ColorLegend, ColorLegendItem


# Palette of distinct colors used when the caller does not supply colors.
# Mirrors RiaColorTables::categoryColors() in the C++ code.
_DEFAULT_PALETTE: List[str] = [
    "#803e75",
    "#d41c84",
    "#f6768e",
    "#c10020",
    "#7f180d",
    "#f13a13",
    "#ff7a5c",
    "#817066",
    "#ff6800",
    "#593315",
    "#ff8e00",
    "#cea262",
    "#f4c800",
    "#93aa00",
    "#3b5417",
    "#007d34",
    "#367d7b",
    "#00538a",
    "#a6bdd7",
    "#2e4ce0",
]


@add_method(Case)
def set_discrete_property_category_names(
    self: Case,
    property_name: str,
    value_names: Dict[int, str],
    value_colors: Optional[Dict[int, str]] = None,
    legend_name: Optional[str] = None,
) -> Optional[ColorLegend]:
    """Bind integer values of a discrete grid property to text labels.

    Use this after uploading a discrete property via set_grid_property(...,
    data_type="INTEGER") or set_active_cell_property(..., data_type="INTEGER")
    to display text labels instead of raw integers in the 3D view legend.

    Arguments:
        property_name (str): Name of the discrete property result.
        value_names (Dict[int, str]): Mapping from integer value to label.
            An empty dict removes any existing mapping for this property.
        value_colors (Optional[Dict[int, str]]): Optional per-value colors as
            strings accepted by QColor (e.g. "red", "#ff8800"). Values without
            a color entry get an auto-assigned palette color.
        legend_name (Optional[str]): Optional name for the color legend.
            Defaults to the property name.

    Returns:
        The created ColorLegend, or None if value_names was empty.
    """
    project = self.ancestor(Project)
    if project is None:
        raise RuntimeError("Could not find parent project")

    collection = project.color_legend_collection()
    if collection is None:
        raise RuntimeError("Could not find ColorLegendCollection in project")

    collection.delete_color_legend(case=self, result_name=property_name)

    if not value_names:
        return None

    name = legend_name if legend_name else property_name
    legend = collection.create_color_legend(name=name)

    colors = value_colors or {}
    palette_index = 0
    for value, label in value_names.items():
        color = colors.get(value)
        if color is None:
            color = _DEFAULT_PALETTE[palette_index % len(_DEFAULT_PALETTE)]
            palette_index += 1
        legend.add_color_legend_item(
            category_value=value,
            category_name=label,
            color=color,
        )

    collection.set_default_color_legend_for_result(
        case=self,
        result_name=property_name,
        color_legend=legend,
    )

    return legend


def _find_default_legend(case: Case, property_name: str) -> Optional[ColorLegend]:
    """Look up the color legend bound to (case, property_name).

    Returns None when no mapping has been registered.
    """
    project = case.ancestor(Project)
    if project is None:
        raise RuntimeError("Could not find parent project")

    collection = project.color_legend_collection()
    if collection is None:
        raise RuntimeError("Could not find ColorLegendCollection in project")

    return collection.find_default_legend_for_result(
        case=case,
        result_name=property_name,
    )


@add_method(Case)
def discrete_property_category_names(
    self: Case,
    property_name: str,
) -> Dict[int, str]:
    """Return the integer-value to label mapping for a discrete property.

    Inverse of set_discrete_property_category_names. Returns an empty
    dict when no mapping is registered for this property.

    Arguments:
        property_name (str): Name of the discrete property result.

    Returns:
        Dict mapping each integer category value to its label string.
    """
    legend = _find_default_legend(self, property_name)
    if legend is None:
        return {}

    return {
        item.category_value: item.category_name
        for item in legend.children("ColorLegendItems", ColorLegendItem)
    }


@add_method(Case)
def discrete_property_category_colors(
    self: Case,
    property_name: str,
) -> Dict[int, str]:
    """Return the integer-value to color mapping for a discrete property.

    Inverse of the value_colors argument to
    set_discrete_property_category_names. Colors are returned as hex
    strings (e.g. "#ff8800") in the same form accepted by the setter.
    Returns an empty dict when no mapping is registered.

    Arguments:
        property_name (str): Name of the discrete property result.

    Returns:
        Dict mapping each integer category value to its hex color.
    """
    legend = _find_default_legend(self, property_name)
    if legend is None:
        return {}

    return {
        item.category_value: item.color
        for item in legend.children("ColorLegendItems", ColorLegendItem)
    }
