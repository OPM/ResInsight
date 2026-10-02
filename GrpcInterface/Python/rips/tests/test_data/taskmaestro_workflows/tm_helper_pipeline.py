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


class Wrapped(BaseModel):
    """A number wrapped in another model."""

    inner: Number


class Pair(BaseModel):
    """Two numbers to add."""

    first: float
    second: float


class Values(BaseModel):
    """Collected numbers."""

    values: list[float]


class Item(BaseModel):
    """One mapped item."""

    key: str
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


class Wrap(Task[Number, Wrapped]):
    """Wrap a number."""

    name = "wrap"

    def run(self, input: Number, ctx: ExecutionContext) -> Wrapped:
        return Wrapped(inner=input)


class Add(Task[Pair, Number]):
    """Add two numbers."""

    name = "add"

    def run(self, input: Pair, ctx: ExecutionContext) -> Number:
        return Number(value=input.first + input.second)


class Sum(Task[Values, Number]):
    """Sum collected numbers."""

    name = "sum"

    def run(self, input: Values, ctx: ExecutionContext) -> Number:
        return Number(value=sum(input.values))


class PerItem(Task[Item, Number]):
    """Return the value of a mapped item."""

    name = "per_item"

    def run(self, input: Item, ctx: ExecutionContext) -> Number:
        return Number(value=input.value)


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
