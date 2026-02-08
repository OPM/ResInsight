# Integration Plan: RiaSumoExplorerConnector in RimCloudDataSourceCollection

## Overview

Make data from the Python-based `RiaSumoExplorerConnector` available in the existing `RimCloudDataSourceCollection` UI, allowing users to choose between OAuth2-based (`RiaSumoConnector`) and Python Explorer-based (`RiaSumoExplorerConnector`) access to Sumo cloud data.

## Background

Currently, `RimCloudDataSourceCollection` only supports `RiaSumoConnector` (OAuth2 REST API). The newer `RiaSumoExplorerConnector` wraps the Python `fmu.sumo.explorer` API via a local FastAPI server. Both connectors provide identical hierarchical data (Assets → Cases → Ensembles → Vectors/Realizations) with near-identical APIs.

## Approach: Preference-Based Connector Selection with Type Conversion

**Key Design Decisions:**
1. **Reuse existing data structures** - `RimSummarySumoDataSource` works for both connectors
2. **User preference controls connector choice** - Add `useSumoExplorerForCloudData` boolean to preferences
3. **Type conversion layer** - Convert Explorer types (SumoExplorerAsset) to OAuth types (SumoAsset) since they have identical fields
4. **Minimal code changes** - Use conditional logic based on preference rather than full abstraction layer
5. **Backward compatible** - Default to OAuth2 connector, existing projects continue to work

## Implementation Steps

### 1. Add Type Conversion Utilities

**File:** `ApplicationLibCode/Application/Tools/Cloud/RiaSumoExplorerDefines.h`
**File:** `ApplicationLibCode/Application/Tools/Cloud/RiaSumoExplorerDefines.cpp`

Add conversion functions to map Explorer types to OAuth types:

```cpp
// In RiaSumoExplorerDefines namespace
SumoAsset toSumoAsset(const SumoExplorerAsset& asset);
SumoCase toSumoCase(const SumoExplorerCase& explorerCase);
SumoCaseId toSumoCaseId(const QString& caseId);
QString fromSumoCaseId(const SumoCaseId& caseId);
```

These simply copy field values since the structures are identical except for naming.

### 2. Update Preferences

**File:** `ApplicationLibCode/Application/RiaPreferencesSumoExplorer.h`
**File:** `ApplicationLibCode/Application/RiaPreferencesSumoExplorer.cpp`

Add preference field to choose connector type:

```cpp
caf::PdmField<bool> m_useSumoExplorerForCloudData;
bool useSumoExplorerForCloudData() const;
```

Initialize to `false` (default to OAuth2 for backward compatibility). Add to UI with description explaining the choice.

### 3. Update RimCloudDataSourceCollection

**File:** `ApplicationLibCode/ProjectDataModel/Cloud/RimCloudDataSourceCollection.h`
**File:** `ApplicationLibCode/ProjectDataModel/Cloud/RimCloudDataSourceCollection.cpp`

#### 3a. Add Support for Both Connectors

Add member variables:
```cpp
QPointer<RiaSumoConnector> m_sumoConnector;  // existing
QPointer<RiaSumoExplorerConnector> m_sumoExplorerConnector;  // new
bool m_useExplorerConnector;  // cache preference value
```

#### 3b. Initialize Both Connectors in Constructor

```cpp
RimCloudDataSourceCollection::RimCloudDataSourceCollection()
{
    // ... existing field initialization ...

    auto prefs = RiaApplication::instance()->preferences();
    m_useExplorerConnector = prefs->sumoExplorerPreferences()->useSumoExplorerForCloudData();

    if (m_useExplorerConnector) {
        m_sumoExplorerConnector = RiaApplication::instance()->makeSumoExplorerConnector();
    } else {
        m_sumoConnector = RiaApplication::instance()->makeSumoConnector();
    }
}
```

#### 3c. Update Authentication Logic in `fieldChangedByUi()`

Handle authentication differently based on connector type:

```cpp
if (changedField == &m_authenticate)
{
    if (m_useExplorerConnector && m_sumoExplorerConnector) {
        // Start server if not running
        if (!m_sumoExplorerConnector->isServerRunning()) {
            m_sumoExplorerConnector->startServer();
            m_sumoExplorerConnector->waitForServerReady();
        }
    } else if (m_sumoConnector) {
        m_sumoConnector->requestTokenWithCancelButton();
    }
    m_authenticate = false;
}
```

#### 3d. Update `calculateValueOptions()`

Add conditional logic to use appropriate connector:

