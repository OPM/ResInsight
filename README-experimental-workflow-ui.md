# Using the experimental Workflow UI

The Workflow UI adds a **Workflows** node to the ResInsight project tree. It
requires a Python environment containing `rips`, `taskmaestro`, and the
workflow package.

These instructions assume ResInsight is already installed and working.
Python 3.12 or newer and Git are required.

## 1. Create the Python environment

Choose a permanent location for the environment and workflow files:

```bash
mkdir -p ~/resinsight-workflows
python3 -m venv ~/resinsight-workflows/.venv
source ~/resinsight-workflows/.venv/bin/activate
python -m pip install --upgrade pip
```

Install `rips` from the `Python` directory supplied with ResInsight:

```bash
python -m pip install -e /path/to/ResInsight/Python
```

For a source checkout, use `/path/to/ResInsight/GrpcInterface/Python` instead.

Clone and install the example workflows. This also installs `taskmaestro`:

```bash
git clone https://github.com/OPM/taskmaestro-resinsight.git \
    ~/resinsight-workflows/taskmaestro-resinsight
python -m pip install -e ~/resinsight-workflows/taskmaestro-resinsight
```

The Workflow UI requires **taskmaestro 0.3.x** (0.3.0 or newer, below 0.4) and
**taskmaestro-resinsight 0.3.0** or newer. Check the installed versions:

```bash
python -m pip show taskmaestro taskmaestro-resinsight | grep -E "^(Name|Version)"
```

If `taskmaestro` is older, upgrade it from Git:

```bash
python -m pip install --upgrade "git+https://github.com/OPM/taskmaestro.git@main"
```

## 2. Register the workflows

Workflows that installed packages register, such as
`resinsight.completions`, appear automatically in the **Installed** folder
under **Workflows**. They are read-only, but they can be run and duplicated.

ResInsight also discovers workflow folders in `~/.taskmaestro/workflows`. To
use the example folders from `taskmaestro-resinsight` directly, link them in:

```bash
mkdir -p ~/.taskmaestro/workflows
for workflow in ~/resinsight-workflows/taskmaestro-resinsight/workflows/*/; do
    ln -sfn "$(realpath "$workflow")" \
        ~/.taskmaestro/workflows/"$(basename "$workflow")"
done
```

Workflows you create in ResInsight are saved to this folder by default.

## 3. Configure ResInsight

Open **Edit → Preferences** and set:

1. **Scripting → Python Executable Location** to the absolute path printed by:

   ```bash
   realpath ~/resinsight-workflows/.venv/bin/python
   ```

2. **Scripting → Enable Python Script Server** to on.
3. **System → Experimental Features → Workflows** to on.

Restart ResInsight. The **Workflows** node should now appear in the project
tree. Select a workflow job, provide its inputs, and click **Run**.

## 4. Create and edit workflows

Selecting a workflow opens the workflow editor in a tab next to the 3D views.
It shows a palette of all installed tasks, the workflow graph, and a status
line with validation results.

- **New workflow:** right-click **Workflows** and choose **New Workflow**.
- **Copy a workflow:** choose **Duplicate as Editable** on any workflow,
  including installed ones.
- **Add a task:** drag it from the palette onto the graph, double-click it in
  the palette, or right-click the graph and use **Add Task**. Tasks that need
  a ResInsight connection are connected to a *connect* task automatically.
- **Connect tasks:** drag from an output port (right side of a task) to an
  input port (left side). While dragging, valid targets are green; invalid
  ones are grey and show the reason as a tooltip.
- **Config inputs:** every required input that is not connected becomes a
  config input, shown in the property view when the task is selected.
  Optional inputs can be turned on under **Optional Inputs** in the task's
  context menu.
- **Rename, Set as Result Task, Delete:** right-click a task or connection.
  F2 renames and Del deletes the selection. Ctrl+Z and Ctrl+Y undo and redo.

The workflow is validated with taskmaestro shortly after each edit. Problems
are shown as badges on the tasks and in the status line. A workflow that
fails validation can still be run after confirming.

Running a workflow with unsaved edits runs the edited version from a
temporary folder. The files on disk are not changed until you save.

### Saving

**Save** (Ctrl+S) writes `workflow.yaml` and `input.yaml` to the workflow
folder. **Save As** saves to a new folder. Note that:

- The first save keeps the original as `workflow.yaml.bak`.
- The YAML is rewritten. The comment block at the top of `workflow.yaml` is
  kept; other comments are lost.
- `input.yaml` gets the values of the workflow's first job (**Default**).
  Selected cases, well paths, and views are not saved.
- Folders outside `~/.taskmaestro/workflows` are not found when ResInsight
  starts or rescans.

Use **Rescan Workflows** on the **Workflows** node after changing workflow
folders or installing packages. ResInsight asks before closing or rescanning
when there are unsaved edits.

Workflows that use `collect`, `map`, or nested workflows open read-only, and
the status line explains why. They can still be run.

## Troubleshooting

- **No Workflows node:** Enable the experimental feature and restart
  ResInsight.
- **The Workflows node is empty:** Check that each directory under
  `~/.taskmaestro/workflows` contains `workflow.yaml`, then use **Rescan
  Workflows**.
- **The task palette is empty or the Installed folder is missing:** The task
  catalog failed to load. Hover over *Task catalog not available* in the
  palette to see why. Check that
  `taskmaestro` and the task packages are installed in the configured Python
  environment.
- **A Python module cannot be found:** Confirm that **Python Executable
  Location** points to the virtual environment created above.
- **Workflows fail to load with a message about the taskmaestro version:**
  Upgrade `taskmaestro` as described above.
- **The workflow cannot connect:** Enable **Python Script Server** and restart
  ResInsight.
