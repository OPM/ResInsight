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

#include "RicThemeColorEditorFeature.h"

#include "RiaDefines.h"
#include "RiaGuiApplication.h"
#include "RiaPreferences.h"

#include "RiuGuiTheme.h"
#include "RiuMainWindow.h"
#include "RiuQssSyntaxHighlighter.h"
#include "RiuTextEditWithCompletion.h"

#include "cafAppEnum.h"

#include <QAction>
#include <QColorDialog>
#include <QComboBox>
#include <QCompleter>
#include <QGridLayout>
#include <QPushButton>
#include <QStringListModel>

CAF_CMD_SOURCE_INIT( RicThemeColorEditorFeature, "RicThemeColorEditorFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicThemeColorEditorFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicThemeColorEditorFeature::onActionTriggered( bool isChecked )
{
    RiaDefines::ThemeEnum theme = RiaGuiApplication::instance()->preferences()->guiTheme();

    QDialog* dialog = new QDialog( RiuMainWindow::instance() );
    connect( dialog, &QDialog::close, [this, theme]() { RiuGuiTheme::updateGuiTheme( theme ); } );
    dialog->setModal( false );
    dialog->setWindowTitle( "Theme Color Editor Dialog" );

    QGridLayout* layout = new QGridLayout();

    layout->addWidget( new QLabel( "GUI theme" ), 0, 0 );

    QComboBox*                          themeSelector = new QComboBox();
    caf::AppEnum<RiaDefines::ThemeEnum> themes;
    for ( size_t index = 0; index < themes.size(); index++ )
    {
        themeSelector->addItem( themes.uiTextFromIndex( index ), QVariant::fromValue( index ) );
        if ( static_cast<RiaDefines::ThemeEnum>( index ) == theme )
        {
            themeSelector->setCurrentIndex( static_cast<int>( index ) );
        }
    }
    layout->addWidget( themeSelector, 0, 1 );

    QFrame* line = new QFrame();
    line->setFrameShape( QFrame::HLine );
    layout->addWidget( line, 1, 0, 1, 2 );
    QWidget* widget = new QWidget();
    layout->addWidget( widget, 2, 0, 1, 2 );

    TextEditWithCompletion* editor = new TextEditWithCompletion();
    editor->setAcceptRichText( false );
    QCompleter* completer = new QCompleter( RiuMainWindow::instance() );
    completer->setModel( RiuGuiTheme::getQssCompletionModel( completer ) );
    completer->setModelSorting( QCompleter::CaseInsensitivelySortedModel );
    completer->setCaseSensitivity( Qt::CaseInsensitive );
    completer->setWrapAround( false );
    editor->setCompleter( completer );
    QssSyntaxHighligter* highlighter = new QssSyntaxHighligter( editor->document() );

    auto generateColorFields = [themeSelector, widget, editor, completer, this]() -> void {
        QLayoutItem* item;
        if ( widget->layout() )
        {
            while ( ( item = widget->layout()->takeAt( 0 ) ) != NULL )
            {
                delete item->widget();
                delete item;
            }
            delete widget->layout();
        }
        QGridLayout*           innerLayout = new QGridLayout();
        int                    row         = 0;
        int                    column      = 0;
        RiaDefines::ThemeEnum  theme       = static_cast<RiaDefines::ThemeEnum>( themeSelector->currentData().toInt() );
        QMap<QString, QString> variableValueMap   = RiuGuiTheme::getVariableValueMap( theme );
        QMap<QString, QString> variableGuiTextMap = RiuGuiTheme::getVariableGuiTextMap( theme );
        for ( const QString variableName : variableValueMap.keys() )
        {
            innerLayout->addWidget( new QLabel( !variableGuiTextMap[variableName].isEmpty() ? variableGuiTextMap[variableName]
                                                                                            : variableName ),
                                    row,
                                    column );
            QPushButton* colorBox = new QPushButton( "" );
            colorBox->setStyleSheet( QString( "background-color: %0;" ).arg( variableValueMap.value( variableName ) ) );
            connect( colorBox,
                     &QPushButton::clicked,
                     [this, variableValueMap, variableName, theme, editor, widget, colorBox]() -> void {
                         QColor color = QColorDialog::getColor( colorBox->palette().color( QPalette::Button ), widget );
                         if ( color.isValid() )
                         {
                             colorBox->setStyleSheet( QString( "background-color: %0;" ).arg( color.name() ) );
                             colorBox->style()->unpolish( colorBox );
                             colorBox->style()->polish( colorBox );
                             RiuGuiTheme::changeVariableValue( theme, variableName, color.name() );
                             editor->setPlainText( RiuGuiTheme::applyVariableValueMapToStyleSheet( theme ) );
                         }
                     } );
            innerLayout->addWidget( colorBox, row++, column + 1 );
            if ( row == 10 )
            {
                row = 0;
                column += 2;
            }
        }
        widget->setLayout( innerLayout );
    };

    // A more elegant way, but not supported in old Qt version.
    // connect( themeSelector, qOverload<int>( &QComboBox::currentIndexChanged ), [=]() {
    connect( themeSelector, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), [=]() {
        generateColorFields();
        RiaDefines::ThemeEnum theme = static_cast<RiaDefines::ThemeEnum>( themeSelector->currentData().toInt() );
        RiuGuiTheme::updateGuiTheme( static_cast<RiaDefines::ThemeEnum>( theme ) );
        editor->setPlainText( RiuGuiTheme::loadStyleSheet( theme ) );
    } );

    generateColorFields();

    RiuGuiTheme::updateGuiTheme( theme );
    editor->setPlainText( RiuGuiTheme::loadStyleSheet( theme ) );

    line = new QFrame();
    line->setFrameShape( QFrame::HLine );
    layout->addWidget( line, 3, 0, 1, 2 );
    layout->addWidget( editor, 5, 0, 1, 2 );

    QPushButton* button = new QPushButton( "Apply style sheet changes" );
    layout->addWidget( button, 6, 1 );
    connect( button, &QPushButton::clicked, [this, themeSelector, editor, generateColorFields]() {
        RiaDefines::ThemeEnum theme = static_cast<RiaDefines::ThemeEnum>( themeSelector->currentData().toInt() );
        RiuGuiTheme::writeStyleSheetToFile( theme, editor->toPlainText() );
        generateColorFields();
    } );

    dialog->setLayout( layout );

    dialog->show();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicThemeColorEditorFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Open Theme Color Editor" );
}
