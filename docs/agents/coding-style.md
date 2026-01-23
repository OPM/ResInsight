# Coding Style Guidelines

This document describes the coding style and formatting conventions for ResInsight.

## Code Formatting

### clang-format

ResInsight uses clang-format for C++ code formatting:
- **Configuration**: `.clang-format` file in repository root
- **Version**: Use clang-format-19 to enforce style

### Python Formatting

Python code should be formatted using `ruff`:

```bash
# Format source code
python -m ruff format test_polygons.py 

# Check code style
python -m ruff check --fix test_polygons.py
```

## Language Standards

- **C++**: C++23 standard
- **Python**: Python 3.8+

## Best Practices

### General Guidelines

1. **Minimal Changes**: Make the smallest possible changes to achieve the goal
2. **Preserve Formatting**: Do not reformat unrelated code
3. **Comments**: Match the style of existing comments in the file
4. **Libraries**: Use existing libraries whenever possible; only add new libraries or update versions if absolutely necessary

### Code Quality

1. Always validate changes don't break existing behavior
2. Always validate changes don't introduce security vulnerabilities
3. Fix any vulnerabilities related to your changes
4. Run linters, builds and tests before making code changes to understand any existing issues
5. Always try to lint, build and test code changes as soon as possible after making them

### Testing

1. Only run linters, builds and tests that already exist
2. Do not add new linting, building or testing tools unless necessary to fix the issue
3. It is unacceptable to remove or edit unrelated tests
4. Documentation changes do not need to be linted, built or tested unless there are specific tests for documentation

## Commit Conventions

When creating commits:
- Use issue number at the start of the title: `#12773 Python: Add API for creating valve templates`
- Follow git conventions for commit messages
- Always run python formatting/check on changed files before commits
