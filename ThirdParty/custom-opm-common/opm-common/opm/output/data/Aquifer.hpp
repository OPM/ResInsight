/*
  Copyright 2019 Statoil ASA.

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

#ifndef OPM_OUTPUT_AQUIFER_HPP
#define OPM_OUTPUT_AQUIFER_HPP

#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace Opm { namespace data {

    enum class AquiferType
    {
        Fetkovich, CarterTracy, Numerical,
    };

    struct FetkovichData
    {
        double initVolume{};
        double prodIndex{};
        double timeConstant{};

        bool operator==(const FetkovichData& other) const;

        // MessageBufferType API should be similar to Dune::MessageBufferIF
        template <class MessageBufferType>
        void write(MessageBufferType& buffer) const;

        // MessageBufferType API should be similar to Dune::MessageBufferIF
        template <class MessageBufferType>
        void read(MessageBufferType& buffer);
    };

    struct CarterTracyData
    {
        double timeConstant{};
        double influxConstant{};
        double waterDensity{};
        double waterViscosity{};

        double dimensionless_time{};
        double dimensionless_pressure{};

        bool operator==(const CarterTracyData& other) const;

        // MessageBufferType API should be similar to Dune::MessageBufferIF
        template <class MessageBufferType>
        void write(MessageBufferType& buffer) const;

        // MessageBufferType API should be similar to Dune::MessageBufferIF
        template <class MessageBufferType>
        void read(MessageBufferType& buffer);
    };

    struct NumericAquiferData
    {
        std::vector<double> initPressure{};

        bool operator==(const NumericAquiferData& other) const;

        // MessageBufferType API should be similar to Dune::MessageBufferIF
        template <class MessageBufferType>
        void write(MessageBufferType& buffer) const;

        // MessageBufferType API should be similar to Dune::MessageBufferIF
        template <class MessageBufferType>
        void read(MessageBufferType& buffer);
    };

    namespace detail {
        template <AquiferType>
        struct TypeMap;

        template <> struct TypeMap<AquiferType::CarterTracy>
        {
            using Alternative = CarterTracyData;
        };

        template <> struct TypeMap<AquiferType::Fetkovich>
        {
            using Alternative = FetkovichData;
        };

        template <> struct TypeMap<AquiferType::Numerical>
        {
            using Alternative = NumericAquiferData;
        };

        template <AquiferType t>
        using TypeMap_t = typename TypeMap<t>::Alternative;
    } // namespace detail

    class TypeSpecificData
    {
    private:
        template <typename T>
        bool is() const
        {
            return std::holds_alternative<T>(this->options_);
        }

        template <typename T>
        const T* get() const
        {
            return this->template is<T>()
                ? &std::get<T>(this->options_)
                : nullptr;
        }

        template <typename T>
        T* get()
        {
            return this->template is<T>()
                ? &std::get<T>(this->options_)
                : nullptr;
        }

    public:
        TypeSpecificData() = default;

        TypeSpecificData(const TypeSpecificData&) = default;
        TypeSpecificData(TypeSpecificData&&) = default;

        TypeSpecificData& operator=(const TypeSpecificData&) = default;
        TypeSpecificData& operator=(TypeSpecificData&&) = default;

        bool operator==(const TypeSpecificData& that) const
        {
            return std::visit(Equal{}, this->options_, that.options_);
        }

        template <AquiferType t>
        auto* create()
        {
            return &this->options_.emplace<detail::TypeMap_t<t>>();
        }

        template <AquiferType t>
        bool is() const
        {
            return this->template is<detail::TypeMap_t<t>>();
        }

        template <AquiferType t>
        auto const* get() const
        {
            return this->template get<detail::TypeMap_t<t>>();
        }

        template <AquiferType t>
        auto* getMutable()
        {
            return this->template get<detail::TypeMap_t<t>>();
        }

        template <typename MessageBufferType>
        void write(MessageBufferType& buffer) const
        {
            buffer.write(this->options_.index());
            std::visit(Write<MessageBufferType>{buffer}, this->options_);
        }

        template <typename MessageBufferType>
        void read(MessageBufferType& buffer)
        {
            auto type = 0 * this->options_.index();
            buffer.read(type);

            if (type < std::variant_size_v<Types>) {
                this->create(type);

                std::visit(Read<MessageBufferType>{buffer}, this->options_);
            }
        }

    private:
        using Types = std::variant<std::monostate,
                                   CarterTracyData,
                                   FetkovichData,
                                   NumericAquiferData>;

        struct Equal
        {
            template <typename T1, typename T2>
            bool operator()(const T1&, const T2&) const
            {
                return false;
            }

            template <typename T>
            bool operator()(const T& e1, const T& e2) const
            {
                return e1 == e2;
            }

            bool operator()(const std::monostate&,
                            const std::monostate&) const
            {
                return true;
            }
        };

        template <typename MessageBufferType>
        class Read
        {
        public:
            explicit Read(MessageBufferType& buffer)
                : buffer_{ buffer }
            {}

            template <typename T>
            void operator()(T& alternative)
            {
                return alternative.read(this->buffer_);
            }

            void operator()(std::monostate&)
            {}

        private:
            MessageBufferType& buffer_;
        };

        template <typename MessageBufferType>
        class Write
        {
        public:
            explicit Write(MessageBufferType& buffer)
                : buffer_{ buffer }
            {}

            template <typename T>
            void operator()(const T& alternative) const
            {
                return alternative.write(this->buffer_);
            }

            void operator()(const std::monostate&) const
            {}

        private:
            MessageBufferType& buffer_;
        };

        Types options_{};

        void create(const std::size_t option);
    };

    struct AquiferData
    {
        int aquiferID = 0;         //< One-based ID, range 1..NANAQ
        double pressure = 0.0;     //< Aquifer pressure
        double fluxRate = 0.0;     //< Aquifer influx rate (liquid aquifer)
        double volume = 0.0;       //< Produced liquid volume
        double initPressure = 0.0; //< Aquifer's initial pressure
        double datumDepth = 0.0;   //< Aquifer's pressure reference depth

        TypeSpecificData typeData{};

        double get(const std::string& key) const;

        bool operator==(const AquiferData& other) const;

        // MessageBufferType API should be similar to Dune::MessageBufferIF
        template <class MessageBufferType>
        void write(MessageBufferType& buffer) const;

        // MessageBufferType API should be similar to Dune::MessageBufferIF
        template <class MessageBufferType>
        void read(MessageBufferType& buffer);

    private:
        using GetSummaryValue = double (AquiferData::*)() const;
        using SummaryValueDispatchTable = std::unordered_map<std::string, GetSummaryValue>;

        static SummaryValueDispatchTable summaryValueDispatchTable_;

        double aquiferFlowRate() const;
        double aquiferPressure() const;
        double aquiferTotalProduction() const;
        double carterTracyDimensionlessTime() const;
        double carterTracyDimensionlessPressure() const;
    };

    // TODO: not sure what extension we will need
    using Aquifers = std::map<int, AquiferData>;

    template <class MessageBufferType>
    void FetkovichData::write(MessageBufferType& buffer) const
    {
        buffer.write(this->initVolume);
        buffer.write(this->prodIndex);
        buffer.write(this->timeConstant);
    }

    template <class MessageBufferType>
    void FetkovichData::read(MessageBufferType& buffer)
    {
        buffer.read(this->initVolume);
        buffer.read(this->prodIndex);
        buffer.read(this->timeConstant);
    }

    template <class MessageBufferType>
    void CarterTracyData::write(MessageBufferType& buffer) const
    {
        buffer.write(this->timeConstant);
        buffer.write(this->influxConstant);
        buffer.write(this->waterDensity);
        buffer.write(this->waterViscosity);
        buffer.write(this->dimensionless_time);
        buffer.write(this->dimensionless_pressure);
    }

    template <class MessageBufferType>
    void CarterTracyData::read(MessageBufferType& buffer)
    {
        buffer.read(this->timeConstant);
        buffer.read(this->influxConstant);
        buffer.read(this->waterDensity);
        buffer.read(this->waterViscosity);
        buffer.read(this->dimensionless_time);
        buffer.read(this->dimensionless_pressure);
    }

    template <class MessageBufferType>
    void NumericAquiferData::write(MessageBufferType& buffer) const
    {
        buffer.write(this->initPressure.size());

        for (const auto& pressure : this->initPressure) {
            buffer.write(pressure);
        }
    }

    template <class MessageBufferType>
    void NumericAquiferData::read(MessageBufferType& buffer)
    {
        decltype(this->initPressure.size()) size{};
        buffer.read(size);

        this->initPressure.resize(size, 0.0);
        for (auto& pressure : this->initPressure) {
            buffer.read(pressure);
        }
    }

    template <class MessageBufferType>
    void AquiferData::write(MessageBufferType& buffer) const
    {
        buffer.write(this->aquiferID);
        buffer.write(this->pressure);
        buffer.write(this->fluxRate);
        buffer.write(this->volume);
        buffer.write(this->initPressure);
        buffer.write(this->datumDepth);

        this->typeData.write(buffer);
    }

    template <class MessageBufferType>
    void AquiferData::read(MessageBufferType& buffer)
    {
        buffer.read(this->aquiferID);
        buffer.read(this->pressure);
        buffer.read(this->fluxRate);
        buffer.read(this->volume);
        buffer.read(this->initPressure);
        buffer.read(this->datumDepth);

        this->typeData.read(buffer);
    }
}} // Opm::data

#endif // OPM_OUTPUT_AQUIFER_HPP
