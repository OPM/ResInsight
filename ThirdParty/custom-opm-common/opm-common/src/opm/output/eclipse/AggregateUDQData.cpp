/*
  Copyright 2018 Statoil ASA

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <opm/common/OpmLog/OpmLog.hpp>
#include <opm/output/eclipse/AggregateUDQData.hpp>
#include <opm/output/eclipse/AggregateGroupData.hpp>
#include <opm/output/eclipse/WriteRestartHelpers.hpp>
#include <opm/output/eclipse/InteHEAD.hpp>
#include <opm/output/eclipse/VectorItems/intehead.hpp>

#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/Runspec.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>

#include <opm/input/eclipse/Schedule/UDQ/UDQState.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQInput.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQConfig.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQInput.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQActive.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQDefine.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQAssign.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQEnums.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQParams.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQFunctionTable.hpp>

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <fmt/format.h>
#include <iostream>
#include <sstream>
#include <string>


// #####################################################################
// Class Opm::RestartIO::Helpers::AggregateGroupData
// ---------------------------------------------------------------------

namespace VI = ::Opm::RestartIO::Helpers::VectorItems;
namespace {

    // maximum number of groups
    std::size_t ngmaxz(const std::vector<int>& inteHead)
    {
        return inteHead[20];
    }

    // maximum number of wells
    std::size_t nwmaxz(const std::vector<int>& inteHead)
    {
        return inteHead[163];
    }


    // function to return true if token is a function
    bool tokenTypeFunc(const Opm::UDQTokenType& token) {
        bool type = false;
        if (Opm::UDQ::scalarFunc(token) ||
            Opm::UDQ::elementalUnaryFunc(token) ||
            (token == Opm::UDQTokenType::table_lookup)) {
            type = true;
        }
        return type;
    }

    // function to return true if token is a binary operator: type power (exponentiation)
    bool tokenTypeBinaryPowOp(const Opm::UDQTokenType& token) {
        return (token == Opm::UDQTokenType::binary_op_pow) ? true: false;
    }

    // function to return true if token is a binary operator: type multiply or divide
    bool tokenTypeBinaryMulDivOp(const Opm::UDQTokenType& token) {
        bool type = false;
        std::vector <Opm::UDQTokenType> type_1 = {
            Opm::UDQTokenType::binary_op_div,
            Opm::UDQTokenType::binary_op_mul
        };
        for (const auto& tok_type : type_1) {
            if (token == tok_type) {
                type = true;
                break;
            }
        }
        return type;
    }

    // function to return true if token is a binary operator: type add or subtract
    bool tokenTypeBinaryAddSubOp(const Opm::UDQTokenType& token) {
        bool type = false;
        std::vector <Opm::UDQTokenType> type_1 = {
            Opm::UDQTokenType::binary_op_add,
            Opm::UDQTokenType::binary_op_sub
        };
        for (const auto& tok_type : type_1) {
            if (token == tok_type) {
                type = true;
                break;
            }
        }
        return type;
    }

    // function to return true if token is a binary union operator
    bool tokenTypeBinaryUnionOp(const Opm::UDQTokenType& token) {
        bool type = false;
        std::vector <Opm::UDQTokenType> type_1 = {
            Opm::UDQTokenType::binary_op_uadd,
            Opm::UDQTokenType::binary_op_umul,
            Opm::UDQTokenType::binary_op_umin,
            Opm::UDQTokenType::binary_op_umax
        };
        for (const auto& tok_type : type_1) {
            if (token == tok_type) {
                type = true;
                break;
            }
        }
        return type;
    }

        // function to return true if token is an open or close parenthesis token
    bool tokenTypeParen(const Opm::UDQTokenType& token) {
        bool type = false;
        std::vector <Opm::UDQTokenType> type_1 = {
            Opm::UDQTokenType::open_paren,
            Opm::UDQTokenType::close_paren,
        };
        for (const auto& tok_type : type_1) {
            if (token == tok_type) {
                type = true;
                break;
            }
        }
        return type;
    }

    // A function to return true if the token is an operator
    bool operatorToken(const Opm::UDQTokenType& token) {
        bool opTok = false;
        if (Opm::UDQ::scalarFunc(token) ||
            Opm::UDQ::elementalUnaryFunc(token) ||
            Opm::UDQ::binaryFunc(token) ||
            Opm::UDQ::setFunc(token)) {
            opTok = true;
        }
        return opTok;
    }

        // function to return index number of last binary token not inside bracket that is ending the expression
    int noOperators(const std::vector<Opm::UDQToken>& modTokens) {
        int noOp = 0;
        for (const auto& modToken : modTokens) {
            if (operatorToken(modToken.type()) || tokenTypeParen(modToken.type())) {
                noOp +=1;
            }
        }
        return noOp;
    }

    // function to return the precedence of the current operator/function
    int opFuncPrec(const Opm::UDQTokenType& token) {
        int prec = 0;
        if (tokenTypeFunc(token)) prec = 6;
        if (Opm::UDQ::cmpFunc(token)) prec = 5;
        if (tokenTypeBinaryPowOp(token)) prec = 4;
        if (tokenTypeBinaryMulDivOp(token)) prec = 3;
        if (tokenTypeBinaryAddSubOp(token)) prec = 2;
        if (tokenTypeBinaryUnionOp(token)) prec = 1;
        return prec;
    }

    struct substOuterParentheses {
        std::vector<Opm::UDQToken> highestLevOperators;
        std::map<std::size_t, std::vector<Opm::UDQToken>> substitutedTokens;
        int noleadingOpenPar;
        bool leadChangeSign;
    };

    // function to return
    //      a vector of functions and operators at the highest level,
    //      a map of substituted tokens,
    //      the number of leading open_paren that bracket the whole expression,
    //      a logical flag indicating whether there is a leading change of sign in the expression

    substOuterParentheses substitute_outer_parenthesis(const std::vector<Opm::UDQToken>& modTokens, int noLeadOpenPar, bool leadChgSgn) {
        std::map <std::size_t, std::vector<Opm::UDQToken>> substTok;
        std::vector<Opm::UDQToken> highLevOp;
        std::vector<std::size_t> startParen;
        std::vector<std::size_t> endParen;
        std::size_t level = 0;
        std::size_t search_pos = 0;
        std::size_t subS_max = 0;

        while (search_pos < modTokens.size()) {
            if (modTokens[search_pos].type() == Opm::UDQTokenType::open_paren  && level == 0) {
                startParen.emplace_back(search_pos);
                level +=1;
            }
            else if (modTokens[search_pos].type() == Opm::UDQTokenType::open_paren) {
                level +=1;
            }
            else if (modTokens[search_pos].type() == Opm::UDQTokenType::close_paren && level == +1) {
                endParen.emplace_back(search_pos);
                level -=1;
            }
            else if (modTokens[search_pos].type() == Opm::UDQTokenType::close_paren) {
                level -=1;
            }
            search_pos += 1;
        }


        //
        //Store all the operators at the highest level
        //include the ecl_expr tokens and replace the content of parentheses with a comp_expr
        if (startParen.size() >= 1) {
            if (startParen[0] > 0) {
                //First store all tokens before the first start_paren
                for (std::size_t i = 0; i < startParen[0]; i++) {
                        highLevOp.emplace_back(modTokens[i]);
                }
            }

            //
            // Replace content of all parentheses at the highest level by an comp_expr
            // store all tokens including () for all tokens inside a pair of ()
            // also store the tokens between sets of () and at the end of an expression

            std::string comp_expr;
            for (std::size_t ind = 0; ind < startParen.size();  ind++) {
                std::vector<Opm::UDQToken> substringToken;
                for (std::size_t i = startParen[ind]; i < endParen[ind]+1; i++) {
                    substringToken.emplace_back(modTokens[i]);
                }
                // store the content inside the parenthesis
                std::pair<std::size_t, std::vector<Opm::UDQToken>> groupPair = std::make_pair(ind, substringToken);
                substTok.insert(groupPair);

                //
                // make the vector of high level tokens
                //
                //first add ecl_expr instead of content of (...)

                comp_expr = std::to_string(ind);
                highLevOp.emplace_back(Opm::UDQToken(comp_expr, Opm::UDQTokenType::comp_expr));
                //
                // store all tokens between end_paren before and start_paren after current ()
                subS_max = (ind == startParen.size()-1) ? modTokens.size() : startParen[ind+1];

                if ((endParen[ind] + 1) < subS_max) {
                    for (std::size_t i = endParen[ind] + 1; i < subS_max; i++) {
                        highLevOp.emplace_back(modTokens[i]);
                    }
                }
            }
        } else {
            //
            // treat the case with no ()
            for (std::size_t i = 0; i < modTokens.size(); i++) {
                highLevOp.emplace_back(modTokens[i]);
            }
        }
        //
        // check if there is a leading minus-sign (change sign)
        if ((modTokens[0].type() == Opm::UDQTokenType::binary_op_sub)) {
            if (startParen.size() > 0) {
                // if followed by start_paren linked to end_paren before end of data
                // set flag and remove from operator list because it is considered as a highest precedence operator
                // unless () go from token 2 two the end of expression
                if ((startParen[0] == 1)  && (endParen[0] < modTokens.size()-1)) {
                    leadChgSgn = true;
                }
            } else {
                // set flag and remove from operator list
                leadChgSgn = true;
            }
            if (leadChgSgn) {
                // remove from operator list because it is considered as a highest precedence operator and is
                // therefore not a normal "binary_op_sub" operator
                std::vector<Opm::UDQToken> temp_high_lev_op(highLevOp.begin()+1, highLevOp.end());
                highLevOp = temp_high_lev_op;
            }
        } else if (startParen.size() >= 1) {
        //
        // check for leading start_paren combined with end_paren at end of data
            if ((startParen[0] == 0) && (endParen[0] == modTokens.size()-1)) {
                //
                // remove leading and trailing ()
                const std::vector<Opm::UDQToken> modTokens_red(modTokens.begin()+1, modTokens.end()-1);
                noLeadOpenPar +=1;
                //
                // recursive call to itself to re-interpret the token-input

                substOuterParentheses substOpPar = substitute_outer_parenthesis(modTokens_red, noLeadOpenPar, leadChgSgn);
                highLevOp = substOpPar.highestLevOperators;
                substTok = substOpPar.substitutedTokens;
                noLeadOpenPar = substOpPar.noleadingOpenPar;
                leadChgSgn = substOpPar.leadChangeSign;
            }
        }
        // interpretation of token input is completed return resulting object

        return {
            highLevOp,
            substTok,
            noLeadOpenPar,
            leadChgSgn
        };
    }

    // Categorize function in terms of which token-types are used in formula
    //
    // The define_type is (-) the location among a set of tokens of the "top" of the parse tree (AST - abstract syntax tree)
    // i.e. the location of the lowest precedence operator relative to the total set of operators, functions and open-/close - parenthesis
    int define_type(const std::vector<Opm::UDQToken>& tokens)
    {
        int def_type = 0;
        int noLeadOpenPar = 0;
        bool leadChgSgn = false;

        //
        // analyse the expression

        substOuterParentheses expr = substitute_outer_parenthesis(tokens, noLeadOpenPar, leadChgSgn);

        //
        // loop over high level operators to find operator with lowest precedence and highest index

        int curPrec  = 100;
        std::size_t indLowestPrecOper = 0;
        for (std::size_t ind = 0; ind < expr.highestLevOperators.size(); ++ind) {
            if ((expr.highestLevOperators[ind].type() != Opm::UDQTokenType::ecl_expr) &&
                (expr.highestLevOperators[ind].type() != Opm::UDQTokenType::comp_expr) &&
                (expr.highestLevOperators[ind].type() != Opm::UDQTokenType::number))
            {
                const int tmpPrec = opFuncPrec(expr.highestLevOperators[ind].type());
                if (tmpPrec <= curPrec) {
                    curPrec = tmpPrec;
                    indLowestPrecOper = ind;
                }
            }
        }

        //
        // if lowest precedence operator is the first token (and not equal to change sign)
        // NOTE: also for the case with outer () removed
        if ((!expr.leadChangeSign) && (indLowestPrecOper == 0)) {
            // test if operator is a function (precedence = 6)
            if ((curPrec == 6) || (expr.highestLevOperators[indLowestPrecOper].type() == Opm::UDQTokenType::binary_op_sub) ) {
                def_type = -1;
                def_type -= expr.noleadingOpenPar;
            } else {
                // def type is 1 when for all other situations (ecl-expression or number)
                def_type = 1;
            }
        } else {
            //
            // treat cases which start either with (ecl_experessions, open-parenthes or leadChangeSign
            def_type = (expr.leadChangeSign) ?  -1 : 0;
            def_type -= expr.noleadingOpenPar;
            // calculate position of lowest precedence operator
            // account for leading change sign operator
            for (std::size_t ind = 0; ind <= indLowestPrecOper; ind++) {
                //
                //count operators, including functions and parentheses (not original ecl_experessions)
                if (operatorToken(expr.highestLevOperators[ind].type())) {
                    // single operator - subtract one
                    def_type -= 1;
                } else if (expr.highestLevOperators[ind].type() == Opm::UDQTokenType::comp_expr) {
                    // expression in parentheses -  add all operators
                    std::size_t ind_ce = static_cast<std::size_t>(std::stoi(expr.highestLevOperators[ind].str()));
                    auto indSubstTok = expr.substitutedTokens.find(ind_ce);
                    if (indSubstTok != expr.substitutedTokens.end()) {
                        auto& substTokVec = indSubstTok->second;
                        //
                        // count the number of operators & parenthes in this sub-expression
                        def_type -= noOperators(substTokVec);
                    } else {
                        std::cout << "comp_expr index: " << ind_ce << std::endl;
                        throw std::invalid_argument( "Invalid comp_expr index" );
                    }
                } else if ((expr.highestLevOperators[ind].type() != Opm::UDQTokenType::ecl_expr) &&
                        (expr.highestLevOperators[ind].type() != Opm::UDQTokenType::number)) {
                    // unknown token - write warning
                    Opm::OpmLog::warning("define_type - unknown tokenType: " + expr.highestLevOperators[ind].str());
                }
            }
        }

        return def_type;
    }

    namespace iUdq {

        Opm::RestartIO::Helpers::WindowedArray<int>
        allocate(const std::vector<int>& udqDims)
        {
            using WV = Opm::RestartIO::Helpers::WindowedArray<int>;
            int nwin = std::max(udqDims[0], 1);
            return WV {
                WV::NumWindows{ static_cast<std::size_t>(nwin) },
                WV::WindowSize{ static_cast<std::size_t>(udqDims[1]) }
            };
        }

        template <class IUDQArray>
        void staticContrib(const Opm::UDQInput& udq_input, IUDQArray& iUdq)
        {
            if (udq_input.is<Opm::UDQDefine>()) {
                const auto& udq_define = udq_input.get<Opm::UDQDefine>();
                const auto& update_status =  udq_define.status();
                const auto& tokens = udq_define.tokens();
                if (update_status.first == Opm::UDQUpdate::ON) {
                    iUdq[0] = 2;
                } else {
                    iUdq[0] = 0;
                }
                iUdq[1] = define_type(tokens);
            } else {
                iUdq[0] = 0;
                iUdq[1] = 0;
            }
            iUdq[2] = udq_input.index.typed_insert_index;
        }

    } // iUdq

    namespace iUad {

        Opm::RestartIO::Helpers::WindowedArray<int>
        allocate(const std::vector<int>& udqDims)
        {
            using WV = Opm::RestartIO::Helpers::WindowedArray<int>;
            int nwin = std::max(udqDims[2], 1);
            return WV {
                WV::NumWindows{ static_cast<std::size_t>(nwin) },
                WV::WindowSize{ static_cast<std::size_t>(udqDims[3]) }
            };
        }

        template <class IUADArray>
        void staticContrib(const Opm::UDQActive::OutputRecord& udq_record, IUADArray& iUad, int use_cnt_diff)
        {
            iUad[0] = udq_record.uda_code;
            iUad[1] = udq_record.input_index + 1;

            // entry 3  - unknown meaning - value = 1
            iUad[2] = 1;

            iUad[3] = udq_record.use_count;
            iUad[4] = udq_record.use_index + 1 - use_cnt_diff;
        }
    } // iUad


    namespace zUdn {

        Opm::RestartIO::Helpers::WindowedArray<
            Opm::EclIO::PaddedOutputString<8>
        >
        allocate(const std::vector<int>& udqDims)
        {
            using WV = Opm::RestartIO::Helpers::WindowedArray<
                Opm::EclIO::PaddedOutputString<8>>;
            int nwin = std::max(udqDims[0], 1);
            return WV {
                WV::NumWindows{ static_cast<std::size_t>(nwin) },
                WV::WindowSize{ static_cast<std::size_t>(udqDims[4]) }
            };
        }

    template <class zUdnArray>
    void staticContrib(const Opm::UDQInput& udq_input, zUdnArray& zUdn)
    {
        // entry 1 is udq keyword
        zUdn[0] = udq_input.keyword();
        zUdn[1] = udq_input.unit();
    }
    } // zUdn

    namespace zUdl {

        Opm::RestartIO::Helpers::WindowedArray<
            Opm::EclIO::PaddedOutputString<8>
        >
        allocate(const std::vector<int>& udqDims)
        {
            using WV = Opm::RestartIO::Helpers::WindowedArray<
                Opm::EclIO::PaddedOutputString<8>>;
            int nwin = std::max(udqDims[0], 1);
            return WV {
                WV::NumWindows{ static_cast<std::size_t>(nwin) },
                WV::WindowSize{ static_cast<std::size_t>(udqDims[5]) }
            };
        }

    template <class zUdlArray>
    void staticContrib(const Opm::UDQInput& input, zUdlArray& zUdl)
        {
            int l_sstr = 8;
            int max_l_str = 128;
            std::string temp_str;
            // write out the input formula if key is a DEFINE udq
            if (input.is<Opm::UDQDefine>()) {
                const auto& udq_define = input.get<Opm::UDQDefine>();
                const std::string& z_data = udq_define.input_string();
                int n_sstr =  z_data.size()/l_sstr;
                if (static_cast<int>(z_data.size()) > max_l_str) {
                    std::cout << "Too long input data string (max 128 characters): " << z_data << std::endl;
                    throw std::invalid_argument("UDQ - variable: " + udq_define.keyword());
                }
                else {
                    for (int i = 0; i < n_sstr; i++) {
                        if (i == 0) {
                            temp_str = z_data.substr(i*l_sstr, l_sstr);
                            //if first character is a minus sign, change to ~
                            if (temp_str.compare(0,1,"-") == 0) {
                                temp_str.replace(0,1,"~");
                            }
                            zUdl[i] = temp_str;
                        }
                        else {
                            zUdl[i] = z_data.substr(i*l_sstr, l_sstr);
                        }
                    }
                    //add remainder of last non-zero string
                    if ((z_data.size() % l_sstr) > 0)
                        zUdl[n_sstr] = z_data.substr(n_sstr*l_sstr);
                }
            }
        }
    } // zUdl

    namespace iGph {

        Opm::RestartIO::Helpers::WindowedArray<int>
        allocate(const std::vector<int>& udqDims)
        {
            using WV = Opm::RestartIO::Helpers::WindowedArray<int>;
            return WV {
                WV::NumWindows{ static_cast<std::size_t>(udqDims[6]) },
                WV::WindowSize{ static_cast<std::size_t>(1) }
            };
        }

        template <class IGPHArray>
        void staticContrib(const int    inj_phase,
                           IGPHArray&   iGph)
        {
                iGph[0] = inj_phase;
        }
    } // iGph

    namespace iUap {

        Opm::RestartIO::Helpers::WindowedArray<int>
        allocate(const std::vector<int>& udqDims)
        {
            using WV = Opm::RestartIO::Helpers::WindowedArray<int>;
            int nwin = std::max(udqDims[7], 1);
            return WV {
                WV::NumWindows{ static_cast<std::size_t>(nwin) },
                WV::WindowSize{ static_cast<std::size_t>(1) }
            };
        }

        template <class IUAPArray>
        void staticContrib(const int    wg_no,
                           IUAPArray&   iUap)
        {
                iUap[0] = wg_no+1;
        }
    } // iUap

    namespace dUdw {

        Opm::RestartIO::Helpers::WindowedArray<double>
        allocate(const std::vector<int>& udqDims)
        {
            using WV = Opm::RestartIO::Helpers::WindowedArray<double>;
            int nwin = std::max(udqDims[9], 1);
            int nitPrWin = std::max(udqDims[8], 1);
            return WV {
                WV::NumWindows{ static_cast<std::size_t>(nwin) },
                WV::WindowSize{ static_cast<std::size_t>(nitPrWin) }
            };
        }

        template <class DUDWArray>
        void staticContrib(const Opm::UDQState& udq_state,
                           const std::vector<Opm::Well>& wells,
                           const std::string udq,
                           const std::size_t nwmaxz,
                           DUDWArray&   dUdw)
        {
            //initialize array to the default value for the array
            for (std::size_t ind = 0; ind < nwmaxz; ind++) {
                dUdw[ind] = Opm::UDQ::restart_default;
            }
            for (std::size_t ind = 0; ind < wells.size(); ind++) {
                const auto& wname = wells[ind].name();
                if (udq_state.has_well_var(wname, udq)) {
                    dUdw[ind] = udq_state.get_well_var(wname, udq);
                }
            }
        }
    } // dUdw

        namespace dUdg {

        Opm::RestartIO::Helpers::WindowedArray<double>
        allocate(const std::vector<int>& udqDims)
        {
            using WV = Opm::RestartIO::Helpers::WindowedArray<double>;
            int nwin = std::max(udqDims[11], 1);
            return WV {
                WV::NumWindows{ static_cast<std::size_t>(nwin) },
                WV::WindowSize{ static_cast<std::size_t>(udqDims[10]) }
            };
        }

        template <class DUDGArray>
        void staticContrib(const Opm::UDQState& udq_state,
                           const std::vector<const Opm::Group*> groups,
                           const std::string udq,
                           const std::size_t ngmaxz,
                           DUDGArray&   dUdg)
        {
            //initialize array to the default value for the array
            for (std::size_t ind = 0; ind < groups.size(); ind++) {
                if ((groups[ind] == nullptr) || (ind == ngmaxz-1)) {
                    dUdg[ind] = Opm::UDQ::restart_default;
                }
                else {
                    if (udq_state.has_group_var((*groups[ind]).name(), udq)) {
                        dUdg[ind] = udq_state.get_group_var((*groups[ind]).name(), udq);
                    }
                    else {
                        dUdg[ind] = Opm::UDQ::restart_default;
                    }
                }
            }
        }
    } // dUdg

        namespace dUdf {

        Opm::RestartIO::Helpers::WindowedArray<double>
        allocate(const std::vector<int>& udqDims)
        {
            using WV = Opm::RestartIO::Helpers::WindowedArray<double>;
            int nwin = std::max(udqDims[12], 1);
            return WV {
                WV::NumWindows{ static_cast<std::size_t>(nwin) },
                WV::WindowSize{ static_cast<std::size_t>(1) }
            };
        }

        template <class DUDFArray>
        void staticContrib(const Opm::UDQState& udq_state,
                           const std::string udq,
                           DUDFArray&   dUdf)
        {
            //set value for group name "FIELD"
            if (udq_state.has(udq)) {
                dUdf[0] = udq_state.get(udq);
            }
            else {
                dUdf[0] = Opm::UDQ::restart_default;
            }
        }
    } // dUdf
}


// =====================================================================


template < typename T>
std::pair<bool, int > findInVector(const std::vector<T>  & vecOfElements, const T  & element)
{
    std::pair<bool, int > result;

    // Find given element in vector
    auto it = std::find(vecOfElements.begin(), vecOfElements.end(), element);

    if (it != vecOfElements.end())
    {
        result.second = std::distance(vecOfElements.begin(), it);
        result.first = true;
    }
    else
    {
        result.first = false;
        result.second = -1;
    }
    return result;
}


const std::vector<int> Opm::RestartIO::Helpers::igphData::ig_phase(const Opm::Schedule& sched,
                                                                   const std::size_t simStep,
                                                                   const std::vector<int>& inteHead )
{
    auto update_phase = [](int& phase, int new_phase) {
        if (phase != 0)
            throw std::logic_error("Can not write restart files with UDA control on multiple phases in same group");

        phase = new_phase;
    };

    const auto curGroups = sched.restart_groups(simStep);
    std::vector<int> inj_phase(ngmaxz(inteHead), 0);
    for (std::size_t ind = 0; ind < curGroups.size(); ind++) {
        if (curGroups[ind] != nullptr) {
            const auto& group = *curGroups[ind];
            if (group.isInjectionGroup()) {
                int int_phase{0};

                for (const auto& [phase, int_value] : std::vector<std::pair<Phase, int>>{{Opm::Phase::OIL, 1},
                                                                                         {Opm::Phase::WATER, 2},
                                                                                         {Opm::Phase::GAS, 3}}) {
                    if (group.hasInjectionControl(phase)) {
                        const auto& gas_props = group.injectionProperties(phase);
                        if (gas_props.uda_phase())
                            update_phase(int_phase, int_value);
                    }
                }

                if (group.name() == "FIELD") {
                    inj_phase[ngmaxz(inteHead) - 1] = int_phase;
                } else {
                    inj_phase[group.insert_index()-1] = int_phase;
                }
            }
        }
    }

    return inj_phase;
}

std::vector<int>
iuap_data(const Opm::Schedule& sched,
          const std::size_t simStep,
          const std::vector<Opm::UDQActive::InputRecord>& iuap)
{
    //construct the current list of well or group sequence numbers to output the IUAP array
    std::vector<int> wg_no;

    for (std::size_t ind = 0; ind < iuap.size(); ind++) {
        auto& ctrl = iuap[ind].control;
        const auto wg_key = Opm::UDQ::keyword(ctrl);

        if ((wg_key == Opm::UDAKeyword::WCONPROD) ||
            (wg_key == Opm::UDAKeyword::WCONINJE) ||
            (wg_key == Opm::UDAKeyword::WELTARG))
        {
            const auto& well = sched.getWell(iuap[ind].wgname, simStep);
            wg_no.push_back(well.seqIndex());
        }
        else if ((wg_key == Opm::UDAKeyword::GCONPROD) || (wg_key == Opm::UDAKeyword::GCONINJE)) {
            const auto& group = sched.getGroup(iuap[ind].wgname, simStep);
            if (iuap[ind].wgname != "FIELD") {
                wg_no.push_back(group.insert_index() - 1);
            }
        }
        else {
            std::cout << "Invalid Control keyword: " << static_cast<int>(ctrl) << std::endl;
            throw std::invalid_argument("UDQ - variable: " + iuap[ind].udq );
        }

    }

    return wg_no;
}

Opm::RestartIO::Helpers::AggregateUDQData::
AggregateUDQData(const std::vector<int>& udqDims)
    : iUDQ_ (iUdq::allocate(udqDims)),
      iUAD_ (iUad::allocate(udqDims)),
      zUDN_ (zUdn::allocate(udqDims)),
      zUDL_ (zUdl::allocate(udqDims)),
      iGPH_ (iGph::allocate(udqDims)),
      iUAP_ (iUap::allocate(udqDims)),
      dUDW_ (dUdw::allocate(udqDims)),
      dUDG_ (dUdg::allocate(udqDims)),
      dUDF_ (dUdf::allocate(udqDims))
{}

// ---------------------------------------------------------------------

void
Opm::RestartIO::Helpers::AggregateUDQData::
captureDeclaredUDQData(const Opm::Schedule&                 sched,
                       const std::size_t                    simStep,
                       const Opm::UDQState&                 udq_state,
                       const std::vector<int>&              inteHead)
{
    const auto& udqCfg = sched.getUDQConfig(simStep);
    const auto nudq = inteHead[VI::intehead::NO_WELL_UDQS] + inteHead[VI::intehead::NO_GROUP_UDQS] + inteHead[VI::intehead::NO_FIELD_UDQS];
    int cnt_udq = 0;
    for (const auto& udq_input : udqCfg.input()) {
        auto udq_index = udq_input.index.insert_index;
        {
            auto i_udq = this->iUDQ_[udq_index];
            iUdq::staticContrib(udq_input, i_udq);
        }
        {
            auto z_udn = this->zUDN_[udq_index];
            zUdn::staticContrib(udq_input, z_udn);
        }
        {
            auto z_udl = this->zUDL_[udq_index];
            zUdl::staticContrib(udq_input, z_udl);
        }
        cnt_udq += 1;
    }
    if (cnt_udq != nudq)
        OpmLog::error(fmt::format("Inconsistent total number of udqs: {} and sum of well, group and field udqs: {}", cnt_udq, nudq));


    const auto& udq_active = sched[simStep].udq_active.get();
    if (udq_active) {
        const auto& udq_records = udq_active.iuad();
        int cnt_iuad = 0;
        for (std::size_t index = 0; index < udq_records.size(); index++) {
            const auto& record = udq_records[index];
            auto i_uad = this->iUAD_[cnt_iuad];
            const auto& ctrl = record.control;
            const auto wg_key = Opm::UDQ::keyword(ctrl);
            if (((wg_key == Opm::UDAKeyword::GCONPROD) || (wg_key == Opm::UDAKeyword::GCONINJE)) && (record.wg_name() == "FIELD"))
                continue;

            auto use_count_diff = static_cast<int>(index) - cnt_iuad;
            iUad::staticContrib(record, i_uad, use_count_diff);
            cnt_iuad += 1;
        }
        if (cnt_iuad != inteHead[VI::intehead::NO_IUADS])
            OpmLog::error(fmt::format("Inconsistent number of iuad's: {} number of iuads from intehead {}", cnt_iuad, inteHead[VI::intehead::NO_IUADS]));

        const auto& iuap_records = udq_active.iuap();
        const auto iuap_vect = iuap_data(sched, simStep, iuap_records);
        for (std::size_t index = 0; index < iuap_vect.size(); index++) {
            const auto& wg_no = iuap_vect[index];
            auto i_uap = this->iUAP_[index];
            iUap::staticContrib(wg_no, i_uap);
        }
        if (iuap_vect.size() != static_cast<std::size_t>(inteHead[VI::intehead::NO_IUAPS]))
            OpmLog::error(fmt::format("Inconsistent number of iuap's: {} number of iuap's from intehead {}", iuap_vect.size(), inteHead[VI::intehead::NO_IUAPS]));

        if (inteHead[VI::intehead::NO_IUADS] > 0) {
            Opm::RestartIO::Helpers::igphData igph_dat;
            auto igph = igph_dat.ig_phase(sched, simStep, inteHead);
            for (std::size_t index = 0; index < igph.size(); index++) {
                auto i_igph = this->iGPH_[index];
                iGph::staticContrib(igph[index], i_igph);
            }
            if (igph.size() != static_cast<std::size_t>(inteHead[VI::intehead::NGMAXZ]))
                OpmLog::error(fmt::format("Inconsistent number of igph's: {} number of igph's from intehead {}", igph.size(), inteHead[VI::intehead::NGMAXZ]));
        }
    }

    std::size_t i_wudq = 0;
    const auto& wells = sched.getWells(simStep);
    const auto nwmax = nwmaxz(inteHead);
    int cnt_dudw = 0;
    for (const auto& udq_input : udqCfg.input()) {
        if (udq_input.var_type() ==  UDQVarType::WELL_VAR) {
            const std::string& udq = udq_input.keyword();
            auto i_dudw = this->dUDW_[i_wudq];
            dUdw::staticContrib(udq_state, wells, udq, nwmax, i_dudw);
            i_wudq++;
            cnt_dudw += 1;
        }
    }
    if (cnt_dudw != inteHead[VI::intehead::NO_WELL_UDQS])
        OpmLog::error(fmt::format("Inconsistent number of dudw's: {} number of dudw's from intehead {}", cnt_dudw, inteHead[VI::intehead::NO_WELL_UDQS]));

    std::size_t i_gudq = 0;
    const auto curGroups = sched.restart_groups(simStep);
    const auto ngmax = ngmaxz(inteHead);
    int cnt_dudg = 0;
    for (const auto& udq_input : udqCfg.input()) {
        if (udq_input.var_type() ==  UDQVarType::GROUP_VAR) {
            const std::string& udq = udq_input.keyword();
            auto i_dudg = this->dUDG_[i_gudq];
            dUdg::staticContrib(udq_state, curGroups, udq, ngmax, i_dudg);
            i_gudq++;
            cnt_dudg += 1;
        }
    }
    if (cnt_dudg != inteHead[VI::intehead::NO_GROUP_UDQS])
        OpmLog::error(fmt::format("Inconsistent number of dudg's: {} number of dudg's from intehead {}", cnt_dudg, inteHead[VI::intehead::NO_GROUP_UDQS]));

    std::size_t i_fudq = 0;
    int cnt_dudf = 0;
    for (const auto& udq_input : udqCfg.input()) {
        if (udq_input.var_type() ==  UDQVarType::FIELD_VAR) {
            const std::string& udq = udq_input.keyword();
            auto i_dudf = this->dUDF_[i_fudq];
            dUdf::staticContrib(udq_state, udq, i_dudf);
            i_fudq++;
            cnt_dudf += 1;
        }
    }
    if (cnt_dudf != inteHead[VI::intehead::NO_FIELD_UDQS])
        OpmLog::error(fmt::format("Inconsistent number of dudf's: {} number of dudf's from intehead {}", cnt_dudf, inteHead[VI::intehead::NO_FIELD_UDQS]));


}

