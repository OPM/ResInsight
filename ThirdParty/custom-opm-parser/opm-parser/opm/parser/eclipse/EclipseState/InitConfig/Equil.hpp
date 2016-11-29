#ifndef OPM_EQUIL_HPP
#define OPM_EQUIL_HPP

namespace Opm {

    class DeckKeyword;
    class DeckRecord;

    class EquilRecord {
        public:
            explicit EquilRecord( const DeckRecord& );

            double datumDepth() const;
            double datumDepthPressure() const;
            double waterOilContactDepth() const;
            double waterOilContactCapillaryPressure() const;
            double gasOilContactDepth() const;
            double gasOilContactCapillaryPressure() const;

            bool liveOilInitConstantRs() const;
            bool wetGasInitConstantRv() const;
            int initializationTargetAccuracy() const;

        private:
            double datum_depth;
            double datum_depth_ps;
            double water_oil_contact_depth;
            double water_oil_contact_capillary_pressure;
            double gas_oil_contact_depth;
            double gas_oil_contact_capillary_pressure;

            bool live_oil_init_proc;
            bool wet_gas_init_proc;
            int init_target_accuracy;
    };

    class Equil {
        public:
            using const_iterator = std::vector< EquilRecord >::const_iterator;

            Equil() = default;
            explicit Equil( const DeckKeyword& );

            const EquilRecord& getRecord( size_t id ) const;

            size_t size() const;
            bool empty() const;

            const_iterator begin() const;
            const_iterator end() const;

        private:
            std::vector< EquilRecord > records;
    };

}

#endif //OPM_EQUIL_HPP
