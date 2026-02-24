/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022 Equinor ASA
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

//==================================================================================================
//
//
//
//==================================================================================================
class RiaNetworkTools
{
public:
    // clang-format off
    static constexpr auto RIA_URL_BASE                   = "https://resinsight.org";
    static constexpr auto RIA_URL_FALLBACK_BASE          = "https://opm.github.io/ResInsight-UserDocumentation";
    static constexpr auto RIA_URL_SEARCH                 = "https://resinsight.org/search/?q=";
    static constexpr auto RIA_URL_USERS_GUIDE            = "https://resinsight.org/getting-started/overview/";
    static constexpr auto RIA_URL_RELEASE_NOTES          = "https://resinsight.org/releases/release-notes/latest/";
    static constexpr auto RIA_URL_ISSUES                 = "https://github.com/OPM/ResInsight/issues";
    static constexpr auto RIA_URL_NEW_ISSUE              = "https://github.com/OPM/ResInsight/issues/new";
    static constexpr auto RIA_URL_CALCULATOR_EXPRESSIONS = "https://resinsight.org/calculated-data/calculatorexpressions/";
    static constexpr auto RIA_URL_SCRIPTING_COMMAND_FILE = "https://resinsight.org/scripting/commandfile/";
    // clang-format on

    static void openSearchPage( const QString& searchText );
    static void openUrlWithErrorReporting( const QString& urlString );
    static void openUrl( const QString& urlString );
    static void createAndOpenUrlWithFallback( const QString& urlSubString );
    static void openUrlWithFallback( const QStringList& urlList );

private:
    static bool doesResourceExist( const QString& urlString );
};
