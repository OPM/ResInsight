"""Check that the Python examples do not use deprecated rips APIs.

Deprecated rips APIs emit ``warnings.warn(..., DeprecationWarning)``. The list of
deprecated functions and keyword arguments is extracted from the rips sources, so
new deprecations are picked up without editing this script. Several deprecated
methods share their name with a supported method on another class (for example
``View.export_property`` and ``Case.export_property``). Calls that match both
signatures are resolved using the receiver name hints in RECEIVER_HINTS.

Positional use of a deprecated keyword argument is not detected.

Add ``# rips: allow-deprecated`` to a line to suppress a finding.

Usage: python check_deprecated_api_usage.py [files or folders ...]
"""

import ast
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Callable, TypeGuard

HERE = Path(__file__).parent
RIPS = HERE / "rips"
GENERATED_CLASSES = RIPS / "generated" / "generated_classes.py"
DEFAULT_TARGETS = [RIPS / "PythonExamples"]
SUPPRESS_MARKER = "rips: allow-deprecated"

# A call that matches both a deprecated and a supported signature is
# deprecated unless the receiver expression matches the given pattern
# (case insensitive). Every ambiguous deprecated name needs an entry.
RECEIVER_HINTS: dict[str, str] = {
    "compute_statistics": r"stat",
    "export_property": r"case",
    "import_formation_names": r"project",
    "replace_source_cases": r"group",
}


def _string_literal(node: ast.expr | None) -> bool:
    return isinstance(node, ast.JoinedStr) or (
        isinstance(node, ast.Constant) and isinstance(node.value, str)
    )


def _wbs_plot_with_well_name(call: ast.Call) -> bool:
    well_path = call.args[0] if call.args else None
    for keyword in call.keywords:
        if keyword.arg == "well_path":
            well_path = keyword.value
    return _string_literal(well_path)


# Deprecations that depend on the argument values and cannot be derived from
# the function signature.
MANUAL_RULES: dict[str, Callable[[ast.Call], bool]] = {
    "create_well_bore_stability_plot": _wbs_plot_with_well_name,
}


@dataclass
class Signature:
    params: list[str]
    required: set[str]
    has_varargs: bool
    has_varkw: bool

    @staticmethod
    def from_def(func: ast.FunctionDef) -> "Signature":
        args = func.args
        positional = [a.arg for a in args.posonlyargs + args.args]
        if positional and positional[0] in ("self", "cls"):
            positional = positional[1:]
        required = set(positional[: len(positional) - len(args.defaults)])
        kwonly = [a.arg for a in args.kwonlyargs]
        required |= {
            a.arg
            for a, default in zip(args.kwonlyargs, args.kw_defaults)
            if default is None
        }
        return Signature(
            positional + kwonly,
            required,
            args.vararg is not None,
            args.kwarg is not None,
        )

    def accepts(self, call: ast.Call) -> bool:
        if any(isinstance(a, ast.Starred) for a in call.args) or any(
            k.arg is None for k in call.keywords
        ):
            return True
        if len(call.args) > len(self.params) and not self.has_varargs:
            return False
        provided = set(self.params[: len(call.args)])
        for keyword in call.keywords:
            if keyword.arg is None:
                continue
            if keyword.arg not in self.params and not self.has_varkw:
                return False
            provided.add(keyword.arg)
        return self.required <= provided


@dataclass
class Deprecation:
    name: str
    message: str
    location: str
    signature: Signature
    keyword: str | None = None
    whole_function: bool = False
    manual: bool = False


@dataclass
class Registry:
    deprecations: list[Deprecation] = field(default_factory=list)
    supported: dict[str, list[Signature]] = field(default_factory=dict)


def _is_deprecation_warn(node: ast.AST) -> TypeGuard[ast.Call]:
    return (
        isinstance(node, ast.Call)
        and ast.unparse(node.func) in ("warnings.warn", "warn")
        and any(
            ast.unparse(a) == "DeprecationWarning"
            for a in node.args[1:] + [k.value for k in node.keywords]
        )
    )


def _warn_message(call: ast.Call) -> str:
    message = call.args[0] if call.args else None
    if isinstance(message, ast.Constant) and isinstance(message.value, str):
        return message.value
    return ast.unparse(message) if message is not None else ""


def _find_warnings(
    body: list[ast.stmt], guard: ast.expr | None = None
) -> list[tuple[ast.Call, ast.expr | None]]:
    """Return deprecation warnings in body with the innermost enclosing if-test."""
    found = []
    for stmt in body:
        if isinstance(stmt, ast.If):
            found += _find_warnings(stmt.body, stmt.test)
            found += _find_warnings(stmt.orelse, guard)
        elif isinstance(stmt, (ast.FunctionDef, ast.AsyncFunctionDef, ast.ClassDef)):
            continue
        else:
            for node in ast.walk(stmt):
                if _is_deprecation_warn(node):
                    found.append((node, guard))
    return found


def _guarded_parameter(test: ast.expr, signature: Signature) -> str | None:
    """Parameter name if test is a plain check of one parameter's value."""
    if any(isinstance(n, ast.Call) for n in ast.walk(test)):
        return None
    names = {
        n.id
        for n in ast.walk(test)
        if isinstance(n, ast.Name) and n.id in signature.params
    }
    return names.pop() if len(names) == 1 else None


