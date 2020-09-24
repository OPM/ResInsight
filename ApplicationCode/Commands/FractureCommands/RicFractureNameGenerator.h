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

#include "RimOilField.h"
#include "RimProject.h"

#include <QString>

//==================================================================================================
///
//==================================================================================================
class RicFractureNameGenerator
{
public:
    static QString nameForNewFracture();
    static QString nameForNewFractureModel();
    static QString nameForNewFractureModelTemplate();

private:
    template <typename T>
    static QString nameForNewObject( const QString& namePattern )
    {
        std::vector<T*> oldObjects;
        RimProject::current()->activeOilField()->descendantsIncludingThisOfType( oldObjects );

        size_t objectNum = oldObjects.size();

        bool    found;
        QString name;

        do
        {
            found = false;
            name  = QString( namePattern ).arg( objectNum, 2, 10, QChar( '0' ) );
            for ( T* object : oldObjects )
            {
                if ( object->name() == name )
                {
                    found = true;
                    break;
                }
            }

            objectNum++;
        } while ( found );

        return name;
    }
};
