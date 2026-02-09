/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RiaDefines.h"
#include "RiaWellLogTrackDefines.h"

#include "cvfMath.h"
#include "cvfObject.h"

#include <map>
#include <utility>
#include <vector>

#include <QString>

struct RigWellPathFormation
{
    double  mdTop{ 0.0 };
    double  mdBase{ 0.0 };
    double  tvdTop{ 0.0 };
    double  tvdBase{ 0.0 };
    QString formationName;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RigWellPathFormations : public cvf::Object
{
public:
    using FormationLevel = RiaDefines::WellLogTrackFormationLevel;

public:
    RigWellPathFormations( const std::vector<RigWellPathFormation>& formations, const QString& filePath, const QString& key );

    void depthAndFormationNamesUpToLevel( FormationLevel        level,
                                          std::vector<QString>* names,
                                          std::vector<double>*  depths,
                                          bool                  includeFluids,
                                          RiaDefines::DepthType depthType ) const;

    std::vector<FormationLevel> formationsLevelsPresent() const;

    QString filePath() const;
    QString keyInFile() const;

    size_t formationNamesCount() const;

private:
    struct DepthComp
    {
        bool operator()( const double& depth1, const double& depth2 ) const
        {
            if ( cvf::Math::abs( depth1 - depth2 ) < 0.1 )
            {
                return false;
            }
            return depth1 < depth2;
        }
    };

    struct LevelAndName
    {
        LevelAndName() = default;
        LevelAndName( FormationLevel level, QString name )
            : level( level )
            , name( name )
        {
        }

        FormationLevel level;
        QString        name;
    };

    enum PickPosition
    {
        TOP,
        BASE
    };

private:
    void evaluateFormations( const std::vector<std::pair<RigWellPathFormation, FormationLevel>>& formations,
                             const FormationLevel&                                               maxLevel,
                             std::vector<QString>*                                               names,
                             std::vector<double>*                                                depths,
                             RiaDefines::DepthType                                               depthType ) const;

    void evaluateFluids( const std::vector<RigWellPathFormation>& fluidFormations,
                         std::vector<QString>*                    names,
                         std::vector<double>*                     depths,
                         RiaDefines::DepthType                    depthType ) const;

    void evaluateFormationsForOnePosition( const std::vector<std::pair<RigWellPathFormation, FormationLevel>>& formations,
                                           const FormationLevel&                                               maxLevel,
                                           const PickPosition&                                                 position,
                                           std::map<double, LevelAndName, DepthComp>*                          uniqueListMaker,
                                           RiaDefines::DepthType                                               depthType ) const;

    void depthAndFormationNamesWithoutDuplicatesOnDepth( std::vector<QString>* names,
                                                         std::vector<double>*  measuredDepths,
                                                         RiaDefines::DepthType depthType ) const;

    bool           isFluid( QString formationName );
    FormationLevel detectLevel( QString formationName );

private:
    QString m_filePath;
    QString m_keyInFile;

    std::map<FormationLevel, bool> m_formationsLevelsPresent;

    std::vector<std::pair<RigWellPathFormation, FormationLevel>> m_formations;
    std::vector<RigWellPathFormation>                            m_fluids;
};
