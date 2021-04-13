
#include "RiaPreferencesSummary.h"
#include "cafPdmUiCheckBoxEditor.h"

// #include "RiaColorTables.h"
// #include "RiaValidRegExpValidator.h"
// #include "RifReaderSettings.h"
// #include "RiuGuiTheme.h"
//
// #include "cafPdmFieldCvfColor.h"
// #include "cafPdmSettings.h"
// #include "cafPdmUiCheckBoxEditor.h"
// #include "cafPdmUiComboBoxEditor.h"
// #include "cafPdmUiFieldHandle.h"
// #include "cafPdmUiFilePathEditor.h"
// #include "cafPdmUiLineEditor.h"
//
// #include <QDate>
// #include <QDir>
// #include <QLocale>
// #include <QRegExp>
// #include <QStandardPaths>

namespace caf
{
/*
    template <>
    void RiaPreferences::SummaryRestartFilesImportModeType::setUp()
    {
        addItem(RiaPreferences::SummaryRestartFilesImportMode::IMPORT, "IMPORT", "Unified");
        addItem(RiaPreferences::SummaryRestartFilesImportMode::SEPARATE_CASES, "SEPARATE_CASES", "Separate Cases");
        addItem(RiaPreferences::SummaryRestartFilesImportMode::NOT_IMPORT, "NOT_IMPORT", "Skip");
        setDefault(RiaPreferences::SummaryRestartFilesImportMode::IMPORT);
    }

    template <>
    void RiaPreferences::SummaryHistoryCurveStyleModeType::setUp()
    {
        addItem(RiaPreferences::SummaryHistoryCurveStyleMode::SYMBOLS, "SYMBOLS", "Symbols");
        addItem(RiaPreferences::SummaryHistoryCurveStyleMode::LINES, "LINES", "Lines");
        addItem(RiaPreferences::SummaryHistoryCurveStyleMode::SYMBOLS_AND_LINES, "SYMBOLS_AND_LINES", "Symbols and
   Lines"); setDefault(RiaPreferences::SummaryHistoryCurveStyleMode::SYMBOLS);
    }

    template <>
    void RiaPreferences::PageSizeEnum::setUp()
    {
        addItem(QPageSize::A3, "A3", "A3");
        addItem(QPageSize::A4, "A4", "A4");
        addItem(QPageSize::A5, "A5", "A5");
        addItem(QPageSize::A6, "A6", "A6");
        addItem(QPageSize::Letter, "LETTER", "US Letter");
        addItem(QPageSize::Legal, "LEGAL", "US Legal");
        addItem(QPageSize::Ledger, "LEDGER", "US Ledger");
        addItem(QPageSize::Tabloid, "TABLOID", "US Tabloid");
        setDefault(QPageSize::A4);
    }

    template <>
    void RiaPreferences::PageOrientationEnum::setUp()
    {
        addItem(QPageLayout::Portrait, "PORTRAIT", "Portrait");
        addItem(QPageLayout::Landscape, "LANDSCAPE", "Landscape");
        setDefault(QPageLayout::Portrait);
    }
*/

template <>
void RiaPreferencesSummary::SummaryReaderModeType::setUp()
{
    addItem( RiaPreferencesSummary::SummaryReaderMode::LIBECL, "LIBECL", "Default Reader (ecl)" );
    addItem( RiaPreferencesSummary::SummaryReaderMode::HDF5_OPM_COMMON, "HDF5_OPM_COMMON", "[BETA] H5 Reader (HDF5 Eclipse)" );
    addItem( RiaPreferencesSummary::SummaryReaderMode::OPM_COMMON, "OPM_COMMON", "[BETA] Performance Reader (omp-common)" );
    setDefault( RiaPreferencesSummary::SummaryReaderMode::LIBECL );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RiaPreferencesSummary, "RiaPreferencesSummary" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesSummary::RiaPreferencesSummary()
{
    CAF_PDM_InitField( &m_createOptimizedSummaryDataFile,
                       "createOptimizedSummaryDataFile",
                       true,
                       "Create Optimized Summary Data Files [BETA]",
                       "",
                       "If not present, create optimized file with extension '*.LODSMRY'",
                       "" );
    m_createOptimizedSummaryDataFile.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitField( &m_useOptimizedSummaryDataFile,
                       "useOptimizedSummaryDataFile",
                       true,
                       "Use Optimized Summary Data Files [BETA]",
                       "",
                       "If not present, read optimized file with extension '*.LODSMRY'",
                       "" );
    m_useOptimizedSummaryDataFile.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitField( &m_createH5SummaryDataFile,
                       "createH5SummaryDataFile",
                       false,
                       "Create H5 Summary Data Files [BETA]",
                       "",
                       "If not present, create summary file with extension '*.H5'",
                       "" );
    m_createH5SummaryDataFile.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_summaryReader, "summaryReaderType", "Summary Data File Reader", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesSummary::SummaryReaderMode RiaPreferencesSummary::summaryDataReader() const
{
    return m_summaryReader();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesSummary::useOptimizedSummaryDataFiles() const
{
    return m_useOptimizedSummaryDataFile();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesSummary::createOptimizedSummaryDataFiles() const
{
    return m_createOptimizedSummaryDataFile();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesSummary::createH5SummaryDataFiles() const
{
    return m_createH5SummaryDataFile();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferencesSummary::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                   QString                    uiConfigName,
                                                   caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_createOptimizedSummaryDataFile || field == &m_useOptimizedSummaryDataFile ||
         field == &m_createH5SummaryDataFile )
    {
        auto myAttr = dynamic_cast<caf::PdmUiCheckBoxEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->m_useNativeCheckBoxLabel = true;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferencesSummary::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_summaryReader );

    if ( m_summaryReader == SummaryReaderMode::OPM_COMMON )
    {
        uiOrdering.add( &m_createOptimizedSummaryDataFile );
        uiOrdering.add( &m_useOptimizedSummaryDataFile );
    }
    else if ( m_summaryReader == SummaryReaderMode::HDF5_OPM_COMMON )
    {
        uiOrdering.add( &m_createH5SummaryDataFile );
    }

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RiaPreferencesSummary::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;
    *useOptionsOnly = true;

    if ( fieldNeedingOptions == &m_summaryReader )
    {
        std::vector<SummaryReaderMode> availableModes;

        availableModes.push_back( SummaryReaderMode::LIBECL );
#ifdef USE_HDF5
        availableModes.push_back( SummaryReaderMode::HDF5_OPM_COMMON );
#endif // USE_HDF5
        availableModes.push_back( SummaryReaderMode::OPM_COMMON );

        for ( auto enumValue : availableModes )
        {
            options.push_back( caf::PdmOptionItemInfo( SummaryReaderModeType::uiText( enumValue ), enumValue ) );
        }
    }
    return options;
}
