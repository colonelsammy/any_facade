#include "catch.hpp"
#include "any_facade.hpp"
#include <map>
#include <vector>
#include <typeinfo>

namespace af = any_facade;

namespace
{
    struct TestInterface1
    {
        virtual ~TestInterface1() {}
    };

    struct NotComparableType
    {
        NotComparableType(int v) : value(v) {}
        int value;
    };
    struct LessThanComparableType
    {
        LessThanComparableType(int v) : value(v) {}
        int value;
    };
    struct EqualsComparableType
    {
        EqualsComparableType(int v) : value(v) {}
        int value;
    };
    struct FullyComparableType
    {
        FullyComparableType(int v) : value(v) {}
        int value;
    };

    struct DefaultComparableType
    {
        DefaultComparableType(int v) : value(v) {}
        int value;
        friend bool operator==(const DefaultComparableType& lhs, const DefaultComparableType& rhs)
        {
            return (lhs.value == rhs.value);
        }
        friend bool operator<(const DefaultComparableType& lhs, const DefaultComparableType& rhs)
        {
            return (lhs.value < rhs.value);
        }
    };
}

namespace any_facade
{
    // Comparison specializations
    //
    template<>
    bool less_than_comparable::less<LessThanComparableType>(const LessThanComparableType& lhs, const LessThanComparableType& rhs)
    {
        return (lhs.value < rhs.value);
    }

    template<>
    bool equality_comparable::equals<EqualsComparableType>(const EqualsComparableType& lhs, const EqualsComparableType& rhs)
    {
        return (lhs.value == rhs.value);
    }

    template<>
    bool less_than_comparable::less<FullyComparableType>(const FullyComparableType& lhs, const FullyComparableType& rhs)
    {
        return (lhs.value < rhs.value);
    }
    template<>
    bool equality_comparable::equals<FullyComparableType>(const FullyComparableType& lhs, const FullyComparableType& rhs)
    {
        return (lhs.value == rhs.value);
    }

    //
    // forwarder<any<...> > doesn't *need* any methods defined....
    // (unless you want to use them...)
    //
    template <>
    class forwarder<any<TestInterface1, not_comparable> >
    {
    public:
        // no methods
    };
    template <>
    class forwarder<any<TestInterface1, less_than_comparable> >
    {
    public:
        // no methods
    };
    template <>
    class forwarder<any<TestInterface1, equality_comparable> >
    {
    public:
        // no methods
    };
    template <>
    class forwarder<any<TestInterface1, less_than_equals_comparable> >
    {
    public:
        // no methods
    };

    // implementation of value_type_operations does nothing
    template <typename Derived, typename Base, typename ValueType>
    class value_type_operations : public Base
    {
    public:
    };
}

namespace AnyComparisonUnitTests
{
    TEST_CASE("Object without comparison operators doesn't compile","")
    {
        typedef TestInterface1 Interface;
        typedef af::not_comparable Comparable;
        typedef af::any<Interface, Comparable> Any;

        Any a(NotComparableType(4));
        Any b(NotComparableType(7));
        // not_comparable - must not compile
        //REQUIRE(a < b);
        //REQUIRE(a == b);
    }

    TEST_CASE("Object without == operator doesn't compile","")
    {
        typedef TestInterface1 Interface;
        typedef af::less_than_comparable Comparable;
        typedef af::any<Interface, Comparable> Any;

        Any a(LessThanComparableType(4));
        Any b(LessThanComparableType(7));
        // less_than_comparable - should compile with specialization of less_than_comparable::less<LessThanComparableType>
        REQUIRE(a < b);
        // less_than_comparable - must not compile
        //REQUIRE(a == b);
    }

    TEST_CASE("Object without < operator doesn't compile","")
    {
        typedef TestInterface1 Interface;
        typedef af::equality_comparable Comparable;
        typedef af::any<Interface, Comparable> Any;

        Any a(EqualsComparableType(4));
        Any b(EqualsComparableType(7));
        Any c(EqualsComparableType(7));
        // equality_comparable - must not compile
        //REQUIRE(a < b);
        // equality_comparable - should compile with specialization of equality_comparable::equals<EqualsComparableType>
        REQUIRE(a != b);
        REQUIRE(c == b);
    }

    TEST_CASE("Object with comparison specialzation compares correctly","")
    {
        typedef TestInterface1 Interface;
        typedef af::less_than_equals_comparable Comparable;
        typedef af::any<Interface, Comparable> Any;

        Any a(FullyComparableType(4));
        Any b(FullyComparableType(7));
        Any c(FullyComparableType(7));
        Any d(FullyComparableType(11));
        // less_than_equals_comparable - should compile with specialization of less_than_comparable::less<FullyComparableType>
        REQUIRE(a < b);
        REQUIRE(c < d);

        // less_than_equals_comparable - should compile with specialization of equality_comparable::equals<FullyComparableType>
        REQUIRE(a != b);
        REQUIRE(c == b);
    }

    TEST_CASE("Object with comparison operators compares correctly","")
    {
        typedef TestInterface1 Interface;
        typedef af::less_than_equals_comparable Comparable;
        typedef af::any<Interface, Comparable> Any;

        Any a(DefaultComparableType(4));
        Any b(DefaultComparableType(7));
        Any c(DefaultComparableType(7));
        Any d(DefaultComparableType(11));
        // less_than_equals_comparable
        REQUIRE(a < b);
        REQUIRE(c < d);

        // less_than_equals_comparable
        REQUIRE(a != b);
        REQUIRE(c == b);
    }
}
