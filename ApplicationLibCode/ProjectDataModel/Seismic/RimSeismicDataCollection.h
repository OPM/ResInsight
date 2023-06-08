/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "cafPdmField.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmObject.h"

#include <QString>

class RimSeismicDataInterface;
class RimSeismicData;
class RimSeismicDifferenceData;

class RimSeismicDataCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimSeismicDataCollection();
    ~RimSeismicDataCollection() override;

    RimSeismicDataInterface* importSeismicFromFile( const QString file );
    RimSeismicDataInterface* createDifferenceSeismicData( RimSeismicData* data1, RimSeismicData* data2 );

    bool isEmpty();

    std::vector<RimSeismicData*>           seismicData() const;
    std::vector<RimSeismicDifferenceData*> differenceData() const;

protected:
    void onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects ) override;
    void updateViews();
    void updateTreeForAllViews();

private:
    caf::PdmChildArrayField<RimSeismicData*>           m_seismicData;
    caf::PdmChildArrayField<RimSeismicDifferenceData*> m_differenceData;
};
