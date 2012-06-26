//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
//   This library is free software: you can redistribute it and/or modify 
//   it under the terms of the GNU General Public License as published by 
//   the Free Software Foundation, either version 3 of the License, or 
//   (at your option) any later version. 
//    
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY 
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or 
//   FITNESS FOR A PARTICULAR PURPOSE.   
//    
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>> 
//   for more details. 
//
//##################################################################################################

#include "cafProgressInfo.h"
#include <QPointer>
#include <QProgressDialog>
#include <QCoreApplication>
#include <QThread>
#include <assert.h>

namespace caf {

//==================================================================================================
///
/// \class caf::ProgressInfo
/// 
/// 
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ProgressInfo::ProgressInfo(int maxProgressValue, const QString& title)
{
    ProgressInfoStatic::start(maxProgressValue, title);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ProgressInfo::~ProgressInfo()
{
    ProgressInfoStatic::finished();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ProgressInfo::setProgressDescription(const QString& description)
{
    ProgressInfoStatic::setProgressDescription(description);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ProgressInfo::setProgress(int progressValue)
{
    ProgressInfoStatic::setProgress(progressValue);
}








//==================================================================================================
///
/// Internal file-scope private functions to implement the progress dialog
/// 
/// 
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
static QProgressDialog* progressDialog()
{
     static QPointer<QProgressDialog> progDialog;
     if (progDialog.isNull())
     {
         progDialog = new QProgressDialog();

         progDialog->hide();
     }
     return progDialog;
}

//--------------------------------------------------------------------------------------------------
/// A static vector containing the maximum values for the progress on each sublevel
//--------------------------------------------------------------------------------------------------
static std::vector<int>& maxProgressStack()
{
     static std::vector<int> m_maxProgressStack;
  
     return m_maxProgressStack;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
static std::vector<QString>& titleStack()
{
    static std::vector<QString> m_titleStack;

    return m_titleStack;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
static std::vector<QString>& descriptionStack()
{
    static std::vector<QString> m_descriptionStack;

    return m_descriptionStack;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
static std::vector<int>& progressStack()
{
    static std::vector<int> m_progressStack;

    return m_progressStack;
}

//--------------------------------------------------------------------------------------------------
/// Calculate the total number of progress values we would need if we only look at the levels from 
/// \a levelDepth and below (increasing subdivision)
//--------------------------------------------------------------------------------------------------
static int subLevelsMaxProgressValue(size_t levelDepth)
{
    int levCount = 1;
    for (; levelDepth < maxProgressStack().size(); ++levelDepth)
    {
        levCount *=  maxProgressStack()[levelDepth];
    }
    return levCount;
}

//--------------------------------------------------------------------------------------------------
/// Calculate the total progress value based on the current level subdivision and progress
//--------------------------------------------------------------------------------------------------
static int currentTotalProgress()
{
    int progress = 0;
    for (size_t i = 0; i < progressStack().size(); ++i)
    {
        progress = progress + progressStack()[i]* subLevelsMaxProgressValue(i+1);
    }
    return progress;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
static int currentTotalMaxProgressValue()
{
    return subLevelsMaxProgressValue(0);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
static QString currentComposedLabel()
{
    QString labelText;

    for (size_t i = 0; i < titleStack().size(); ++i)
    {
        if (!titleStack()[i].isEmpty()) labelText += titleStack()[i];
        if (!titleStack()[i].isEmpty() && !descriptionStack()[i].isEmpty()) labelText +=": ";
        if (!descriptionStack()[i].isEmpty()) labelText += descriptionStack()[i] ;
        if (!(titleStack()[i].isEmpty() && descriptionStack()[i].isEmpty())) labelText += "\n";
    }
     return labelText;
  
}

static bool isWrongThread()
{
   return  !(progressDialog()->thread() == QThread::currentThread());
}
//==================================================================================================
///
/// \class caf::ProgressInfoStatic
/// 
/// 
/// 
//==================================================================================================


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ProgressInfoStatic::start(int maxProgressValue, const QString& title)
{
    if (isWrongThread()) return;

    if (!maxProgressStack().size())
    {
        progressDialog()->setWindowModality(Qt::WindowModal);
        progressDialog()->setMinimum(0);
        progressDialog()->setWindowTitle(title);
        progressDialog()->setCancelButton(NULL);
        progressDialog()->show();
    }

    maxProgressStack().push_back(maxProgressValue);
    progressStack().push_back(0);
    titleStack().push_back(title);
    descriptionStack().push_back("");

    progressDialog()->setMaximum(currentTotalMaxProgressValue());
    progressDialog()->setValue(currentTotalProgress());
    progressDialog()->setLabelText(currentComposedLabel());

    QCoreApplication::processEvents();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ProgressInfoStatic::setProgressDescription(const QString& description)
{
    if (isWrongThread()) return;

    descriptionStack().back() = description;

    progressDialog()->setLabelText(currentComposedLabel());
    QCoreApplication::processEvents();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ProgressInfoStatic::setProgress(int progressValue)
{
    if (isWrongThread()) return;

    // Guard against the max value set for theis level
    if (progressValue < 0 ) progressValue = 0;
    if (progressValue > maxProgressStack().back() ) progressValue = maxProgressStack().back();

    progressStack().back() = progressValue;

    assert(currentTotalProgress() <= progressDialog()->maximum());
    int totProg = currentTotalProgress();

    progressDialog()->setMaximum(currentTotalMaxProgressValue());
    progressDialog()->setValue(currentTotalProgress());
    QCoreApplication::processEvents();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ProgressInfoStatic::finished()
{
    if (isWrongThread()) return;

    assert(maxProgressStack().size() && progressStack().size() && titleStack().size() && descriptionStack().size());

    // Set progress to max value, and leave it there until somebody touches the progress again

    progressStack().back() = maxProgressStack().back();
    progressDialog()->setValue(currentTotalProgress());

    // Pop all the stacks
    maxProgressStack().pop_back();
    progressStack().pop_back();
    titleStack().pop_back();
    descriptionStack().pop_back();

    // Update the text to reflect the "previous level"
    progressDialog()->setLabelText(currentComposedLabel());

    // If we are finishing the last level, clean up
    if (!maxProgressStack().size())
    {
        if (progressDialog() != NULL)
        {
            progressDialog()->hide();
            delete progressDialog();
        }
    }

    // Make sure the Gui is repainted
    QCoreApplication::processEvents();
}




} // namespace caf 
