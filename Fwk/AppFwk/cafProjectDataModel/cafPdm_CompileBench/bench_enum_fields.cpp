// Compile-time benchmark: PdmObjects with caf::AppEnum field types
// Used to measure the compile-time cost of AppEnum template instantiation

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

// clang-format off

// Each object gets its own enum to exercise distinct AppEnum<T> instantiations
#define BENCH_ENUM_OBJECT( N ) \
enum class BenchEnum##N { ValueA, ValueB, ValueC, ValueD }; \
class BenchEnumObj##N : public caf::PdmObject \
{ \
    CAF_PDM_HEADER_INIT; \
public: \
    BenchEnumObj##N() \
    { \
        CAF_PDM_InitObject( "BenchEnumObj" #N ); \
        CAF_PDM_InitField( &m_enumA, "EnumA", BenchEnum##N::ValueA, "Enum A" ); \
        CAF_PDM_InitField( &m_enumB, "EnumB", BenchEnum##N::ValueB, "Enum B" ); \
        CAF_PDM_InitField( &m_enumC, "EnumC", BenchEnum##N::ValueC, "Enum C" ); \
    } \
    caf::PdmField<caf::AppEnum<BenchEnum##N>> m_enumA; \
    caf::PdmField<caf::AppEnum<BenchEnum##N>> m_enumB; \
    caf::PdmField<caf::AppEnum<BenchEnum##N>> m_enumC; \
};

BENCH_ENUM_OBJECT( 01 ) CAF_PDM_SOURCE_INIT( BenchEnumObj01, "BenchEnumObj01" );
BENCH_ENUM_OBJECT( 02 ) CAF_PDM_SOURCE_INIT( BenchEnumObj02, "BenchEnumObj02" );
BENCH_ENUM_OBJECT( 03 ) CAF_PDM_SOURCE_INIT( BenchEnumObj03, "BenchEnumObj03" );
BENCH_ENUM_OBJECT( 04 ) CAF_PDM_SOURCE_INIT( BenchEnumObj04, "BenchEnumObj04" );
BENCH_ENUM_OBJECT( 05 ) CAF_PDM_SOURCE_INIT( BenchEnumObj05, "BenchEnumObj05" );
BENCH_ENUM_OBJECT( 06 ) CAF_PDM_SOURCE_INIT( BenchEnumObj06, "BenchEnumObj06" );
BENCH_ENUM_OBJECT( 07 ) CAF_PDM_SOURCE_INIT( BenchEnumObj07, "BenchEnumObj07" );
BENCH_ENUM_OBJECT( 08 ) CAF_PDM_SOURCE_INIT( BenchEnumObj08, "BenchEnumObj08" );
BENCH_ENUM_OBJECT( 09 ) CAF_PDM_SOURCE_INIT( BenchEnumObj09, "BenchEnumObj09" );
BENCH_ENUM_OBJECT( 10 ) CAF_PDM_SOURCE_INIT( BenchEnumObj10, "BenchEnumObj10" );
BENCH_ENUM_OBJECT( 11 ) CAF_PDM_SOURCE_INIT( BenchEnumObj11, "BenchEnumObj11" );
BENCH_ENUM_OBJECT( 12 ) CAF_PDM_SOURCE_INIT( BenchEnumObj12, "BenchEnumObj12" );
BENCH_ENUM_OBJECT( 13 ) CAF_PDM_SOURCE_INIT( BenchEnumObj13, "BenchEnumObj13" );
BENCH_ENUM_OBJECT( 14 ) CAF_PDM_SOURCE_INIT( BenchEnumObj14, "BenchEnumObj14" );
BENCH_ENUM_OBJECT( 15 ) CAF_PDM_SOURCE_INIT( BenchEnumObj15, "BenchEnumObj15" );
BENCH_ENUM_OBJECT( 16 ) CAF_PDM_SOURCE_INIT( BenchEnumObj16, "BenchEnumObj16" );
BENCH_ENUM_OBJECT( 17 ) CAF_PDM_SOURCE_INIT( BenchEnumObj17, "BenchEnumObj17" );
BENCH_ENUM_OBJECT( 18 ) CAF_PDM_SOURCE_INIT( BenchEnumObj18, "BenchEnumObj18" );
BENCH_ENUM_OBJECT( 19 ) CAF_PDM_SOURCE_INIT( BenchEnumObj19, "BenchEnumObj19" );
BENCH_ENUM_OBJECT( 20 ) CAF_PDM_SOURCE_INIT( BenchEnumObj20, "BenchEnumObj20" );

// clang-format on