```cpp
QList<caf::PdmOptionItemInfo> RimCloudDataSourceCollection::calculateValueOptions(...)
{
    // Check which connector is active and granted
    bool isGranted = false;
    if (m_useExplorerConnector && m_sumoExplorerConnector) {
        isGranted = m_sumoExplorerConnector->isServerRunning();
    } else if (m_sumoConnector) {
        isGranted = m_sumoConnector->isGranted();
    }

    if (!isGranted) return {};

    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_sumoFieldName)
    {
        if (m_useExplorerConnector) {
            // Use explorer connector
            if (m_sumoExplorerConnector->assets().empty()) {
                m_sumoExplorerConnector->requestAssetsBlocking();
            }
            for (const auto& asset : m_sumoExplorerConnector->assets()) {
                if (m_sumoFieldName().isEmpty()) m_sumoFieldName = asset.name;
                options.push_back({asset.name, asset.name});
            }
        } else {
            // Use OAuth connector (existing code)
            if (m_sumoConnector->assets().empty()) {
                m_sumoConnector->requestAssetsBlocking();
            }
            for (const auto& asset : m_sumoConnector->assets()) {
                if (m_sumoFieldName().isEmpty()) m_sumoFieldName = asset.name;
                options.push_back({asset.name, asset.name});
            }
        }
    }
    else if (fieldNeedingOptions == &m_sumoCaseId && !m_sumoFieldName().isEmpty())
    {
        if (m_useExplorerConnector) {
            // Use explorer connector
            if (m_sumoExplorerConnector->cases().empty()) {
                m_sumoExplorerConnector->requestCasesForFieldBlocking(m_sumoFieldName);
            }
            for (const auto& explorerCase : m_sumoExplorerConnector->cases()) {
                options.push_back({explorerCase.name, explorerCase.caseId});
            }
        } else {
            // Use OAuth connector (existing code)
            // ... existing code ...
        }
    }
    else if (fieldNeedingOptions == &m_sumoEnsembleNames && !m_sumoCaseId().isEmpty())
    {
        if (m_useExplorerConnector) {
            // Use explorer connector
            if (m_sumoExplorerConnector->ensembles().empty()) {
                m_sumoExplorerConnector->requestEnsemblesForCaseBlocking(m_sumoCaseId);
            }
            for (const auto& ensemble : m_sumoExplorerConnector->ensembles()) {
                options.push_back({ensemble.ensembleName, ensemble.ensembleName});
            }
        } else {
            // Use OAuth connector (existing code)
            // ... existing code ...
        }
    }

    return options;
}
```

#### 3e. Update `defineUiOrdering()`

Update authentication status message:

```cpp
bool isGranted = false;
if (m_useExplorerConnector && m_sumoExplorerConnector) {
    isGranted = m_sumoExplorerConnector->isServerRunning();
} else if (m_sumoConnector) {
    isGranted = m_sumoConnector->isGranted();
}

QString statusType = m_useExplorerConnector ? "Server Status" : "Authentication Status";
QString text = statusType + ": ";
text += isGranted ? "<font color='#228B22'>✔ Granted</font>" : "<font color='#FFA500'>❌ Not Granted</font>";
```

#### 3f. Update `addDataSources()`

Handle data fetching from both connectors:

```cpp
std::vector<RimSummarySumoDataSource*> RimCloudDataSourceCollection::addDataSources()
{
    std::vector<RimSummarySumoDataSource*> dataSources;
    auto sumoCaseId = SumoCaseId(m_sumoCaseId);

    for (const auto& ensembleName : m_sumoEnsembleNames())
    {
        // Check for duplicates (existing code)
        bool createNewDataSource = true;
        for (const auto dataSource : sumoDataSources()) {
            if (dataSource->caseId() == sumoCaseId && dataSource->ensembleName() == ensembleName) {
                createNewDataSource = false;
                break;
            }
        }
        if (!createNewDataSource) continue;

        // Get case name
        QString caseName;
        if (m_useExplorerConnector) {
            for (const auto& explorerCase : m_sumoExplorerConnector->cases()) {
                if (explorerCase.caseId == m_sumoCaseId()) {
                    caseName = explorerCase.name;
                    break;
                }
            }
        } else {
            for (const auto& sumoCase : m_sumoConnector->cases()) {
                if (sumoCase.caseId == sumoCaseId) {
                    caseName = sumoCase.name;
                    break;
                }
            }
        }

        // Request metadata
        std::vector<QString> realizationIds;
        std::vector<QString> vectorNames;

        if (m_useExplorerConnector) {
            m_sumoExplorerConnector->requestRealizationIdsBlocking(m_sumoCaseId(), ensembleName);
            m_sumoExplorerConnector->requestVectorNamesBlocking(m_sumoCaseId(), ensembleName);

            // Convert Explorer types to OAuth types
            for (const auto& r : m_sumoExplorerConnector->realizationIds()) {
                realizationIds.push_back(QString::number(r.realizationId));
            }
            for (const auto& v : m_sumoExplorerConnector->vectorNames()) {
                vectorNames.push_back(v.name);
            }
        } else {
            m_sumoConnector->requestRealizationIdsForEnsembleBlocking(sumoCaseId, ensembleName);
            m_sumoConnector->requestVectorNamesForEnsembleBlocking(sumoCaseId, ensembleName);
            realizationIds = m_sumoConnector->realizationIds();
            vectorNames = m_sumoConnector->vectorNames();
        }

        // Create data source (existing code pattern)
        auto dataSource = new RimSummarySumoDataSource();
        dataSource->setCaseId(sumoCaseId);
        dataSource->setCaseName(caseName);
        dataSource->setEnsembleName(ensembleName);
        dataSource->setRealizationIds(realizationIds);
        dataSource->setVectorNames(vectorNames);
        dataSource->setConnectorType(m_useExplorerConnector ? "Explorer" : "OAuth2");
        dataSource->updateName();

        m_sumoDataSources.push_back(dataSource);
        dataSources.push_back(dataSource);
    }

    // ... existing UI update code ...
    return dataSources;
}
```

