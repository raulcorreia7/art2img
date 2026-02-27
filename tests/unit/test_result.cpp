// Unit tests: Result type

#include "doctest.h"

#include <art2img.hpp>

#include <stdexcept>
#include <string>

using namespace art2img;

TEST_SUITE("Result") {
    TEST_CASE("Result value construction") {
        Result<int> r(42);
        CHECK(r.ok());
        CHECK(r);
        CHECK(r.value() == 42);
        CHECK(r.code() == error::none);
        CHECK(r.message().empty());
    }

    TEST_CASE("Result error construction") {
        Result<int> r(Error{error::invalid_argument, "test error"});
        CHECK(!r.ok());
        CHECK(!r);
        CHECK(r.code() == error::invalid_argument);
        CHECK(r.message() == "test error");
    }

    TEST_CASE("Result value access throws on error") {
        Result<int> r(Error{error::io_error, "fail"});
        // std::get throws std::bad_variant_access on wrong type
        CHECK_THROWS_AS(r.value(), std::bad_variant_access);
    }

    TEST_CASE("Result error access on success throws") {
        Result<int> r(42);
        // error() throws std::bad_variant_access when result holds value
        CHECK_THROWS_AS(r.error(), std::bad_variant_access);
    }

    TEST_CASE("Result void specialization success") {
        Result<void> r;
        CHECK(r.ok());
        CHECK(r);
        CHECK(r.code() == error::none);
    }

    TEST_CASE("Result void specialization error") {
        Result<void> r(Error{error::corrupted_data, "bad data"});
        CHECK(!r.ok());
        CHECK(!r);
        CHECK(r.code() == error::corrupted_data);
        CHECK(r.message() == "bad data");
    }

    TEST_CASE("Result with complex type") {
        Result<std::string> r(std::string("hello"));
        CHECK(r.ok());
        CHECK(r.value() == "hello");
    }

}  // TEST_SUITE
