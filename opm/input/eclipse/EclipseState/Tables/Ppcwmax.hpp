/*
  Copyright (C) 2023 NORCE

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
#ifndef PPCWMAX_HPP
#define PPCWMAX_HPP

#include <cstddef>
#include <vector>

namespace Opm {
class Deck;

struct PpcwmaxRecord {
    double max_cap_pres;
    bool option;

    PpcwmaxRecord() = default;
    PpcwmaxRecord(double pres, bool optn) :
        max_cap_pres(pres),
        option(optn)
    {};

    bool operator==(const PpcwmaxRecord& other) const {
        return this->max_cap_pres == other.max_cap_pres &&
               this->option == other.option;
    }

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(max_cap_pres);
        serializer(option);
    }
};

class Ppcwmax {
public:
    Ppcwmax() = default;
    explicit Ppcwmax(const Deck& deck);
    static Ppcwmax serializationTestObject();
    std::size_t size() const;
    bool empty() const;
    const PpcwmaxRecord& operator[](const std::size_t index) const;

    bool operator==(const Ppcwmax& other) const {
        return this->data == other.data;
    }

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(data);
    }

private:
    std::vector<PpcwmaxRecord> data;  

};
}

#endif
