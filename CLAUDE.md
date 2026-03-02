# Claude Agent Instructions

You are an automated contributor to the ResInsight repository.

## Source of Truth

You MUST follow the canonical agent guidelines located in the `docs/agents/` directory:

- **[docs/agents/core.md](docs/agents/core.md)** - Core agent guidelines, architecture overview, PDM system, Python integration
- **[docs/agents/build.md](docs/agents/build.md)** - Build system, prerequisites, test commands
- **[docs/agents/coding-style.md](docs/agents/coding-style.md)** - Code formatting, style conventions
- **[docs/agents/repo-map.md](docs/agents/repo-map.md)** - Repository structure and file locations

These documents contain the authoritative information about the project. Always consult them when working on code.

## Claude-Specific Rules

### Change Philosophy
- Prefer minimal diffs - make the smallest possible changes to achieve the goal
- Do not reformat unrelated code
- Preserve existing code style and comments
- Only modify files directly related to your task

### Safety Guidelines
- Never modify CI/CD configuration unless explicitly asked
- Always validate changes don't break existing behavior
- Always validate changes don't introduce security vulnerabilities
- If unsure about a change, stop and explain the uncertainty to the user

### Testing
- Run existing linters and tests before making changes
- Run linters and tests after making changes
- Do not add new testing tools unless necessary
- Do not remove or modify unrelated tests

## Output Expectations

### Before Making Changes
- Explain your reasoning and approach
- List the specific files that will be modified
- Describe the expected impact of the changes

### When Making Changes
- Make incremental, focused changes
- Test each change before proceeding
- Report progress frequently using the `report_progress` tool

### After Making Changes
- Verify the changes work as expected
- Run relevant tests and linters
- Document any issues or limitations discovered

## Working with Python

When working with Python code:
- Always format code with `ruff format`
- Always check style with `ruff check --fix`
- Run these before committing changes

## Commit Messages

Follow these conventions:
- Start with issue number: `#12773 Python: Add API for creating valve templates`
- Use standard git commit message format
- Only use line breaks between paragraphs in the description
- Keep first line concise and descriptive