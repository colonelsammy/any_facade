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

    struct TestInterface2
    {
        virtual ~TestInterface2() {}
        virtual void print(std::ostream& os) const = 0;
    };

    struct NonStreamable
    {
        NonStreamable(int v)
            : value(v)
        {}
        int value;
    };
    struct NonStreamable2
    {
        NonStreamable2(int v)
            : value(v)
        {}
        int value;
    };
}

namespace any_facade
{
    template<>
    bool equality_comparable::equals<NonStreamable>(const NonStreamable& lhs, const NonStreamable& rhs)
    {
        return (lhs.value == rhs.value);
    }
    template<>
    bool less_than_comparable::less<NonStreamable>(const NonStreamable& lhs, const NonStreamable& rhs)
    {
        return (lhs.value < rhs.value);
    }
    template<>
    bool equality_comparable::equals<NonStreamable2>(const NonStreamable2& lhs, const NonStreamable2& rhs)
    {
        return (lhs.value == rhs.value);
    }

    //
    // forwarder<any<...> > doesn't *need* any methods defined....
    // (unless you want to use them...)
    //
    template <>
    class forwarder<interfaces<TestInterface1> >
    {
    public:
        // no methods
    };

    //
    // Method implemented... deriving from the interface is optional but
    // ensures that changes to the interface will get caught by the compiler
    //
    template <>
    class forwarder<interfaces<TestInterface2> > : public TestInterface2
    {
    public:
        void print(std::ostream& os) const
        {
            static_cast<const any<interfaces<TestInterface2> >*>(this)->content->print(os);
        }
    };

    template <typename Derived, typename Base, typename ValueType>
    class value_type_operations : public Base
    {
    public:
        virtual void print(std::ostream& os) const
        {
            os << static_cast<const Derived*>(this)->held;
        }
    };

    template <typename Derived, typename Base>
    class value_type_operations<Derived, Base, NonStreamable> : public Base
    {
    public:
        virtual void print(std::ostream& os) const
        {
            os << "NonStreamable:" << static_cast<const Derived*>(this)->held.value;
        }
    };
}

namespace AnyBasicUnitTests
{
    TEST_CASE("Require default any is empty", "[any]")
    {
        af::any<af::interfaces<TestInterface1> > a;
        REQUIRE(a.empty());
    }

    TEST_CASE("Require any not empty after value type construction", "[any]")
    {
        std::string v("42");
        af::any<af::interfaces<TestInterface1> > a(v);
        REQUIRE(!a.empty());
    }

    TEST_CASE("Require any returns correct value", "[any]")
    {
        std::string v("42");
        af::any<af::interfaces<TestInterface2> > a(v);
        std::ostringstream oss;
        a.print(oss);
        REQUIRE(oss.str() == "42");
    }

    /*TEST_CASE("Require non streamable specialization is called", "[any]")
    {
        NonStreamable v(42);
        af::any<TestInterface2> a(v);
        std::ostringstream oss;
        a.print(oss);
        REQUIRE(oss.str() == "NonStreamable:42");
    }*/

