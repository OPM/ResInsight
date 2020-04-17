/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor AS
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

#include "RiaWellLogUnitTools.h"
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
        DERIVED_FROM_K0FG, // FG in shale
        PROPORTIONAL_TO_SH, // FG in shale
        UNDEFINED = 1000,
    };
    using SourceEnum   = caf::AppEnum<Source>;
    using SourceVector = std::vector<std::pair<Source, SourceAddress>>;

public:
    RigWbsParameter( const QString&      name                     = "",
                     bool                normalizeByHydroStaticPP = false,
                     const SourceVector& validSources             = {},
                     bool                exclusiveOptions         = false );
    RigWbsParameter( const RigWbsParameter& rhs );

    RigWbsParameter& operator=( const RigWbsParameter& rhs );

    const QString&      name() const;
    std::vector<Source> sources() const;
    bool                hasExternalSource() const;
    QString             addressString( Source source ) const;
    QString             units( Source source ) const;
    RigFemResultAddress femAddress( Source source ) const;
    bool                normalizeByHydrostaticPP() const;
    bool                exclusiveOptions() const;
    void                setOptionsExclusive( bool exclusive );

    std::vector<QString> allSourceUiLabels( const QString& delimiter        = " ",
                                            double         userDefinedValue = std::numeric_limits<double>::infinity() );
    QString              sourceUiLabel( Source         currentSource,
                                        const QString& delimiter        = " ",
                                        double         userDefinedValue = std::numeric_limits<double>::infinity() );

    bool operator==( const RigWbsParameter& rhs ) const;
    bool operator<( const RigWbsParameter& rhs ) const;

    static RigWbsParameter PP_Reservoir();
    static RigWbsParameter PP_NonReservoir();
    static RigWbsParameter poissonRatio();
    static RigWbsParameter UCS();
    static RigWbsParameter OBG();
    static RigWbsParameter OBG0();
    static RigWbsParameter SH();
    static RigWbsParameter DF();
    static RigWbsParameter K0_FG();
    static RigWbsParameter K0_SH();
    static RigWbsParameter FG_Shale();
    static RigWbsParameter waterDensity();

    static std::set<RigWbsParameter> allParameters();
    static bool                      findParameter( QString parameterName, RigWbsParameter* foundParam = nullptr );

private:
    struct SourceAddress
    {
        RigFemResultPosEnum resType;
        QString             primary; // i.e. grid field name, las entry, etc.
        QString             secondary; // i.e. grid component name
        QString             units; // The unit string
        SourceAddress( QString primary   = "",
                       QString secondary = "",
                       QString units     = RiaWellLogUnitTools<double>::noUnitString() )
            : primary( primary )
            , secondary( secondary )
            , units( units )
        {
        }
    };

    bool address( Source source, SourceAddress* sourceAddress ) const;

private:
    QString                                       m_name;
    std::vector<std::pair<Source, SourceAddress>> m_sources;
    bool                                          m_normalizeByHydroStaticPP;
    bool m_exclusiveOptions; // Options are exclusive rather than order of preference
};
