/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaPreferencesSystem : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum class EclipseTextFileReaderMode
    {
        MEMORY_MAPPED_FILE,
        FILE,
    };
    using EclipseTextFileReaderModeType = caf::AppEnum<EclipseTextFileReaderMode>;

public:
    RiaPreferencesSystem();

    static RiaPreferencesSystem* current();

    bool    appendClassNameToUiText() const;
    bool    appendFieldKeywordToToolTipText() const;
    bool    showViewIdInProjectTree() const;
    bool    showTestToolbar() const;
    bool    includeFractureDebugInfoFile() const;
    bool    showProjectChangedDialog() const;
    QString holoLensExportFolder() const;
    bool    useShaders() const;
    bool    show3dInformation() const;
    QString gtestFilter() const;
    bool    showProgressBar() const;

    EclipseTextFileReaderMode eclipseTextFileReaderMode() const;

protected:
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    void                          defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                         QString                    uiConfigName,
                                                         caf::PdmUiEditorAttribute* attribute ) override;

private:
    caf::PdmField<bool> m_appendClassNameToUiText;
    caf::PdmField<bool> m_appendFieldKeywordToToolTipText;
    caf::PdmField<bool> m_showViewIdInProjectTree;
    caf::PdmField<bool> m_useShaders;
    caf::PdmField<bool> m_showHud;

    caf::PdmField<bool> m_showProjectChangedDialog;

    caf::PdmField<bool>    m_showTestToolbar;
    caf::PdmField<bool>    m_includeFractureDebugInfoFile;
    caf::PdmField<QString> m_holoLensExportFolder;

    caf::PdmField<bool>    m_showProgressBar;
    caf::PdmField<QString> m_gtestFilter;

    caf::PdmField<EclipseTextFileReaderModeType> m_eclipseReaderMode;
};
