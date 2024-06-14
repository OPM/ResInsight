////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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

// NOTE: This file must be included before any other Qt header files, as the keyword 'signals' is used as a parameter name in
// RegisterCancellingSignalHandler(const std::vector<int>& signals);
// Qt has special treatment of 'signals', and causes compiler issues using PCH
#include <arrow/util/cancel.h>

#include "cvfObject.h"
#include "cvfVector3.h"

#include "cafAppEnum.h"
#include "cafAssert.h"
#include "cafCmdFeature.h"
#include "cafFactory.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObject.h"
#include "cafPdmPtrArrayField.h"
#include "cafPdmPtrField.h"
#include "cafPdmUiOrdering.h"
#include "cafSelectionManager.h"

#include <QApplication>
#include <QObject>
#include <QString>
#include <QTextStream>

#include <map>
#include <vector>

#include "RiaApplication.h"
#include "RiaPreferences.h"
#include "RimPlotCurve.h"
#include "RiuMainWindow.h"
#include "RiuPlotMainWindow.h"

// These files are reported as candidates for PCH file, but including them breaks the Unity build
// #include "cafViewer.h"
// #include "RiuViewer.h"
// #include "cafOpenGLWidget.h"
// #include <QGLWidget>
