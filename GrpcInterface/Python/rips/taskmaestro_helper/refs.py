"""Mapping between Pydantic field annotations and ResInsight object types.

The whitelist below is the single source of truth for which `rips` types
the helper recognises as object pickers. It is referenced both by
`introspect` (to add the `resinsight_type` marker on field schemas) and
by `run` (to resolve `__resinsight_ref__` values back to live `rips`
objects).
"""

from __future__ import annotations

# Maps rips class name -> resinsight_type label sent to the C++ side.
# The label is also the C++ binding subclass selector (RimWorkflow{Label}Binding).
# Rips classes live in `rips.generated.generated_classes` but are re-exported as
# `rips.<Name>`; we match by class name only since the module is generated.
RIPS_TYPE_WHITELIST: dict[str, str] = {
    "Case": "EclipseCase",
    "EclipseCase": "EclipseCase",
    "WellPath": "WellPath",
    "View": "View",
    "EclipseView": "View",
}

_RIPS_MODULE_PREFIXES = ("rips.", "rips")

# Marker used inside input.yaml to denote a ResInsight object reference.
REF_MARKER = "__resinsight_ref__"


def annotation_to_resinsight_type(annotation: object) -> str | None:
    """Return the resinsight_type label for a Pydantic field annotation, or None.

    Handles both bare rips classes and ObjectModel[rips.X] wrappers used by
    the example workflow (e.g. `class GridCase(ObjectModel[rips.EclipseCase])`).
    """
    candidates: list[type] = []
    if isinstance(annotation, type):
        candidates.append(annotation)
        # Walk the MRO for ObjectModel[T] parameterizations: pydantic stores the
        # type argument on the intermediate class in __pydantic_generic_metadata__.
        for cls in annotation.__mro__:
            md = getattr(cls, "__pydantic_generic_metadata__", None)
            if not md:
                continue
            for arg in md.get("args", ()) or ():
                if isinstance(arg, type):
                    candidates.append(arg)

    for cls in candidates:
        module = getattr(cls, "__module__", "") or ""
        if not module.startswith(_RIPS_MODULE_PREFIXES):
            continue
        label = RIPS_TYPE_WHITELIST.get(cls.__name__)
        if label is not None:
            return label
    return None
