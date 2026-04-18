"""
Category name/color binding for discrete (INTEGER) grid cell results.

Provides a convenience API on Case for labeling integer result values with
human-readable names and optional colors. Internally, this reuses the existing
ColorLegend infrastructure: a new custom ColorLegend is created and registered
as the default legend for the (caseId, resultName) pair. When the property is
shown in a 3D view, the legend's item names are used as the category labels.
"""

from typing import Dict, List, Optional, Tuple

from .pdmobject import add_method
from .project import Project
from .resinsight_classes import Case, ColorLegend
from .generated.generated_classes import ColorLegendCollection  # noqa: F401


# Palette of distinct RGB colors used when the caller does not supply colors.
# Mirrors RiaColorTables::categoryColors() in the C++ code.
_DEFAULT_PALETTE: List[Tuple[int, int, int]] = [
    (128, 62, 117),
    (212, 28, 132),
    (246, 118, 142),
    (193, 0, 32),
    (127, 24, 13),
    (241, 58, 19),
    (255, 122, 92),
    (129, 112, 102),
    (255, 104, 0),
    (89, 51, 21),
    (255, 142, 0),
    (206, 162, 98),
    (244, 200, 0),
    (147, 170, 0),
    (59, 84, 23),
    (0, 125, 52),
    (54, 125, 123),
    (0, 83, 138),
    (166, 189, 215),
    (46, 76, 224),
]


def _color_legend_collection(project: Project) -> ColorLegendCollection:
    collections = project.descendants(ColorLegendCollection)
    if not collections:
        raise RuntimeError("Could not find ColorLegendCollection in project")
    return collections[0]


@add_method(Case)
def set_discrete_property_category_names(
    self: Case,
    property_name: str,
    value_names: Dict[int, str],
    value_colors: Optional[Dict[int, Tuple[int, int, int]]] = None,
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
        value_colors (Optional[Dict[int, Tuple[int, int, int]]]): Optional
            per-value RGB colors (each component 0-255). Values without a
            color entry get an auto-assigned palette color.
        legend_name (Optional[str]): Optional name for the color legend.
            Defaults to the property name.

    Returns:
        The created ColorLegend, or None if value_names was empty.
    """
    project = self.ancestor(Project)
    if project is None:
        raise RuntimeError("Could not find parent project")

    collection = _color_legend_collection(project)

    collection.delete_color_legend(case_id=self.id, result_name=property_name)

    if not value_names:
        return None

    name = legend_name if legend_name else property_name
    legend = collection.create_color_legend(name=name)

    colors = value_colors or {}
    palette_index = 0
    for value, label in value_names.items():
        rgb = colors.get(value)
        if rgb is None:
            rgb = _DEFAULT_PALETTE[palette_index % len(_DEFAULT_PALETTE)]
            palette_index += 1
        r, g, b = rgb
        legend.add_color_legend_item(
            category_value=value,
            category_name=label,
            red=int(r),
            green=int(g),
            blue=int(b),
        )

    collection.set_default_color_legend_for_result(
        case_id=self.id,
        result_name=property_name,
        color_legend=legend,
    )

    return legend
