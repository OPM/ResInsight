/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Ceetron Solutions AS
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

#include "cafAppEnum.h"

#include "RigFemResultAddress.h"

#include <QString>

#include <set>

class RigWbsParameter
{
    struct SourceAddress;

public:
    enum Source
    {
        GRID = 0,
        LAS_FILE,
        ELEMENT_PROPERTY_TABLE,
        USER_DEFINED,
        HYDROSTATIC,
        MATTHEWS_KELLY, // FG in shale
        PROPORTIONAL_TO_SH, // FG in shale
        INVALID = 1000,
    };
    using SourceEnum   = caf::AppEnum<Source>;
    using SourceVector = std::vector<std::pair<Source, SourceAddress>>;

public:
    RigWbsParameter( const QString&      name                     = "",
                     bool                normalizeByHydroStaticPP = false,
                     const SourceVector& validSources             = {} );

    const QString&      name() const;
    std::vector<Source> sources() const;
    QString             addressString( Source source ) const;
    RigFemResultAddress femAddress( Source source ) const;
    bool                normalizeByHydrostaticPP() const;
    bool                exclusiveOptions() const;
    void                setOptionsExclusive( bool exclusive );

    std::vector<QString> allSourceLabels( const QString& delimiter        = " ",
                                          double         userDefinedValue = std::numeric_limits<double>::infinity() );
    QString              sourceLabel( Source         currentSource,
                                      const QString& delimiter        = " ",
                                      double         userDefinedValue = std::numeric_limits<double>::infinity() );

    bool operator==( const RigWbsParameter& rhs ) const;
    bool operator<( const RigWbsParameter& rhs ) const;

    static RigWbsParameter PP_Sand();
    static RigWbsParameter PP_Shale();
    static RigWbsParameter poissonRatio();
    static RigWbsParameter UCS();
    static RigWbsParameter OBG();
    static RigWbsParameter OBG0();
    static RigWbsParameter SH();
    static RigWbsParameter DF();
    static RigWbsParameter K0_FG();
    static RigWbsParameter K0_SH();
    static RigWbsParameter FG_Shale();

    static std::set<RigWbsParameter> allParameters();
    static bool                      findParameter( QString parameterName, RigWbsParameter* foundParam = nullptr );

private:
    struct SourceAddress
    {
        RigFemResultPosEnum resType;
        QString             primary; // i.e. grid field name, las entry, etc.
        QString             secondary; // i.e. grid component name
        SourceAddress( QString primary = "", QString secondary = "" )
            : primary( primary )
            , secondary( secondary )
        {
        }
    };

    SourceAddress address( Source source ) const;

private:
    QString                                       m_name;
    std::vector<std::pair<Source, SourceAddress>> m_sources;
    bool                                          m_normalizeByHydroStaticPP;
    bool m_exclusiveOptions; // Options are exclusive rather than order of preference
};
