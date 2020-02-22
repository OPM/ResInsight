
#pragma once

#include "cvfLibCore.h"
#include "cvfLibRender.h"
#include "cvfLibGeometry.h"
#include "cvfLibViewing.h"
#include "cvfOpenGL.h"

#include "cvfuTestSnippet.h"
#include "cvfuSnippetFactory.h"
#include "cvfuInputEvents.h"

#include <QtCore/QtCore>
#if QT_VERSION >= 0x050000
#include <QObject>
#include <QPointer>
#include <QWidget>
#else
#include <QtGui/QtGui>
#endif

// Introduce name of commonly used classes (that are unlikely to create clashes) from the cvf namespace. 
// We allow the use of using-declarations in this include file since its sole usage is as a precompiled header file.
using cvf::ref;
using cvf::Vec3f;
using cvf::Vec3d;
using cvf::Color3f;
using cvf::String;

using cvf::Collection;
using cvf::Array;
using cvf::Vec3fArray;
using cvf::Vec3dArray;
using cvf::FloatArray;
using cvf::DoubleArray;

using cvf::Part;
using cvf::Drawable;
using cvf::DrawableGeo;
using cvf::Model;
using cvf::Scene;
using cvf::Rendering;
using cvf::Camera;
using cvf::RenderSequence;

