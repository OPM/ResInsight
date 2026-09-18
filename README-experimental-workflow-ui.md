# Running ResInsight with the experimental Workflow UI

The Workflow UI adds a **Workflows** node to the project tree (next to
**Scripts**). It discovers [taskmaestro](https://github.com/OPM/taskmaestro)
workflows on disk, builds a property editor for each task's inputs, and runs
the workflow as a Python subprocess that talks back to ResInsight over gRPC.

Design details: [docs/workflow-ui-design.md](docs/workflow-ui-design.md).

## TL;DR

```bash
# 1. Build
cd /home/resinsight/projects/resinsight/build && ninja

# 2. Activate the Python env that has rips + taskmaestro + taskmaestro_resinsight
source /home/resinsight/projects/resinsight/taskmaestro-venv/bin/activate

# 3. Make sure a workflow is discoverable
ls -l ~/.taskmaestro/workflows/        # each subdir must contain workflow.yaml

# 4. Start ResInsight from that shell (so `python3` on PATH is the venv one)
/home/resinsight/projects/resinsight/build/ResInsight
```

Then, **once**, inside ResInsight:

- *Edit → Preferences → System → Experimental Features*: tick **Workflows**.
- *Edit → Preferences → Python*: **Enable Python Script Server** must be on.
- Restart ResInsight. The **Workflows** node appears in the project tree.

## The four things that must be true

### 1. The `workflows` experimental feature is enabled

The whole UI is gated behind `RiaPreferencesSystem::isFeatureEnabled( "workflows" )`
(see `RimWorkflowCollection.cpp` and `RimProject.cpp`). Enable it via
*Preferences → System → Experimental Features → Workflows*. Workflows are
scanned at project init only, so restart after enabling.

### 2. The gRPC server is on

`RimWorkflow::runWorkflow()` refuses to launch if no gRPC port is live. Check
*Preferences → Python → Enable Python Script Server* (default on). Optionally
start with a fixed port: `ResInsight --server 50051`.

### 3. ResInsight can find a Python that has `rips` **and** `taskmaestro`

`RimWorkflow::findPythonExecutable()` tries, in order:

1. *Preferences → Python → Python Executable Location* (if not the default `python`)
2. `python3` on `PATH`
3. `python` on `PATH`

So either **launch ResInsight from a shell with the venv activated**, or set
the preference to an absolute path, e.g.
`/home/resinsight/projects/resinsight/taskmaestro-venv/bin/python3`.

The interpreter is used for both discovery (`python -m rips.taskmaestro_helper introspect <dir>`)
and running (`python -m rips.taskmaestro_helper run <dir> --input … --grpc-port N`).
Verify it works before starting ResInsight:

```bash
python3 -c "import rips, taskmaestro, taskmaestro_resinsight; print(rips.__file__)"
python3 -m rips.taskmaestro_helper introspect ~/.taskmaestro/workflows/resinsight_completions
```

The second command should print JSON with a `tasks` list.

### 4. Workflows live in `~/.taskmaestro/workflows/<name>/workflow.yaml`

The discovery directory is hard-coded. Each immediate subdirectory containing
a `workflow.yaml` becomes one workflow. Normal practice is symlinks:

```bash
mkdir -p ~/.taskmaestro/workflows
ln -s /path/to/taskmaestro-resinsight/workflows/resinsight_completions \
      ~/.taskmaestro/workflows/resinsight_completions
```

Workflows are **not** rescanned at runtime — restart ResInsight (or reload the
project) after adding one.

## Setting up the Python environment

Create a dedicated venv containing `rips` (from this checkout), `taskmaestro`
and `taskmaestro-resinsight` (which provides the example workflows), then
link a workflow into the discovery directory:

```bash
cd /home/resinsight/projects/resinsight
python3 -m venv taskmaestro-venv
source taskmaestro-venv/bin/activate
pip install -e GrpcInterface/Python                 # rips, editable install from local source (see below)
pip install taskmaestro                             # or: pip install -e /path/to/taskmaestro
pip install -e /path/to/taskmaestro-resinsight      # provides the example workflows

mkdir -p ~/.taskmaestro/workflows
ln -s /path/to/taskmaestro-resinsight/workflows/resinsight_completions \
      ~/.taskmaestro/workflows/resinsight_completions
```

If an existing `taskmaestro-venv` or workflow symlink was copied from another
machine, its absolute paths will be stale (e.g. `bin/python` or the editable
installs pointing at directories that don't exist). Delete it
(`rm -rf taskmaestro-venv`, `rm ~/.taskmaestro/workflows/<name>`) and rerun
the steps above.

### Installing `rips` from the local source tree

Always install `rips` from this checkout, not from PyPI, so the Python side
matches the C++ gRPC interface you built:

```bash
source taskmaestro-venv/bin/activate
pip install -e /home/resinsight/projects/resinsight/GrpcInterface/Python
```

`-e` (editable) links the venv to `GrpcInterface/Python/rips` instead of
copying it — edits to the Python code take effect immediately. Notes:

- **Build before installing.** `ninja` runs protoc and writes the
  `*_pb2.py` / `*_pb2_grpc.py` stubs into `GrpcInterface/Python/rips/generated/`
  (`GrpcInterface/CMakeLists.txt`). After changing a `.proto`, rebuild — no
  reinstall needed thanks to the editable install.
- Verify: `python -c "import rips; print(rips.__file__)"` should print a path
  under `/home/resinsight/projects/resinsight/GrpcInterface/Python/rips/`.

## Troubleshooting

| Symptom | Cause / fix |
|---|---|
| No **Workflows** node in the tree | `workflows` feature not ticked in *Preferences → System → Experimental Features*, or ResInsight not restarted. |
| Node present but empty | `~/.taskmaestro/workflows/` has no subdir with `workflow.yaml`, or the introspect helper failed. Run the `introspect` command above by hand. |
| `ModuleNotFoundError: taskmaestro` / `rips` | ResInsight picked the wrong Python. Activate the venv before launching, or set the absolute path in *Preferences → Python*. |
| Run button does nothing / "gRPC server not running" | Enable *Python Script Server* in preferences and restart. |
| Edited `input.yaml` defaults not shown | Defaults are read at startup; restart ResInsight. |

## Related files

- `ApplicationLibCode/Application/RiaExperimentalFeatures.cpp` — feature registry (`"workflows"`)
- `ApplicationLibCode/ProjectDataModel/Workflow/` — `RimWorkflowCollection`, `RimWorkflow`, bindings
- `GrpcInterface/Python/rips/taskmaestro_helper/` — `introspect.py`, `run.py`, `refs.py`
- `docs/workflow-ui-design.md` — full design document
