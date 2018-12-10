/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicfCommandObject.h"

#include "cafPdmField.h"

//==================================================================================================
// RicfSingleCaseReplace represents the parsed command from text file able to replace one case ID with
// a new file name
//
// This is the preferred interface on file, based on discussion with @hhgs 2018-02-02
//
// Multiple objects of this type can be aggregated into RicfMultipleReplaceCase
//==================================================================================================
class RicfSingleCaseReplace : public RicfCommandObject
{
    CAF_PDM_HEADER_INIT;

public:
    RicfSingleCaseReplace();

    int     caseId() const;
    QString filePath() const;

    void execute() override;

private:
    caf::PdmField<QString> m_newGridFile;
    caf::PdmField<int>     m_caseId;
};

//==================================================================================================
// RicfMultipleReplaceCase represents multiple caseId-gridFileName pairs
//
// NB!  This object has no support for parsing a text command. This object is created by aggregating
//      multiple RicfSingleCaseReplace objects
//
//==================================================================================================
class RicfMultiCaseReplace : public RicfCommandObject
{
    CAF_PDM_HEADER_INIT;

public:
    RicfMultiCaseReplace();

    void setCaseReplacePairs(const std::map<int, QString>& caseIdToGridFileNameMap);

    void execute() override;

private:
    std::map<int, QString> m_caseIdToGridFileNameMap;
};
