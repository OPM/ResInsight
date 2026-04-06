// Compile-time benchmark: PdmObjects with container field types
// Used to measure the compile-time cost of std::vector, std::optional, std::pair specializations

#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <QString>
#include <optional>
#include <vector>

// clang-format off

#define BENCH_CONTAINER_OBJECT( N ) \
class BenchContainerObj##N : public caf::PdmObject \
{ \
    CAF_PDM_HEADER_INIT; \
public: \
    BenchContainerObj##N() \
    { \
        CAF_PDM_InitObject( "BenchContainerObj" #N ); \
        CAF_PDM_InitFieldNoDefault( &m_vecDouble,   "VecDouble",   "Vector of doubles" ); \
        CAF_PDM_InitFieldNoDefault( &m_vecInt,      "VecInt",      "Vector of ints" ); \
        CAF_PDM_InitFieldNoDefault( &m_vecString,   "VecString",   "Vector of strings" ); \
        CAF_PDM_InitFieldNoDefault( &m_optDouble,   "OptDouble",   "Optional double" ); \
        CAF_PDM_InitFieldNoDefault( &m_optString,   "OptString",   "Optional string" ); \
        auto pairInit = std::make_pair( false, 0.0 ); \
        CAF_PDM_InitField( &m_pairBoolDbl, "PairBoolDbl", pairInit, "Pair bool/double" ); \
        auto pairStrInit = std::make_pair( false, QString() ); \
        CAF_PDM_InitField( &m_pairBoolStr, "PairBoolStr", pairStrInit, "Pair bool/string" ); \
    } \
    caf::PdmField<std::vector<double>>          m_vecDouble; \
    caf::PdmField<std::vector<int>>             m_vecInt; \
    caf::PdmField<std::vector<QString>>         m_vecString; \
    caf::PdmField<std::optional<double>>        m_optDouble; \
    caf::PdmField<std::optional<QString>>       m_optString; \
    caf::PdmField<std::pair<bool, double>>      m_pairBoolDbl; \
    caf::PdmField<std::pair<bool, QString>>     m_pairBoolStr; \
};

BENCH_CONTAINER_OBJECT( 01 ) CAF_PDM_SOURCE_INIT( BenchContainerObj01, "BenchContainerObj01" );
BENCH_CONTAINER_OBJECT( 02 ) CAF_PDM_SOURCE_INIT( BenchContainerObj02, "BenchContainerObj02" );
BENCH_CONTAINER_OBJECT( 03 ) CAF_PDM_SOURCE_INIT( BenchContainerObj03, "BenchContainerObj03" );
BENCH_CONTAINER_OBJECT( 04 ) CAF_PDM_SOURCE_INIT( BenchContainerObj04, "BenchContainerObj04" );
BENCH_CONTAINER_OBJECT( 05 ) CAF_PDM_SOURCE_INIT( BenchContainerObj05, "BenchContainerObj05" );
BENCH_CONTAINER_OBJECT( 06 ) CAF_PDM_SOURCE_INIT( BenchContainerObj06, "BenchContainerObj06" );
BENCH_CONTAINER_OBJECT( 07 ) CAF_PDM_SOURCE_INIT( BenchContainerObj07, "BenchContainerObj07" );
BENCH_CONTAINER_OBJECT( 08 ) CAF_PDM_SOURCE_INIT( BenchContainerObj08, "BenchContainerObj08" );
BENCH_CONTAINER_OBJECT( 09 ) CAF_PDM_SOURCE_INIT( BenchContainerObj09, "BenchContainerObj09" );
BENCH_CONTAINER_OBJECT( 10 ) CAF_PDM_SOURCE_INIT( BenchContainerObj10, "BenchContainerObj10" );
BENCH_CONTAINER_OBJECT( 11 ) CAF_PDM_SOURCE_INIT( BenchContainerObj11, "BenchContainerObj11" );
BENCH_CONTAINER_OBJECT( 12 ) CAF_PDM_SOURCE_INIT( BenchContainerObj12, "BenchContainerObj12" );
BENCH_CONTAINER_OBJECT( 13 ) CAF_PDM_SOURCE_INIT( BenchContainerObj13, "BenchContainerObj13" );
BENCH_CONTAINER_OBJECT( 14 ) CAF_PDM_SOURCE_INIT( BenchContainerObj14, "BenchContainerObj14" );
BENCH_CONTAINER_OBJECT( 15 ) CAF_PDM_SOURCE_INIT( BenchContainerObj15, "BenchContainerObj15" );
BENCH_CONTAINER_OBJECT( 16 ) CAF_PDM_SOURCE_INIT( BenchContainerObj16, "BenchContainerObj16" );
BENCH_CONTAINER_OBJECT( 17 ) CAF_PDM_SOURCE_INIT( BenchContainerObj17, "BenchContainerObj17" );
BENCH_CONTAINER_OBJECT( 18 ) CAF_PDM_SOURCE_INIT( BenchContainerObj18, "BenchContainerObj18" );
BENCH_CONTAINER_OBJECT( 19 ) CAF_PDM_SOURCE_INIT( BenchContainerObj19, "BenchContainerObj19" );
BENCH_CONTAINER_OBJECT( 20 ) CAF_PDM_SOURCE_INIT( BenchContainerObj20, "BenchContainerObj20" );

// clang-format on
