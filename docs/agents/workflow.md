# Agent Workflow Guidelines

This document describes how AI agents should operate when making changes to ResInsight.

## Safety Guidelines

- Never modify CI/CD configuration unless explicitly asked
- Always validate changes don't break existing behavior
- Always validate changes don't introduce security vulnerabilities
- If unsure about a change, stop and explain the uncertainty to the user

## Output Expectations

### Before Making Changes
- Explain your reasoning and approach
- List the specific files that will be modified
- Describe the expected impact of the changes

### When Making Changes
- Make incremental, focused changes
- Test each change before proceeding
- Report progress frequently

### After Making Changes
- Verify the changes work as expected
- Run relevant tests and linters
- Document any issues or limitations discovered
