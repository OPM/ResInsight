/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

class RimOpmFlowJobFileHelper
{
public:
    RimOpmFlowJobFileHelper();
    ~RimOpmFlowJobFileHelper();

    void setWorkingDirectory( QString workingDir );
    void setName( QString name );
    void setUseEnsembleNames( int currentEnsembleId );

    QString deckName() const;
    QString workingDirectory() const;
    QString deckExtension() const;
    QString baseDeckName() const;
    QString restartDeckName( QString inputDeckName ) const;
    QString fluxnumFileName( bool relativePath = true ) const;
    QString fluxFileName() const;

private:
    QString m_workingDirectory;
    QString m_name;
    int     m_currentEnsembleId;
};