### 4. Track Connector Type in Data Source

**File:** `ApplicationLibCode/ProjectDataModel/Summary/Sumo/RimSummarySumoDataSource.h`
**File:** `ApplicationLibCode/ProjectDataModel/Summary/Sumo/RimSummarySumoDataSource.cpp`

Add field to track which connector created the data source:

```cpp
caf::PdmField<QString> m_connectorType;  // "OAuth2" or "Explorer"
QString connectorType() const;
void setConnectorType(const QString& type);
```

This allows `RimSummaryEnsembleSumo` to know which connector to use for loading parquet data.

### 5. Update Ensemble Data Loading

**File:** `ApplicationLibCode/ProjectDataModel/Summary/Sumo/RimSummaryEnsembleSumo.cpp`

Update data loading methods to support both connectors:

```cpp
QByteArray RimSummaryEnsembleSumo::requestParquetDataBlocking(...)
{
    if (m_sumoDataSource && m_sumoDataSource->connectorType() == "Explorer") {
        auto connector = RiaApplication::instance()->makeSumoExplorerConnector();
        if (connector && connector->isServerRunning()) {
            QString caseIdStr = m_sumoDataSource->caseId().get();
            return connector->requestSummaryDataBlocking(caseIdStr, ensembleId, vectorName);
        }
    } else {
        // Use OAuth connector (existing code)
        return m_sumoConnector->requestParquetDataBlocking(...);
    }
}
```

Similar updates for parameter loading.

## Critical Files to Modify

1. **ApplicationLibCode/Application/Tools/Cloud/RiaSumoExplorerDefines.h/cpp** - Type conversion functions
2. **ApplicationLibCode/Application/RiaPreferencesSumoExplorer.h/cpp** - Add connector selection preference
3. **ApplicationLibCode/ProjectDataModel/Cloud/RimCloudDataSourceCollection.h/cpp** - Main integration logic
4. **ApplicationLibCode/ProjectDataModel/Summary/Sumo/RimSummarySumoDataSource.h/cpp** - Track connector type
5. **ApplicationLibCode/ProjectDataModel/Summary/Sumo/RimSummaryEnsembleSumo.cpp** - Support both connectors for data loading

## Verification Steps

1. **Test OAuth2 Flow (Existing Functionality)**
   - Launch ResInsight with `useSumoExplorerForCloudData = false` (default)
   - Navigate to Cloud Data collection
   - Click Authenticate → verify OAuth2 flow works
   - Select Field/Case/Ensemble → verify dropdowns populate
   - Add data source → verify it appears in project tree
   - Create ensemble → verify data loads correctly

2. **Test Explorer Flow (New Functionality)**
   - Set preference `useSumoExplorerForCloudData = true`
   - Restart ResInsight (or reload preferences)
   - Navigate to Cloud Data collection
   - Click Authenticate → verify Python server starts
   - Verify status shows "Server Status: ✔ Granted"
   - Select Field/Case/Ensemble → verify dropdowns populate from Explorer
   - Add data source → verify it appears with connectorType="Explorer"
   - Create ensemble → verify data loads from Explorer connector

3. **Test Server Auto-Start**
   - With `autoStartServer = true` in preferences
   - Verify server starts automatically on application launch
   - Verify Cloud Data collection can use server immediately

4. **Test Error Handling**
   - Explorer connector: Stop Python, verify error message shown
   - OAuth2 connector: Deny authentication, verify appropriate message
   - Verify switching between connectors via preferences works

5. **Test Backward Compatibility**
   - Open existing project files created with OAuth2 connector
   - Verify they load correctly regardless of current preference
   - Verify ensembles still load data correctly

## Benefits

- **User choice** - Select between OAuth2 (direct API) and Python Explorer (SDK-based) access
- **Minimal changes** - Reuses existing data structures and UI
- **Backward compatible** - Defaults to OAuth2, existing functionality preserved
- **Future-proof** - Easy to add more connector types or deprecate old ones
- **Clean separation** - Type conversion isolated in dedicated functions

## Risks and Mitigation

| Risk | Mitigation |
|------|------------|
| Explorer server fails to start | Check server status, show clear error message with troubleshooting steps |
| Type conversion introduces bugs | Create unit tests for conversion functions, validate field mapping |
| Performance differences between connectors | Both use blocking HTTP, similar performance; monitor and optimize if needed |
| Project file compatibility | Store only connector type string, both access same backend data |
