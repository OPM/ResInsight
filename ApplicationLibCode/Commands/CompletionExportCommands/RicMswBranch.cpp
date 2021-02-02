#include "RicMswBranch.h"
#include "RicMswCompletions.h"
#include "RicMswSegment.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswBranch::RicMswBranch( const QString& label, double initialMD, double initialTVD )
    : RicMswItem( label )
    , m_initialMD( initialMD )
    , m_initialTVD( initialTVD )
    , m_branchNumber( -1 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswBranch::addSegment( std::unique_ptr<RicMswSegment> segment )
{
    m_segments.push_back( std::move( segment ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswBranch::insertAfterSegment( const RicMswSegment* insertAfter, std::unique_ptr<RicMswSegment> insertItem )
{
    auto it = std::find_if( m_segments.begin(), m_segments.end(), [insertAfter]( auto& item ) {
        return item.get() == insertAfter;
    } );

    m_segments.insert( it, std::move( insertItem ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswBranch::sortSegments()
{
    std::sort( m_segments.begin(),
               m_segments.end(),
               []( const std::unique_ptr<RicMswSegment>& lhs, const std::unique_ptr<RicMswSegment>& rhs ) {
                   return *lhs < *rhs;
               } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswBranch::startMD() const
{
    return m_initialMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswBranch::startTVD() const
{
    return m_initialTVD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswBranch::endMD() const
{
    if ( !m_segments.empty() )
    {
        return m_segments.back()->endMD();
    }
    return m_initialMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswBranch::endTVD() const
{
    if ( !m_segments.empty() )
    {
        return m_segments.back()->endTVD();
    }
    return m_initialTVD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RicMswBranch::branchNumber() const
{
    return m_branchNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswBranch::setBranchNumber( int branchNumber )
{
    m_branchNumber = branchNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<const RicMswSegment*> RicMswBranch::segments() const
{
    std::vector<const RicMswSegment*> allSegments;
    for ( const auto& segment : m_segments )
    {
        allSegments.push_back( segment.get() );
    }
    return allSegments;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RicMswSegment*> RicMswBranch::segments()
{
    std::vector<RicMswSegment*> allSegments;
    for ( auto& segment : m_segments )
    {
        allSegments.push_back( segment.get() );
    }
    return allSegments;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RicMswBranch::segmentCount() const
{
    return m_segments.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<const RicMswBranch*> RicMswBranch::branches() const
{
    std::vector<const RicMswBranch*> branches;
    for ( const auto& branch : m_branches )
    {
        branches.push_back( branch.get() );
    }
    return branches;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RicMswBranch*> RicMswBranch::branches()
{
    std::vector<RicMswBranch*> branches;
    for ( auto& branch : m_branches )
    {
        branches.push_back( branch.get() );
    }
    return branches;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswBranch::addChildBranch( std::unique_ptr<RicMswBranch> branch )
{
    m_branches.push_back( std::move( branch ) );
}
