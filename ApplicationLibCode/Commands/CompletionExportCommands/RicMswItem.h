/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021- Equinor ASA
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

class RicMswItem
{
public:
    RicMswItem( const QString& label );
    virtual ~RicMswItem() = default;

    QString label() const;
    void    setLabel( const QString& label );

    virtual double startMD() const = 0;
    virtual double endMD() const   = 0;
    double         deltaMD() const;

    virtual double startTVD() const = 0;
    virtual double endTVD() const   = 0;
    double         deltaTVD() const;

    bool operator<( const RicMswItem& rhs ) const;

protected:
    QString m_label;
};
