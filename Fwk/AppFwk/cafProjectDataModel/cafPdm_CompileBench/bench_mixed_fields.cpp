// Compile-time benchmark: PdmObjects with all field types combined
// This file represents the highest-signal benchmark: the full PDM field stack
// including basic, enum, container, and FilePath types in every object.

#include "cafAppEnum.h"
#include "cafFilePath.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <QString>
#include <optional>
#include <vector>

// clang-format off

#define BENCH_MIXED_OBJECT( N ) \
enum class BenchMixedEnum##N { Alpha, Beta, Gamma, Delta }; \
class BenchMixedObj##N : public caf::PdmObject \
{ \
    CAF_PDM_HEADER_INIT; \
public: \
    BenchMixedObj##N() \
    { \
        CAF_PDM_InitObject( "BenchMixedObj" #N ); \
        CAF_PDM_InitField( &m_intVal,    "IntVal",    0,                           "Int" ); \
        CAF_PDM_InitField( &m_dblVal,    "DblVal",    0.0,                         "Double" ); \
        CAF_PDM_InitField( &m_boolVal,   "BoolVal",   false,                       "Bool" ); \
        CAF_PDM_InitField( &m_strVal,    "StrVal",    QString(),                   "String" ); \
        CAF_PDM_InitField( &m_enumVal,   "EnumVal",   BenchMixedEnum##N::Alpha,    "Enum" ); \
        CAF_PDM_InitFieldNoDefault( &m_vecDbl,    "VecDbl",    "Vector of doubles" ); \
        CAF_PDM_InitFieldNoDefault( &m_vecStr,    "VecStr",    "Vector of strings" ); \
        CAF_PDM_InitFieldNoDefault( &m_optDbl,    "OptDbl",    "Optional double" ); \
        CAF_PDM_InitFieldNoDefault( &m_fileA,     "FileA",     "File A" ); \
        auto pairInit = std::make_pair( false, 0.0 ); \
        CAF_PDM_InitField( &m_pairVal, "PairVal", pairInit, "Pair" ); \
    } \
    caf::PdmField<int>                              m_intVal; \
    caf::PdmField<double>                           m_dblVal; \
    caf::PdmField<bool>                             m_boolVal; \
    caf::PdmField<QString>                          m_strVal; \
    caf::PdmField<caf::AppEnum<BenchMixedEnum##N>>  m_enumVal; \
    caf::PdmField<std::vector<double>>              m_vecDbl; \
    caf::PdmField<std::vector<QString>>             m_vecStr; \
    caf::PdmField<std::optional<double>>            m_optDbl; \
    caf::PdmField<caf::FilePath>                    m_fileA; \
    caf::PdmField<std::pair<bool, double>>          m_pairVal; \
};

BENCH_MIXED_OBJECT( 01 ) CAF_PDM_SOURCE_INIT( BenchMixedObj01, "BenchMixedObj01" );
BENCH_MIXED_OBJECT( 02 ) CAF_PDM_SOURCE_INIT( BenchMixedObj02, "BenchMixedObj02" );
BENCH_MIXED_OBJECT( 03 ) CAF_PDM_SOURCE_INIT( BenchMixedObj03, "BenchMixedObj03" );
BENCH_MIXED_OBJECT( 04 ) CAF_PDM_SOURCE_INIT( BenchMixedObj04, "BenchMixedObj04" );
BENCH_MIXED_OBJECT( 05 ) CAF_PDM_SOURCE_INIT( BenchMixedObj05, "BenchMixedObj05" );
BENCH_MIXED_OBJECT( 06 ) CAF_PDM_SOURCE_INIT( BenchMixedObj06, "BenchMixedObj06" );
BENCH_MIXED_OBJECT( 07 ) CAF_PDM_SOURCE_INIT( BenchMixedObj07, "BenchMixedObj07" );
BENCH_MIXED_OBJECT( 08 ) CAF_PDM_SOURCE_INIT( BenchMixedObj08, "BenchMixedObj08" );
BENCH_MIXED_OBJECT( 09 ) CAF_PDM_SOURCE_INIT( BenchMixedObj09, "BenchMixedObj09" );
BENCH_MIXED_OBJECT( 10 ) CAF_PDM_SOURCE_INIT( BenchMixedObj10, "BenchMixedObj10" );
BENCH_MIXED_OBJECT( 11 ) CAF_PDM_SOURCE_INIT( BenchMixedObj11, "BenchMixedObj11" );
BENCH_MIXED_OBJECT( 12 ) CAF_PDM_SOURCE_INIT( BenchMixedObj12, "BenchMixedObj12" );
BENCH_MIXED_OBJECT( 13 ) CAF_PDM_SOURCE_INIT( BenchMixedObj13, "BenchMixedObj13" );
BENCH_MIXED_OBJECT( 14 ) CAF_PDM_SOURCE_INIT( BenchMixedObj14, "BenchMixedObj14" );
BENCH_MIXED_OBJECT( 15 ) CAF_PDM_SOURCE_INIT( BenchMixedObj15, "BenchMixedObj15" );
BENCH_MIXED_OBJECT( 16 ) CAF_PDM_SOURCE_INIT( BenchMixedObj16, "BenchMixedObj16" );
BENCH_MIXED_OBJECT( 17 ) CAF_PDM_SOURCE_INIT( BenchMixedObj17, "BenchMixedObj17" );
BENCH_MIXED_OBJECT( 18 ) CAF_PDM_SOURCE_INIT( BenchMixedObj18, "BenchMixedObj18" );
BENCH_MIXED_OBJECT( 19 ) CAF_PDM_SOURCE_INIT( BenchMixedObj19, "BenchMixedObj19" );
BENCH_MIXED_OBJECT( 20 ) CAF_PDM_SOURCE_INIT( BenchMixedObj20, "BenchMixedObj20" );

// clang-format on
