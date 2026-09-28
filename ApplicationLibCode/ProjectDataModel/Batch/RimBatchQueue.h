/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "cafPdmObject.h"

#include "cafPdmPointer.h"

#include <QString>
#include <QStringList>

#include <memory>
#include <utility>

class RimProcess;

//==================================================================================================
///
///
//==================================================================================================
class RimBatchQueue : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    static RimBatchQueue* createBatchQueue();

    virtual void queueProcess( std::shared_ptr<RimProcess> process, int nProcesses ) = 0;
    virtual void stopProcess()                                                       = 0;

protected:
    std::pair<bool, QStringList> runCommand( QStringList command );
    QStringList                  buildLaunchScript();
    QString                      generateJobName();

    std::shared_ptr<RimProcess> m_process;
};
