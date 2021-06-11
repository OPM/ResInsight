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
#include <QTextEdit>

class QCompleter;

class TextEditWithCompletion : public QTextEdit
{
    Q_OBJECT
public:
    TextEditWithCompletion( QWidget* parent = nullptr );
    ~TextEditWithCompletion() override;

    void        setCompleter( QCompleter* completer );
    QCompleter* completer();

protected:
    void keyPressEvent( QKeyEvent* e ) override;
    void focusInEvent( QFocusEvent* e ) override;

private slots:
    void insertCompletion( const QString& completion );

private:
    QString textUnderCursor() const;

private:
    QCompleter* m_completer;
};
