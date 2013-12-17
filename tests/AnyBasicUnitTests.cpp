#include "catch.hpp"
#include "any_facade.hpp"

namespace af = any_facade;

namespace
{
    struct TestInterface1
    {
        virtual ~TestInterface1() {}
    };

    struct TestInterface2
    {
        virtual ~TestInterface2() {}
        virtual int value() const = 0;
    };
}

namespace any_facade
{
    //
    // forwarder<any<...> > doesn't *need* any methods defined....
    // (unless you want to use them...)
    //
    template <>
    class forwarder<any<TestInterface1> >
    {
    public:
        // no methods
    };

    //
    // Method implemented... deriving from the interface is optional but
    // ensures that changes to the interface will get caught by the compiler
    //
    template <>
    class forwarder<any<TestInterface2> > : public TestInterface2
    {
    public:
        int value() const
        {
            return static_cast<const any<TestInterface2>*>(this)->content->value();
        }
    };

    template <typename Base>
    class value_type_operations<Base, int> : public Base
    {
    public:
        value_type_operations(const int & value)
            : Base(value)
        {}
        virtual int value() const
        {
            return this->held;
        }
    };
}

namespace AnyBasicUnitTests
{
    TEST_CASE("Require default any is empty", "[any]")
    {
        af::any<TestInterface1> a;
        REQUIRE(a.empty());
    }

    TEST_CASE("Require any not empty after value type construction", "[any]")
    {
        int value(42);
        af::any<TestInterface1> a(value);
        REQUIRE(!a.empty());
    }

    TEST_CASE("Require any returns correct value", "[any]")
    {
        int value(42);
        af::any<TestInterface2> a(value);
        REQUIRE(a.value() == 42);
    }
}
