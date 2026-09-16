/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RiaHpcDefines.h"

#include "cafAppEnum.h"

namespace caf
{
template <>
void caf::AppEnum<RiaDefines::BatchSchedulerType>::setUp()
{
    addItem( RiaDefines::BatchSchedulerType::LOCAL_COMPUTER, "LOCAL_COMPUTER", "Local Computer" );
    addItem( RiaDefines::BatchSchedulerType::SLURM, "SLURM", "Slurm" );
    addItem( RiaDefines::BatchSchedulerType::LSF, "LSF", "LSF" );
    setDefault( RiaDefines::BatchSchedulerType::LOCAL_COMPUTER );
}
} // namespace caf
