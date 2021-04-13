#pragma once

// #include "RiaDefines.h"
// #include "RiaFontCache.h"
// #include "RiaGuiApplication.h"
// #include "RiaQDateTimeTools.h"
//
// #include "cafAppEnum.h"
// #include "cafPdmChildField.h"
// #include "cafPdmField.h"
// #include "cafPdmObject.h"
//
// // Include to make Pdm work for cvf::Color
// #include "cafPdmFieldCvfColor.h"
//
// #include <QPageLayout>
// #include <QPageSize>
// #include <QStringList>
//
// #include <map>
//
// class RifReaderSettings;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaPreferencesSummary : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum class SummaryReaderMode
    {
        LIBECL,
        OPM_COMMON,
        HDF5_OPM_COMMON
    };
    typedef caf::AppEnum<SummaryReaderMode> SummaryReaderModeType;

public:
    RiaPreferencesSummary();

    SummaryReaderMode summaryDataReader() const;
    bool              useOptimizedSummaryDataFiles() const;
    bool              createOptimizedSummaryDataFiles() const;
    bool              createH5SummaryDataFiles() const;

protected:
    void                          defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                         QString                    uiConfigName,
                                                         caf::PdmUiEditorAttribute* attribute ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

private:
    caf::PdmField<bool> m_createOptimizedSummaryDataFile;
    caf::PdmField<bool> m_useOptimizedSummaryDataFile;

    caf::PdmField<bool> m_createH5SummaryDataFile;

    caf::PdmField<SummaryReaderModeType> m_summaryReader;
};
