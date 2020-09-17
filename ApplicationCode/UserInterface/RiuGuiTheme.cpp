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

#include "RiuGuiTheme.h"

#include "RiaApplication.h"
#include "RiaGuiApplication.h"
#include "RiaPreferences.h"
#include "RiuThemesDirectory.h"
#include "cafAppEnum.h"

#include <QAbstractItemModel>
#include <QColor>
#include <QCompleter>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QRegExp>
#include <QRegularExpression>
#include <QString>
#include <QStringListModel>
#include <QStyle>
#include <QWidget>

#include "qwt_legend_label.h"
#include "qwt_picker.h"
#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_marker.h"
#include "qwt_symbol.h"

QMap<RiaDefines::ThemeEnum, QMap<QString, QString>>                 RiuGuiTheme::s_variableValueMap         = {};
QMap<RiaDefines::ThemeEnum, QMap<QString, QString>>                 RiuGuiTheme::s_variableGuiTextMap       = {};
QMap<QString, QMap<QString, QMap<QString, QMap<QString, QString>>>> RiuGuiTheme::s_qwtPlotItemPropertiesMap = {};
QMap<QString, CustomStyleSheetApplicator>                           RiuGuiTheme::s_customStyleSheetApplicators =
    {{QString( "QwtPlot\\[\"(?<plotName>[a-zA-Z0-9-_\\*]+)\"\\]::curve\\[\"(?<itemName>[a-zA-Z0-9-_\\*]+)\"\\]\\s*\\{("
               "?<properties>([\\n\\r]*\\s*((line-color|symbol-color):"
               "\\s*([a-zA-Z0-9#]+)\\s*;))*)[\\n\\r]*\\s*\\}" ),
      []( QRegularExpressionMatch& match ) {
          QRegExp plotNameRegExp( match.captured( "plotName" ) );
          QRegExp itemNameRegExp( match.captured( "itemName" ) );

          QRegularExpression lineColorRegExp( "line-color:\\s*([#0-9a-zA-Z]+)" );
          QString            lineColor = lineColorRegExp.match( match.captured( "properties" ) ).captured( 1 );
          QRegularExpression symbolColorRegExp( "symbol-color:\\s*([#0-9a-zA-Z]+)" );
          QString            symbolColor = symbolColorRegExp.match( match.captured( "properties" ) ).captured( 1 );
          if ( !lineColor.isEmpty() )
          {
              storeQwtStyleSheetProperty( match.captured( "plotName" ),
                                          QString( "curve" ),
                                          match.captured( "itemName" ),
                                          "line-color",
                                          lineColor );
          }
          if ( !symbolColor.isEmpty() )
          {
              // Symbols get the same color assigned as curves.
              storeQwtStyleSheetProperty( match.captured( "plotName" ),
                                          QString( "curve" ),
                                          match.captured( "itemName" ),
                                          "symbol-color",
                                          symbolColor );
          }
          if ( lineColor.isEmpty() && symbolColor.isEmpty() ) return;

          const QWidgetList topLevelWidgets = RiaGuiApplication::instance()->topLevelWidgets();
          for ( QWidget* widget : topLevelWidgets )
          {
              for ( QwtPlot* plotWidget : widget->findChildren<QwtPlot*>() )
              {
                  if ( plotNameRegExp.exactMatch( plotWidget->property( "qss-class" ).toString() ) )
                  {
                      for ( QwtPlotItem* item : plotWidget->itemList() )
                      {
                          if ( QwtPlotCurve* curve = dynamic_cast<QwtPlotCurve*>( item ) )
                          {
                              if ( itemNameRegExp.exactMatch( item->title().text() ) || match.captured( "itemName" ) == "*" )
                              {
                                  QPen pen = curve->pen();
                                  pen.setColor( QColor( lineColor ) );
                                  curve->setPen( pen );

                                  if ( curve->symbol() && curve->symbol()->style() != QwtSymbol::NoSymbol )
                                  {
                                      QPen pen = curve->symbol()->pen();
                                      pen.setColor( QColor( symbolColor ) );
                                      QwtSymbol* symbol = cloneCurveSymbol( curve );
                                      symbol->setPen( pen );
                                      curve->setSymbol( symbol );
                                  }
                              }
                          }
                      }
                  }
                  plotWidget->replot();
              }
          }
      }},
     {QString( "QwtPlot\\[\"(?<plotName>[a-zA-Z0-9-_\\*]+)\"\\]::grid\\[\"(?<itemName>[a-zA-Z0-9-_\\*]+)\"\\]\\s*\\{("
               "?<properties>([\\n\\r]*\\s*((color):"
               "\\s*([a-zA-Z0-9#]+)\\s*;))*)[\\n\\r]*\\s*\\}" ),
      []( QRegularExpressionMatch& match ) {
          QRegExp            plotNameRegExp( match.captured( "plotName" ) );
          QRegExp            itemNameRegExp( match.captured( "itemName" ) );
          QRegularExpression colorRegExp( "color:\\s*([#0-9a-zA-Z]+)" );
          QString            color           = colorRegExp.match( match.captured( "properties" ) ).captured( 1 );
          const QWidgetList  topLevelWidgets = RiaGuiApplication::instance()->topLevelWidgets();

          if ( !color.isEmpty() )
          {
              storeQwtStyleSheetProperty( match.captured( "plotName" ),
                                          QString( "grid" ),
                                          match.captured( "itemName" ),
                                          "color",
                                          color );
          }
          for ( QWidget* widget : topLevelWidgets )
          {
              for ( QwtPlot* plotWidget : widget->findChildren<QwtPlot*>() )
              {
                  if ( plotNameRegExp.exactMatch( plotWidget->property( "qss-class" ).toString() ) ||
                       match.captured( "plotName" ) == "*" )
                  {
                      for ( QwtPlotItem* item : plotWidget->itemList() )
                      {
                          if ( QwtPlotGrid* grid = dynamic_cast<QwtPlotGrid*>( item ) )
                          {
                              if ( itemNameRegExp.exactMatch( item->title().text() ) || match.captured( "itemName" ) == "*" )
                              {
                                  QPen pen = grid->majorPen();
                                  pen.setColor( QColor( color ) );
                                  grid->setPen( pen );
                              }
                          }
                      }
                  }
                  plotWidget->replot();
              }
          }
      }},
     {QString( "QwtPlot\\[\"(?<plotName>[a-zA-Z0-9-_\\*]+)\"\\]::legend\\s*\\{("
               "?<properties>([\\n\\r]*\\s*((text-color):"
               "\\s*([a-zA-Z0-9#]+)\\s*;))*)[\\n\\r]*\\s*\\}" ),
      []( QRegularExpressionMatch& match ) {
          QRegExp            plotNameRegExp( match.captured( "plotName" ) );
          QRegExp            itemNameRegExp( match.captured( "itemName" ) );
          QRegularExpression colorRegExp( "text-color:\\s*([#0-9a-zA-Z]+)" );
          QString            color           = colorRegExp.match( match.captured( "properties" ) ).captured( 1 );
          const QWidgetList  topLevelWidgets = RiaGuiApplication::instance()->topLevelWidgets();

          if ( !color.isEmpty() )
          {
              storeQwtStyleSheetProperty( match.captured( "plotName" ),
                                          QString( "legend" ),
                                          match.captured( "itemName" ),
                                          "text-color",
                                          color );
          }
          for ( QWidget* widget : topLevelWidgets )
          {
              for ( QwtPlot* plotWidget : widget->findChildren<QwtPlot*>() )
              {
                  if ( plotNameRegExp.exactMatch( plotWidget->property( "qss-class" ).toString() ) ||
                       match.captured( "plotName" ) == "*" )
                  {
                      for ( QwtLegendLabel* label : plotWidget->findChildren<QwtLegendLabel*>() )
                      {
                          QwtText text = label->text();
                          text.setColor( QColor( color ) );
                          label->setText( text );
                          label->repaint();
                      }
                  }
                  plotWidget->replot();
              }
          }
      }},
     {QString( "QwtPlot\\[\"(?<plotName>[a-zA-Z0-9-_\\*]+)\"\\]::lineMarker\\[\"(?<itemName>[a-zA-Z0-9-_\\*]+)\"\\]"
               "\\s*\\{("
               "?<properties>([\\n\\r]*\\s*((color|text-color):"
               "\\s*([a-zA-Z0-9#]+)\\s*;))*)[\\n\\r]*\\s*\\}" ),
      []( QRegularExpressionMatch& match ) {
          QRegExp            plotNameRegExp( match.captured( "plotName" ) );
          QRegExp            itemNameRegExp( match.captured( "itemName" ) );
          QRegularExpression colorRegExp( "color:\\s*([#0-9a-zA-Z]+)" );
          QString            color = colorRegExp.match( match.captured( "properties" ) ).captured( 1 );
          QRegularExpression textColorRegExp( "text-color:\\s*([#0-9a-zA-Z]+)" );
          QString            textColor = textColorRegExp.match( match.captured( "properties" ) ).captured( 1 );

          const QWidgetList topLevelWidgets = RiaGuiApplication::instance()->topLevelWidgets();

          if ( !color.isEmpty() )
          {
              storeQwtStyleSheetProperty( match.captured( "plotName" ),
                                          QString( "lineMarker" ),
                                          match.captured( "itemName" ),
                                          "color",
                                          color );
          }
          if ( !textColor.isEmpty() )
          {
              storeQwtStyleSheetProperty( match.captured( "plotName" ),
                                          QString( "lineMarker" ),
                                          match.captured( "itemName" ),
                                          "text-color",
                                          textColor );
          }
          for ( QWidget* widget : topLevelWidgets )
          {
              for ( QwtPlot* plotWidget : widget->findChildren<QwtPlot*>() )
              {
                  if ( plotNameRegExp.exactMatch( plotWidget->property( "qss-class" ).toString() ) ||
                       match.captured( "plotName" ) == "*" )
                  {
                      for ( QwtPlotItem* item : plotWidget->itemList() )
                      {
                          if ( QwtPlotMarker* marker = dynamic_cast<QwtPlotMarker*>( item ) )
                          {
                              if ( marker->symbol() == nullptr || marker->symbol()->style() == QwtSymbol::NoSymbol )
                              {
                                  if ( itemNameRegExp.exactMatch( item->title().text() ) ||
                                       match.captured( "itemName" ) == "*" )
                                  {
                                      QPen pen = marker->linePen();
                                      pen.setColor( QColor( color ) );
                                      marker->setLinePen( pen );
                                      marker->label().setColor( QColor( textColor ) );
                                  }
                              }
                          }
                      }
                  }
                  plotWidget->replot();
              }
          }
      }},
     {QString( "QwtPlot\\[\"(?<plotName>[a-zA-Z0-9-_\\*]+)\"\\]::pointMarker\\[\"(?<itemName>[a-zA-Z0-9-_\\*]+)\"\\]"
               "\\s*\\{("
               "?<properties>([\\n\\r]*\\s*((color|text-color):"
               "\\s*([a-zA-Z0-9#]+)\\s*;))*)[\\n\\r]*\\s*\\}" ),
      []( QRegularExpressionMatch& match ) {
          QRegExp            plotNameRegExp( match.captured( "plotName" ) );
          QRegExp            itemNameRegExp( match.captured( "itemName" ) );
          QRegularExpression colorRegExp( "color:\\s*([#0-9a-zA-Z]+)" );
          QString            color = colorRegExp.match( match.captured( "properties" ) ).captured( 1 );
          QRegularExpression textColorRegExp( "text-color:\\s*([#0-9a-zA-Z]+)" );
          QString            textColor = textColorRegExp.match( match.captured( "properties" ) ).captured( 1 );

          const QWidgetList topLevelWidgets = RiaGuiApplication::instance()->topLevelWidgets();

          if ( !color.isEmpty() )
          {
              storeQwtStyleSheetProperty( match.captured( "plotName" ),
                                          QString( "pointMarker" ),
                                          match.captured( "itemName" ),
                                          "color",
                                          color );
          }
          if ( !textColor.isEmpty() )
          {
              storeQwtStyleSheetProperty( match.captured( "plotName" ),
                                          QString( "pointMarker" ),
                                          match.captured( "itemName" ),
                                          "text-color",
                                          textColor );
          }
          for ( QWidget* widget : topLevelWidgets )
          {
              for ( QwtPlot* plotWidget : widget->findChildren<QwtPlot*>() )
              {
                  if ( plotNameRegExp.exactMatch( plotWidget->property( "qss-class" ).toString() ) ||
                       match.captured( "plotName" ) == "*" )
                  {
                      for ( QwtPlotItem* item : plotWidget->itemList() )
                      {
                          if ( QwtPlotMarker* marker = dynamic_cast<QwtPlotMarker*>( item ) )
                          {
                              if ( marker->symbol() && marker->symbol()->style() != QwtSymbol::NoSymbol )
                              {
                                  if ( itemNameRegExp.exactMatch( item->title().text() ) ||
                                       match.captured( "itemName" ) == "*" )
                                  {
                                      QPen pen = marker->symbol()->pen();
                                      pen.setColor( QColor( color ) );
                                      QwtSymbol* symbol = cloneMarkerSymbol( marker );
                                      symbol->setPen( pen );
                                      marker->setSymbol( symbol );
                                      marker->label().setColor( QColor( textColor ) );
                                  }
                              }
                          }
                      }
                  }
                  plotWidget->replot();
              }
          }
      }},
     {QString( "QwtPlot\\[\"(?<plotName>[a-zA-Z0-9-_\\*]+)\"\\]::picker"
               "\\s*\\{("
               "?<properties>([\\n\\r]*\\s*((text-color):"
               "\\s*([a-zA-Z0-9#]+)\\s*;))*)[\\n\\r]*\\s*\\}" ),
      []( QRegularExpressionMatch& match ) {
          QRegExp            plotNameRegExp( match.captured( "plotName" ) );
          QRegExp            itemNameRegExp( match.captured( "itemName" ) );
          QRegularExpression textColorRegExp( "text-color:\\s*([#a-zA-Z0-9]+)" );
          QString            textColor = textColorRegExp.match( match.captured( "properties" ) ).captured( 1 );

          const QWidgetList topLevelWidgets = RiaGuiApplication::instance()->topLevelWidgets();

          if ( !textColor.isEmpty() )
          {
              storeQwtStyleSheetProperty( match.captured( "plotName" ), QString( "picker" ), "*", "text-color", textColor );
          }
          for ( QWidget* widget : topLevelWidgets )
          {
              for ( QwtPlot* plotWidget : widget->findChildren<QwtPlot*>() )
              {
                  if ( plotNameRegExp.exactMatch( plotWidget->property( "qss-class" ).toString() ) ||
                       match.captured( "plotName" ) == "*" )
                  {
                      QWidget* canvas = plotWidget->canvas();
                      if ( canvas )
                      {
                          for ( QwtPicker* picker : canvas->findChildren<QwtPicker*>() )
                          {
                              QPen pen = picker->trackerPen();
                              pen.setColor( QColor( textColor ) );
                              picker->setTrackerPen( pen );
                          }
                      }
                  }
                  plotWidget->replot();
              }
          }
      }}};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGuiTheme::updateGuiTheme( RiaDefines::ThemeEnum theme )
{
    s_qwtPlotItemPropertiesMap.clear();
    applyStyleSheet( theme );
    const QWidgetList allWidgets = RiaGuiApplication::instance()->allWidgets();
    for ( QWidget* widget : allWidgets )
    {
        widget->style()->unpolish( widget );
        widget->style()->polish( widget );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuGuiTheme::applyStyleSheet( RiaDefines::ThemeEnum theme )
{
    QString styleSheetPath = getStyleSheetPath( theme );
    QFile   styleSheetFile( styleSheetPath );
    if ( styleSheetFile.exists() )
    {
        if ( styleSheetFile.open( QIODevice::ReadOnly ) )
        {
            RiaGuiApplication* app        = RiaGuiApplication::instance();
            QString            styleSheet = styleSheetFile.readAll();
            preparseStyleSheet( theme, styleSheet );
            app->setStyleSheet( styleSheet );
            styleSheetFile.close();
        }
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGuiTheme::changeVariableValue( RiaDefines::ThemeEnum theme, const QString& variableName, const QString& newValue )
{
    if ( !s_variableValueMap.keys().contains( theme ) )
    {
        s_variableValueMap.insert( theme, {} );
    }
    if ( !s_variableValueMap[theme].keys().contains( variableName ) )
    {
        s_variableValueMap[theme].insert( variableName, newValue );
    }
    else
    {
        s_variableValueMap[theme][variableName] = newValue;
    }
}

QMap<QString, QString> RiuGuiTheme::getVariableValueMap( RiaDefines::ThemeEnum theme )
{
    if ( !s_variableValueMap.keys().contains( theme ) )
    {
        QFile styleSheetFile( getStyleSheetPath( theme ) );
        if ( styleSheetFile.exists() )
        {
            if ( styleSheetFile.open( QIODevice::ReadOnly ) )
            {
                QString styleSheet = styleSheetFile.readAll();
                preparseStyleSheet( theme, styleSheet );
                styleSheetFile.close();
                return s_variableValueMap[theme];
            }
        }
        return QMap<QString, QString>();
    }
    else
    {
        return s_variableValueMap[theme];
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMap<QString, QString> RiuGuiTheme::getVariableGuiTextMap( RiaDefines::ThemeEnum theme )
{
    if ( !s_variableGuiTextMap.keys().contains( theme ) )
    {
        QFile styleSheetFile( getStyleSheetPath( theme ) );
        if ( styleSheetFile.exists() )
        {
            if ( styleSheetFile.open( QIODevice::ReadOnly ) )
            {
                QString styleSheet = styleSheetFile.readAll();
                preparseStyleSheet( theme, styleSheet );
                styleSheetFile.close();
                return s_variableGuiTextMap[theme];
            }
        }
        return QMap<QString, QString>();
    }
    else
    {
        return s_variableGuiTextMap[theme];
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuGuiTheme::applyVariableValueMapToStyleSheet( RiaDefines::ThemeEnum theme )
{
    QFileInfo info( getStyleSheetPath( theme ) );
    QString   absoluteStyleSheetPath = QString( "%0/%1" ).arg( GUI_THEMES_DIR ).arg( info.fileName() );
    QFile     styleSheetFile( absoluteStyleSheetPath );
    if ( styleSheetFile.exists() )
    {
        QString styleSheet;
        if ( styleSheetFile.open( QIODevice::ReadOnly | QIODevice::Truncate | QIODevice::Text ) )
        {
            styleSheet = styleSheetFile.readAll();
            QRegularExpression              variableRegExp( "(?<prefix>[ \\t]*(?<name>\\$[a-zA-z0-9_]+)[ \\t]*:[ "
                                               "\\t]*)(?<value>[a-zA-Z-_0-9#]+)(?<suffix>;[ \\t]*(\\/\\/[ "
                                               "\\t]*(?<descriptor>(.*)))?)" );
            QRegularExpressionMatchIterator matchIterator = variableRegExp.globalMatch( styleSheet );
            while ( matchIterator.hasNext() )
            {
                QRegularExpressionMatch match = matchIterator.next();
                styleSheet.replace( match.captured( 0 ),
                                    QString( "%0%1%2" )
                                        .arg( match.captured( "prefix" ) )
                                        .arg( s_variableValueMap[theme].value( match.captured( "name" ) ) )
                                        .arg( match.captured( "suffix" ) ) );
            }
            styleSheetFile.close();
        }
        if ( styleSheetFile.open( QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text ) )
        {
            styleSheetFile.write( styleSheet.toLatin1() );
            return styleSheet;
        }
    }
    return QString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuGuiTheme::writeStyleSheetToFile( RiaDefines::ThemeEnum theme, const QString& styleSheet )
{
    QFileInfo info( getStyleSheetPath( theme ) );
    QString   absoluteStyleSheetPath = QString( "%0/%1" ).arg( GUI_THEMES_DIR ).arg( info.fileName() );
    QFile     styleSheetFile( absoluteStyleSheetPath );
    if ( styleSheetFile.exists() )
    {
        if ( styleSheetFile.open( QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text ) )
        {
            styleSheetFile.write( styleSheet.toLatin1() );
            styleSheetFile.close();
            QString modifiedStyleSheet = styleSheet;
            preparseStyleSheet( theme, modifiedStyleSheet );
            RiaGuiApplication* app = RiaGuiApplication::instance();
            app->setStyleSheet( modifiedStyleSheet );
            const QWidgetList topLevelWidgets = RiaGuiApplication::instance()->allWidgets();
            for ( QWidget* widget : topLevelWidgets )
            {
                widget->style()->unpolish( widget );
                widget->style()->polish( widget );
            }
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuGuiTheme::loadStyleSheet( RiaDefines::ThemeEnum theme )
{
    QFile   styleSheetFile( getStyleSheetPath( theme ) );
    QString styleSheet;
    if ( styleSheetFile.exists() )
    {
        if ( styleSheetFile.open( QIODevice::ReadOnly ) )
        {
            styleSheet = styleSheetFile.readAll();
        }
    }
    return styleSheet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QAbstractItemModel* RiuGuiTheme::getQssCompletionModel( QCompleter* completer )
{
    QFile file( ":utility/qss-keywords.txt" );
    if ( !file.open( QFile::ReadOnly ) ) return new QStringListModel( completer );

    QStringList words;

    while ( !file.atEnd() )
    {
        QByteArray line = file.readLine();
        if ( !line.isEmpty() ) words << QString::fromUtf8( line.trimmed() );
    }

    return new QStringListModel( words, completer );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QColor RiuGuiTheme::getColorByVariableName( const QString& variable, int theme /*= -1 */ )
{
    if ( dynamic_cast<RiaGuiApplication*>( RiaApplication::instance() ) )
    {
        RiaDefines::ThemeEnum eTheme = RiaGuiApplication::instance()->preferences()->guiTheme();
        if ( theme >= 0 && theme < static_cast<int>( caf::AppEnum<RiaDefines::ThemeEnum>().size() ) )
        {
            eTheme = static_cast<RiaDefines::ThemeEnum>( theme );
        }

        if ( s_variableValueMap.keys().contains( eTheme ) && s_variableValueMap[eTheme].keys().contains( "$" + variable ) )
        {
            return QColor( s_variableValueMap[eTheme]["$" + variable] );
        }
    }
    return Qt::black;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuGuiTheme::getQwtStyleSheetProperty( QString        plotName,
                                               const QString& itemType,
                                               QString        itemName,
                                               const QString& propertyName )
{
    if ( !s_qwtPlotItemPropertiesMap.keys().contains( plotName ) )
    {
        if ( !s_qwtPlotItemPropertiesMap.keys().contains( "*" ) )
        {
            return QString();
        }
        else
        {
            plotName = "*";
        }
    }
    if ( !s_qwtPlotItemPropertiesMap[plotName].keys().contains( itemType ) )
    {
        if ( s_qwtPlotItemPropertiesMap.keys().contains( "*" ) &&
             s_qwtPlotItemPropertiesMap["*"].keys().contains( itemType ) )
        {
            plotName = "*";
        }
        else
        {
            return QString();
        }
    }
    if ( !s_qwtPlotItemPropertiesMap[plotName][itemType].keys().contains( itemName ) )
    {
        if ( !s_qwtPlotItemPropertiesMap[plotName][itemType].keys().contains( "*" ) )
        {
            if ( s_qwtPlotItemPropertiesMap.keys().contains( "*" ) &&
                 s_qwtPlotItemPropertiesMap["*"].keys().contains( itemType ) &&
                 s_qwtPlotItemPropertiesMap["*"][itemType].keys().contains( "*" ) )
            {
                plotName = "*";
                itemName = "*";
            }
            else
            {
                return QString();
            }
        }
        else
        {
            itemName = "*";
        }
    }
    return s_qwtPlotItemPropertiesMap[plotName][itemType][itemName].value( propertyName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGuiTheme::styleQwtItem( QwtPlotItem* item )
{
    QwtPlot* plot = item->plot();
    QString  plotName;
    if ( plot && plot->property( "qss-class" ).isValid() )
    {
        plotName = plot->property( "qss-class" ).toString();
    }
    if ( QwtPlotCurve* curve = dynamic_cast<QwtPlotCurve*>( item ) )
    {
        QPen    pen   = curve->pen();
        QString color = getQwtStyleSheetProperty( plotName, "curve", curve->title().text(), "line-color" );
        if ( !color.isEmpty() ) pen.setColor( QColor( color ) );
        curve->setPen( pen );

        if ( curve->symbol() && curve->symbol()->style() != QwtSymbol::NoSymbol )
        {
            pen                 = curve->symbol()->pen();
            QString symbolColor = getQwtStyleSheetProperty( plotName, "curve", curve->title().text(), "symbol-color" );
            if ( !color.isEmpty() ) pen.setColor( QColor( symbolColor ) );
            QwtSymbol* symbol = cloneCurveSymbol( curve );
            symbol->setPen( pen );
            curve->setSymbol( symbol );
        }
    }
    else if ( QwtPlotGrid* grid = dynamic_cast<QwtPlotGrid*>( item ) )
    {
        QPen    pen   = grid->majorPen();
        QString color = getQwtStyleSheetProperty( plotName, "grid", grid->title().text(), "color" );
        if ( !color.isEmpty() ) pen.setColor( QColor( color ) );
        grid->setPen( pen );
    }
    else if ( QwtPlotMarker* marker = dynamic_cast<QwtPlotMarker*>( item ) )
    {
        if ( marker->symbol() == nullptr || marker->symbol()->style() == QwtSymbol::NoSymbol )
        {
            QPen    pen   = marker->linePen();
            QString color = getQwtStyleSheetProperty( plotName, "lineMarker", marker->title().text(), "color" );
            if ( !color.isEmpty() ) pen.setColor( QColor( color ) );
            marker->setLinePen( pen );
            QString textColor = getQwtStyleSheetProperty( plotName, "lineMarker", marker->title().text(), "text-color" );
            if ( !textColor.isEmpty() ) marker->label().setColor( QColor( textColor ) );
        }
        else
        {
            QwtSymbol* symbol = cloneMarkerSymbol( marker );
            QPen       pen    = symbol->pen();
            QString    color  = getQwtStyleSheetProperty( plotName, "pointMarker", marker->title().text(), "color" );
            if ( !color.isEmpty() ) pen.setColor( QColor( color ) );
            symbol->setPen( pen );
            marker->setSymbol( symbol );
            QString textColor = getQwtStyleSheetProperty( plotName, "pointMarker", marker->title().text(), "text-color" );
            if ( !textColor.isEmpty() ) marker->label().setColor( QColor( textColor ) );
        }
    }
    else if ( QwtPlotMarker* marker = dynamic_cast<QwtPlotMarker*>( item ) )
    {
        if ( marker->symbol() == nullptr || marker->symbol()->style() == QwtSymbol::NoSymbol )
        {
            QPen    pen   = marker->linePen();
            QString color = getQwtStyleSheetProperty( plotName, "lineMarker", marker->title().text(), "color" );
            if ( !color.isEmpty() ) pen.setColor( QColor( color ) );
            marker->setLinePen( pen );
            QString textColor = getQwtStyleSheetProperty( plotName, "lineMarker", marker->title().text(), "text-color" );
            if ( !textColor.isEmpty() ) marker->label().setColor( QColor( textColor ) );
        }
        else
        {
            QwtSymbol* symbol = cloneMarkerSymbol( marker );
            QPen       pen    = symbol->pen();
            QString    color  = getQwtStyleSheetProperty( plotName, "pointMarker", marker->title().text(), "color" );
            if ( !color.isEmpty() ) pen.setColor( QColor( color ) );
            symbol->setPen( pen );
            marker->setSymbol( symbol );
            QString textColor = getQwtStyleSheetProperty( plotName, "pointMarker", marker->title().text(), "text-color" );
            if ( !textColor.isEmpty() ) marker->label().setColor( QColor( textColor ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGuiTheme::styleQwtItem( QwtPicker* item )
{
    QWidget* canvas = item->parentWidget();
    if ( canvas )
    {
        QwtPlot* plot = dynamic_cast<QwtPlot*>( canvas->parentWidget() );
        if ( plot )
        {
            QString plotName;
            if ( plot && plot->property( "qss-class" ).isValid() )
            {
                plotName = plot->property( "qss-class" ).toString();
            }

            QPen    pen       = item->trackerPen();
            QString textColor = getQwtStyleSheetProperty( plotName, "picker", "*", "text-color" );
            if ( !textColor.isEmpty() ) pen.setColor( QColor( textColor ) );
            item->setTrackerPen( pen );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGuiTheme::preparseStyleSheet( RiaDefines::ThemeEnum theme, QString& styleSheet )
{
    QRegularExpression variableRegExp(
        "[ \\t]*(?<name>\\$[a-zA-z0-9_]+)[ \\t]*:[ \\t]*(?<value>[a-zA-Z-_0-9#]+);[ \\t]*(\\/\\/[ "
        "\\t]*(?<descriptor>(.*)))?" );
    QRegularExpressionMatchIterator matchIterator = variableRegExp.globalMatch( styleSheet );

    if ( !s_variableValueMap.keys().contains( theme ) )
    {
        s_variableValueMap.insert( theme, {} );
    }
    else
    {
        s_variableValueMap[theme].clear();
    }

    while ( matchIterator.hasNext() )
    {
        QRegularExpressionMatch match = matchIterator.next();
        styleSheet.replace( match.captured( 0 ), "" );
        if ( s_variableValueMap[theme].keys().contains( match.captured( "name" ) ) )
        {
            styleSheet.replace( match.captured( "name" ), s_variableValueMap[theme].value( match.captured( "name" ) ) );
        }
        else
        {
            styleSheet.replace( match.captured( "name" ), match.captured( "value" ) );
            s_variableValueMap[theme].insert( match.captured( "name" ), match.captured( "value" ) );
            s_variableGuiTextMap[theme].insert( match.captured( "name" ), match.captured( "descriptor" ) );
        }
    }

    QMap<QString, CustomStyleSheetApplicator>::iterator applicatorIterator;
    for ( applicatorIterator = s_customStyleSheetApplicators.begin();
          applicatorIterator != s_customStyleSheetApplicators.end();
          applicatorIterator++ )
    {
        matchIterator = QRegularExpression( applicatorIterator.key() ).globalMatch( styleSheet );
        while ( matchIterator.hasNext() )
        {
            QRegularExpressionMatch match = matchIterator.next();
            applicatorIterator.value()( match );
            styleSheet.replace( match.captured( 0 ), "" );
        }
    }

    styleSheet = styleSheet.simplified();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuGuiTheme::getStyleSheetPath( RiaDefines::ThemeEnum theme )
{
    return QString( ":/themes/%0.qss" ).arg( caf::AppEnum<RiaDefines::ThemeEnum>( theme ).text().toLower() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGuiTheme::storeQwtStyleSheetProperty( const QString& plotName,
                                              const QString& itemType,
                                              const QString& itemName,
                                              const QString& propertyName,
                                              const QString& value )
{
    if ( !s_qwtPlotItemPropertiesMap.keys().contains( plotName ) )
    {
        s_qwtPlotItemPropertiesMap.insert( plotName, QMap<QString, QMap<QString, QMap<QString, QString>>>{} );
    }
    if ( !s_qwtPlotItemPropertiesMap[plotName].keys().contains( itemType ) )
    {
        s_qwtPlotItemPropertiesMap[plotName].insert( itemType, QMap<QString, QMap<QString, QString>>{} );
    }
    if ( !s_qwtPlotItemPropertiesMap[plotName][itemType].keys().contains( itemName ) )
    {
        s_qwtPlotItemPropertiesMap[plotName][itemType].insert( itemName, QMap<QString, QString>{} );
    }
    s_qwtPlotItemPropertiesMap[plotName][itemType][itemName].insert( propertyName, value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Qt::PenStyle RiuGuiTheme::getPenStyleFromString( const QString& style )
{
    if ( style == "solid" )
    {
        return Qt::PenStyle::SolidLine;
    }
    else if ( style == "dash" )
    {
        return Qt::PenStyle::DashLine;
    }
    else if ( style == "dot" )
    {
        return Qt::PenStyle::DotLine;
    }
    else if ( style == "dash-dot" )
    {
        return Qt::PenStyle::DashDotLine;
    }
    else if ( style == "dash-dot-dot" )
    {
        return Qt::PenStyle::DashDotDotLine;
    }
    else
    {
        return Qt::PenStyle::NoPen;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QwtSymbol* RiuGuiTheme::cloneMarkerSymbol( QwtPlotMarker* marker )
{
    QwtSymbol* symbol = new QwtSymbol();
    symbol->setBrush( marker->symbol()->brush() );
    symbol->setPen( marker->symbol()->pen() );
    symbol->setStyle( marker->symbol()->style() );
    if ( marker->symbol()->style() == QwtSymbol::Style::Pixmap )
    {
        symbol->setPixmap( marker->symbol()->pixmap() );
    }
    else if ( marker->symbol()->style() == QwtSymbol::Style::Graphic )
    {
        symbol->setGraphic( marker->symbol()->graphic() );
    }
    symbol->setSize( marker->symbol()->size() );
    symbol->setPinPoint( marker->symbol()->pinPoint() );
    return symbol;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QwtSymbol* RiuGuiTheme::cloneCurveSymbol( QwtPlotCurve* curve )
{
    QwtSymbol* symbol = new QwtSymbol();
    symbol->setBrush( curve->symbol()->brush() );
    symbol->setPen( curve->symbol()->pen() );
    symbol->setStyle( curve->symbol()->style() );
    if ( curve->symbol()->style() == QwtSymbol::Style::Pixmap )
    {
        symbol->setPixmap( curve->symbol()->pixmap() );
    }
    else if ( curve->symbol()->style() == QwtSymbol::Style::Graphic )
    {
        symbol->setGraphic( curve->symbol()->graphic() );
    }
    symbol->setSize( curve->symbol()->size() );
    symbol->setPinPoint( curve->symbol()->pinPoint() );
    return symbol;
}
