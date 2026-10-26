"""Conditional-branch tasks used by the taskmaestro helper tests.

Kept apart from ``tm_helper_pipeline`` because they need a taskmaestro with
``taskmaestro.conditions``.
"""

from __future__ import annotations

from typing import Literal

from pydantic import BaseModel
from taskmaestro import ConditionTask, ExecutionContext, SwitchTask, Task
from tm_helper_pipeline import Number


class ChooseSize(SwitchTask[Number, Literal["small", "large", "huge"]]):
    """Pick a size label from a number."""

    name = "choose_size"

    def decide(
        self, input: Number, ctx: ExecutionContext
    ) -> Literal["small", "large", "huge"]:
        if input.value > 100:
            return "huge"
        return "large" if input.value > 10 else "small"


class IsPositive(ConditionTask[Number]):
    """True when the number is positive."""

    name = "is_positive"

    def decide(self, input: Number, ctx: ExecutionContext) -> bool:
        return input.value > 0


class ReportInput(BaseModel):
    """Two numbers, each taken from whichever branch ran."""

    sized: Number
    signed: Number


class Report(Task[ReportInput, Number]):
    """Add the two numbers."""

    name = "report"

    def run(self, input: ReportInput, ctx: ExecutionContext) -> Number:
        return Number(value=input.sized.value + input.signed.value)
