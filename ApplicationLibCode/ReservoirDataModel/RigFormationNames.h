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

#include <QString>

#include <set>
#include <vector>

#include "cvfColor3.h"

class RigFormationNames
{
public:
    RigFormationNames();

    int     formationIndexFromKLayerIdx( size_t Kidx ) const;
    QString formationNameFromKLayerIdx( size_t Kidx ) const;
    bool    formationColorFromKLayerIdx( size_t Kidx, cvf::Color3f* formationColor ) const;

    std::vector<QString>      formationNames() const;
    std::vector<cvf::Color3f> formationColors() const;

    void appendFormationRange( const QString& name, int kStartIdx, int kEndIdx );
    void appendFormationRangeHeight( const QString& name, int kLayerCount );

    void appendFormationRange( const QString& name, cvf::Color3f color, int kStartIdx, int kEndIdx );
    void appendFormationRangeHeight( const QString& name, cvf::Color3f color, int kLayerCount );

    std::set<int> findKLayers( std::vector<QString> formationNames ) const;

private:
    static cvf::Color3f undefinedColor();
    void                appendFormationRangeWithColor( const QString& name, cvf::Color3f color, int kStartIdx, int kEndIdx );

private:
    std::vector<int>          m_nameIndexPrKLayer;
    std::vector<QString>      m_formationNames;
    std::vector<cvf::Color3f> m_formationColors; // optional color per formation
};
