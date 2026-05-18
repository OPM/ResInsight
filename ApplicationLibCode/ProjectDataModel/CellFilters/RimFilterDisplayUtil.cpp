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

#include "RimFilterDisplayUtil.h"

#include "RimCellFilter.h"

#include <QStringList>

QString RimFilterDisplayUtil::filterNamesJoined( const std::vector<RimCellFilter*>& filters, bool useAndOperation )
{
    QStringList names;
    for ( auto* f : filters )
    {
        if ( f && f->isFilterEnabled() ) names << f->name();
    }
    if ( names.isEmpty() ) return {};

    const QString sep = useAndOperation ? QStringLiteral( " AND " ) : QStringLiteral( " OR " );
    return names.join( sep );
}
