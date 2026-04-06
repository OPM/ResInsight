// Compile-time benchmark: PdmObjects with basic field types (int, double, bool, QString)
// Used to measure the compile-time cost of cafPdmField.h and cafPdmFieldTraits.h

#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <QString>

// clang-format off

#define BENCH_BASIC_OBJECT( N ) \
class BenchBasicObj##N : public caf::PdmObject \
{ \
    CAF_PDM_HEADER_INIT; \
public: \
    BenchBasicObj##N() \
    { \
        CAF_PDM_InitObject( "BenchBasicObj" #N ); \
        CAF_PDM_InitField( &m_intA,  "IntA",  0,         "Int A" ); \
        CAF_PDM_InitField( &m_intB,  "IntB",  0,         "Int B" ); \
        CAF_PDM_InitField( &m_dblA,  "DblA",  0.0,       "Double A" ); \
        CAF_PDM_InitField( &m_dblB,  "DblB",  0.0,       "Double B" ); \
        CAF_PDM_InitField( &m_boolA, "BoolA", false,     "Bool A" ); \
        CAF_PDM_InitField( &m_boolB, "BoolB", false,     "Bool B" ); \
        CAF_PDM_InitField( &m_strA,  "StrA",  QString(), "String A" ); \
        CAF_PDM_InitField( &m_strB,  "StrB",  QString(), "String B" ); \
    } \
    caf::PdmField<int>     m_intA; \
    caf::PdmField<int>     m_intB; \
    caf::PdmField<double>  m_dblA; \
    caf::PdmField<double>  m_dblB; \
    caf::PdmField<bool>    m_boolA; \
    caf::PdmField<bool>    m_boolB; \
    caf::PdmField<QString> m_strA; \
    caf::PdmField<QString> m_strB; \
};

BENCH_BASIC_OBJECT( 01 ) CAF_PDM_SOURCE_INIT( BenchBasicObj01, "BenchBasicObj01" );
BENCH_BASIC_OBJECT( 02 ) CAF_PDM_SOURCE_INIT( BenchBasicObj02, "BenchBasicObj02" );
BENCH_BASIC_OBJECT( 03 ) CAF_PDM_SOURCE_INIT( BenchBasicObj03, "BenchBasicObj03" );
BENCH_BASIC_OBJECT( 04 ) CAF_PDM_SOURCE_INIT( BenchBasicObj04, "BenchBasicObj04" );
BENCH_BASIC_OBJECT( 05 ) CAF_PDM_SOURCE_INIT( BenchBasicObj05, "BenchBasicObj05" );
BENCH_BASIC_OBJECT( 06 ) CAF_PDM_SOURCE_INIT( BenchBasicObj06, "BenchBasicObj06" );
BENCH_BASIC_OBJECT( 07 ) CAF_PDM_SOURCE_INIT( BenchBasicObj07, "BenchBasicObj07" );
BENCH_BASIC_OBJECT( 08 ) CAF_PDM_SOURCE_INIT( BenchBasicObj08, "BenchBasicObj08" );
BENCH_BASIC_OBJECT( 09 ) CAF_PDM_SOURCE_INIT( BenchBasicObj09, "BenchBasicObj09" );
BENCH_BASIC_OBJECT( 10 ) CAF_PDM_SOURCE_INIT( BenchBasicObj10, "BenchBasicObj10" );
BENCH_BASIC_OBJECT( 11 ) CAF_PDM_SOURCE_INIT( BenchBasicObj11, "BenchBasicObj11" );
BENCH_BASIC_OBJECT( 12 ) CAF_PDM_SOURCE_INIT( BenchBasicObj12, "BenchBasicObj12" );
BENCH_BASIC_OBJECT( 13 ) CAF_PDM_SOURCE_INIT( BenchBasicObj13, "BenchBasicObj13" );
BENCH_BASIC_OBJECT( 14 ) CAF_PDM_SOURCE_INIT( BenchBasicObj14, "BenchBasicObj14" );
BENCH_BASIC_OBJECT( 15 ) CAF_PDM_SOURCE_INIT( BenchBasicObj15, "BenchBasicObj15" );
BENCH_BASIC_OBJECT( 16 ) CAF_PDM_SOURCE_INIT( BenchBasicObj16, "BenchBasicObj16" );
BENCH_BASIC_OBJECT( 17 ) CAF_PDM_SOURCE_INIT( BenchBasicObj17, "BenchBasicObj17" );
BENCH_BASIC_OBJECT( 18 ) CAF_PDM_SOURCE_INIT( BenchBasicObj18, "BenchBasicObj18" );
BENCH_BASIC_OBJECT( 19 ) CAF_PDM_SOURCE_INIT( BenchBasicObj19, "BenchBasicObj19" );
BENCH_BASIC_OBJECT( 20 ) CAF_PDM_SOURCE_INIT( BenchBasicObj20, "BenchBasicObj20" );

// clang-format on
