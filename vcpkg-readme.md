# vcpkg Configuration

## Files

| File | Purpose |
|------|---------|
| `vcpkg-configuration.json` | Default registry configuration used by all builds |
| `vcpkg-configuration-rhel8.json` | RHEL8-specific registry configuration with a separate baseline |

## Changing the baseline

The `baseline` field in each configuration file pins the vcpkg registry to a specific commit of [microsoft/vcpkg](https://github.com/microsoft/vcpkg). Packages available at that commit are used for dependency resolution.

- Update `vcpkg-configuration.json` for the default (Ubuntu/Windows) builds.
- Update `vcpkg-configuration-rhel8.json` independently when a different baseline is needed for RHEL8 (Rocky Linux 8) compatibility — e.g. to match older system libraries (glibc 2.28, OpenSSL 1.1).

## RHEL8 CI usage

The workflow `.github/workflows/rhel8-unit-tests.yml` copies `vcpkg-configuration-rhel8.json` over `vcpkg-configuration.json` before invoking cmake:

```yaml
- name: Use RHEL8 vcpkg configuration
  run: cp vcpkg-configuration-rhel8.json vcpkg-configuration.json
```

This ensures vcpkg uses the RHEL8 baseline without affecting other CI runs.
