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

## 2. Register the workflows

ResInsight discovers workflows in `~/.taskmaestro/workflows`:

```bash
mkdir -p ~/.taskmaestro/workflows
for workflow in ~/resinsight-workflows/taskmaestro-resinsight/workflows/*/; do
    ln -sfn "$(realpath "$workflow")" \
        ~/.taskmaestro/workflows/"$(basename "$workflow")"
done
```

## 3. Configure ResInsight

Open **Edit → Preferences** and set:

1. **Scripting → Python Executable Location** to the absolute path printed by:

   ```bash
   realpath ~/resinsight-workflows/.venv/bin/python
   ```

2. **Scripting → Enable Python Script Server** to on.
3. **System → Experimental Features → Workflows** to on.

Restart ResInsight. The **Workflows** node should now appear in the project
tree. Select a workflow, provide its inputs, and click **Run**.

## Troubleshooting

- **No Workflows node:** Enable the experimental feature and restart
  ResInsight.
- **The Workflows node is empty:** Check that each directory under
  `~/.taskmaestro/workflows` contains `workflow.yaml`, then restart ResInsight.
- **A Python module cannot be found:** Confirm that **Python Executable
  Location** points to the virtual environment created above.
- **The workflow cannot connect:** Enable **Python Script Server** and restart
  ResInsight.
