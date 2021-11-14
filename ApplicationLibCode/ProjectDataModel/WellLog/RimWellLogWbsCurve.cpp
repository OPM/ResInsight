#include "RimWellLogWbsCurve.h"

//==================================================================================================
///
///
//==================================================================================================

CAF_PDM_SOURCE_INIT( RimWellLogWbsCurve, "RimWellLogWbsCurve" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogWbsCurve::RimWellLogWbsCurve()
{
    CAF_PDM_InitObject( "Well Bore Stability Curve", RimWellLogCurve::wellLogCurveIconName(), "", "" );

    CAF_PDM_InitField( &m_smoothCurve, "SmoothCurve", false, "Smooth Curve" );
    CAF_PDM_InitField( &m_smoothingThreshold, "SmoothingThreshold", 0.002, "Smoothing Threshold" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogWbsCurve::smoothCurve() const
{
    return m_smoothCurve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellLogWbsCurve::smoothingThreshold() const
{
    return m_smoothingThreshold;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogWbsCurve::setSmoothCurve( bool smooth )
{
    m_smoothCurve = smooth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogWbsCurve::setSmoothingThreshold( double threshold )
{
    m_smoothingThreshold = threshold;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogWbsCurve::performDataExtraction( bool* isUsingPseudoLength )
{
    extractData( isUsingPseudoLength, m_smoothCurve(), m_smoothingThreshold() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogWbsCurve::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimWellLogExtractionCurve::defineUiOrdering( uiConfigName, uiOrdering );

    caf::PdmUiGroup* dataGroup = uiOrdering.findGroup( dataSourceGroupKeyword() );
    dataGroup->add( &m_smoothCurve );
    dataGroup->add( &m_smoothingThreshold );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogWbsCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                           const QVariant&            oldValue,
                                           const QVariant&            newValue )
{
    RimWellLogExtractionCurve::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_smoothCurve || changedField == &m_smoothingThreshold )
    {
        this->loadDataAndUpdate( true );
    }
}
