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
    CAF_PDM_InitObject( "Well Bore Stability Curve", RimWellLogCurve::wellLogCurveIconName() );

    CAF_PDM_InitField( &m_smoothCurve, "SmoothCurve", false, "Smooth Curve" );
    CAF_PDM_InitField( &m_smoothingThreshold, "SmoothingThreshold", 0.002, "Smoothing Threshold" );

    CAF_PDM_InitField( &m_maximumCurvePointInterval, "MaximumCurvePointInterval", std::make_pair( false, 10.0 ), "Maximum Curve Point Interval" );
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
void RimWellLogWbsCurve::enableMaximumCurvePointInterval( bool enable )
{
    m_maximumCurvePointInterval.v().first = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogWbsCurve::setMaximumCurvePointInterval( double interval )
{
    m_maximumCurvePointInterval.v().second = interval;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogWbsCurve::performDataExtraction( bool* isUsingPseudoLength )
{
    auto smoothingThreshold       = createOptional( m_smoothCurve(), m_smoothingThreshold() );
    auto maxDistanceBetweenPoints = createOptional( m_maximumCurvePointInterval() );

    extractData( isUsingPseudoLength, smoothingThreshold, maxDistanceBetweenPoints );
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
    dataGroup->add( &m_maximumCurvePointInterval );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogWbsCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimWellLogExtractionCurve::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_smoothCurve || changedField == &m_smoothingThreshold || changedField == &m_maximumCurvePointInterval )
    {
        this->loadDataAndUpdate( true );
    }
}
