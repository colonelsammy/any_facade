#include "catch.hpp"
#include "any_facade.hpp"
#include <map>
#include <vector>

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
        virtual void print(std::ostream& os) const = 0;
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
        void print(std::ostream& os) const
        {
            static_cast<const any<TestInterface2>*>(this)->content->print(os);
        }
    };

    template <typename Derived, typename Base>
    class value_type_operations : public Base
    {
    public:
        virtual void print(std::ostream& os) const
        {
            os << static_cast<const Derived*>(this)->held;
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
        std::string v("42");
        af::any<TestInterface1> a(v);
        REQUIRE(!a.empty());
    }

    TEST_CASE("Require any returns correct value", "[any]")
    {
        std::string v("42");
        af::any<TestInterface2> a(v);
        std::ostringstream oss;
        a.print(oss);
        REQUIRE(oss.str() == "42");
    }

    TEST_CASE("Require int values compare", "[any]")
    {
        af::any<TestInterface1> a(42);
        af::any<TestInterface1> b(666);
        af::any<TestInterface1> c(a);
        REQUIRE(a == c);
        REQUIRE(a != b);
        REQUIRE(b != c);
        REQUIRE(a < b);
        REQUIRE(b >= c);
        REQUIRE(b > a);
        REQUIRE(c <= b);
        
        // compare to ints
        REQUIRE(42 == a);
        REQUIRE(42 == c);
        REQUIRE(666 == b);
        REQUIRE(42 != b);
        REQUIRE(a == 42);
        REQUIRE(b == 666);
        REQUIRE(b != 42);

        REQUIRE(0 < a);
        REQUIRE(a > 0);
        REQUIRE(a >= 1);
        REQUIRE(42 <= a);
        int v = 0;
        bool equivalent = !(v < a) && !(a < v);
        REQUIRE(!equivalent);
    }

    TEST_CASE("Require different types compare", "[any]")
    {
        af::any<TestInterface1> a(42);
        af::any<TestInterface1> b(42.0);
        REQUIRE(a != b);
        REQUIRE(a != 42.0);
        REQUIRE(42.0 != a);

        bool equivalent = !(a < b) && !(b < a);
        REQUIRE(!equivalent);
        double v = 0;
        equivalent = !(v < a) && !(a < v);
        REQUIRE(!equivalent);
    }

    TEST_CASE("Require any works in map", "[any]")
    {
        typedef af::any<TestInterface2> Any;
        std::map<Any, std::string> m;
        m.insert(std::make_pair(Any(42), "42"));
        m.insert(std::make_pair(Any(666), "666"));
        m.insert(std::make_pair(Any(42.0), "42.0"));
        std::map<Any, std::string>::iterator it = m.find(Any(42));
        REQUIRE(it != m.end());
        std::map<Any, std::string>::iterator itd = m.find(Any(42.0));
        REQUIRE(itd != m.end());
        REQUIRE(it != itd);
        REQUIRE(itd->first == Any(42.0));

        // (int) 42 should ne before (int) 666
        ++it;
        REQUIRE(it != m.end());
        REQUIRE(it->first == Any(666));
    }

    TEST_CASE("Require any can be sorted", "[any]")
    {
        typedef af::any<TestInterface2> Any;
        std::vector<Any> v;
        v.push_back(Any(5));
        v.push_back(Any(7));
        v.push_back(Any(3));
        v.push_back(Any(3.0));
        std::sort(v.begin(), v.end());
        if( v[0] == Any(3.0) )
        {
            WARN("double sorted before int");
            REQUIRE(v[1] == Any(3));
            REQUIRE(v[2] == Any(5));
            REQUIRE(v[3] == Any(7));
        }
        else
        {
            WARN("int sorted before double");
            REQUIRE(v[0] == Any(3));
            REQUIRE(v[1] == Any(5));
            REQUIRE(v[2] == Any(7));
            REQUIRE(v[3] == Any(3.0));
        }
    }
}
