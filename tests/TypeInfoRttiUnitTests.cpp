#include "catch.hpp"

#include "any_facade.hpp"

namespace TypeInfoRttiUnitTests
{
    class A {};

    TEST_CASE("Require int types give same id", "[types]")
    {
        typedef int InterfaceClass; // doesn't need a real class for this test
        REQUIRE(typeid(int) == typeid(const int&));
        REQUIRE(any_facade::type_info<InterfaceClass>::type_id<int>() == any_facade::type_info<InterfaceClass>::type_id<const int&>());
    }

    TEST_CASE("Require class types give same id", "[types]")
    {
        typedef int InterfaceClass; // doesn't need a real class for this test
        REQUIRE(typeid(A) == typeid(const A&));
        REQUIRE(any_facade::type_info<InterfaceClass>::type_id<A>() == any_facade::type_info<InterfaceClass>::type_id<const A&>());
        REQUIRE(any_facade::type_info<InterfaceClass>::type_id<A*>() == any_facade::type_info<InterfaceClass>::type_id<const A*>());
    }

    TEST_CASE("Require different types do not give same id", "[types]")
    {
        typedef int InterfaceClass; // doesn't need a real class for this test
        REQUIRE(typeid(A) != typeid(const int&));
        REQUIRE(any_facade::type_info<InterfaceClass>::type_id<A>() != any_facade::type_info<InterfaceClass>::type_id<const int&>());

        REQUIRE(typeid(int) != typeid(const A&));
        REQUIRE(any_facade::type_info<InterfaceClass>::type_id<int>() != any_facade::type_info<InterfaceClass>::type_id<const A&>());
    }

}
