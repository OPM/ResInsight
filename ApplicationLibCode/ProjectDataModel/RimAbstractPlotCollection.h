/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-  Equinor ASA
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

#include "RimPlot.h"

#include "cafPdmObject.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RimPlotCollection
{
public:
    virtual ~RimPlotCollection() = default;

    virtual void   loadDataAndUpdateAllPlots() = 0;
    virtual size_t plotCount() const           = 0;
    virtual void   deleteAllPlots()            = 0;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RimAbstractPlotCollection : public RimPlotCollection
{
public:
    ~RimAbstractPlotCollection() override = default;

    virtual void removeRimPlot( RimPlot* plot ) = 0;
};

//==================================================================================================
/// Templated Base class of all plot collections.
/// Specialize on plot type (has to derive from RimPlot) and the base class of the collection.
//==================================================================================================
template <typename RimPlotType>
class RimTypedPlotCollection : public RimAbstractPlotCollection
{
    static_assert( std::is_base_of<RimPlot, RimPlotType>::value, "RimPlotType must inherit from RimPlot" );

public:
    ~RimTypedPlotCollection() override = default;

    virtual std::vector<RimPlotType*> plots() const = 0;

    void deleteAllPlots() override
    {
        for ( auto plot : plots() )
        {
            removePlot( plot );
            delete plot;
        }
    }

    void         addPlot( RimPlotType* plot ) { insertPlot( plot, plotCount() ); }
    virtual void insertPlot( RimPlotType* plot, size_t index ) = 0;
    virtual void removePlot( RimPlotType* plot )               = 0;
    void         removeRimPlot( RimPlot* rimPlot ) override
    {
        auto typedPlot = dynamic_cast<RimPlotType*>( rimPlot );
        if ( typedPlot )
        {
            removePlot( typedPlot );
        }
    }

    void loadDataAndUpdateAllPlots() override
    {
        for ( auto plot : plots() )
        {
            plot->loadDataAndUpdate();
        }
    }
};
