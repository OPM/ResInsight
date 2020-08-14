/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RiuTools.h"

#include <QApplication>
#include <QPalette>
#include <QStyleFactory>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Qt::WindowFlags RiuTools::defaultDialogFlags()
{
    Qt::WindowFlags f = Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint;

    return f;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuTools::applyDarkTheme()
{
    qApp->setStyle( QStyleFactory::create( "Fusion" ) );
    QPalette darkPalette;
    QColor   darkColor     = QColor( 45, 45, 45 );
    QColor   disabledColor = QColor( 127, 127, 127 );
    darkPalette.setColor( QPalette::Window, darkColor );
    darkPalette.setColor( QPalette::WindowText, Qt::white );
    darkPalette.setColor( QPalette::Base, QColor( 18, 18, 18 ) );
    darkPalette.setColor( QPalette::AlternateBase, darkColor );
    darkPalette.setColor( QPalette::ToolTipBase, Qt::white );
    darkPalette.setColor( QPalette::ToolTipText, Qt::white );
    darkPalette.setColor( QPalette::Text, Qt::white );
    darkPalette.setColor( QPalette::Disabled, QPalette::Text, disabledColor );
    darkPalette.setColor( QPalette::Button, darkColor );
    darkPalette.setColor( QPalette::ButtonText, Qt::white );
    darkPalette.setColor( QPalette::Disabled, QPalette::ButtonText, disabledColor );
    darkPalette.setColor( QPalette::BrightText, Qt::red );
    darkPalette.setColor( QPalette::Link, QColor( 42, 130, 218 ) );

    darkPalette.setColor( QPalette::Highlight, QColor( 42, 130, 218 ) );
    darkPalette.setColor( QPalette::HighlightedText, Qt::black );
    darkPalette.setColor( QPalette::Disabled, QPalette::HighlightedText, disabledColor );

    qApp->setPalette( darkPalette );

    qApp->setStyleSheet( "QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }" );
}
