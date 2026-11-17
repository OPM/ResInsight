/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RiaReportTable.h"

#include <QString>

#include <vector>

//==================================================================================================
/// Capability mixin implemented by plots / plot windows that can provide structured report data
/// for the report window ("Show Plot Data")
//==================================================================================================
class RiaReportDataProducer
{
public:
    virtual ~RiaReportDataProducer() = default;

    virtual QString                       reportTitle() const    = 0;
    virtual std::vector<RiaReportSection> reportSections() const = 0;

    // Sections used when the producer is shown together with other plots, e.g. as part of a multi plot.
    // Override to reduce the number of sections, typically to one.
    virtual std::vector<RiaReportSection> compactReportSections() const { return reportSections(); }

    // If true, an open report window showing this producer is refreshed when the producer is updated
    virtual bool supportsLiveUpdate() const { return false; }

protected:
    // Combines the compact sections of child producers, used by plots containing other plots.
    // Section titles are prefixed with the child title when a child has more than one section.
    static std::vector<RiaReportSection> childReportSections( const std::vector<const RiaReportDataProducer*>& children )
    {
        std::vector<RiaReportSection> sections;
        for ( auto child : children )
        {
            if ( !child ) continue;

            const QString childTitle    = child->reportTitle();
            auto          childSections = child->compactReportSections();
            for ( auto& section : childSections )
            {
                if ( childSections.size() == 1 || section.title.isEmpty() )
                {
                    if ( !childTitle.isEmpty() ) section.title = childTitle;
                }
                else if ( !childTitle.isEmpty() )
                {
                    section.title = childTitle + " - " + section.title;
                }
                sections.push_back( std::move( section ) );
            }
        }
        return sections;
    }
};
