/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024- Equinor ASA
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

class QOAuth2AuthorizationCodeFlow;

namespace RiaConnectorTools
{
QString tokenDataAsJson( QOAuth2AuthorizationCodeFlow* authCodeFlow );
void    initializeTokenDataFromJson( QOAuth2AuthorizationCodeFlow* authCodeFlow, const QString& tokenDataJson );
void    writeTokenData( const QString& filePath, const QString& tokenDataJson );
QString readTokenData( const QString& filePath );
} // namespace RiaConnectorTools
