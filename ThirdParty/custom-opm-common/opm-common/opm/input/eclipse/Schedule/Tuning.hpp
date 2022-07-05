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

#ifndef OPM_TUNING_HPP
#define OPM_TUNING_HPP

namespace Opm {

    class NextStep {
    public:
        NextStep() = default;
        NextStep(double value, bool every_report);
        double value() const;
        bool every_report() const;
        bool operator==(const NextStep& other) const;
        static NextStep serializeObject();

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer(this->next_tstep);
            serializer(this->persist);
        }

    private:
        double next_tstep;
        bool persist;
    };

    struct Tuning {
        Tuning();

        static Tuning serializeObject();

        // Record1
        double TSINIT;
        double TSMAXZ;
        double TSMINZ;
        double TSMCHP;
        double TSFMAX;
        double TSFMIN;
        double TFDIFF;
        double TSFCNV;
        double THRUPT;
        double TMAXWC = 0.0;
        bool TMAXWC_has_value = false;

        // Record 2
        double TRGTTE;
        double TRGCNV;
        double TRGMBE;
        double TRGLCV;
        double XXXTTE;
        double XXXCNV;
        double XXXMBE;
        double XXXLCV;
        double XXXWFL;
        double TRGFIP;
        double TRGSFT = 0.0;
        bool TRGSFT_has_value = false;
        double THIONX;
        double TRWGHT;

        // Record 3
        int NEWTMX;
        int NEWTMN;
        int LITMAX;
        int LITMIN;
        int MXWSIT;
        int MXWPIT;
        double DDPLIM;
        double DDSLIM;
        double TRGDPR;
        double XXXDPR;
        bool XXXDPR_has_value = false;

        /*
          In addition to the values set in the TUNING keyword this Tuning
          implementation also contains the result of the WSEGITER keyword, which
          is special tuning parameters to be applied to the multisegment well
          model. Observe that the maximum number of well iterations - MXWSIT -
          is specified by both the TUNING keyword and the WSEGITER keyword, but
          with different defaults.
        */
        int WSEG_MAX_RESTART;
        double WSEG_REDUCTION_FACTOR;
        double WSEG_INCREASE_FACTOR;


        bool operator==(const Tuning& data) const;
        bool operator !=(const Tuning& data) const {
            return !(*this == data);
        }

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer(TSINIT);
            serializer(TSMAXZ);
            serializer(TSMINZ);
            serializer(TSMCHP);
            serializer(TSFMAX);
            serializer(TSFMIN);
            serializer(TFDIFF);
            serializer(TSFCNV);
            serializer(THRUPT);
            serializer(TMAXWC);
            serializer(TMAXWC_has_value);

            serializer(TRGTTE);
            serializer(TRGCNV);
            serializer(TRGMBE);
            serializer(TRGLCV);
            serializer(XXXTTE);
            serializer(XXXCNV);
            serializer(XXXMBE);
            serializer(XXXLCV);
            serializer(XXXWFL);
            serializer(TRGFIP);
            serializer(TRGSFT);
            serializer(TRGSFT_has_value);
            serializer(THIONX);
            serializer(TRWGHT);

            serializer(NEWTMX);
            serializer(NEWTMN);
            serializer(LITMAX);
            serializer(LITMIN);
            serializer(MXWSIT);
            serializer(MXWPIT);
            serializer(DDPLIM);
            serializer(DDSLIM);
            serializer(TRGDPR);
            serializer(XXXDPR);
            serializer(XXXDPR_has_value);

            serializer(WSEG_MAX_RESTART);
            serializer(WSEG_REDUCTION_FACTOR);
            serializer(WSEG_INCREASE_FACTOR);
        }
    };

} //namespace Opm

#endif
