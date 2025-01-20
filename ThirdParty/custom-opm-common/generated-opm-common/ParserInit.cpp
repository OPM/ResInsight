
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/Builtin.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/ParserInitA.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/ParserInitB.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/ParserInitC.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/ParserInitD.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/ParserInitE.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/ParserInitF.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/ParserInitG.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/ParserInitH.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/ParserInitI.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/ParserInitJ.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/ParserInitK.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/ParserInitL.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/ParserInitM.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/ParserInitN.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/ParserInitO.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/ParserInitP.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/ParserInitQ.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/ParserInitR.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/ParserInitS.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/ParserInitT.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/ParserInitU.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/ParserInitV.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/ParserInitW.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/ParserInitX.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/ParserInitY.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/ParserInitZ.hpp>

namespace Opm {
namespace ParserKeywords {
void addDefaultKeywords(Parser& p);
void addDefaultKeywords(Parser& p) {
    addDefaultKeywordsA(p);
    addDefaultKeywordsB(p);
    addDefaultKeywordsC(p);
    addDefaultKeywordsD(p);
    addDefaultKeywordsE(p);
    addDefaultKeywordsF(p);
    addDefaultKeywordsG(p);
    addDefaultKeywordsH(p);
    addDefaultKeywordsI(p);
    addDefaultKeywordsJ(p);
    addDefaultKeywordsK(p);
    addDefaultKeywordsL(p);
    addDefaultKeywordsM(p);
    addDefaultKeywordsN(p);
    addDefaultKeywordsO(p);
    addDefaultKeywordsP(p);
    addDefaultKeywordsQ(p);
    addDefaultKeywordsR(p);
    addDefaultKeywordsS(p);
    addDefaultKeywordsT(p);
    addDefaultKeywordsU(p);
    addDefaultKeywordsV(p);
    addDefaultKeywordsW(p);
    addDefaultKeywordsX(p);
    addDefaultKeywordsY(p);
    addDefaultKeywordsZ(p);

}
}
void Parser::addDefaultKeywords() {
    ParserKeywords::addDefaultKeywords(*this);
}
}
