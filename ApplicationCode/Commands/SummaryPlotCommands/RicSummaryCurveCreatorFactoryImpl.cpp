/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicSummaryCurveCreatorFactoryImpl.h"
#include "RicSummaryCurveCreator.h"
#include "RicSummaryCurveCreatorDialog.h"


RicSummaryCurveCreatorFactoryImpl* RicSummaryCurveCreatorFactoryImpl::ms_instance = nullptr;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCreatorFactoryImpl::RicSummaryCurveCreatorFactoryImpl()
{
    m_dialogWithSplitter = new RicSummaryCurveCreatorDialog(nullptr);
    m_curveCreator = nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCreatorFactoryImpl::~RicSummaryCurveCreatorFactoryImpl()
{
    if (m_dialogWithSplitter != nullptr)
        delete m_dialogWithSplitter;
    if (m_curveCreator != nullptr)
        delete m_curveCreator;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCreator* RicSummaryCurveCreatorFactoryImpl::curveCreator()
{
    if (m_curveCreator)
    {
        delete m_curveCreator;
        m_curveCreator = nullptr;
    }

    // Recreate curve creator to make sure initialization is stable
    m_curveCreator = new RicSummaryCurveCreator();

    if (m_dialogWithSplitter)
    {
        m_dialogWithSplitter->setCurveCreator(m_curveCreator);
    }

    return m_curveCreator;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCreatorDialog* RicSummaryCurveCreatorFactoryImpl::dialog()
{
    if (m_dialogWithSplitter == nullptr)
        m_dialogWithSplitter = new RicSummaryCurveCreatorDialog(nullptr);

    return m_dialogWithSplitter;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCreatorFactoryImpl* RicSummaryCurveCreatorFactoryImpl::instance()
{
    if (ms_instance == nullptr)
        ms_instance = new RicSummaryCurveCreatorFactoryImpl();
    return ms_instance;
}
