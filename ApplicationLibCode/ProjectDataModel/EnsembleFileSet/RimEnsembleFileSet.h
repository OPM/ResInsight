/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RimNamedObject.h"

#include "Summary/RiaSummaryDefines.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"

//==================================================================================================
///
///
//==================================================================================================
class RimEnsembleFileSet : public RimNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    caf::Signal<> fileSetChanged;
    caf::Signal<> nameChanged;

public:
    RimEnsembleFileSet();

    QStringList createPaths( const QString& extension ) const;
    void        findAndSetPathPatternAndRangeString( const QStringList& filePaths );

    void                                setNameTemplate( const QString& name );
    void                                updateName( const std::set<QString>& existingEnsembleNames );
    void                                setUsePathKey1( bool useKey1 );
    void                                setUsePathKey2( bool useKey2 );
    std::pair<std::string, std::string> nameKeys() const;
    QString                             nameTemplateText() const;

    void setGroupingMode( RiaDefines::EnsembleGroupingMode groupingMode );

private:
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;

    void setPathPattern( const QString& pathPattern );
    void setRangeString( const QString& rangeString );

private:
    caf::PdmField<QString> m_pathPattern;
    caf::PdmField<QString> m_realizationSubSet;

    caf::PdmField<QString> m_nameTemplateString;
    caf::PdmField<bool>    m_autoName;
    caf::PdmField<bool>    m_useKey1;
    caf::PdmField<bool>    m_useKey2;

    caf::PdmField<caf::AppEnum<RiaDefines::EnsembleGroupingMode>> m_groupingMode;
};
