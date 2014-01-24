#include "catch.hpp"
#include "any_facade.hpp"

namespace af = any_facade;

namespace
{
    struct TestCallInterface
    {
        virtual ~TestCallInterface() {}
        virtual int f0() const = 0;
        virtual int f1(int) const = 0;
    };
}

namespace any_facade
{
    //
    // forwarder<any<...> > doesn't *need* any methods defined....
    // (unless you want to use them...)
    //
    template <>
    class forwarder<any<TestCallInterface> >
    {
    public:
        // no methods
    };

    template <typename Derived, typename Base, typename ValueType>
    class value_type_operations : public Base
    {
    public:
        virtual int f0() const { return static_cast<const Derived*>(this)->held;}
        virtual int f1(int t1) const {return static_cast<const Derived*>(this)->held + t1;}
    };
}

namespace AnyCallUnitTests
{
    TEST_CASE("Require call zero params call f0", "[call]")
    {
        af::any<TestCallInterface> a(1);
        REQUIRE(a.call(&TestCallInterface::f0) == 1);
    }

    TEST_CASE("Require call one param call f1", "[call]")
    {
        af::any<TestCallInterface> a(1);
        REQUIRE(a.call(&TestCallInterface::f1, 2) == 3);
    }
}
