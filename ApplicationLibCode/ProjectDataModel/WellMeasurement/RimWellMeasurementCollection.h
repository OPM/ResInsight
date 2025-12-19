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

#include "RimWellMeasurement.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmObjectCollection.h"

#include <set>

class RimWellMeasurementFilePath;

//==================================================================================================
///
//==================================================================================================
class RimWellMeasurementCollection : public caf::PdmObjectCollection<RimWellMeasurement>
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellMeasurementCollection();
    ~RimWellMeasurementCollection() override;

    // Convenience accessor (delegates to base class items())
    std::vector<RimWellMeasurement*> measurements() const;

    // Domain-specific methods
    void updateAllCurves();
    void deleteAllEmptyCurves();

    std::set<QString> importedFiles() const;

    void addFilePath( const QString& filePath );
    void removeFilePath( RimWellMeasurementFilePath* measurementFilePath );
    void removeMeasurementsForFilePath( RimWellMeasurementFilePath* measurementFilePath );

protected:
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

private:
    caf::PdmChildArrayField<RimWellMeasurementFilePath*> m_importedFiles;
};
