# Spell Checking Workflows

ResInsight uses automated spell checking to maintain code quality and catch typos in code, comments, and documentation.

## Two Workflows

### 1. spell-check.yml (Full Repository Check)

**When it runs:** On every push to any branch

**What it does:**
- Checks all code in four directories:
  - `ApplicationExeCode/`
  - `ApplicationLibCode/`
  - `Fwk/AppFwk/`
  - `GrpcInterface/`
- Uses `codespell` to find and suggest fixes for typos
- Automatically creates a PR with fixes if typos are found
- PR will be created on a branch named `spell-check-patches-*`

**Use case:** Regular maintenance and keeping the entire codebase clean

### 2. spell-check-pr.yml (PR Review Check)

**When it runs:** On pull requests that modify files in the target directories

**What it does:**
- Only checks files that were changed in the PR
- Uses `codespell` for spell checking
- Checks these file types: `.cpp`, `.h`, `.inl`, `.py`, `.md`, `.txt`, `.cmake`, `.yml`, `.yaml`
- Skips binary files: `.svg`, `.xml`, `.json`
- Fails the PR check if typos are found (must be fixed before merge)

**Use case:** Preventing new typos from being introduced in PRs

## Configuration Files

### .codespellrc

Central configuration file for codespell that defines:
- Files to skip (e.g., SVG, JSON, XML files)
- Ignore words file location
- Default directories to check
- File types to include
- Output formatting options

This configuration is automatically used by both GitHub workflows and local runs.

### .codespell-ignore

Used by `codespell` (both workflows). Contains domain-specific terms that should not be flagged:

- Petroleum engineering terms: `perm`, `porosity`, `wellbore`, `facies`
- Eclipse file formats: `EGRID`, `INIT`, `UNRST`, `SMSPEC`
- Simulation tools: `Abaqus`, `ODB`, `VTK`
- Common abbreviations: `API`, `GUI`, `RGB`, `CSV`
- Units: `ft`, `mD`, `bbl`, `psi`

**To add a word:** Simply add it to `.codespell-ignore` (one word per line, comments start with `#`)

## How to Fix Typos

### From Full Repository Check

When the spell-check workflow creates a PR:

1. Review the PR created by the workflow
2. Check if the suggested changes are correct
3. If correct: merge the PR
4. If incorrect: close the PR and add the word to `.codespell-ignore`

### From PR Review Check

When your PR fails the spell check:

1. Read the error message to see which words are flagged
2. Fix the typos in your code OR
3. If the word is correct (domain-specific term), add it to `.codespell-ignore`
4. Push the changes to your PR
5. The check will run again automatically

## Common Domain Terms Already Whitelisted

The following are already in the whitelist:
- Reservoir simulation: `perm`, `permeability`, `porosity`, `geomech`, `seismic`
- Eclipse formats: `EGRID`, `GRID`, `INIT`, `UNRST`, `SMSPEC`, `ESMRY`
- Tools: `Abaqus`, `ODB`, `VTK`, `OSDU`, `FMU`, `VFP`
- Abbreviations: `Md`, `Tvd`, `Rkb`, `Rft`, `Plt`
- Organizations: `ResInsight`, `Equinor`, `Ceetron`, `OPM`

## Testing Locally

You can run spell checks locally before pushing:

### Using codespell (recommended for quick checks)

```bash
# Install codespell
pip install codespell

# Check specific files (uses .codespellrc automatically)
codespell myfile.cpp

# Check a directory (uses .codespellrc automatically)
codespell ApplicationLibCode/

# Check all default directories (defined in .codespellrc)
codespell

# Apply fixes automatically (use with caution)
codespell --write-changes

# Override config file settings if needed
codespell --ignore-words=custom-ignore.txt --skip="*.log,*.tmp" myfile.cpp
```

## Troubleshooting

### False Positives

If a word is flagged but is correct:
1. Add it to `.codespell-ignore`

### Words That Should Be Fixed

If the spell checker suggests a change that seems wrong, check:
- Is the word used correctly in context?
- Is it a technical term that should be in the whitelist?
- Is there a better/clearer word to use?

### Workflow Not Running

- Check that your changes touch files in the monitored directories
- Check that the file extensions are supported
- Look at the GitHub Actions tab for error messages
