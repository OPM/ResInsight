"""Small taskmaestro tasks used by the taskmaestro helper tests."""

from __future__ import annotations

from pydantic import BaseModel
from taskmaestro import EmptyConfig, ExecutionContext, Task, Workflow


class Number(BaseModel):
    """A single number."""

    value: float


class StartInput(BaseModel):
    """Configured start value."""

    value: float


class SpecialNumber(Number):
    """A number subclass, compatible with Number."""


class Start(Task[StartInput, Number]):
    """Produce the configured value."""

    name = "start"

    def run(self, input: StartInput, ctx: ExecutionContext) -> Number:
        return Number(value=input.value)


class Constant(Task[EmptyConfig, Number]):
    """Produce the number one."""

    name = "constant"

    def run(self, input: EmptyConfig, ctx: ExecutionContext) -> Number:
        return Number(value=1.0)


class Double(Task[Number, Number]):
    """Double a number."""

    name = "double"

    def run(self, input: Number, ctx: ExecutionContext) -> Number:
        return Number(value=2 * input.value)


class Special(Task[Number, SpecialNumber]):
    """Produce a SpecialNumber."""

    name = "special"

    def run(self, input: Number, ctx: ExecutionContext) -> SpecialNumber:
        return SpecialNumber(value=input.value)


registered_workflow = (
    Workflow.builder("registered_flow")
    .add_task(Start, config_fields=["value"])
    .add_task(Double, depends_on=Start)
    .build()
)


def _local_workflow() -> Workflow:
    class LocalTask(Task[EmptyConfig, Number]):
        name = "local"

        def run(self, input: EmptyConfig, ctx: ExecutionContext) -> Number:
            return Number(value=0.0)

    return Workflow.builder("local_flow").add_task(LocalTask).build()


local_workflow = _local_workflow()
