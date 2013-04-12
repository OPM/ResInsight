/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include <QtGui/QWidget>

class QDockWidget;
class QTextEdit;



//==================================================================================================
//
// RiuResultInfoPanel 
//
//==================================================================================================
class RiuResultInfoPanel : public QWidget
{
	Q_OBJECT

public:
	RiuResultInfoPanel(QDockWidget* parent);

	void			setInfo(const QString& info);

    virtual QSize	sizeHint () const;

private:
	static void	    convertStringToHTML(QString* str);

private:
    QTextEdit*		m_textEdit;
};

