#ifndef OPM_EQUIL_HPP
#define OPM_EQUIL_HPP

#include <cstddef>
#include <vector>

namespace Opm {
    class DeckKeyword;
    class EquilRecord {
        public:
            double datumDepth() const;
            double datumDepthPressure() const;
            double waterOilContactDepth() const;
            double waterOilContactCapillaryPressure() const;
            double gasOilContactDepth() const;
            double gasOilContactCapillaryPressure() const;

            bool liveOilInitConstantRs() const;
            bool wetGasInitConstantRv() const;
            int initializationTargetAccuracy() const;

            EquilRecord();

            EquilRecord( double datum_depth_arg, double datum_depth_pc_arg, double woc_depth, double woc_pc, double goc_depth, double goc_pc, bool live_oil_init, bool wet_gas_init, int target_accuracy);

            bool operator==(const EquilRecord& data) const;

            template<class Serializer>
            void serializeOp(Serializer& serializer)
            {
                serializer(datum_depth);
                serializer(datum_depth_ps);
                serializer(water_oil_contact_depth);
                serializer(water_oil_contact_capillary_pressure);
                serializer(gas_oil_contact_depth);
                serializer(gas_oil_contact_capillary_pressure);
                serializer(live_oil_init_proc);
                serializer(wet_gas_init_proc);
                serializer(init_target_accuracy);
            }

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

            static Equil serializeObject();

            const EquilRecord& getRecord( size_t id ) const;

            size_t size() const;
            bool empty() const;

            const_iterator begin() const;
            const_iterator end() const;

            bool operator==(const Equil& data) const;

            template<class Serializer>
            void serializeOp(Serializer& serializer)
            {
                serializer.vector(m_records);
            }

        private:
            std::vector< EquilRecord > m_records;
    };

}

#endif //OPM_EQUIL_HPP
