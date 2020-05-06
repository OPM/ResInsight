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

#include "cvfObject.h"
#include <QString>

#include <vector>

#include "cvfColor3.h"

class RigFormationNames : public cvf::Object
{
public:
    RigFormationNames();
    ~RigFormationNames() override;

    int formationIndexFromKLayerIdx( size_t Kidx ) const
    {
        if ( Kidx >= m_nameIndexPrKLayer.size() ) return -1;
        return m_nameIndexPrKLayer[Kidx];
    }

    QString formationNameFromKLayerIdx( size_t Kidx );

    bool formationColorFromKLayerIdx( size_t Kidx, cvf::Color3f* formationColor );

    const std::vector<QString>&      formationNames() const { return m_formationNames; }
    const std::vector<cvf::Color3f>& formationColors() const { return m_formationColors; }

    void appendFormationRange( const QString& name, int kStartIdx, int kEndIdx );
    void appendFormationRangeHeight( const QString& name, int kLayerCount );

    void appendFormationRange( const QString& name, cvf::Color3f color, int kStartIdx, int kEndIdx );
    void appendFormationRangeHeight( const QString& name, cvf::Color3f color, int kLayerCount );

private:
    static cvf::Color3f undefinedColor();
    void appendFormationRangeWithColor( const QString& name, cvf::Color3f color, int kStartIdx, int kEndIdx );

private:
    std::vector<int>          m_nameIndexPrKLayer;
    std::vector<QString>      m_formationNames;
    std::vector<cvf::Color3f> m_formationColors; // optional color per formation
};
