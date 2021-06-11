/*
  Copyright 2016 Statoil ASA.

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

#ifndef SIMULATION_DATA_CONTAINER_HPP
#define SIMULATION_DATA_CONTAINER_HPP

#include <cstddef>
#include <string>
#include <map>
#include <vector>

namespace Opm {


    /// The SimulationDataContainer is a simple container to manage
    /// simulation data. The container is instantiated with information
    /// of how many cells, faces and phases are present in the
    /// reservoirmodel. You can then add data to the container by using the
    ///
    ///   registerCellData()
    ///   registerFaceData()
    ///
    /// functions. The container owns and manages the data, but
    /// mutable references are returned with the getCellData() and
    /// getFaceData() methods, and the content will typically be
    /// modified by external scope.
    class SimulationDataContainer
    {
    public:
        /// Main constructor setting the sizes for the contained data
        /// types.
        /// \param num_cells   number of elements in cell data vectors
        /// \param num_faces   number of elements in face data vectors
        /// \param num_phases  number of phases, the number of components
        ///                    in any data vector must equal 1 or this
        ///                    number (this behaviour and argument is deprecated).
        SimulationDataContainer(size_t num_cells, size_t num_faces, size_t num_phases);

        /// Copy constructor.
        /// Must be defined explicitly because class contains non-value objects
        /// (the reference pointers pressure_ref_ etc.) that should not simply
        /// be copied.
        SimulationDataContainer(const SimulationDataContainer&);

        /// Copy assignment operator.
        /// Must be defined explicitly because class contains non-value objects
        /// (the reference pointers pressure_ref_ etc.) that should not simply
        /// be copied.
        SimulationDataContainer& operator=(const SimulationDataContainer&);

        /// Efficient O(1) swap.
        void swap(SimulationDataContainer& other);

        size_t numPhases() const;
        size_t numFaces() const;
        size_t numCells() const;

        bool hasCellData( const std::string& name ) const;

        /// Will register a data vector of size numCells() *
        /// components.
        void registerCellData( const std::string& name , size_t components , double initialValue = 0.0 );
        std::vector<double>& getCellData( const std::string& name );
        const std::vector<double>& getCellData( const std::string& name ) const;

        bool hasFaceData( const std::string& name ) const;
        void registerFaceData( const std::string& name , size_t components , double initialValue = 0.0 );
        std::vector<double>& getFaceData( const std::string& name );
        const std::vector<double>& getFaceData( const std::string& name ) const;

        /// Will return the number of components of the celldata with
        /// name @name:
        ///
        ///    numCellDataComponents( "PRESSURE" )   -> 1
        ///    numCellDataComponents( "SATURATION" ) -> 3
        ///
        /// for a three phase model.
        size_t numCellDataComponents( const std::string& name ) const;
        bool equal(const SimulationDataContainer& other) const;


        /// Will set the values of component nr @component in the
        /// field @key. All the cells in @cells will be set to the
        /// values in @values.
        void setCellDataComponent( const std::string& key , size_t component , const std::vector<int>& cells , const std::vector<double>& values);

        // Direct explicit field access for certain default fields.
        // These methods are all deprecated, and will eventually be moved to
        // concrete subclasses.

        std::vector<double>& pressure    ();
        std::vector<double>& temperature ();
        std::vector<double>& saturation  ();

        std::vector<double>& facepressure();
        std::vector<double>& faceflux    ();

        const std::vector<double>& pressure    () const;
        const std::vector<double>& temperature () const;
        const std::vector<double>& saturation  () const;

        const std::vector<double>& facepressure() const;
        const std::vector<double>& faceflux    () const;

        const std::map<std::string, std::vector<double>>& cellData() const;
        std::map<std::string, std::vector<double>>& cellData();

    private:
        void addDefaultFields();
        void setReferencePointers();

        size_t m_num_cells;
        size_t m_num_faces;
        size_t m_num_phases;

        std::map< std::string , std::vector<double> > m_cell_data;
        std::map< std::string , std::vector<double> > m_face_data;

        std::vector<double>* pressure_ref_;
        std::vector<double>* temperature_ref_;
        std::vector<double>* saturation_ref_;
        std::vector<double>* facepressure_ref_;
        std::vector<double>* faceflux_ref_;
    };


    // Inline implementations of the direct accessors required to guarantee
    // performance.


    inline std::vector<double>& SimulationDataContainer::pressure( ) {
        return *pressure_ref_;
    }

    inline std::vector<double>& SimulationDataContainer::temperature() {
        return *temperature_ref_;
    }

    inline std::vector<double>& SimulationDataContainer::saturation() {
        return *saturation_ref_;
    }

    inline std::vector<double>& SimulationDataContainer::facepressure() {
        return *facepressure_ref_;
    }

    inline std::vector<double>& SimulationDataContainer::faceflux() {
        return *faceflux_ref_;
    }

    inline const std::vector<double>& SimulationDataContainer::pressure( ) const {
        return *pressure_ref_;
    }

    inline const std::vector<double>& SimulationDataContainer::temperature() const {
        return *temperature_ref_;
    }

    inline const std::vector<double>& SimulationDataContainer::saturation() const {
        return *saturation_ref_;
    }

    inline const std::vector<double>& SimulationDataContainer::facepressure() const {
        return *facepressure_ref_;
    }

    inline const std::vector<double>& SimulationDataContainer::faceflux() const {
        return *faceflux_ref_;
    }



}

#endif
