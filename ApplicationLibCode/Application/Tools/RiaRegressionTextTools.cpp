/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023-     Equinor ASA
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

#include "RiaRegressionTextTools.h"

#include "ExponentialRegression.hpp"
#include "LinearRegression.hpp"
#include "LogarithmicRegression.hpp"
#include "LogisticRegression.hpp"
#include "PolynomialRegression.hpp"
#include "PowerFitRegression.hpp"

#include <QStringList>

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaRegressionTextTools::generateRegressionText( const regression::LinearRegression& reg )
{
    QString sign = reg.intercept() < 0.0 ? "-" : "+";
    return QString( "y = %1x %2 %3" ).arg( formatDouble( reg.slope() ) ).arg( sign ).arg( formatDouble( std::fabs( reg.intercept() ) ) ) +
           QString( "<br>R<sup>2</sup> = %1" ).arg( reg.r2() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaRegressionTextTools::generateRegressionText( const regression::PolynomialRegression& reg )
{
    QString str = "y = ";

    bool                isFirst = true;
    std::vector<double> coeffs  = reg.coeffisients();
    QStringList         parts;
    for ( int i = static_cast<int>( coeffs.size() ) - 1; i >= 0; i-- )
    {
        double coeff = coeffs[i];
        // Skip zero coeffs
        if ( coeff != 0.0 )
        {
            if ( coeff < 0.0 )
                parts.append( "-" );
            else if ( !isFirst )
                parts.append( "+" );

            if ( i == 0 )
            {
                parts.append( QString( "%1" ).arg( formatDouble( std::fabs( coeff ) ) ) );
            }
            else if ( i == 1 )
            {
                parts.append( QString( "%1x" ).arg( formatDouble( std::fabs( coeff ) ) ) );
            }
            else
            {
                parts.append( QString( " %1x<sup>%2</sup>" ).arg( formatDouble( std::fabs( coeff ) ) ).arg( i ) );
            }

            isFirst = false;
        }
    }

    return str + parts.join( " " ) + QString( "<br>R<sup>2</sup> = %1" ).arg( reg.r2() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaRegressionTextTools::generateRegressionText( const regression::PowerFitRegression& reg )
{
    return QString( "y = %1 + x<sup>%2</sup>" ).arg( formatDouble( reg.scale() ) ).arg( formatDouble( reg.exponent() ) ) +
           QString( "<br>R<sup>2</sup> = %1" ).arg( reg.r2() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaRegressionTextTools::generateRegressionText( const regression::ExponentialRegression& reg )
{
    return QString( "y = %1 * e<sup>%2x</sup>" ).arg( formatDouble( reg.a() ) ).arg( formatDouble( reg.b() ) ) +
           QString( "<br>R<sup>2</sup> = %1" ).arg( reg.r2() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaRegressionTextTools::generateRegressionText( const regression::LogarithmicRegression& reg )
{
    return QString( "y = %1 + %2 * ln(x)" ).arg( formatDouble( reg.a() ) ).arg( formatDouble( reg.b() ) ) +
           QString( "<br>R<sup>2</sup> = %1" ).arg( reg.r2() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaRegressionTextTools::formatDouble( double v )
{
    return QString::number( v, 'g', 2 );
}
