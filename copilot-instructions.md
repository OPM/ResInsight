# GitHub Copilot Instructions

You are an AI pair programming assistant helping developers work on the ResInsight codebase.

## Source of Truth

You MUST follow the canonical agent guidelines located in the `docs/agents/` directory:

- **[docs/agents/core.md](docs/agents/core.md)** - Core agent guidelines, architecture overview, PDM system, Python integration
- **[docs/agents/build.md](docs/agents/build.md)** - Build system, prerequisites, test commands
- **[docs/agents/coding-style.md](docs/agents/coding-style.md)** - Code formatting, style conventions
- **[docs/agents/repo-map.md](docs/agents/repo-map.md)** - Repository structure and file locations

These documents contain the authoritative information about the project. Always consult them when making suggestions.

## Copilot-Specific Guidelines

### Code Suggestions
- Suggest minimal, focused changes that solve the immediate problem
- Match the existing code style and patterns in the file
- Use modern C++23 features where appropriate
- Follow the PDM pattern for ResInsight-specific code

### Code Completion
- Complete code based on surrounding context
- Use existing class patterns and naming conventions
- Follow the clang-format style defined in `.clang-format`
- For Python code, follow PEP 8 and use snake_case

### Best Practices
1. **Understand Context**: Read nearby code to understand patterns and conventions
2. **Minimal Changes**: Suggest the smallest change that accomplishes the goal
3. **Type Safety**: Leverage C++23 type system and avoid implicit conversions
4. **Error Handling**: Use `std::expected` or appropriate error handling patterns
5. **Testing**: Suggest test cases when appropriate

### Language-Specific Guidelines

#### C++
- Use C++23 standard library features
- Follow RAII principles
- Use smart pointers (`std::unique_ptr`, `std::shared_ptr`) appropriately
- Prefer `auto` for complex types, but be explicit when clarity matters
- Use `const` and `constexpr` liberally

#### Python
- Format with `ruff format`
- Check style with `ruff check`
- Use type hints where helpful
- Follow snake_case naming convention
- Keep code compatible with Python 3.8+

### PDM System Patterns

When working with PDM (Project Data Model) objects:

1. **Field Access**: Always use `()` to access field values:
   ```cpp
   // Correct
   QString name = m_myTextField();
   
   // Incorrect
   QString name = m_myTextField;
   ```

2. **Scriptable Fields**: Use scriptable initialization for Python API:
   ```cpp
   CAF_PDM_InitScriptableField(&m_fieldName, "FieldName", defaultValue, "Display Name");
   ```

3. **UI Attributes**: Use modern `setAttribute` pattern:
   ```cpp
   m_field.uiCapability()->setAttribute( caf::PdmUiComboBoxEditor::Keys::ADJUST_WIDTH_TO_CONTENTS, true );
   ```

### Common Patterns

#### Adding a new PDM field
```cpp
// In header (.h)
caf::PdmField<QString> m_myField;

// In source (.cpp) constructor
CAF_PDM_InitScriptableField(&m_myField, "MyField", QString(), "Display Name");
```

#### Field validation
```cpp
m_percentage.setRange(0.0, 100.0);
m_count.setMinValue(0);
m_value.setMaxValue(1000);
```

#### GRPC method integration
```cpp
QString RimcClassName_method::classKeywordReturnedType() const
{
    return RimTargetClass::classKeywordStatic();
}
```

## When to Stop and Ask

If you encounter any of these situations, stop and ask the developer:
- Unclear requirements or specifications
- Changes that would affect multiple subsystems
- Breaking changes to public APIs
- Security-sensitive code modifications
- Performance-critical code sections

## Testing Reminders

- Run `ninja -C build` to build after C++ changes
- Run `ctest` to run tests
- For Python: Run `ruff format` and `ruff check --fix` before committing
- Test Python API changes with pytest: `python -m pytest tests/test_*.py`

## Git Commit Message Format

When suggesting commit messages:
```
#12773 Python: Add API for creating valve templates

- Added RimcWellPath_addValveTemplate method
- Made RimValveTemplate scriptable
- Updated Python tests
```

Start with issue number, followed by a brief description, then bullet points for details.
