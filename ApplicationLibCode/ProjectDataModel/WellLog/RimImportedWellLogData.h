/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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
#include "cvfObject.h"

#include <QString>
#include <vector>

class RigImportedWellLogData;
class RimWellLogChannelData;

//==================================================================================================
/// PDM class for storing imported well log data that can be persisted to project files
//==================================================================================================
class RimImportedWellLogData : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimImportedWellLogData();

    // Setters for data
    void setDepthValues( const std::vector<double>& depths );
    void setTvdMslValues( const std::vector<double>& tvdMsl );
    void setTvdRkbValues( const std::vector<double>& tvdRkb );
    void setChannelData( const QString& name, const std::vector<double>& values );

    // Getters for data
    std::vector<double>  depthValues() const;
    std::vector<double>  tvdMslValues() const;
    std::vector<double>  tvdRkbValues() const;
    std::vector<QString> channelNames() const;
    std::vector<double>  channelValues( const QString& name ) const;

    bool hasTvdMslValues() const;
    bool hasTvdRkbValues() const;

    // Method to produce RigImportedWellLogData from PDM data
    cvf::ref<RigImportedWellLogData> createRigData() const;

private:
    caf::PdmField<std::vector<double>>              m_depthValues;
    caf::PdmField<std::vector<double>>              m_tvdMslValues;
    caf::PdmField<std::vector<double>>              m_tvdRkbValues;
    caf::PdmChildArrayField<RimWellLogChannelData*> m_channelDataObjects;
};