"""JSON schemas for task input/output types, as consumed by ResInsight.

The generator mirrors taskmaestro's own CLI schema generator (opaque Python
objects become ``{"not": {}, "x-taskmaestro-opaque": true, ...}``) and adds
``x-ri-python-type``/``x-ri-python-bases`` to models and opaque types. The bases
list the class MRO so ResInsight can check port compatibility the same way
taskmaestro does (``issubclass``) without importing anything.
"""

from __future__ import annotations

import inspect
from typing import Any

from pydantic import BaseModel
from pydantic.json_schema import GenerateJsonSchema, JsonSchemaValue
from pydantic_core import core_schema

_IGNORED_BASES = {object, BaseModel}


def python_type_name(cls: type[Any]) -> str:
    return f"{cls.__module__}.{cls.__qualname__}"


def python_bases(cls: type[Any]) -> list[str]:
    """Return the dotted names of ``cls`` and its bases, most derived first."""
    names: list[str] = []
    for base in inspect.getmro(cls):
        if base in _IGNORED_BASES or base.__module__ == "typing":
            continue
        name = python_type_name(base)
        if name not in names:
            names.append(name)
    return names


def _type_annotations(cls: type[Any]) -> dict[str, Any]:
    return {
        "x-ri-python-type": python_type_name(cls),
        "x-ri-python-bases": python_bases(cls),
    }


class ResInsightSchemaGenerator(GenerateJsonSchema):
    """Schema generator that marks opaque types and records Python class bases."""

    def is_instance_schema(
        self, schema: core_schema.IsInstanceSchema
    ) -> JsonSchemaValue:
        cls = schema["cls"]
        return {
            "not": {},
            "x-taskmaestro-opaque": True,
            "x-taskmaestro-python-type": python_type_name(cls),
            **_type_annotations(cls),
        }

    def model_schema(self, schema: core_schema.ModelSchema) -> JsonSchemaValue:
        json_schema = super().model_schema(schema)
        json_schema.update(_type_annotations(schema["cls"]))
        return json_schema


def model_json_schema(model: type[Any]) -> dict[str, Any]:
    if isinstance(model, type) and issubclass(model, BaseModel):
        return model.model_json_schema(schema_generator=ResInsightSchemaGenerator)
    from pydantic import TypeAdapter

    return TypeAdapter(model).json_schema(schema_generator=ResInsightSchemaGenerator)


def task_descriptor(task_id: str, cls: type[Any]) -> dict[str, Any]:
    """Describe a task class for the ResInsight task palette and port typing."""
    from taskmaestro.task import get_input_type, get_output_type

    return {
        "id": task_id,
        "name": getattr(cls, "name", cls.__name__),
        "python_type": python_type_name(cls),
        "description": inspect.getdoc(cls) or "",
        "timeout_seconds": getattr(cls, "timeout_seconds", None),
        "input_schema": model_json_schema(get_input_type(cls)),
        "output_schema": model_json_schema(get_output_type(cls)),
    }
