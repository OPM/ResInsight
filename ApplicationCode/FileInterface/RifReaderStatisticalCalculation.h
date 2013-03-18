/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RifReaderInterface.h"


class RifReaderStatisticalCalculation : public RifReaderInterface
{
public:
    RifReaderStatisticalCalculation();
    virtual ~RifReaderStatisticalCalculation();

    // Virtual interface implementation
    virtual bool        open(const QString& fileName, RigEclipseCase* eclipseCase);

    virtual void        close() {}

    virtual bool        staticResult(const QString& result, PorosityModelResultType matrixOrFracture, std::vector<double>* values );
    virtual bool        dynamicResult(const QString& result, PorosityModelResultType matrixOrFracture, size_t stepIndex, std::vector<double>* values );

    void                setMatrixResultNames(const QStringList& staticResultNames, const QStringList& dynamicResultNames);
    void                setFractureResultNames(const QStringList& staticResultNames, const QStringList& dynamicResultNames);

    void                setTimeSteps(const std::vector<QDateTime>& timesteps);

private:
    void                buildMetaData(RigEclipseCase* eclipseCase);

private:
    std::vector<QDateTime>    m_timeSteps;
    
    QStringList         m_matrixDynamicResultNames;
    QStringList         m_fractureDynamicResultNames;

    QStringList         m_matrixStaticResultNames;
    QStringList         m_fractureStaticResultNames;

};
