/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021  Equinor ASA
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

#include <list>

class RimFaultRASettings;
class RimGenericParameter;

class RifFaultRAXmlWriter
{
public:
    RifFaultRAXmlWriter( RimFaultRASettings* settings );
    ~RifFaultRAXmlWriter();

    bool writeCalculateFile( QString filename, int faultID, QString& outErrorText );
    bool writeCalibrateFile( QString filename, int faultID, QString& outErrorText );

private:
    bool writeParametersToXML( QString filename, std::list<RimGenericParameter*>& params, QString& outErrorText );

    RimFaultRASettings* m_settings;
};
