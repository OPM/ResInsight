#include "RimEnsembleParameterColorHandlerInterface.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
namespace caf
{
template <>
void AppEnum<RimEnsembleParameterColorHandlerInterface::ColorMode>::setUp()
{
    addItem( RimEnsembleParameterColorHandlerInterface::ColorMode::SINGLE_COLOR, "SINGLE_COLOR", "Single Color" );
    addItem( RimEnsembleParameterColorHandlerInterface::ColorMode::BY_ENSEMBLE_PARAM,
             "BY_ENSEMBLE_PARAM",
             "By Ensemble Parameter" );
    setDefault( RimEnsembleParameterColorHandlerInterface::ColorMode::SINGLE_COLOR );
}
} // namespace caf
