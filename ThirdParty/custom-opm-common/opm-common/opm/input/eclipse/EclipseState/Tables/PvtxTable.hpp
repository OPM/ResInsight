/*
  Copyright 2015 Statoil ASA.

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
#ifndef OPM_PARSER_PVTX_TABLE_HPP
#define OPM_PARSER_PVTX_TABLE_HPP

#include <opm/input/eclipse/EclipseState/Tables/ColumnSchema.hpp>
#include <opm/input/eclipse/EclipseState/Tables/SimpleTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/TableColumn.hpp>
#include <opm/input/eclipse/EclipseState/Tables/TableSchema.hpp>

#include <string>
#include <utility>
#include <vector>

#include <stddef.h>

/*
  This class is a common base class for the PVTG and PVTO tables. The
  PVTO and PVTG keywords have a quite complex structure. The structure
  consists of alternating records of saturated data and corresponding
  undersaturated tables, this structure is again repeated for the
  different satnum regions.


  PVTO

--  RSO    PRESSURE    B-OIL     VISCOSITY
--          (BAR)                  (CP)

 [ 20.59  {  50.00    1.10615     1.180 } ]        \
          {  75.00    1.10164     1.247 }          |
          { 100.00    1.09744     1.315 }          |
          { 125.00    1.09351     1.384 }          |
          { 150.00    1.08984     1.453 }/         |
                                                   |
 [ 28.19  {  70.00    1.12522     1.066 } ]        |
          {  95.00    1.12047     1.124 }          |
          { 120.00    1.11604     1.182 }          |-- Pvtnum region 1
          { 145.00    1.11191     1.241 }          |
          { 170.00    1.10804     1.300 }/         |
                                                   |
 [ 36.01  {  90.00    1.14458     0.964 } ]        |
          { 115.00    1.13959     1.014 }          |
          { 140.00    1.13494     1.064 }          |
          { 165.00    1.13060     1.115 }          |
          { 190.00    1.12653     1.166 }/         |
/                                                  /
  404.60    594.29    1.97527     0.21564          \
            619.29    1.96301     0.21981          |
            644.29    1.95143     0.22393          |-- Pvtnum region 2
            669.29    1.94046     0.22801          |
            694.29    1.93005     0.23204 /        |
/                                                  /
  404.60    594.29    1.97527     0.21564          \
            619.29    1.96301     0.21981          |
            644.29    1.95143     0.22393          |
            669.29    1.94046     0.22801          |
            694.29    1.93005     0.23204 /        |-- Pvtnum region 3
  404.60    594.29    1.97527     0.21564          |
            619.29    1.96301     0.21981          |
            644.29    1.95143     0.22393          |
            669.29    1.94046     0.22801          |
            694.29    1.93005     0.23204 /        /
/


In pvtnum region1 the saturated records are marked with [ ... ], and
the corresponding undersaturated tables are marked with { ... }. So
for pvtnum region1 the table of saturated properties looks like:

   RSO       PRESSURE    B-OIL       VISCOSITY
   20.59     50.00       1.10615     1.180
   28.19     70.00       1.12522     1.066
   36.01     90.00       1.14458     0.964

In the PvtxTable class this table is available as the method
getSaturatedTable( ). For each RS value there is a table of
undersaturated properties; since the saturated table for region1 has
three rows there are three such tables, these tables are available as
getUnderSaturatedTable( index ). In this particular example the first
undersaturated table looks like:

     PRESSURE    B-OIL       VISCOSITY
      50.00      1.10615     1.180
      75.00      1.10164     1.247
     100.00      1.09744     1.315
     125.00      1.09351     1.384
     150.00      1.08984     1.453

The first row actually corresponds to saturated values.
*/

 namespace Opm {

    class DeckKeyword;

    class PvtxTable {
    public:
        static size_t numTables( const DeckKeyword& keyword);
        static std::vector<std::pair<size_t , size_t> > recordRanges( const DeckKeyword& keyword);

        PvtxTable() = default;
        explicit PvtxTable(const std::string& columnName);

        static PvtxTable serializeObject();

        const SimpleTable& getUnderSaturatedTable(size_t tableNumber) const;
        void init(const DeckKeyword& keyword, size_t tableIdx);
        size_t size() const;
        double evaluate(const std::string& column, double outerArg, double innerArg) const;
        double getArgValue(size_t index) const;
        const SimpleTable& getSaturatedTable() const;

        /*
          Will iterate over the internal undersaturated tables; same
          as getUnderSaturatedTable( ).
        */
        std::vector< SimpleTable >::const_iterator begin() const;
        std::vector< SimpleTable >::const_iterator end()   const;

        bool operator==(const PvtxTable& data) const;

        template <class Serializer>
        void serializeOp(Serializer& serializer)
        {
            m_outerColumnSchema.serializeOp(serializer);
            m_outerColumn.serializeOp(serializer);
            m_underSaturatedSchema.serializeOp(serializer);
            m_saturatedSchema.serializeOp(serializer);
            serializer.vector(m_underSaturatedTables);
            m_saturatedTable.serializeOp(serializer);
        }

    protected:
        ColumnSchema m_outerColumnSchema;
        TableColumn m_outerColumn;

        TableSchema m_underSaturatedSchema;
        TableSchema m_saturatedSchema;
        std::vector< SimpleTable > m_underSaturatedTables;
        SimpleTable m_saturatedTable;
    };
}

#endif
