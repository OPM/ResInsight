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

## Copyright Headers

- New files must use the **current year** (the year the file is created) in the copyright header
- Never change the copyright year in existing files — the year reflects when the file was originally created
- Example header for a file created in 2026:

```cpp
/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
//
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

### Lambda Functions

Keep lambdas short and readable:

- **1–3 lines**: keep inline
- **4+ lines or contains control flow** (if/for/while): extract to a named method (e.g. `onApplyClicked()`) and call it from a single-line lambda

```cpp
// Good – short inline lambda
addNewButton( "Show Report", [this]() { showReport(); } );

// Good – multi-statement, no control flow, still inline
addNewButton( "Clear Data",
              [this]()
              {
                  clearData();
                  updateConnectedEditors();
              } );

// Good – complex logic extracted to a named method
addNewButton( "Apply", [this]() { onApplyClicked(); } );
void MyClass::onApplyClicked() { /* complex logic here */ }
```

## Commit Conventions

When creating commits:
- Use issue number at the start of the title: `#12773 Python: Add API for creating valve templates`
- Follow git conventions for commit messages
- Always run python formatting/check on changed files before commits
