/*
  Copyright 2024 Equinor ASA.

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
#include "config.h"
#define BOOST_TEST_MODULE ReservoirCouplingTests

#include <boost/test/unit_test.hpp>
#include <opm/input/eclipse/Schedule/ResCoup/ReservoirCouplingInfo.hpp>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/EclipseState/Grid/FieldPropsManager.hpp>
#include <opm/input/eclipse/EclipseState/Tables/TableManager.hpp>
#include <opm/input/eclipse/Python/Python.hpp>
#include <opm/common/utility/OpmInputError.hpp>

using namespace Opm;
namespace {
Schedule make_schedule(const std::string& schedule_string) {
    Parser parser;
    auto python = std::make_shared<Python>();
    Deck deck = parser.parseString(schedule_string);
    EclipseGrid grid(10,10,10);
    TableManager table ( deck );
    FieldPropsManager fp( deck, Phases{true, true, true}, grid, table);
    Runspec runspec (deck);
    Schedule schedule(deck, grid , fp, runspec, python);
    return schedule;
}
}

BOOST_AUTO_TEST_CASE(SLAVES) {
    std::string deck_string = R"(

SCHEDULE
SLAVES
  'RES-1'  'RC-01_MOD1_PRED'   1*  '../mod1'  4 /
  'RES-2'  'RC-01_MOD2_PRED'   1*  '../mod2'  1 /
/

)";

    const auto& schedule = make_schedule(deck_string);
    const auto& rescoup = schedule[0].rescoup();
    BOOST_CHECK(rescoup.hasSlave("RES-1"));
    auto slave = rescoup.slave("RES-1");
    BOOST_CHECK_EQUAL(slave.name(), "RES-1");
    BOOST_CHECK_EQUAL(slave.dataFilename(), "RC-01_MOD1_PRED");
    BOOST_CHECK_EQUAL(slave.directoryPath(), "../mod1");
    BOOST_CHECK_EQUAL(slave.numprocs(), 4);
}

BOOST_AUTO_TEST_CASE(GRUPMAST_OK) {
    std::string deck_string = R"(

SCHEDULE
SLAVES
  'RES-1'  'RC-01_MOD1_PRED'   1*  '../mod1'  4 /
  'RES-2'  'RC-01_MOD2_PRED'   1*  '../mod2'  1 /
/

GRUPTREE
 'PLAT-A' 'FIELD' /

 'MOD1'   'PLAT-A' /

 'B1_M'   'MOD1' /
 'D1_M'   'MOD1' /
 'C1_M'   'MOD1' /

 'E1_M'   'PLAT-A' /
/

GRUPMAST
  'D1_M' 'RES-1'  'MANI-D'  1*  /
  'B1_M' 'RES-1'  'MANI-B'  1*  /
  'C1_M' 'RES-1'  'MANI-C'  1*  /
  'E1_M' 'RES-2'  'E1'  1*  /
/
)";

    const auto& schedule = make_schedule(deck_string);
    const auto& rescoup = schedule[0].rescoup();
    BOOST_CHECK(rescoup.hasMasterGroup("D1_M"));
    auto master_group = rescoup.masterGroup("D1_M");
    BOOST_CHECK_EQUAL(master_group.name(), "D1_M");
    BOOST_CHECK_EQUAL(master_group.slaveName(), "RES-1");
    BOOST_CHECK_EQUAL(master_group.slaveGroupName(), "MANI-D");
    BOOST_CHECK_EQUAL(master_group.flowLimitFraction(), 1e+20);
}

BOOST_AUTO_TEST_CASE(GRUPMAST_FAIL_MISSING_MASTER_GROUP) {
    try {
        std::string deck_string = R"(
SCHEDULE

SLAVES
  'RES-1'  'RC-01_MOD1_PRED'   1*  '../mod1'  4 /
  'RES-2'  'RC-01_MOD2_PRED'   1*  '../mod2'  1 /
/

GRUPMAST
  'D1_M' 'RES-1'  'MANI-D'  1*  /
  'B1_M' 'RES-1'  'MANI-B'  1*  /
  'C1_M' 'RES-1'  'MANI-C'  1*  /
  'E1_M' 'RES-2'  'E1'  1*  /
/
        )";
        make_schedule(deck_string);
        BOOST_FAIL("Expected Opm::OpmInputError not thrown");
    }
    catch (const Opm::OpmInputError& e) {
        BOOST_CHECK_EQUAL(std::string(e.what()),
            "Problem with keyword GRUPMAST\nIn <memory string> line 9\nGroup 'D1_M': Not defined. Master groups should be defined in advance by using GRUPTREE before referenced in GRUPMAST.");
    }
    catch (const std::exception& e) {
        BOOST_FAIL("Expected Opm::OpmInputError but caught std::exception");
    }
    catch (...) {
        BOOST_FAIL("Expected Opm::OpmInputError but caught unknown exception");
    }
}

BOOST_AUTO_TEST_CASE(GRUPMAST_FAIL_SUBORDINATE_GROUPS) {
    try {
        std::string deck_string = R"(
SCHEDULE

SLAVES
  'RES-1'  'RC-01_MOD1_PRED'   1*  '../mod1'  4 /
  'RES-2'  'RC-01_MOD2_PRED'   1*  '../mod2'  1 /
/

GRUPTREE
 'PLAT-A' 'FIELD' /

 'MOD1'   'PLAT-A' /

 'B1_M'   'MOD1' /
 'D1_M'   'MOD1' /
 'C1_M'   'MOD1' /

 'E1_M'   'PLAT-A' /
/

GRUPMAST
  'FIELD' 'RES-1'  'MANI-D'  1*  /
  'B1_M' 'RES-1'  'MANI-B'  1*  /
  'C1_M' 'RES-1'  'MANI-C'  1*  /
  'E1_M' 'RES-2'  'E1'  1*  /
/
        )";
        make_schedule(deck_string);
        BOOST_FAIL("Expected Opm::OpmInputError not thrown");
    }
    catch (const Opm::OpmInputError& e) {
        BOOST_CHECK_EQUAL(std::string(e.what()),
            "Problem with keyword GRUPMAST\nIn <memory string> line 21\nGroup 'FIELD' has subgroups: A master group cannot contain any wells or subordinate groups.");
    }
    catch (const std::exception& e) {
        BOOST_FAIL("Expected Opm::OpmInputError but caught std::exception");
    }
    catch (...) {
        BOOST_FAIL("Expected Opm::OpmInputError but caught unknown exception");
    }
}

BOOST_AUTO_TEST_CASE(GRUPMAST_FAIL_SUBORDINATE_WELLS) {
    try {
        std::string deck_string = R"(

SCHEDULE

SLAVES
  'RES-1'  'RC-01_MOD1_PRED'   1*  '../mod1'  4 /
  'RES-2'  'RC-01_MOD2_PRED'   1*  '../mod2'  1 /
/

GRUPTREE
 'PLAT-A' 'FIELD' /

 'MOD1'   'PLAT-A' /

 'B1_M'   'MOD1' /
 'D1_M'   'MOD1' /
 'C1_M'   'MOD1' /

 'E1_M'   'PLAT-A' /
/

WELSPECS
 'C-4H' 'D1_M' 11 17 1* 'GAS' /
/

GRUPMAST
  'D1_M' 'RES-1'  'MANI-D'  1*  /
  'B1_M' 'RES-1'  'MANI-B'  1*  /
  'C1_M' 'RES-1'  'MANI-C'  1*  /
  'E1_M' 'RES-2'  'E1'  1*  /
/
        )";
        make_schedule(deck_string);
        BOOST_FAIL("Expected Opm::OpmInputError not thrown");
    }
    catch (const Opm::OpmInputError& e) {
        BOOST_CHECK_EQUAL(std::string(e.what()),
            "Problem with keyword GRUPMAST\nIn <memory string> line 26\nGroup 'D1_M' has wells: A master group cannot contain any wells or subordinate groups.");
    }
    catch (const std::exception& e) {
        BOOST_FAIL("Expected Opm::OpmInputError but caught std::exception");
    }
    catch (...) {
        BOOST_FAIL("Expected Opm::OpmInputError but caught unknown exception");
    }
}

BOOST_AUTO_TEST_CASE(GRUPMAST_FAIL_MISSING_SLAVE) {
    try {
        std::string deck_string = R"(

SCHEDULE

GRUPTREE
 'PLAT-A' 'FIELD' /

 'MOD1'   'PLAT-A' /

 'B1_M'   'MOD1' /
 'D1_M'   'MOD1' /
 'C1_M'   'MOD1' /

 'E1_M'   'PLAT-A' /
/

GRUPMAST
  'D1_M' 'RES-1'  'MANI-D'  1*  /
  'B1_M' 'RES-1'  'MANI-B'  1*  /
  'C1_M' 'RES-1'  'MANI-C'  1*  /
  'E1_M' 'RES-2'  'E1'  1*  /
/
        )";
        make_schedule(deck_string);
        BOOST_FAIL("Expected Opm::OpmInputError not thrown");
    }
    catch (const Opm::OpmInputError& e) {
        BOOST_CHECK_EQUAL(std::string(e.what()),
            "Problem with keyword GRUPMAST\nIn <memory string> line 17\nSlave reservoir 'RES-1': Not defined. Slave reservoirs should be defined in advance by using SLAVES before referenced in GRUPMAST.");
    }
    catch (const std::exception& e) {
        BOOST_FAIL("Expected Opm::OpmInputError but caught std::exception");
    }
    catch (...) {
        BOOST_FAIL("Expected Opm::OpmInputError but caught unknown exception");
    }
}
