/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RigFaciesProperties
{
public:
    RigFaciesProperties( const QString& fieldName, const QString& formationName, const QString& faciesName );
    const QString& fieldName() const;
    const QString& formationName() const;
    const QString& faciesName() const;

    void appendValues( double porosity, double youngsModulus, double poissonsRatio, double m_K_Ic, double proppantEmbedment );
    double getYoungsModulus( double porosity ) const;
    double getPoissonsRatio( double porosity ) const;
    double getK_Ic( double porosity ) const;
    double getProppantEmbedment( double porosity ) const;

    const std::vector<double>& porosity() const;
    const std::vector<double>& youngsModulus() const;
    const std::vector<double>& poissonsRatio() const;
    const std::vector<double>& K_Ic() const;
    const std::vector<double>& proppantEmbedment() const;

private:
    QString m_fieldName;
    QString m_formationName;
    QString m_faciesName;

    std::vector<double> m_porosity;
    std::vector<double> m_youngsModulus;
    std::vector<double> m_poissonsRatio;
    std::vector<double> m_K_Ic;
    std::vector<double> m_proppantEmbedment;
};
