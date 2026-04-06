// Compile-time benchmark: PdmObjects with caf::FilePath field types
// Used to measure the compile-time cost of cafFilePath.h specializations

#include "cafFilePath.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <vector>

// clang-format off

#define BENCH_FILEPATH_OBJECT( N ) \
class BenchFilePathObj##N : public caf::PdmObject \
{ \
    CAF_PDM_HEADER_INIT; \
public: \
    BenchFilePathObj##N() \
    { \
        CAF_PDM_InitObject( "BenchFilePathObj" #N ); \
        CAF_PDM_InitFieldNoDefault( &m_fileA,  "FileA",  "File A" ); \
        CAF_PDM_InitFieldNoDefault( &m_fileB,  "FileB",  "File B" ); \
        CAF_PDM_InitFieldNoDefault( &m_fileC,  "FileC",  "File C" ); \
        CAF_PDM_InitFieldNoDefault( &m_files,  "Files",  "File list" ); \
    } \
    caf::PdmField<caf::FilePath>              m_fileA; \
    caf::PdmField<caf::FilePath>              m_fileB; \
    caf::PdmField<caf::FilePath>              m_fileC; \
    caf::PdmField<std::vector<caf::FilePath>> m_files; \
};

BENCH_FILEPATH_OBJECT( 01 ) CAF_PDM_SOURCE_INIT( BenchFilePathObj01, "BenchFilePathObj01" );
BENCH_FILEPATH_OBJECT( 02 ) CAF_PDM_SOURCE_INIT( BenchFilePathObj02, "BenchFilePathObj02" );
BENCH_FILEPATH_OBJECT( 03 ) CAF_PDM_SOURCE_INIT( BenchFilePathObj03, "BenchFilePathObj03" );
BENCH_FILEPATH_OBJECT( 04 ) CAF_PDM_SOURCE_INIT( BenchFilePathObj04, "BenchFilePathObj04" );
BENCH_FILEPATH_OBJECT( 05 ) CAF_PDM_SOURCE_INIT( BenchFilePathObj05, "BenchFilePathObj05" );
BENCH_FILEPATH_OBJECT( 06 ) CAF_PDM_SOURCE_INIT( BenchFilePathObj06, "BenchFilePathObj06" );
BENCH_FILEPATH_OBJECT( 07 ) CAF_PDM_SOURCE_INIT( BenchFilePathObj07, "BenchFilePathObj07" );
BENCH_FILEPATH_OBJECT( 08 ) CAF_PDM_SOURCE_INIT( BenchFilePathObj08, "BenchFilePathObj08" );
BENCH_FILEPATH_OBJECT( 09 ) CAF_PDM_SOURCE_INIT( BenchFilePathObj09, "BenchFilePathObj09" );
BENCH_FILEPATH_OBJECT( 10 ) CAF_PDM_SOURCE_INIT( BenchFilePathObj10, "BenchFilePathObj10" );
BENCH_FILEPATH_OBJECT( 11 ) CAF_PDM_SOURCE_INIT( BenchFilePathObj11, "BenchFilePathObj11" );
BENCH_FILEPATH_OBJECT( 12 ) CAF_PDM_SOURCE_INIT( BenchFilePathObj12, "BenchFilePathObj12" );
BENCH_FILEPATH_OBJECT( 13 ) CAF_PDM_SOURCE_INIT( BenchFilePathObj13, "BenchFilePathObj13" );
BENCH_FILEPATH_OBJECT( 14 ) CAF_PDM_SOURCE_INIT( BenchFilePathObj14, "BenchFilePathObj14" );
BENCH_FILEPATH_OBJECT( 15 ) CAF_PDM_SOURCE_INIT( BenchFilePathObj15, "BenchFilePathObj15" );
BENCH_FILEPATH_OBJECT( 16 ) CAF_PDM_SOURCE_INIT( BenchFilePathObj16, "BenchFilePathObj16" );
BENCH_FILEPATH_OBJECT( 17 ) CAF_PDM_SOURCE_INIT( BenchFilePathObj17, "BenchFilePathObj17" );
BENCH_FILEPATH_OBJECT( 18 ) CAF_PDM_SOURCE_INIT( BenchFilePathObj18, "BenchFilePathObj18" );
BENCH_FILEPATH_OBJECT( 19 ) CAF_PDM_SOURCE_INIT( BenchFilePathObj19, "BenchFilePathObj19" );
BENCH_FILEPATH_OBJECT( 20 ) CAF_PDM_SOURCE_INIT( BenchFilePathObj20, "BenchFilePathObj20" );

// clang-format on
