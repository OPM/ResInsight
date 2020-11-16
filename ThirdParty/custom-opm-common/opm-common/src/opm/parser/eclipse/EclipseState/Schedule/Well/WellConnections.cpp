/*
  Copyright 2013 Statoil ASA.

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

#include <cassert>
#include <cmath>
#include <limits>
#include <iostream>

#include <opm/parser/eclipse/Units/Units.hpp>
#include <opm/io/eclipse/rst/connection.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/FieldPropsManager.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well/Connection.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well/WellConnections.hpp>

namespace Opm {

namespace {

    // Compute direction permutation corresponding to completion's
    // direction.  First two elements of return value are directions
    // perpendicular to completion while last element is direction
    // along completion.
inline std::array< size_t, 3> directionIndices(const Opm::Connection::Direction direction)
    {
        switch (direction) {
        case Opm::Connection::Direction::X:
            return {{ 1,2,0 }};

        case Opm::Connection::Direction::Y:
            return {{ 2,0,1}};

        case Opm::Connection::Direction::Z:
            return {{ 0,1,2 }};
        }
        // All enum values should be handled above. Therefore
        // we should never reach this one. Anyway for the sake
        // of reduced warnings we throw an exception.
        throw std::invalid_argument("unhandled enum value");

    }

    // Permute (diagonal) permeability components according to
    // completion's direction.
    inline std::array<double,3>
    permComponents(const Opm::Connection::Direction direction,
                   const std::array<double,3>& perm)
    {
        const auto p = directionIndices(direction);

        return {{ perm[ p[0] ],
                  perm[ p[1] ],
                  perm[ p[2] ] }};
    }

    // Permute cell's geometric extent according to completion's
    // direction.  Honour net-to-gross ratio.
    //
    // Note: 'extent' is intentionally accepted by modifiable value
    // rather than reference-to-const to support NTG manipulation.
    inline std::array<double,3>
    effectiveExtent(const Opm::Connection::Direction direction,
                    const double                     ntg,
                    std::array<double,3>             extent)
    {
        // Vertical extent affected by net-to-gross ratio.
        extent[2] *= ntg;

        const auto p = directionIndices(direction);

        std::array<double,3>
            D = {{ extent[ p[0] ] ,
                   extent[ p[1] ] ,
                   extent[ p[2] ] }};

        return D;
    }

    // Compute Peaceman's effective radius of single completion.
    inline double
    effectiveRadius(const std::array<double,3>& K,
                    const std::array<double,3>& D)
    {
        const double K01   = K[0] / K[1];
        const double K10   = K[1] / K[0];

        const double D0_sq = D[0] * D[0];
        const double D1_sq = D[1] * D[1];

        const double num = std::sqrt((std::sqrt(K10) * D0_sq) +
                                     (std::sqrt(K01) * D1_sq));
        const double den = std::pow(K01, 0.25) + std::pow(K10, 0.25);

        // Note: Analytic constant 0.28 derived for infintely sized
        // formation with repeating well placement.
        return 0.28 * (num / den);
    }


} // anonymous namespace

    WellConnections::WellConnections() :
        headI(0),
        headJ(0)
    {
    }

    WellConnections::WellConnections(Connection::Order order,int headIArg, int headJArg) :
        m_ordering(order),
        headI(headIArg),
        headJ(headJArg)
    {
    }


    WellConnections::WellConnections(Connection::Order order,
                                     int headIArg, int headJArg,
                                     const std::vector<Connection>& connections) :
        m_ordering(order),
        headI(headIArg),
        headJ(headJArg),
        m_connections(connections)
    {
    }


    WellConnections WellConnections::serializeObject()
    {
        WellConnections result;
        result.m_ordering = Connection::Order::DEPTH;
        result.headI = 1;
        result.headJ = 2;
        result.m_connections = {Connection::serializeObject()};

        return result;
    }


    WellConnections::WellConnections(const WellConnections& src, const EclipseGrid& grid) :
        m_ordering(src.ordering()),
        headI(src.headI),
        headJ(src.headJ)
    {
        for (const auto& c : src) {
            if (grid.cellActive(c.getI(), c.getJ(), c.getK()))
                this->add(c);
        }
    }

    std::vector<const Connection *> WellConnections::output(const EclipseGrid& grid) const {
        if (this->m_connections.empty())
            return {};

        std::vector<const Connection*> out;
        for (const auto& conn : this->m_connections)
            if (grid.cellActive(conn.getI(), conn.getJ(), conn.getK()))
                out.push_back( &conn );

        if (!this->m_connections[0].attachedToSegment() && (this->m_ordering != Connection::Order::INPUT)) {
            std::sort(out.begin(), out.end(), [](const Opm::Connection* conn1, const Opm::Connection* conn2)
                                                {
                                                    return conn1->sort_value() < conn2->sort_value();
                                                });
        }
        return out;
    }


    void WellConnections::addConnection(int i, int j , int k ,
                                        std::size_t global_index,
                                        int complnum,
                                        double depth,
                                        Connection::State state,
                                        double CF,
                                        double Kh,
                                        double rw,
                                        double r0,
                                        double skin_factor,
                                        const int satTableId,
                                        const Connection::Direction direction,
                                        const Connection::CTFKind ctf_kind,
                                        const std::size_t seqIndex,
                                        const double segDistStart,
                                        const double segDistEnd,
                                        const bool defaultSatTabId)
    {
        int conn_i = (i < 0) ? this->headI : i;
        int conn_j = (j < 0) ? this->headJ : j;
        Connection conn(conn_i, conn_j, k, global_index, complnum, depth, state, CF, Kh, rw, r0,
                        skin_factor, satTableId, direction, ctf_kind,
                        seqIndex, segDistStart, segDistEnd, defaultSatTabId);
        this->add(conn);
    }



    void WellConnections::addConnection(int i, int j , int k ,
                                        std::size_t global_index,
                                        double depth,
                                        Connection::State state ,
                                        double CF,
                                        double Kh,
                                        double rw,
                                        double r0,
                                        double skin_factor,
                                        const int satTableId,
                                        const Connection::Direction direction,
                                        const Connection::CTFKind ctf_kind,
                                        const std::size_t seqIndex,
                                        const double segDistStart,
                                        const double segDistEnd,
                                        const bool defaultSatTabId)
    {
        int complnum = (this->m_connections.size() + 1);
        this->addConnection(i,
                            j,
                            k,
                            global_index,
                            complnum,
                            depth,
                            state,
                            CF,
                            Kh,
                            rw,
                            r0,
                            skin_factor,
                            satTableId,
                            direction,
                            ctf_kind,
                            seqIndex,
                            segDistStart,
                            segDistEnd,
                            defaultSatTabId);
    }

    void WellConnections::loadCOMPDAT(const DeckRecord& record, const EclipseGrid& grid, const FieldPropsManager& field_properties) {
        const auto permx        = field_properties.try_get<double>("PERMX");
        const auto permy        = field_properties.try_get<double>("PERMY");
        const auto permz        = field_properties.try_get<double>("PERMZ");
        const auto& ntg         = field_properties.get_double("NTG");
        const auto& satnum_data = field_properties.get_int("SATNUM");

        this->loadCOMPDAT(record, grid, satnum_data, permx, permy, permz, ntg);
    }

    void WellConnections::loadCOMPDAT(const DeckRecord& record,
                                      const EclipseGrid& grid,
                                      const std::vector<int>& satnum_data,
                                      const std::vector<double>* permx,
                                      const std::vector<double>* permy,
                                      const std::vector<double>* permz,
                                      const std::vector<double>& ntg) {

        const auto& itemI = record.getItem( "I" );
        const auto defaulted_I = itemI.defaultApplied( 0 ) || itemI.get< int >( 0 ) == 0;
        const int I = !defaulted_I ? itemI.get< int >( 0 ) - 1 : this->headI;

        const auto& itemJ = record.getItem( "J" );
        const auto defaulted_J = itemJ.defaultApplied( 0 ) || itemJ.get< int >( 0 ) == 0;
        const int J = !defaulted_J ? itemJ.get< int >( 0 ) - 1 : this->headJ;

        int K1 = record.getItem("K1").get< int >(0) - 1;
        int K2 = record.getItem("K2").get< int >(0) - 1;
        Connection::State state = Connection::StateFromString( record.getItem("STATE").getTrimmedString(0) );

        int satTableId = -1;
        bool defaultSatTable = true;
        const auto& r0Item = record.getItem("PR");
        const auto& CFItem = record.getItem("CONNECTION_TRANSMISSIBILITY_FACTOR");
        const auto& diameterItem = record.getItem("DIAMETER");
        const auto& KhItem = record.getItem("Kh");
        const auto& satTableIdItem = record.getItem("SAT_TABLE");
        const auto direction = Connection::DirectionFromString(record.getItem("DIR").getTrimmedString(0));
        double skin_factor = record.getItem("SKIN").getSIDouble(0);
        double rw;

        if (satTableIdItem.hasValue(0) && satTableIdItem.get < int > (0) > 0)
        {
            satTableId = satTableIdItem.get< int >(0);
            defaultSatTable = false;
        }

        if (diameterItem.hasValue(0))
            rw = 0.50 * diameterItem.getSIDouble(0);
        else
            // The Eclipse100 manual does not specify a default value for the wellbore
            // diameter, but the Opm codebase has traditionally implemented a default
            // value of one foot. The same default value is used by Eclipse300.
            rw = 0.5*unit::feet;

        for (int k = K1; k <= K2; k++) {
            if (!grid.cellActive(I, J, k))
                continue;

            size_t active_index = grid.activeIndex(I,J,k);
            double CF = -1;
            double Kh = -1;
            double r0 = -1;
            auto ctf_kind = ::Opm::Connection::CTFKind::DeckValue;

            if (defaultSatTable)
                satTableId = satnum_data[active_index];

            auto same_ijk = [&]( const Connection& c ) {
                return c.sameCoordinate( I,J,k );
            };

            if (r0Item.hasValue(0))
                r0 = r0Item.getSIDouble(0);

            if (KhItem.hasValue(0) && KhItem.getSIDouble(0) > 0.0)
                Kh = KhItem.getSIDouble(0);

            if (CFItem.hasValue(0) && CFItem.getSIDouble(0) > 0.0)
                CF = CFItem.getSIDouble(0);

            /* We start with the absolute happy path; both CF and Kh are explicitly given in the deck. */
            if (CF > 0 && Kh > 0)
                goto CF_done;

            /* We must calculate CF and Kh from the items in the COMPDAT record and cell properties. */
            if (permx && permy && permz) {
                // Angle of completion exposed to flow.  We assume centre
                // placement so there's complete exposure (= 2\pi).
                const double angle = 6.2831853071795864769252867665590057683943387987502116419498;
                std::array<double,3> cell_perm = {{ permx->operator[](active_index),
                                                    permy->operator[](active_index),
                                                    permz->operator[](active_index)}};
                std::array<double,3> cell_size = grid.getCellDims(I,J,k);
                const auto& K = permComponents(direction, cell_perm);
                const auto& D = effectiveExtent(direction, ntg[active_index], cell_size);

                if (r0 < 0)
                    r0 = effectiveRadius(K,D);

                if (CF < 0) {
                    if (Kh < 0)
                        Kh = std::sqrt(K[0] * K[1]) * D[2];
                    CF = angle * Kh / (std::log(r0 / std::min(rw, r0)) + skin_factor);
                    ctf_kind = ::Opm::Connection::CTFKind::Defaulted;
                } else {
                    if (KhItem.defaultApplied(0) || KhItem.getSIDouble(0) < 0) {
                        Kh = CF * (std::log(r0 / std::min(r0, rw)) + skin_factor) / angle;
                    } else {
                        if (Kh < 0)
                            Kh = std::sqrt(K[0] * K[1]) * D[2];
                    }
                }
            } else
                throw std::invalid_argument("Missing PERM values to calculate connection factors");


        CF_done:
            if (r0 < 0)
                r0 = RestartIO::RstConnection::inverse_peaceman(CF, Kh, rw, skin_factor);

            auto prev = std::find_if( this->m_connections.begin(),
                                      this->m_connections.end(),
                                      same_ijk );
            if (prev == this->m_connections.end()) {
                std::size_t noConn = this->m_connections.size();
                this->addConnection(I,J,k,
                                    grid.getGlobalIndex(I,J,k),
                                    grid.getCellDepth( I,J,k ),
                                    state,
                                    CF,
                                    Kh,
                                    rw,
                                    r0,
                                    skin_factor,
                                    satTableId,
                                    direction, ctf_kind,
                                    noConn, 0., 0., defaultSatTable);
            } else {
                std::size_t css_ind = prev->sort_value();
                int conSegNo = prev->segment();
                double conSDStart = prev->getSegDistStart();
                double conSDEnd = prev->getSegDistEnd();
                double depth = grid.getCellDepth(I,J,k);
                *prev = Connection(I,J,k,
                                   grid.getGlobalIndex(I,J,k),
                                   prev->complnum(),
                                   depth,
                                   state,
                                   CF,
                                   Kh,
                                   rw,
                                   r0,
                                   skin_factor,
                                   satTableId,
                                   direction, ctf_kind,
                                   prev->sort_value(), conSDStart, conSDEnd, defaultSatTable);

                prev->updateSegment(conSegNo,
                                    depth,
                                    css_ind,
                                    conSDStart,
                                    conSDEnd);
            }
        }
    }


    size_t WellConnections::size() const {
        return m_connections.size();
    }

    const Connection& WellConnections::get(size_t index) const {
        return (*this)[index];
    }

    const Connection& WellConnections::operator[](size_t index) const {
        return this->m_connections.at(index);
    }


    const Connection& WellConnections::lowest() const {
        if (this->m_connections.empty())
            throw std::logic_error("Tried to get lowest connection from empty set");

        const auto max_iter = std::max_element(this->m_connections.begin(),
                                               this->m_connections.end(),
                                               [](const Connection& c1, const Connection& c2)
                                               {
                                                   return c1.depth() < c2.depth();
                                               });

        return *max_iter;
    }


    const Connection& WellConnections::getFromIJK(const int i, const int j, const int k) const {
        for (size_t ic = 0; ic < size(); ++ic) {
            if (get(ic).sameCoordinate(i, j, k)) {
                return get(ic);
            }
        }
        throw std::runtime_error(" the connection is not found! \n ");
    }


    Connection& WellConnections::getFromIJK(const int i, const int j, const int k) {
      for (size_t ic = 0; ic < size(); ++ic) {
        if (get(ic).sameCoordinate(i, j, k)) {
          return this->m_connections[ic];
        }
      }
      throw std::runtime_error(" the connection is not found! \n ");
    }


    void WellConnections::add( Connection connection ) {
        m_connections.emplace_back( connection );
    }

    bool WellConnections::allConnectionsShut( ) const {
        if (this->size() == 0)
            return false;


        auto shut = []( const Connection& c ) {
            return c.state() == Connection::State::SHUT;
        };

        return std::all_of( this->m_connections.begin(),
                            this->m_connections.end(),
                            shut );
    }



    void WellConnections::order()
    {
        if (m_connections.empty())
            return;

        if (this->m_connections[0].attachedToSegment())
            this->orderMSW();
        else if (this->m_ordering == Connection::Order::TRACK)
            this->orderTRACK();

    }


    void WellConnections::orderMSW() {
        std::sort(this->m_connections.begin(), this->m_connections.end(), [](const Opm::Connection& conn1, const Opm::Connection& conn2)
                  {
                      return conn1.sort_value() < conn2.sort_value();
                  });
    }


    void WellConnections::orderTRACK() {
        // Find the first connection and swap it into the 0-position.
        const double surface_z = 0.0;
        size_t first_index = findClosestConnection(this->headI, this->headJ, surface_z, 0);
        std::swap(m_connections[first_index], m_connections[0]);

        // Repeat for remaining connections.
        //
        // Note that since findClosestConnection() is O(n), this is an
        // O(n^2) algorithm. However, it should be acceptable since
        // the expected number of connections is fairly low (< 100).

        if( this->m_connections.empty() ) return;

        for (size_t pos = 1; pos < m_connections.size() - 1; ++pos) {
            const auto& prev = m_connections[pos - 1];
            const double prevz = prev.depth();
            size_t next_index = findClosestConnection(prev.getI(), prev.getJ(), prevz, pos);
            std::swap(m_connections[next_index], m_connections[pos]);
        }
    }


    size_t WellConnections::findClosestConnection(int oi, int oj, double oz, size_t start_pos)
    {
        size_t closest = std::numeric_limits<size_t>::max();
        int min_ijdist2 = std::numeric_limits<int>::max();
        double min_zdiff = std::numeric_limits<double>::max();
        for (size_t pos = start_pos; pos < m_connections.size(); ++pos) {
            const auto& connection = m_connections[ pos ];

            const double depth = connection.depth();
            const int ci = connection.getI();
            const int cj = connection.getJ();
            // Using square of distance to avoid non-integer arithmetics.
            const int ijdist2 = (ci - oi) * (ci - oi) + (cj - oj) * (cj - oj);
            if (ijdist2 < min_ijdist2) {
                min_ijdist2 = ijdist2;
                min_zdiff = std::abs(depth - oz);
                closest = pos;
            } else if (ijdist2 == min_ijdist2) {
                const double zdiff = std::abs(depth - oz);
                if (zdiff < min_zdiff) {
                    min_zdiff = zdiff;
                    closest = pos;
                }
            }
        }
        assert(closest != std::numeric_limits<size_t>::max());
        return closest;
    }

    bool WellConnections::operator==( const WellConnections& rhs ) const {
        return this->size() == rhs.size() &&
            this->m_ordering == rhs.m_ordering &&
            std::equal( this->begin(), this->end(), rhs.begin() );
    }

    bool WellConnections::operator!=( const WellConnections& rhs ) const {
        return !( *this == rhs );
    }


    void WellConnections::filter(const ActiveGridCells& grid) {
        auto new_end = std::remove_if(m_connections.begin(),
                                      m_connections.end(),
                                      [&grid](const Connection& c) { return !grid.cellActive(c.getI(), c.getJ(), c.getK()); });
        m_connections.erase(new_end, m_connections.end());
    }

}
