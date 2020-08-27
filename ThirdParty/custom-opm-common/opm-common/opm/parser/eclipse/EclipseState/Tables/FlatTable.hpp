#ifndef OPM_FLAT_TABLE_HPP
#define OPM_FLAT_TABLE_HPP

namespace Opm {

class DeckKeyword;

template< typename T >
struct FlatTable : public std::vector< T > {
    FlatTable() = default;
    explicit FlatTable( const DeckKeyword& );
    explicit FlatTable(const std::vector<T>& data) :
        std::vector<T>(data)
    {}

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer.vector(*this);
    }
};

struct DENSITYRecord {
    static constexpr std::size_t size = 3;

    double oil;
    double water;
    double gas;

    bool operator==(const DENSITYRecord& data) const {
        return oil == data.oil &&
               water == data.water &&
               gas == data.gas;
    }

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(oil);
        serializer(water);
        serializer(gas);
    }
};

struct DensityTable : public FlatTable< DENSITYRecord > {
    using FlatTable< DENSITYRecord >::FlatTable;

    static DensityTable serializeObject()
    {
        return DensityTable({{1.0, 2.0, 3.0}});
    }
};

struct PVTWRecord {
    static constexpr std::size_t size = 5;

    double reference_pressure;
    double volume_factor;
    double compressibility;
    double viscosity;
    double viscosibility;

    bool operator==(const PVTWRecord& data) const {
        return reference_pressure == data.reference_pressure &&
               volume_factor == data.volume_factor &&
               compressibility == data.compressibility &&
               viscosity == data.viscosity &&
               viscosibility == data.viscosibility;
    }

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(reference_pressure);
        serializer(volume_factor);
        serializer(compressibility);
        serializer(viscosity);
        serializer(viscosibility);
    }
};

struct PvtwTable : public FlatTable< PVTWRecord > {
    using FlatTable< PVTWRecord >::FlatTable;

    static PvtwTable serializeObject()
    {
        return PvtwTable({{1.0, 2.0, 3.0, 4.0, 5.0}});
    }
};

struct ROCKRecord {
    static constexpr std::size_t size = 2;

    double reference_pressure;
    double compressibility;

    bool operator==(const ROCKRecord& data) const {
        return reference_pressure == data.reference_pressure &&
               compressibility == data.compressibility;
    }

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(reference_pressure);
        serializer(compressibility);
    }
};

struct RockTable : public FlatTable< ROCKRecord > {
    using FlatTable< ROCKRecord >::FlatTable;

    static RockTable serializeObject()
    {
        return RockTable({{1.0, 2.0}});
    }
};

struct PVCDORecord {
    static constexpr std::size_t size = 5;

    double reference_pressure;
    double volume_factor;
    double compressibility;
    double viscosity;
    double viscosibility;

    bool operator==(const PVCDORecord& data) const {
        return reference_pressure == data.reference_pressure &&
               volume_factor == data.volume_factor &&
               compressibility == data.compressibility &&
               viscosity == data.viscosity &&
               viscosibility == data.viscosibility;
    }

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(reference_pressure);
        serializer(volume_factor);
        serializer(compressibility);
        serializer(viscosity);
        serializer(viscosibility);
    }
};

struct PvcdoTable : public FlatTable< PVCDORecord > {
    using FlatTable< PVCDORecord >::FlatTable;

    static PvcdoTable serializeObject()
    {
        return PvcdoTable({{1.0, 2.0, 3.0, 4.0, 5.0}});
    }
};

struct PlmixparRecord {
    static constexpr std::size_t size = 1;

    double todd_langstaff;

    bool operator==(const PlmixparRecord& data) const {
        return todd_langstaff == data.todd_langstaff;
    }

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(todd_langstaff);
    }
};

struct PlmixparTable : public FlatTable< PlmixparRecord> {
    using FlatTable< PlmixparRecord >::FlatTable;

    static PlmixparTable serializeObject()
    {
        return PlmixparTable({PlmixparRecord{1.0}});
    }
};

struct PlyvmhRecord {
    static constexpr std::size_t size = 4;

    double k_mh;
    double a_mh;
    double gamma;
    double kappa;

    bool operator==(const PlyvmhRecord& data) const {
        return k_mh == data.k_mh &&
               a_mh == data.a_mh &&
               gamma == data.gamma &&
               kappa == data.kappa;
    }

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(k_mh);
        serializer(a_mh);
        serializer(gamma);
        serializer(kappa);
    }
};

struct PlyvmhTable : public FlatTable<PlyvmhRecord> {
    using FlatTable< PlyvmhRecord >::FlatTable;

    static PlyvmhTable serializeObject()
    {
        return PlyvmhTable({{1.0, 2.0, 3.0, 4.0}});
    }
};

struct ShrateRecord {
    static constexpr std::size_t size = 1;

    double rate;

    bool operator==(const ShrateRecord& data) const {
        return rate == data.rate;
    }

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(rate);
    }
};

struct ShrateTable : public FlatTable<ShrateRecord> {
    using FlatTable< ShrateRecord >::FlatTable;

    static ShrateTable serializeObject()
    {
        return ShrateTable({ShrateRecord{1.0}});
    }
};

struct Stone1exRecord {
    static constexpr std::size_t size = 1;

    double eta;

    bool operator==(const Stone1exRecord& data) const {
        return eta == data.eta;
    }

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(eta);
    }
};

struct Stone1exTable : public FlatTable<Stone1exRecord> {
    using FlatTable< Stone1exRecord >::FlatTable;

    static Stone1exTable serializeObject()
    {
        return Stone1exTable({Stone1exRecord{1.0}});
    }
};

struct TlmixparRecord {
    static constexpr std::size_t size = 2;

    double viscosity;
    double density;

    bool operator==(const TlmixparRecord& data) const {
        return viscosity == data.viscosity &&
               density == data.density;
    }

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(viscosity);
        serializer(density);
    }
};

struct TlmixparTable : public FlatTable< TlmixparRecord> {
    using FlatTable< TlmixparRecord >::FlatTable;

    static TlmixparTable serializeObject()
    {
        return TlmixparTable({{1.0, 2.0}});
    }
};

struct VISCREFRecord {
    static constexpr std::size_t size = 2;

    double reference_pressure;
    double reference_rs;

    bool operator==(const VISCREFRecord& data) const {
        return reference_pressure == data.reference_pressure &&
              reference_rs == data.reference_rs;
    }

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(reference_pressure);
        serializer(reference_rs);
    }
};

struct ViscrefTable : public FlatTable< VISCREFRecord > {
    using FlatTable< VISCREFRecord >::FlatTable;

    static ViscrefTable serializeObject()
    {
        return ViscrefTable({{1.0, 2.0}});
    }
};

struct WATDENTRecord {
    static constexpr std::size_t size = 3;

    double reference_temperature;
    double first_coefficient;
    double second_coefficient;

    bool operator==(const WATDENTRecord& data) const {
        return reference_temperature == data.reference_temperature &&
               first_coefficient == data.first_coefficient &&
               second_coefficient == data.second_coefficient;
    }

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(reference_temperature);
        serializer(first_coefficient);
        serializer(second_coefficient);
    }
};

struct WatdentTable : public FlatTable< WATDENTRecord > {
    using FlatTable< WATDENTRecord >::FlatTable;

    static WatdentTable serializeObject()
    {
        return WatdentTable({{1.0, 2.0, 3.0}});
    }
};

}

#endif //OPM_FLAT_TABLE_HPP
