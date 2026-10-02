# Workflow UI design

## What this branch adds

A new **Workflows** node in the project tree, next to **Scripts**. It lists
[taskmaestro](https://github.com/OPM/taskmaestro) workflows found on disk and
the workflows that installed packages register. For each one it
auto-generates a property editor for the config inputs, and it runs the
workflow as a subprocess that talks back to ResInsight over the existing gRPC
channel.

Workflows can also be **created and edited** in ResInsight. A graph editor
with a task palette builds the workflow from registered tasks and saves it as
a standard `workflow.yaml` + `input.yaml`.

The task classes are the source of truth. ResInsight reads the Pydantic input
and output models of each task, which are exported as JSON schemas, to build
the property editors and to check which ports can be connected. There is no
manifest file or plugin descriptor.

## Architecture at a glance

```
ResInsight (C++)                              Python helper (rips.taskmaestro_helper)
─────────────────                             ───────────────────────────────────────
RimWorkflowTaskCatalog ────── catalog ─────►  entry points → task descriptors + registered workflows
RimWorkflowCollection
  folders in ~/.taskmaestro/workflows ─ load ─►  workflow.yaml + input.yaml → definition JSON
  Installed (RimWorkflowInstalledCollection)    (from the catalog)
RimWorkflow
  RimWorkflowDefinition (value struct)
  edits → applyEdit → undo/redo, dirty
  Save ──────────────── save (stdin JSON) ──►  definition JSON → workflow.yaml + input.yaml
  RiuWorkflowValidationRunner ─ save --describe ► temp folder + taskmaestro describe
  RimWorkflowJob (one or more)
    RimWorkflowTaskInput (one per task)
      RimWorkflow<Type>Binding (one per config field, see table below)
    Run ─ writes input.yaml ─ QProcess ─────►  run <dir> | run --registered <id>
                                       ◄─────  JSON events → task states in the graph
```

The C++ side never parses YAML and doesn't import Pydantic. It only handles
JSON (Qt's `QJsonDocument`); `input.yaml` for a run is written as JSON text,
which YAML accepts. The Python helper does everything that touches YAML or
the upstream library. Every helper call goes through
`RimWorkflowHelperProcess::runSync` (or the async validation runner), which
finds the Python interpreter and decodes the single JSON document on stdout.

## The Python helper

`python -m rips.taskmaestro_helper <subcommand>`
([__main__.py](../GrpcInterface/Python/rips/taskmaestro_helper/__main__.py)).
Every subcommand except `run` prints exactly one JSON document. Output
written while task modules are imported goes to stderr. On failure the
helper exits with code 2 and prints
`{"status": "invalid", "error": {"message", "task", "field"}}`.
taskmaestro 0.3.0 or newer is required (`_compat.py`).

| Subcommand | Purpose |
|---|---|
| `catalog [--no-workflows]` | All tasks from the `taskmaestro.tasks` entry points, as descriptors (`id`, `name`, `python_type`, `description`, `input_schema`, `output_schema`), and all workflows from `taskmaestro.workflows`. A broken plugin is reported in `errors`; the rest still load. |
| `load <dir> [--describe]` | Reads `workflow.yaml` + `input.yaml` into a definition JSON (below). |
| `save --out-dir D [--workflow-only] [--describe] [--backup]` | Reads a definition JSON on stdin and writes the YAML files atomically. `--describe` also runs `taskmaestro workflow describe` on the result, which is how validation works. |
| `export-registered <id> [--describe]` | A registered workflow as a definition JSON. |
| `run <dir>` / `run --registered <id>` | Runs a workflow (see [The run flow](#the-run-flow)). |

`schema.py` generates the task schemas. It adds `x-ri-python-type` and
`x-ri-python-bases` (the class MRO) to models and opaque types, so that C++
can check subclass relations without Python.

## The definition model

`RimWorkflowDefinition`
([RimWorkflowDefinition.h](../ApplicationLibCode/ProjectDataModel/Workflow/RimWorkflowDefinition.h))
is a plain value struct that mirrors the helper's definition JSON:

```json
{ "format": 1, "name": "resinsight_completions", "result_task": null,
  "editable": true, "readonly_reasons": [], "warnings": [],
  "header_comment": "# ...", "passthrough": {"runner": {}},
  "nodes": [{"name": "add_perf_1", "task": "resinsight.add_perforation",
             "config_fields": ["event_date", "start_md", "end_md"]}],
  "edges": [{"from": "connect_to_resinsight", "output": null,
             "to": "add_perf_1", "input": "resinsight"}],
  "inputs": {"add_perf_1": {"start_md": 3000.0}},
  "task_types": {"resinsight.add_perforation": {"...": "descriptor"}},
  "describe": {}, "describe_error": null }
```

- A `null` `output`/`input` on an edge means the whole model. `[task, field]`
  routing and fan-in (`{field: task}`) are both expressed as edges.
- `task_types` embeds the descriptor of every task used, so a workflow can
  still be edited when the catalog fails to load.
- `save` always writes `name:` on every entry and refers to tasks by instance
  name, because taskmaestro cannot resolve default names in `depends_on`.
  Linear (implicit) workflows are converted to explicit edges on load.
- The definition is never written to the `.rsp` file. Undo and redo store
  value copies (`RimWorkflow`, capped at 100 steps).

All edits are pure functions in `RimWorkflowDefinitionTools` that return
`std::expected<RimWorkflowDefinition, QString>`: `addTask`, `removeTask`,
`connect`, `disconnect`, `renameTask`, `renameWorkflow`, `setResultTask`
and `setOptionalInputConfigured`. After each edit, `normalize` re-derives the
config fields: every required input field that no edge covers becomes a
config field, as do optional fields the user has turned on.

`addTask` gives the node a unique instance name. It also auto-wires inputs of
type `RipsInstance` to a `resinsight.connect` node, which is created the first
time it is needed.

### Read-only workflows

A workflow opens read-only, showing the reasons in the editor's status line,
when:

- it is **installed** (registered by a package). Use *Duplicate as Editable*
  to get an editable copy.
- it uses features the editor does not support: `collect`, `map`, nested
  `workflow:` entries, or task classes that cannot be imported.

Read-only workflows can still be run.

## Port typing

`RimWorkflowPortCompatibility` lists the input and output ports of a task from
its schemas. The whole model is a port with an empty name, and the opaque
`value` field of an `ObjectModel` is hidden. `isCompatible(produced, expected)`
checks the following, in order:

1. `Any` accepts everything.
2. `anyOf`: every produced option must match; at least one expected option
   must match.
3. Opaque types: the expected Python type must be among the produced type's
   bases, so `PerforationOutput` does not connect to `WellPath` even though
   both wrap `rips.WellPath`.
4. Models: compare `x-ri-python-type` against the produced bases.
5. Arrays: compare the items.
6. Scalars: `type` and `format` must be equal. bool → int is allowed;
   int → float is not. Enums must be a subset.

`canConnect` also rejects self-loops, cycles, and a whole-input edge on a task
that already has field edges (and the reverse). The error text is shown as a
tooltip while dragging.

## Workflow graph and editor

Selecting a `RimWorkflow` (or one of its jobs) shows `RiuWorkflowEditorWidget`
in a central dock tab:

- the graph (`RiuWorkflowGraphView`);
- the task palette (`RiuWorkflowTaskPalette`) to the right of the graph: a
  filter field above a tree grouped by id prefix. Drag a task onto the
  graph, or double-click it;
- a status line for load errors, read-only reasons, validation results and
  unsaved changes.

Each port shows its name with its type below it in smaller text (from the
task schemas, as `input_types`/`output_types` in the graph JSON). The
tooltip also shows the description.

`RiuWorkflowGraphLayout` computes the layout: topological ranks with a Kahn
sort and longest-path ranks, then one barycenter pass to order nodes. It
tolerates cycles. The layout is always automatic; nodes moved by hand keep
their position only for the session. After an edit the viewport is kept.

In an editable workflow:

- Drag from an output port to an input port to connect. Valid targets turn
  green, invalid ones grey with the reason as a tooltip. Escape cancels.
- Unwired optional inputs are drawn as hollow ports.
- Right-click a task for Rename (F2), Set as Result Task, Optional Inputs ▸
  and Delete (Del). Right-click a connection for Delete Connection.
  Right-click the background for Add Task ▸, Undo, Redo, Save, Save As,
  Duplicate as Editable and Fit. There is no toolbar.
- Ctrl+Z/Ctrl+Y undo and redo; Ctrl+S saves.

Selecting a task shows its `RimWorkflowTaskInput` from the displayed job in
the property view. Editing is locked while a job of the workflow is running.
During a run, task states are drawn on the nodes.

## Validation

Two layers of validation run:

- **Static**, in C++ (`RimWorkflowDefinitionTools::validate`): unknown tasks,
  invalid names, required opaque inputs that are not wired, multiple sinks
  without a result task, and so on. The results are shown as badges on the
  nodes right away.
- **Live**, through taskmaestro: `RiuWorkflowValidationRunner` waits 400 ms
  after the last edit, then runs `save --out-dir <tmp> --describe` in the
  background. A new edit kills a stale run, results for an old revision are
  dropped, and a 30 s watchdog stops hung processes.
  `RimWorkflowValidationTools::issuesFromHelperResult` maps the errors to
  tasks; anything left over goes to the status line.

Running a workflow whose last validation failed asks for confirmation first.

## Saving

- **Save** writes `workflow.yaml` and `input.yaml` in the workflow folder.
  The first save makes `workflow.yaml.bak`. Before overwriting, ResInsight
  warns if the file changed on disk since it was loaded, or if the folder is
  a symlink into a git checkout.
- **Save As** and **New Workflow** ask for a name and a folder. The default
  folder is `~/.taskmaestro/workflows/<name>`; a folder elsewhere is not
  discovered on the next rescan, and ResInsight warns about that. `*.py` files
  next to the original `workflow.yaml` are copied along, so tasks given as a
  dotted path (`pipeline.MyTask`) still import.
- `input.yaml` gets the literal values of the **first job** (the "Default"
  job). Object references (cases, well paths, views) are not saved.
- YAML comments other than the leading header block are lost, and the YAML
  is reformatted.

Closing ResInsight or the project, and **Rescan Workflows**, ask before
discarding unsaved edits.

## Type / format → binding mapping

`createBinding()` dispatches in this order — first `resinsight_type` (rips
object pickers take priority), then `format`, then bare `type`. Everything
unrecognised falls through to the string binding.

| Field annotation (Pydantic, in the workflow's task)              | Helper emits                          | C++ binding                  | PdmField type                       | Renders as                              |
|------------------------------------------------------------------|---------------------------------------|------------------------------|-------------------------------------|------------------------------------------|
| `rips.EclipseCase` / `ObjectModel[rips.EclipseCase]`             | `resinsight_type: EclipseCase`        | `RimWorkflowCaseBinding`     | `PdmPtrField<RimEclipseCase*>`      | Combo box of loaded cases                |
| `rips.WellPath` / `ObjectModel[rips.WellPath]`                   | `resinsight_type: WellPath`           | `RimWorkflowWellPathBinding` | `PdmPtrField<RimWellPath*>`         | Combo box of well paths                  |
| `rips.View` / `rips.EclipseView` / wrapper                       | `resinsight_type: View`               | `RimWorkflowViewBinding`     | `PdmPtrField<RimEclipseView*>`      | Combo box of `Case / View` labels        |
| `datetime.date` / `datetime.datetime`                            | `type: string`, `format: date`*       | `RimWorkflowDateBinding`     | `PdmField<QDate>`                   | Calendar widget (`PdmUiDateEditor`)      |
| `pathlib.Path` / `pydantic.FilePath` / `pydantic.NewPath`        | `type: string`, `format: path` / `file-path` | `RimWorkflowFilePathBinding` | `PdmField<caf::FilePath>` (file mode) | Text field + `[Browse…]` (file dialog)   |
| `pydantic.DirectoryPath`                                         | `type: string`, `format: directory-path` | `RimWorkflowFilePathBinding` | `PdmField<caf::FilePath>` (dir mode)  | Text field + `[Browse…]` (folder dialog) |
| `bool`                                                           | `type: boolean`                       | `RimWorkflowBoolBinding`     | `PdmField<bool>`                    | Checkbox                                 |
| `int`                                                            | `type: integer`                       | `RimWorkflowIntBinding`      | `PdmField<int>`                     | Spinbox                                  |
| `float`                                                          | `type: number`                        | `RimWorkflowFloatBinding`    | `PdmField<double>`                  | Decimal text field                       |
| `str` (or anything unrecognised)                                 | `type: string`                        | `RimWorkflowStringBinding`   | `PdmField<QString>`                 | Plain text field                         |

\*`format` extraction goes through `pydantic.TypeAdapter(field).json_schema()`
so any future Pydantic-native format Pydantic supports is picked up
automatically — no hand-maintained map. The helper reassembles `Annotated`
forms before asking, otherwise `Annotated[Path, PathType('dir')]` would lose
its `dir` marker.

`RimWorkflowFilePathBinding` is one class with an `m_selectDirectory` bool
toggled at `defineEditorAttribute` time; the same `caf::PdmField<caf::FilePath>`
flips between file-open and folder-pick dialogs based on the schema's format.

## Pre-filling from `input.yaml`

A binding's initial value comes from, in order:

1. the task's block in `input.yaml` (the definition's `inputs`), or
2. the field's `default=` in the Pydantic model, or
3. the `PdmField` default (empty string, 0.0, today's date, and so on).

Bindings are kept with their values when the workflow is edited, as long as
the field still has the same kind of binding (`syncTaskInputs`). Values of
fields that become wired are kept aside and restored if the field is
unwired again. Renaming a task carries its values along.

## The rips object reference round-trip

For fields typed as rips classes, the config field schema carries
`resinsight_type` but no `default` (the C++ picker provides the value
interactively). When the user picks a case in the UI and runs the job, the
C++ binding's `toJsonValue()` writes:

```yaml
{"load_model": {"case": {"__resinsight_ref__": "EclipseCase", "case_id": 0}}}
```

The reverse side, `rips.taskmaestro_helper.run.resolve_refs`
([run.py](../GrpcInterface/Python/rips/taskmaestro_helper/run.py)), walks
the YAML dict and replaces every `{__resinsight_ref__: <type>, …id…}` map
with `{"value": <live rips object>}` before handing it to
`taskmaestro.JobConfiguration`. The `"value": …` shape is exactly what
Pydantic's `ObjectModel[T]` validator accepts, so workflow authors who
declare `case: GridCase` get a real `rips.EclipseCase` injected without
any custom validator code.

ID schemes:

| `resinsight_type` | Key in the YAML map  | Resolves via                                 |
|-------------------|----------------------|----------------------------------------------|
| `EclipseCase`     | `case_id`            | `rips.Project.case(case_id=…)`              |
| `WellPath`        | `well_path_name`     | `rips.Project.well_path_by_name(…)`         |
| `View`            | `view_id`            | scan `rips.Project.views()` by `.id`        |

Whitelist lives in
[refs.py](../GrpcInterface/Python/rips/taskmaestro_helper/refs.py) — the
single source of truth for the rips types the helper recognises and
resolves. Adding a new rips-typed
picker means: extend the whitelist, add a `RimWorkflow<X>Binding` class,
register it in `createBinding()`.

## The run flow

1. The user runs a `RimWorkflowJob` (*Run* in the context menu or the
   property page).
2. `RimWorkflowJob::runJob()` checks that the gRPC port is live, finds a
   Python interpreter (`RiaPreferences::pythonExecutable()` → `python3` →
   `python`), and writes the job's values to `input.yaml` in a temp folder.
3. `RimWorkflow::prepareRunSource()` picks what to run:

   | Workflow | Command |
   |---|---|
   | Installed | `run --registered <id>` |
   | Saved, no unsaved edits | `run <workflow folder>` |
   | Unsaved edits | `run <edit folder>` with `PYTHONPATH=<original folder>` |

   The edit folder is a private temp folder holding the current revision
   (`save --workflow-only`). The files on disk are not touched until Save.
4. `RiuWorkflowJobRunner` starts the helper with
   `--input <tmp>/input.yaml --grpc-port <N> --run-id <id>`. The helper
   connects to ResInsight, resolves refs, and runs the workflow through
   `taskmaestro.Runner`.
5. The helper emits newline-delimited JSON events. Task state events update
   the job and the colours of the graph nodes; everything else goes to the
   log. Cancel terminates the process.

## File layout (new)

```
ApplicationLibCode/ProjectDataModel/Workflow/
├── RimWorkflow.{h,cpp}                  # one workflow: definition, undo, save, run source
├── RimWorkflowCollection.{h,cpp}        # the project-tree collection, rescan
├── RimWorkflowInstalledCollection.{h,cpp} # the "Installed" folder
├── RimWorkflowJob.{h,cpp}               # one set of input values; runs the workflow
├── RimWorkflowTaskInput.{h,cpp}         # one entry per task
├── RimWorkflowDefinition.{h,cpp}        # value structs: nodes, edges, issues
├── RimWorkflowDefinitionTools.{h,cpp}   # JSON conversion, edit operations, static validation
├── RimWorkflowPortCompatibility.{h,cpp} # ports and type checks
├── RimWorkflowSchemaTools.{h,cpp}       # JSON schema helpers
├── RimWorkflowTaskCatalog.{h,cpp}       # cached `catalog` output
├── RimWorkflowHelperProcess.{h,cpp}     # runs the Python helper
├── RimWorkflowValidationTools.{h,cpp}   # helper result → issues
├── RimWorkflowDescribeTools.{h,cpp}     # graph from `taskmaestro describe` (read-only workflows)
├── RimWorkflowFieldBinding.{h,cpp}      # abstract binding base
└── RimWorkflow<Type>Binding.{h,cpp}     # Bool, Int, Float, String, Date, FilePath,
                                         # Array, Case, WellPath, View

ApplicationLibCode/Commands/WorkflowCommands/
├── RicNewWorkflowFeature, RicDuplicateWorkflowAsEditableFeature,
│   RicSaveWorkflowFeature, RicSaveWorkflowAsFeature, RicRescanWorkflowsFeature
├── RicAddWorkflowTaskFeature, RicDeleteWorkflowTaskFeature,
│   RicDeleteWorkflowConnectionFeature, RicRenameWorkflowTaskFeature,
│   RicSetWorkflowResultTaskFeature, RicToggleWorkflowOptionalInputFeature
├── RicNewWorkflowJobFeature, RicRunWorkflowJobFeature, RicCancelWorkflowJobFeature
├── RicWorkflowFeatureTools.{h,cpp}      # shared helpers for the features
└── RicWorkflowLocationUi.{h,cpp}        # name + folder dialog

ApplicationLibCode/UserInterface/
├── RiuWorkflowEditorWidget.{h,cpp}      # the dock: graph, palette, status
├── RiuWorkflowGraphView.{h,cpp}         # QGraphicsView with editing
├── RiuWorkflowGraphLayout.{h,cpp}       # pure layout (QtCore only)
├── RiuWorkflowTaskPalette.{h,cpp}
├── RiuWorkflowValidationRunner.{h,cpp}  # debounced async validation
└── RiuWorkflowJobRunner.{h,cpp}         # runs a job, parses events

GrpcInterface/Python/rips/taskmaestro_helper/
├── __main__.py                          # CLI dispatch
├── _compat.py                           # taskmaestro version check, HelperError
├── catalog.py                           # catalog, export-registered
├── definition.py                        # load, save
├── describe.py                          # taskmaestro describe, in process
├── schema.py                            # task schemas and descriptors
├── refs.py                              # rips type whitelist + REF_MARKER
└── run.py                               # resolve refs + invoke Runner

GrpcInterface/Python/rips/tests/
├── test_taskmaestro_helper.py
├── test_taskmaestro_helper_definition.py   # run with --noconftest
└── test_data/taskmaestro_workflows/

ApplicationLibCode/UnitTests/
├── RimWorkflowDefinition-Test.cpp       # fixtures in TestData/RimWorkflowDefinition/
├── RimWorkflowJobSync-Test.cpp
├── RimWorkflowDescribeTools-Test.cpp
└── RiuWorkflowGraphLayout-Test.cpp
```

Touched (existing files):

- `ApplicationLibCode/ProjectDataModel/RimProject.{h,cpp}`: the
  `workflowCollection` child field, attached at the `MainWindow.Scripts` tab
  and marked with `disableIO()`.
- `ApplicationLibCode/UserInterface/RiuMainWindow.{h,cpp}`: hosts the editor
  dock and connects graph selection to the property view.
- `ApplicationLibCode/Application/RiaGuiApplication.{h,cpp}`: asks before
  unsaved workflow edits are discarded.

## Constraints

- **gRPC server must be on.** The run command refuses to launch otherwise.
  Set in *Preferences → Python → Enable gRPC server*.
- **`PATH` must point at the Python that has `rips` and `taskmaestro`
  installed**, because `findPythonExecutable()` searches `PATH`. Either
  launch ResInsight from a shell with the venv on `PATH`, or set
  *Preferences → Python → Python Executable Location* to an absolute path.
- **Rescan to pick up changes on disk.** Use *Rescan Workflows* on the
  Workflows node after adding folders to `~/.taskmaestro/workflows/` or
  installing packages. The task catalog is cached until then.
- **The collection is not serialised** to `.rsp`. Jobs and their values
  live for one ResInsight session; save the workflow to keep the Default
  job's values in `input.yaml`.

## Where to extend

- **New rips-typed picker** (e.g. `rips.SummaryCase`): add the class to
  `RIPS_TYPE_WHITELIST` in `refs.py`, add the resolver branch to
  `make_rips_resolver` in `run.py`, add a `RimWorkflowSummaryCaseBinding`
  modelled on `RimWorkflowCaseBinding`, register it in `createBinding()`.
- **New scalar/format**: usually nothing on the Python side (TypeAdapter
  handles it). On the C++ side, add a binding subclass and a one-line
  dispatch case.
- **Change the discovery directory**: edit
  `RimWorkflowCollection::discoveryDirectory()`. A `RiaPreferences` field
  + Preferences UI would be a small follow-up if multiple roots are wanted.
- **New edit operation**: add a pure function to `RimWorkflowDefinitionTools`
  with a unit test, call it through `RimWorkflow::applyEdit`, and expose it
  with a `Ric*WorkflowFeature` that reads its arguments from `userData()`.
- **Supporting `collect`/`map`/nested workflows in the editor**: extend the
  definition JSON and `RimWorkflowDefinitionNode`, drop the read-only reason
  in `definition.py` and `structuralReadOnlyReasons`, and teach `save` to
  write them.
