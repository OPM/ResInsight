/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"

class RimWellMeasurement;
class RimWellMeasurementFilePath;

//==================================================================================================
///
//==================================================================================================
class RimWellMeasurementCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellMeasurementCollection();
    ~RimWellMeasurementCollection() override;

    std::vector<RimWellMeasurement*> measurements() const;

    void updateAllCurves();
    void deleteAllEmptyCurves();
    void insertMeasurement( RimWellMeasurement* insertBefore, RimWellMeasurement* measurement );
    void appendMeasurement( RimWellMeasurement* measurement );
    void deleteMeasurement( RimWellMeasurement* measurementToDelete );
    void deleteAllMeasurements();
    bool isEmpty() const;

    std::set<QString> importedFiles() const;

    void addFilePath( const QString& filePath );
    void removeFilePath( RimWellMeasurementFilePath* measurementFilePath );
    void removeMeasurementsForFilePath( RimWellMeasurementFilePath* measurementFilePath );

protected:
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

private:
    caf::PdmChildArrayField<RimWellMeasurement*>         m_measurements;
    caf::PdmChildArrayField<RimWellMeasurementFilePath*> m_importedFiles;
};
