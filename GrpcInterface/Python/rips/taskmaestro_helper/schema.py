"""JSON schemas for task input/output types, as consumed by ResInsight.

taskmaestro's ``TaskSchemaGenerator`` marks opaque Python objects as
``{"not": {}, "x-taskmaestro-opaque": true, ...}``. This module adds
``x-ri-python-type``/``x-ri-python-bases`` to models and opaque types. The bases
list the class MRO so ResInsight can check port compatibility the same way
taskmaestro does (``issubclass``) without importing anything.
"""

from __future__ import annotations

import inspect
from typing import Any

from pydantic import BaseModel
from taskmaestro.introspection import TaskSchemaGenerator, python_type_name
from taskmaestro.introspection import model_json_schema as tm_model_json_schema
from taskmaestro.introspection import task_descriptor as tm_task_descriptor

_IGNORED_BASES = {object, BaseModel}


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


class ResInsightSchemaGenerator(TaskSchemaGenerator):
    """Schema generator that records Python class bases for port compatibility."""

    @classmethod
    def type_annotations(cls, python_type: type[Any]) -> dict[str, Any]:
        return _type_annotations(python_type)


def model_json_schema(model: type[Any]) -> dict[str, Any]:
    return tm_model_json_schema(model, ResInsightSchemaGenerator)


def task_descriptor(task_id: str, cls: type[Any]) -> dict[str, Any]:
    """Describe a task class for the ResInsight task palette and port typing."""
    return tm_task_descriptor(task_id, cls, ResInsightSchemaGenerator)
