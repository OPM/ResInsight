/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include <string>

class RimSummaryCase;

//==================================================================================================
///
///
//==================================================================================================
class RimWellDiskConfig
{
public:
    RimWellDiskConfig();
    ~RimWellDiskConfig();

    bool        isSingleProperty() const;
    std::string getSingleProperty() const;
    void        setSingleProperty( const std::string& singleProperty );

    void        setOilProperty( const std::string& oilProperty );
    std::string getOilProperty( bool isInjector ) const;

    void        setGasProperty( const std::string& gasProperty );
    std::string getGasProperty( bool isInjector ) const;

    void        setWaterProperty( const std::string& waterProperty );
    std::string getWaterProperty( bool isInjector ) const;

    void setOilProperty( const std::string& oilPropertyProducer, const std::string& oilPropertyInjector );
    void setGasProperty( const std::string& gasPropertyProducer, const std::string& gasPropertyInjector );
    void setWaterProperty( const std::string& waterPropertyProducer, const std::string& waterPropertyInjector );

    RimSummaryCase* sourceCase() const;
    void            setSourceCase( RimSummaryCase* summaryCase );

private:
    bool        m_isSingleProperty;
    std::string m_singleProperty;

    bool        m_isCombinedProductionAndInjection;
    std::string m_oilProperty;
    std::string m_gasProperty;
    std::string m_waterProperty;

    std::string m_oilPropertyInjector;
    std::string m_gasPropertyInjector;
    std::string m_waterPropertyInjector;

    RimSummaryCase* m_summaryCase;
};
