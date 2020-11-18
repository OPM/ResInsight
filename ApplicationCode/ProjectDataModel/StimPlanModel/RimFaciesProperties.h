/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "cafFilePath.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include <QString>

class RimEclipseResultDefinition;
class RimColorLegend;
class RimEclipseCase;

//==================================================================================================
///
//==================================================================================================
class RimFaciesProperties : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimFaciesProperties();
    ~RimFaciesProperties() override;

    caf::Signal<> changed;

    QString filePath() const;
    void    setFilePath( const QString& filePath );

    void setFaciesCodeName( int code, const QString& name );
    void clearFaciesCodeNames();

    void setEclipseCase( RimEclipseCase* eclipseCase );

    void loadDataAndUpdate();

    RimColorLegend* colorLegend() const;
    void            setColorLegend( RimColorLegend* colorLegend );

    const RimEclipseResultDefinition* faciesDefinition() const;

protected:
    void                          defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                         QString                    uiConfigName,
                                                         caf::PdmUiEditorAttribute* attribute ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering );
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    QString generatePropertiesTable();

    caf::PdmField<caf::FilePath>                    m_filePath;
    caf::PdmField<QString>                          m_propertiesTable;
    caf::PdmChildField<RimEclipseResultDefinition*> m_faciesDefinition;
    caf::PdmPtrField<RimColorLegend*>               m_colorLegend;

    std::map<int, QString> m_faciesCodeNames;
};
