#pragma once

namespace caf
{
namespace PdmObjectHandleTools
{
    // Utility function to get all objects of a certain type from a vector of objects.
    // firstAncestorOrThisOfType is applied to the referring objects of the source objects
    //
    // Example usage:
    // std::vector<RimSummaryEnsemble*> ensembles;
    // std::vector<RimDepthTrackPlot*> plots =
    //   caf::PdmObjectHandleTools::referringAncestorOfType<RimDepthTrackPlot, RimSummaryEnsemble>( ensembles );
    //
    template <typename DestinationType, typename SourceType>
    std::set<DestinationType*> referringAncestorOfType( const std::vector<SourceType*>& sourceObjects )
    {
        std::set<DestinationType*> destinationObjects;
        for ( auto source : sourceObjects )
        {
            if ( !source ) continue;

            for ( auto object : source->objectsWithReferringPtrFields() )
            {
                if ( !object ) continue;

                if ( auto candidate = object->template firstAncestorOrThisOfType<DestinationType>() )
                {
                    destinationObjects.insert( candidate );
                }
            }
        }

        return destinationObjects;
    }
} //namespace PdmObjectHandleTools
} //namespace caf
