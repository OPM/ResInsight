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

#include "gtest/gtest.h"

#include "RiaHpcTools.h"

#include <QStringList>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaHpcTools, decodeSlurmQueues )
{
    QStringList input;
    input.append( "PARTITION AVAIL  TIMELIMIT  NODES  STATE NODELIST" );
    input.append( "normal*      up   infinite      1 drain* node-21-03" );
    input.append( "normal*      up   infinite     70    mix node-20-[00-27],node-21-[00-02,04-31]" );
    input.append( "normal*      up   infinite      1   idle node-21-32" );
    input.append(
        "extra        up   infinite     53 drain* node-22-[00,03,08-10,16-19,26,43],node-23-[00,16-19,21,42-43],node-24-[14,27,33]" );
    input.append( "extra        up   infinite      1  drain node-25-37" );
    input.append(
        "extra        up   infinite    200  alloc node-22-[01-02,04-07,11-15,20-25],node-23-[15,20],node-24-[00-13,15-26,28-32,34-43]" );
    input.append( "extra        up   infinite     51   idle node-22-[27-42],node-23-[01-14,22-41],node-25-43" );

    auto queueNames = RiaHpcTools::decodeSlurmQueues( input );

    ASSERT_EQ( queueNames.length(), 2 );

    ASSERT_TRUE( queueNames.contains( "normal" ) );
    ASSERT_TRUE( queueNames.contains( "extra" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaHpcTools, decodeLsfQueues )
{
    QStringList input;
    input.append( "QUEUE_NAME      PRIO STATUS          MAX JL/U JL/P JL/H NJOBS  PEND   RUN  SUSP  RSV" );
    input.append( "urgent           99  Open:Active       -    -    -    -     0     0     0     0    0" );
    input.append( "wait             90  Open:Active       -    -    -    -     0     0     0     0    0" );
    input.append( "extra            60  Open:Active       -    -    -    -     0     0     0     0    0" );
    input.append( "super            60  Open:Active       -    -    -    -     0     0     0     0    0" );
    input.append( "normal           50  Open:Active       -    -    -    -     0     0     0     0    0" );

    auto queueNames = RiaHpcTools::decodeLsfQueues( input );

    ASSERT_EQ( queueNames.length(), 5 );

    ASSERT_TRUE( queueNames.contains( "normal" ) );
    ASSERT_TRUE( queueNames.contains( "extra" ) );
    ASSERT_TRUE( queueNames.contains( "super" ) );
    ASSERT_TRUE( queueNames.contains( "wait" ) );
    ASSERT_TRUE( queueNames.contains( "urgent" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaHpcTools, decodeSlurmJobs )
{
    QStringList input;

    input.append( "80 RI_DROGON-2 0:51 RUNNING simulations compute_node" );
    input.append( "79 RI_DROGON-1 0:54 RUNNING simulations compute_node" );
    input.append( "78 RI_DROGON-0 0:57 RUNNING simulations compute_node" );
    input.append( "77 RI_DROGON-3 1:00 RUNNING normal compute_node" );

    auto jobInfos = RiaHpcTools::decodeSlurmJobs( input );

    ASSERT_TRUE( jobInfos.size() == 4 );

    ASSERT_TRUE( jobInfos.contains( "77" ) );
    ASSERT_TRUE( jobInfos.contains( "78" ) );
    ASSERT_TRUE( jobInfos.contains( "79" ) );
    ASSERT_TRUE( jobInfos.contains( "80" ) );

    ASSERT_TRUE( jobInfos["80"].id == "80" );
    ASSERT_TRUE( jobInfos["79"].id == "79" );
    ASSERT_TRUE( jobInfos["78"].id == "78" );
    ASSERT_TRUE( jobInfos["77"].id == "77" );

    ASSERT_TRUE( jobInfos["80"].name == "RI_DROGON-2" );
    ASSERT_TRUE( jobInfos["79"].name == "RI_DROGON-1" );
    ASSERT_TRUE( jobInfos["78"].name == "RI_DROGON-0" );
    ASSERT_TRUE( jobInfos["77"].name == "RI_DROGON-3" );

    ASSERT_TRUE( jobInfos["80"].queue == "simulations" );
    ASSERT_TRUE( jobInfos["79"].queue == "simulations" );
    ASSERT_TRUE( jobInfos["78"].queue == "simulations" );
    ASSERT_TRUE( jobInfos["77"].queue == "normal" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaHpcTools, decodeLsfJobs )
{
    QStringList input;

    input.append( "612950     flow TEST-B               22 second(s) RUN      mr              3*st-rsb04-04-01         " );
    input.append( "612949     flow TEST-A               30 second(s) RUN      mr              5*st-rsb03-03-08  " );
    input.append( "612952     flow TEST-C               12 second(s) RUN      jr              7*st-rsv17-17-16" );
    input.append( "612951     flow TEST-D               15 second(s) RUN      mr              6*st-rsb04-04-02         " );

    auto jobInfos = RiaHpcTools::decodeLsfJobs( input );

    ASSERT_TRUE( jobInfos.size() == 4 );

    ASSERT_TRUE( jobInfos.contains( "612949" ) );
    ASSERT_TRUE( jobInfos.contains( "612950" ) );
    ASSERT_TRUE( jobInfos.contains( "612951" ) );
    ASSERT_TRUE( jobInfos.contains( "612952" ) );

    ASSERT_TRUE( jobInfos["612950"].id == "612950" );
    ASSERT_TRUE( jobInfos["612949"].id == "612949" );
    ASSERT_TRUE( jobInfos["612952"].id == "612952" );
    ASSERT_TRUE( jobInfos["612951"].id == "612951" );

    ASSERT_TRUE( jobInfos["612950"].name == "flow TEST-B" );
    ASSERT_TRUE( jobInfos["612949"].name == "flow TEST-A" );
    ASSERT_TRUE( jobInfos["612952"].name == "flow TEST-C" );
    ASSERT_TRUE( jobInfos["612951"].name == "flow TEST-D" );

    ASSERT_TRUE( jobInfos["612949"].queue == "mr" );
    ASSERT_TRUE( jobInfos["612950"].queue == "mr" );
    ASSERT_TRUE( jobInfos["612952"].queue == "jr" );
    ASSERT_TRUE( jobInfos["612951"].queue == "mr" );
}
