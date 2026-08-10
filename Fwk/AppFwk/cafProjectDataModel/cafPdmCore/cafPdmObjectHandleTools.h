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

    // Delete all objects in the vector, then clear it.
    //
    // Use this instead of a hand written delete loop. Objects that are still owned by a
    // PdmChildArrayField must not be deleted this way, call deleteChildren() on the field instead so
    // the observer signals are disconnected first.
    //
    template <typename ObjectType>
    void deleteObjects( std::vector<ObjectType*>& objects )
    {
        for ( auto* object : objects )
        {
            delete object;
        }

        objects.clear();
    }
} //namespace PdmObjectHandleTools
} //namespace caf
