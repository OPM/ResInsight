/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021    Equinor ASA
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

#include <list>

#include <QString>

class RimFaultRAPostprocSettings
{
public:
    RimFaultRAPostprocSettings();
    ~RimFaultRAPostprocSettings();

    void setBaseDirectory( QString baseDir );
    void setStepsToLoad( std::list<int> steps );

    std::list<int>& stepsToLoad();

    QString postprocParameterFilename() const;
    QString databaseDirectory() const;
    QString macrisCalcCalibPath() const;
    QString macrisCalcPath() const;

protected:
    QString        m_baseDir;
    std::list<int> m_steps;
};
