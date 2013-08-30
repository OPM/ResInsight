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

#include "../ApplicationCode/SocketInterface/RiaSocketServer.h"

namespace riOctavePlugin
{
    const int connectTimeOutMilliSecs   =  5000;
    const int shortTimeOutMilliSecs     =  5000;
    const int longTimeOutMilliSecs      = 60000;

    // Octave data structure : CaseInfo
    char caseInfo_CaseId[]      = "CaseId";
    char caseInfo_CaseName[]    = "CaseName";
    char caseInfo_CaseType[]    = "CaseType";
    char caseInfo_CaseGroupId[] = "CaseGroupId";

    // Octave data structure:  PropertyInfo 
    char propertyInfo_PropName[]    = "PropName";
    char propertyInfo_PropType[]    = "PropType";

    // Octave data structure : CaseGroupInfo
    char caseGroupInfo_CaseGroupId[]      = "CaseGroupId";
    char caseGroupInfo_CaseGroupName[]    = "CaseName";

    // Octave data structure : TimeStepDate
    char timeStepDate_Year[]     = "Year";
    char timeStepDate_Month[]    = "Month";
    char timeStepDate_Day[]      = "Day";
    char timeStepDate_Hour[]     = "Hour";
    char timeStepDate_Minute[]   = "Minute";
    char timeStepDate_Second[]   = "Second";
}

