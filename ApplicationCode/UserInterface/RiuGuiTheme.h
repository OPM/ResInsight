/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RiaPreferences.h"

#include <functional>

#include <QMap>
#include <QPen>

class QWidget;
class QAbstractItemModel;
class QCompleter;
class QColor;
class QRegularExpressionMatch;
class QString;
class QwtPlotItem;
class QwtSymbol;
class QwtPlotMarker;
class QwtPlotCurve;
class QwtPicker;
class QwtText;

typedef std::function<void( QRegularExpressionMatch& )> CustomStyleSheetApplicator;

class RiuGuiTheme
{
public:
    static void updateGuiTheme( RiaDefines::ThemeEnum theme );
    static bool applyStyleSheet( RiaDefines::ThemeEnum theme );
    static void changeVariableValue( RiaDefines::ThemeEnum theme, const QString& variableName, const QString& newValue );
    static QMap<QString, QString> getVariableValueMap( RiaDefines::ThemeEnum theme );
    static QMap<QString, QString> getVariableGuiTextMap( RiaDefines::ThemeEnum theme );
    static QString                applyVariableValueMapToStyleSheet( RiaDefines::ThemeEnum theme );
    static bool                   writeStyleSheetToFile( RiaDefines::ThemeEnum theme, const QString& styleSheet );
    static QString                loadStyleSheet( RiaDefines::ThemeEnum theme );
    static QAbstractItemModel*    getQssCompletionModel( QCompleter* completer );
    static QColor                 getColorByVariableName( const QString& variable, int theme = -1 );
    static QString
                getQwtStyleSheetProperty( QString plotName, const QString& itemType, QString itemName, const QString& propertyName );
    static void styleQwtItem( QwtPlotItem* item );
    static void styleQwtItem( QwtPicker* item );

private:
    static void         preparseStyleSheet( RiaDefines::ThemeEnum theme, QString& styleSheet );
    static QString      getStyleSheetPath( RiaDefines::ThemeEnum theme );
    static void         storeQwtStyleSheetProperty( const QString& plotName,
                                                    const QString& itemType,
                                                    const QString& itemName,
                                                    const QString& propertyName,
                                                    const QString& value );
    static Qt::PenStyle getPenStyleFromString( const QString& style );
    static QwtSymbol*   cloneMarkerSymbol( QwtPlotMarker* marker );
    static QwtSymbol*   cloneCurveSymbol( QwtPlotCurve* curve );

private:
    static QMap<RiaDefines::ThemeEnum, QMap<QString, QString>>                 s_variableValueMap;
    static QMap<RiaDefines::ThemeEnum, QMap<QString, QString>>                 s_variableGuiTextMap;
    static QMap<QString, QMap<QString, QMap<QString, QMap<QString, QString>>>> s_qwtPlotItemPropertiesMap;
    static QMap<QString, CustomStyleSheetApplicator>                           s_customStyleSheetApplicators;
};