    TEST_CASE("Require int values compare", "[any]")
    {
        af::any<af::interfaces<TestInterface1> > a(42);
        af::any<af::interfaces<TestInterface1> > b(666);
        af::any<af::interfaces<TestInterface1> > c(a);
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
        af::any<af::interfaces<TestInterface1> > a(42);
        af::any<af::interfaces<TestInterface1> > b(42.0);
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
        typedef af::any<af::interfaces<TestInterface2> > Any;
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
        typedef af::any<af::interfaces<TestInterface2> > Any;
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

    struct I0
    {
        virtual ~I0() {}
        virtual void print(std::ostream&) = 0;
    };
    class placeholder : public I0
    {
    public:
        virtual ~placeholder() {}
        virtual bool equals(const placeholder& other) const = 0;
        virtual bool less(const placeholder& other) const = 0;
        virtual const std::type_info& type() const = 0;
    };

    template <class T>
    bool defaultEquals(const T& lhs, const T& rhs)
    {
        return (lhs == rhs);
    }
    template <typename T>
    bool defaultLess(const T& lhs, const T& rhs)
    {
        return (lhs < rhs);
    }

    template<template<typename>class Derived, typename ValueType>
    class less_than_equals : public placeholder
    {
    public:
        virtual bool equals(const placeholder& other) const
        {
            // safe - types match
            const Derived<ValueType>* otherType = static_cast<const Derived<ValueType>*>(&other);
            //return (static_cast<const Derived<ValueType>*>(this)->v == otherType->v );
            return defaultEquals(static_cast<const Derived<ValueType>*>(this)->v, otherType->v);
        }
        virtual bool less(const placeholder& other) const
        {
            // safe - types match
            const Derived<ValueType>* otherType = static_cast<const Derived<ValueType>*>(&other);
            //return (static_cast<const Derived<ValueType>*>(this)->v < otherType->v );
            return defaultLess(static_cast<const Derived<ValueType>*>(this)->v, otherType->v);
        }
    };

    // The Curiously Recurring Template Pattern (CRTP)
    template<template<typename>class Derived, typename ValueType>
    class value_type_operations;
    
    template <typename ValueType>
    class Derived : public value_type_operations<Derived, ValueType>
    {
    public:
        typedef ValueType V2;
        Derived(const ValueType& t)
            : v(t)
        {}
        // ...
        virtual const std::type_info& type() const
        {
            return typeid(ValueType);
        }
    public:
        ValueType v;
    };
    
    // The Curiously Recurring Template Pattern (CRTP)
    template<template<typename>class Derived, typename ValueType>
    class value_type_operations : public less_than_equals< Derived, ValueType >
    {
    public:
        // methods within Base can use template to access members of Derived
        virtual void print(std::ostream& os)
        {
            os << static_cast<Derived<ValueType>*>(this)->v;
        }
        virtual bool equals(const placeholder& rhs) const
        {
            // safe - types match
            if( rhs.type() == typeid(int) && this->type() == typeid(double) )
            {
                const Derived<int>* otherType = static_cast<const Derived<int>*>(&rhs);
                int v1 = static_cast<int>(otherType->v);
                return (v1 == static_cast<const Derived<ValueType>*>(this)->v);
            }
            return less_than_equals<Derived, ValueType>::equals(rhs);
        }
    };

    template<>
    bool defaultEquals<NonStreamable>(const NonStreamable& lhs, const NonStreamable& rhs)
    {
        return (lhs.value == rhs.value);
    }
    template<>
    bool defaultLess<NonStreamable>(const NonStreamable& lhs, const NonStreamable& rhs)
    {
        return (lhs.value < rhs.value);
    }

    template<template<typename>class Derived>
    class value_type_operations<Derived, NonStreamable> : public less_than_equals< Derived, NonStreamable >
    {
    public:
        // methods within Base can use template to access members of Derived
        virtual void print(std::ostream& os)
        {
            os << "NonStreamable:" << static_cast<Derived<NonStreamable>*>(this)->v.value;
        }
    };

    /*typedef char no[2];
    template<typename T> no& operator == (const T&, const T&);

    template <typename T>
    struct opEqualExists
    {
        static const bool value = (sizeof(*(T*)(0) == *(T*)(0)) != sizeof(no));
    };*/
    /*template <bool B, class T = void>
    struct enable_if_c {
      typedef bool type;
    };*/
    /*template <>
    struct enable_if_c<false> {
    };*/

    /*template <typename T, typename U>
    struct has_equals {
        typedef char yes[1];
        typedef char no[2];
 
        template <typename C, bool (*)(const U&, const U&)> struct ptmf_helper {};

        template <typename C>
        static yes& has_member_helper(ptmf_helper<C, &C::equals>*);
 
        template<typename> static no& has_member_helper(...);
 
        static const bool value = sizeof(has_member_helper<T>(0)) == sizeof(yes);
    };

    template <typename T, typename U>
    struct has_less_than {
        typedef char yes[1];
        typedef char no[2];
 
        template <typename C, bool (*)(const U&, const U&)> struct ptmf_helper {};

        template <typename C>
        static yes& has_member_helper(ptmf_helper<C, &C::less>*);
 
        template<typename> static no& has_member_helper(...);
 
        static const bool value = sizeof(has_member_helper<T>(0)) == sizeof(yes);
    };*/

    /*
    template <typename T, void (T::*)(std::vector<unsigned char>& data) const> struct ptmf_helper {};
    template<typename T> no_tag has_member_helper_readTransducerData(...);

    template<typename T>
    yes_tag has_member_helper_readTransducerData(ptmf_helper<T, &T::readTransducerData>* p);

    template<typename T> struct has_member_readTransducerData
    {
        static const bool value = sizeof(has_member_helper_readTransducerData<T>(0)) == sizeof(yes_tag);
    };
    */
    /*template <class T>
    typename enable_if_c<opEqualExists<T>::value, T>::type 
    defaultEquals(const T& lhs, const T& rhs)
    {
        return (lhs == rhs);
    }*/

    /*using af::less_than_equals_comparable;
    using af::equality_comparable;
    using af::less_than_comparable;
    using af::not_comparable;

    template <typename Interface, typename Comparable>
    class placeholder2;

    template <typename Interface>
    class placeholder2<Interface, less_than_equals_comparable> : public Interface
    {
    public:
        virtual ~placeholder2() {}
        virtual bool equals(const placeholder2& other) const = 0;
        virtual bool less(const placeholder2& other) const = 0;
    };
    template <typename Interface>
    class placeholder2<Interface, equality_comparable> : public Interface
    {
    public:
        virtual ~placeholder2() {}
        virtual bool equals(const placeholder2& other) const = 0;
    };
    template <typename Interface>
    class placeholder2<Interface, less_than_comparable> : public Interface
    {
    public:
        virtual ~placeholder2() {}
        virtual bool less(const placeholder2& other) const = 0;
    };

    template <typename Interface>
    class placeholder2<Interface, not_comparable> : public Interface
    {
    public:
        virtual ~placeholder2() {}
    };*/

    template <typename Derived, typename Interface, typename Comparable>
    class holder2;

    template <typename Derived, typename Interface>
    class holder2<Derived, Interface, af::less_than_equals_comparable> : public af::placeholder<Interface, af::less_than_equals_comparable>
    {
    public:
        virtual bool equals(const af::placeholder<Interface, af::less_than_equals_comparable>& other) const
        {
            const Derived* otherType = static_cast<const Derived*>(&other);
            return af::less_than_equals_comparable::equals(static_cast<const Derived*>(this)->held, otherType->held);
        }
        virtual bool less(const af::placeholder<Interface, af::less_than_equals_comparable>& other) const
        {
            const Derived* otherType = static_cast<const Derived*>(&other);
            return af::less_than_equals_comparable::less(static_cast<const Derived*>(this)->held, otherType->held);
        }
    };

    template <typename Derived, typename Interface>
    class holder2<Derived, Interface, af::less_than_comparable> : public af::placeholder<Interface, af::less_than_comparable>
    {
    public:
        virtual bool less(const af::placeholder<Interface, af::less_than_comparable>& other) const
        {
            const Derived* otherType = static_cast<const Derived*>(&other);
            return af::less_than_comparable::less(static_cast<const Derived*>(this)->held, otherType->held);
        }
    };

    template <typename Derived, typename Interface>
    class holder2<Derived, Interface, af::equality_comparable> : public af::placeholder<Interface, af::equality_comparable>
    {
    public:
        virtual bool equals(const af::placeholder<Interface, af::equality_comparable>& other) const
        {
            const Derived* otherType = static_cast<const Derived*>(&other);
            return af::equality_comparable::equals(static_cast<const Derived*>(this)->held, otherType->held);
        }
    };

    template <typename Interface, typename Comparable, typename ValueType>
    class holder : public holder2<holder<Interface, Comparable, ValueType>, Interface, Comparable>
    {
    public:
        holder(ValueType v)
            : held(v)
        {}
        virtual void print(std::ostream&) const
        {
        }
        virtual af::type_info<af::any<Interface,Comparable> > type() const
        {
            return af::type_info<af::any<Interface,Comparable> >::template type_id<ValueType>();
        }
        virtual af::placeholder<Interface, Comparable> * clone() const
        {
            return new holder(*this);
        }
        ValueType held;
    };

    template <typename Interface = af::interfaces<>, typename Comparable = af::less_than_equals_comparable>
    struct AnyTest
    {
        //static const bool eq = Comparable::equals;
        //static const bool lt = Comparable::less_than;
        //static const bool eq = has_equals<Comparable, AnyTest>::value;
        //static const bool lt = has_less_than<Comparable, AnyTest>::value;
        template <typename ValueType>
        AnyTest(const ValueType& v)
            : p(new holder<Interface, Comparable, ValueType>(v))
        {
        }
        ~AnyTest()
        { delete p;}

        friend bool operator==(const AnyTest& lhs, const AnyTest& rhs)
        {
            return lhs.p->equals(*rhs.p);
        }
        friend bool operator<(const AnyTest& lhs, const AnyTest& rhs)
        {
            return lhs.p->less(*rhs.p);
        }
        virtual void print(std::ostream& oss)
        {
            p->print(oss);
        }
        af::placeholder<Interface, Comparable>* p;
    };

    TEST_CASE("3","")
    {
        Derived<int> tmp(42);
        Derived<int> tmp2(666);
        Derived<int> c(tmp);
        bool v = tmp.equals(tmp2);
        REQUIRE(!v);
        tmp.print(std::cout);
        v = tmp.equals(c);
        REQUIRE(v);
        Derived<double> tmpd(42.0);
        v = tmpd.equals(tmp);
        REQUIRE(v);
    }
    TEST_CASE("4","")
    {
        Derived<NonStreamable> v(42);
        Derived<NonStreamable> v2(42);
        bool b = v.equals(v2);
        REQUIRE(b);
        std::ostringstream oss;
        v.print(oss);
        REQUIRE(oss.str() == "NonStreamable:42");

        af::any<af::interfaces<TestInterface2>, af::not_comparable> a1(4);
        af::any<af::interfaces<TestInterface2>, af::not_comparable> a2(7);
        a1.print(oss);
        //REQUIRE(a1 < a2);
        //REQUIRE(a1 == a2);

        af::any<af::interfaces<TestInterface2> > a3(4);
        af::any<af::interfaces<TestInterface2> > a4(7);
        af::any<af::interfaces<TestInterface2> > a5(7);
        a3.print(oss);
        REQUIRE(a3 < a4);
        //REQUIRE(a4 == a5);

        af::any<af::interfaces<TestInterface2>, af::less_than_comparable> a31(4);
        af::any<af::interfaces<TestInterface2>, af::less_than_comparable> a41(7);
        a31.print(oss);
        REQUIRE(a31 < a41);
        //REQUIRE(a31 == a41);

        af::any<af::interfaces<TestInterface2>, af::equality_comparable> a32(NonStreamable(7));
        af::any<af::interfaces<TestInterface2>, af::equality_comparable> a42(NonStreamable(7));
        a32.print(oss);
        //REQUIRE(a32 < a42);
        REQUIRE(a32 == a42);

        /*af::any<> a33(4);
        af::any<> a43(7);
        af::any<> a53(7);
        REQUIRE(a33 < a43);
        REQUIRE(a43 == a53);*/

        AnyTest<af::interfaces<TestInterface2>, af::equality_comparable> x(NonStreamable2(42));
        AnyTest<af::interfaces<TestInterface2>, af::equality_comparable> y(NonStreamable2(42));
        REQUIRE(x == y);
        //REQUIRE(x < y);
    }

}
