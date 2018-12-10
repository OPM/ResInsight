/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
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

#include "cvfBase.h"
#include "cvfObject.h"
#include <QString>

#include <vector>

class RigFormationNames: public cvf::Object
{
public:
    RigFormationNames();
    ~RigFormationNames() override;

    int formationIndexFromKLayerIdx(size_t Kidx) 
    {
        if(Kidx >= m_nameIndexPrKLayer.size()) return -1;
        return m_nameIndexPrKLayer[Kidx];
    }

    QString formationNameFromKLayerIdx(size_t Kidx);

    const std::vector<QString>& formationNames() const { return m_formationNames;}

    void appendFormationRange(const QString& name, int kStartIdx, int kEndIdx);
    void appendFormationRangeHeight(const QString& name, int kLayerCount);

private:

    std::vector<int> m_nameIndexPrKLayer;
    std::vector<QString> m_formationNames;
};


