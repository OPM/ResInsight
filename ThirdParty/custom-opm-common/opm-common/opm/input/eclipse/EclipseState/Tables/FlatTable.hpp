#ifndef OPM_FLAT_TABLE_HPP
#define OPM_FLAT_TABLE_HPP

#include <cstddef>
#include <initializer_list>
#include <string_view>
#include <vector>

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

template <typename RecordType>
class FlatTableWithCopy
{
public:
    FlatTableWithCopy() = default;
    explicit FlatTableWithCopy(const DeckKeyword& kw,
                               std::string_view   expect = "");
    explicit FlatTableWithCopy(std::initializer_list<RecordType> records);

    auto size()  const { return this->table_.size(); }
    bool empty() const { return this->table_.empty(); }
    auto begin() const { return this->table_.begin(); }
    auto end()   const { return this->table_.end(); }

    const RecordType& operator[](const std::size_t tableID) const
    {
        return this->table_[tableID];
    }

    const RecordType& at(const std::size_t tableID) const
    {
        return this->table_.at(tableID);
    }

    bool operator==(const FlatTableWithCopy& other) const
    {
        return this->table_ == other.table_;
    }

    template <class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer.vector(this->table_);
    }

protected:
    std::vector<RecordType> table_{};
};

struct GRAVITYRecord {
    static constexpr std::size_t size = 3;

    double oil_api;
    double water_sg;
    double gas_sg;

    bool operator==(const GRAVITYRecord& data) const {
        return this->oil_api == data.oil_api &&
               this->water_sg == data.water_sg &&
               this->gas_sg == data.gas_sg;
    }

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(this->oil_api);
        serializer(this->water_sg);
        serializer(this->gas_sg);
    }
};

struct GravityTable : public FlatTableWithCopy<GRAVITYRecord>
{
    GravityTable() = default;
    explicit GravityTable(const DeckKeyword& kw);
    explicit GravityTable(std::initializer_list<GRAVITYRecord> records);

    static GravityTable serializeObject()
    {
        return GravityTable({{1.0, 2.0, 3.0}});
    }

    template <class Serializer>
    void serializeOp(Serializer& serializer)
    {
        FlatTableWithCopy::serializeOp(serializer);
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

struct DensityTable : public FlatTableWithCopy<DENSITYRecord>
{
    DensityTable() = default;
    explicit DensityTable(const DeckKeyword& kw);
    explicit DensityTable(const GravityTable& gravity);
    explicit DensityTable(std::initializer_list<DENSITYRecord> records);

    static DensityTable serializeObject()
    {
        return DensityTable({{1.0, 2.0, 3.0}});
    }

    template <class Serializer>
    void serializeOp(Serializer& serializer)
    {
        FlatTableWithCopy::serializeOp(serializer);
    }
};

struct DiffCoeffRecord {
    static constexpr std::size_t size = 8;

    double oil_mw;
    double gas_mw;
    double gas_in_gas;
    double oil_in_gas;
    double gas_in_oil;
    double oil_in_oil;
    double gas_in_oil_cross_phase;
    double oil_in_oil_cross_phase;

    bool operator==(const DiffCoeffRecord& data) const {
        return oil_mw == data.oil_mw &&
               gas_mw == data.gas_mw &&
               gas_in_gas == data.gas_in_gas &&
               oil_in_gas == data.oil_in_gas &&
               gas_in_oil == data.gas_in_oil &&
               oil_in_oil == data.oil_in_oil &&
               gas_in_oil_cross_phase == data.gas_in_oil_cross_phase &&
               oil_in_oil_cross_phase == data.oil_in_oil_cross_phase;
    }

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(oil_mw);
        serializer(gas_mw);
        serializer(gas_in_gas);
        serializer(oil_in_gas);
        serializer(gas_in_oil);
        serializer(oil_in_oil);
        serializer(gas_in_oil_cross_phase);
        serializer(oil_in_oil_cross_phase);
    }
};

struct DiffCoeffTable : public FlatTable< DiffCoeffRecord > {
    using FlatTable< DiffCoeffRecord >::FlatTable;

    static DiffCoeffTable serializeObject()
    {
        return DiffCoeffTable({{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0}});
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

struct PvtwTable : public FlatTableWithCopy<PVTWRecord>
{
    PvtwTable() = default;
    explicit PvtwTable(const DeckKeyword& kw);
    explicit PvtwTable(std::initializer_list<PVTWRecord> records);

    static PvtwTable serializeObject()
    {
        return PvtwTable({{1.0, 2.0, 3.0, 4.0, 5.0}});
    }

    template <class Serializer>
    void serializeOp(Serializer& serializer)
    {
        FlatTableWithCopy::serializeOp(serializer);
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

struct SatFuncLETRecord {
    static constexpr std::size_t size = 17;

    double s1_residual;
    double s1_critical;
    double l1_relperm;
    double e1_relperm;
    double t1_relperm;
    double krt1_relperm;
    double s2_residual;
    double s2_critical;
    double l2_relperm;
    double e2_relperm;
    double t2_relperm;
    double krt2_relperm;
    double l_pc;
    double e_pc;
    double t_pc;
    double pcir_pc;
    double pct_pc;

    bool operator==(const SatFuncLETRecord& data) const {
        return s1_residual == data.s1_residual &&
               s1_critical == data.s1_critical &&
               l1_relperm == data.l1_relperm &&
               e1_relperm == data.e1_relperm &&
               t1_relperm == data.t1_relperm &&
               krt1_relperm == data.krt1_relperm &&
               s2_residual == data.s2_residual &&
               s2_critical == data.s2_critical &&
               l2_relperm == data.l2_relperm &&
               e2_relperm == data.e2_relperm &&
               t2_relperm == data.t2_relperm &&
               krt2_relperm == data.krt2_relperm &&
               l_pc == data.l_pc &&
               e_pc == data.e_pc &&
               t_pc == data.t_pc &&
               pcir_pc == data.pcir_pc &&
               pct_pc == data.pct_pc;
    }

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(s1_residual);
        serializer(s1_critical);
        serializer(l1_relperm);
        serializer(e1_relperm);
        serializer(t1_relperm);
        serializer(krt1_relperm);
        serializer(s2_residual);
        serializer(s2_critical);
        serializer(l2_relperm);
        serializer(e2_relperm);
        serializer(t2_relperm);
        serializer(krt2_relperm);
        serializer(l_pc);
        serializer(e_pc);
        serializer(t_pc);
        serializer(pcir_pc);
        serializer(pct_pc);
    }
};

struct SwofletTable : public FlatTable< SatFuncLETRecord > {
    using FlatTable< SatFuncLETRecord >::FlatTable;

    static SwofletTable serializeObject()
    {
        return SwofletTable({{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0}});
    }
};


struct SgofletTable : public FlatTable< SatFuncLETRecord > {
    using FlatTable< SatFuncLETRecord >::FlatTable;

    static SgofletTable serializeObject()
    {
        return SgofletTable({{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0}});
    }
};

}

#endif //OPM_FLAT_TABLE_HPP
