#include "RimAbstractWellPath.h"

#include "RigWellPath.h"

CAF_PDM_ABSTRACT_SOURCE_INIT( RimAbstractWellPath, "AbstractWellPath" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimAbstractWellPath::startMD() const
{
    if ( wellPathGeometry() )
    {
        return wellPathGeometry()->measuredDepths().front();
    }
    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimAbstractWellPath::endMD() const
{
    if ( wellPathGeometry() )
    {
        return wellPathGeometry()->measuredDepths().back();
    }
    return std::numeric_limits<double>::infinity();
}
