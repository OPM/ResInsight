
#include <opm/input/eclipse/Parser/Parser.hpp>
#include<opm/input/eclipse/Parser/ParserKeywords/ParserInitI.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/I.hpp>

namespace Opm {
namespace ParserKeywords {
void addDefaultKeywordsI([[maybe_unused]] Parser& p){
    //Builtin keywords;
    p.addParserKeyword( IHOST() );
    p.addParserKeyword( IMBNUM() );
    p.addParserKeyword( IMBNUMMF() );
    p.addParserKeyword( IMKRVD() );
    p.addParserKeyword( IMPCVD() );
    p.addParserKeyword( IMPES() );
    p.addParserKeyword( IMPLICIT() );
    p.addParserKeyword( IMPORT() );
    p.addParserKeyword( IMPTVD() );
    p.addParserKeyword( IMSPCVD() );
    p.addParserKeyword( INCLUDE() );
    p.addParserKeyword( INIT() );
    p.addParserKeyword( INRAD() );
    p.addParserKeyword( INSPEC() );
    p.addParserKeyword( INTPC() );
    p.addParserKeyword( IONROCK() );
    p.addParserKeyword( IONXROCK() );
    p.addParserKeyword( IONXSURF() );
    p.addParserKeyword( IPCG() );
    p.addParserKeyword( IPCW() );
    p.addParserKeyword( ISGCR() );
    p.addParserKeyword( ISGL() );
    p.addParserKeyword( ISGLPC() );
    p.addParserKeyword( ISGU() );
    p.addParserKeyword( ISOGCR() );
    p.addParserKeyword( ISOLNUM() );
    p.addParserKeyword( ISOWCR() );
    p.addParserKeyword( ISWCR() );
    p.addParserKeyword( ISWL() );
    p.addParserKeyword( ISWLPC() );
    p.addParserKeyword( ISWU() );


}
}
}