def build_registry() -> Registry:
    if not GENERATED_CLASSES.exists():
        sys.exit(
            f"{GENERATED_CLASSES} is missing. It is generated when building ResInsight "
            "with the Python gRPC interface enabled."
        )

    registry = Registry()
    sources = sorted(RIPS.glob("*.py")) + [GENERATED_CLASSES]
    for path in sources:
        tree = ast.parse(path.read_text(encoding="utf-8"), filename=str(path))
        for func in ast.walk(tree):
            if not isinstance(func, ast.FunctionDef):
                continue
            signature = Signature.from_def(func)
            location = f"{path.relative_to(HERE).as_posix()}:{func.lineno}"
            warnings_found = _find_warnings(func.body)
            whole_function = False
            for warn, guard in warnings_found:
                deprecation = Deprecation(
                    func.name, _warn_message(warn), location, signature
                )
                if guard is None:
                    deprecation.whole_function = whole_function = True
                elif keyword := _guarded_parameter(guard, signature):
                    deprecation.keyword = keyword
                elif func.name in MANUAL_RULES:
                    deprecation.manual = True
                else:
                    sys.exit(
                        f"{location}: cannot classify the DeprecationWarning in "
                        f"{func.name}(). Add a rule to MANUAL_RULES in {Path(__file__).name}."
                    )
                registry.deprecations.append(deprecation)
            if not whole_function:
                registry.supported.setdefault(func.name, []).append(signature)
    return registry


def validate_registry(registry: Registry) -> None:
    """Make sure every deprecation can be detected without guessing."""
    errors = []
    whole = {d.name for d in registry.deprecations if d.whole_function}
    manual = {d.name for d in registry.deprecations if d.manual}

    for deprecation in registry.deprecations:
        if deprecation.keyword:
            clashes = [
                s
                for s in registry.supported.get(deprecation.name, [])
                if s is not deprecation.signature and deprecation.keyword in s.params
            ]
            if clashes:
                errors.append(
                    f"{deprecation.location}: keyword '{deprecation.keyword}' of "
                    f"{deprecation.name}() is also a supported keyword of another method "
                    "with the same name"
                )

    for name in sorted(whole):
        if registry.supported.get(name) and name not in RECEIVER_HINTS:
            errors.append(
                f"{name}() is deprecated, but a supported method has the same name. "
                "Add a receiver pattern to RECEIVER_HINTS."
            )

    for name in sorted(set(RECEIVER_HINTS) - whole):
        errors.append(f"RECEIVER_HINTS entry '{name}' is not a deprecated method")
    for name in sorted(set(MANUAL_RULES) - manual):
        errors.append(f"MANUAL_RULES entry '{name}' has no matching DeprecationWarning")

    if errors:
        sys.exit("\n".join(errors))


def _receiver_name(node: ast.expr) -> str:
    while True:
        if isinstance(node, ast.Name):
            return node.id
        if isinstance(node, ast.Attribute):
            return node.attr
        if isinstance(node, ast.Call):
            node = node.func
        elif isinstance(node, ast.Subscript):
            node = node.value
        else:
            return ""


def _is_deprecated_call(
    call: ast.Call, deprecation: Deprecation, registry: Registry
) -> bool:
    if deprecation.keyword:
        return any(k.arg == deprecation.keyword for k in call.keywords)
    if deprecation.manual:
        return MANUAL_RULES[deprecation.name](call)

    if not deprecation.signature.accepts(call):
        return False
    supported = registry.supported.get(deprecation.name, [])
    if not any(s.accepts(call) for s in supported):
        return True
    assert isinstance(call.func, ast.Attribute)
    receiver = _receiver_name(call.func.value)
    return not re.search(RECEIVER_HINTS[deprecation.name], receiver, re.IGNORECASE)


def check_file(path: Path, registry: Registry) -> list[str]:
    source = path.read_text(encoding="utf-8")
    lines = source.splitlines()
    findings = []
    by_name: dict[str, list[Deprecation]] = {}
    for deprecation in registry.deprecations:
        by_name.setdefault(deprecation.name, []).append(deprecation)

    for node in ast.walk(ast.parse(source, filename=str(path))):
        if not (isinstance(node, ast.Call) and isinstance(node.func, ast.Attribute)):
            continue
        if SUPPRESS_MARKER in lines[node.lineno - 1]:
            continue
        for deprecation in by_name.get(node.func.attr, []):
            if _is_deprecated_call(node, deprecation, registry):
                findings.append(f"{path}:{node.lineno}: {deprecation.message}")
    return findings


def main() -> None:
    targets = [Path(a) for a in sys.argv[1:]] or DEFAULT_TARGETS
    files = sorted(
        f for t in targets for f in ([t] if t.is_file() else t.rglob("*.py"))
    )
    if not files:
        sys.exit("no Python files found, nothing was checked")

    registry = build_registry()
    validate_registry(registry)

    findings = [f for path in files for f in check_file(path, registry)]
    if findings:
        sys.exit("Deprecated rips API usage found:\n" + "\n".join(findings))

    print(
        f"{len(files)} files checked against {len(registry.deprecations)} deprecated "
        "rips APIs: OK"
    )


if __name__ == "__main__":
    main()
