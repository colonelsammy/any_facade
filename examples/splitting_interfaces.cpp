//          Copyright Malcolm Noyes 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "employee.h"
#include <member_function_traits.hpp>
#include <vector>
#include <iostream>
#include <sstream>
#include <cassert>

// This example shows how to manually adapt boost::any to allow 
// (some) of the operations to be implemented with a "tell don't ask" idiom.
// In this third example we will allow a more generic function call.
//
// You will *not* need boost to build this example.

/*
What we'd like to do...

std::ostringstream os;
for( AnyIterator iter = database.begin(); iter != database.end(); ++iter )
{
  boost::any& a = *iter;
  a.print(os);
}

double total = 0.0;
for( AnyIterator iter = database.begin(); iter != database.end(); ++iter )
{
  boost::any& a = *iter;
  total += a.accumlate(total, month);
}
*/
namespace
{
    template <typename I1, typename I2>
    struct wrap;

    template <>
    struct wrap<void, void>
    {};

    template <>
    struct wrap<void, wrap<void,void> >
    {};

    template <>
    struct wrap< void, wrap<void, wrap<void,void> > >
    {};

    template <>
    struct wrap< void, wrap< void, wrap<void, wrap<void,void> > > >
    {};

    template <>
    struct wrap< void, wrap< void, wrap< void, wrap<void, wrap<void,void> > > > >
    {};

    template <typename I1>
    struct wrap<I1, void> : I1
    {};
    
    template <typename I1, typename I2>
    struct wrap : I1, I2
    {
    };

    template <typename I1, typename I2 = void, typename I3 = void, typename I4 = void, typename I5 = void, typename I6 = void>
    class any_tell_dont_ask
    {
    public: // structors

        any_tell_dont_ask()
            : content(0)
        {
        }

        template<typename ValueType>
        any_tell_dont_ask(const ValueType & value)
            : content(new holder<ValueType>(value))
        {
        }

        any_tell_dont_ask(const any_tell_dont_ask & other)
            : content(other.content ? other.content->clone() : 0)
        {
        }

        ~any_tell_dont_ask()
        {
            delete content;
        }

    public: // modifiers

        any_tell_dont_ask & swap(any_tell_dont_ask & rhs)
        {
            std::swap(content, rhs.content);
            return *this;
        }

        any_tell_dont_ask & operator=(any_tell_dont_ask rhs)
        {
            any_tell_dont_ask(rhs).swap(*this);
            return *this;
        }

        // interface forwarding
        template <typename Function>
        typename any_facade::member_function_traits<Function>::result_type operator()(Function fn)
        {
            return (content->*fn)();
        }
        template <typename Function>
        typename any_facade::member_function_traits<Function>::result_type operator()(Function fn,
            typename any_facade::member_function_traits<Function>::arg1_type t1)
        {
            return (content->*fn)(t1);
        }
        template <typename Function>
        typename any_facade::member_function_traits<Function>::result_type operator()(Function fn,
            typename any_facade::member_function_traits<Function>::arg1_type t1,
            typename any_facade::member_function_traits<Function>::arg2_type t2)
        {
            return (content->*fn)(t1, t2);
        }

    public: // queries

        bool empty() const
        {
            return !content;
        }

    private: // types

        class placeholder : public wrap<I1, wrap<I2, wrap<I3, wrap<I4, wrap<I5, wrap<I6, void> > > > > >
        {
        public: // structors
            virtual ~placeholder() {}
        public: // queries

            virtual placeholder * clone() const = 0;

        };

        template<typename ValueType>
        class holder : public placeholder
        {
        public: // structors

            holder(const ValueType & value)
                : held(value)
            {
            }

        public: // queries

            virtual placeholder * clone() const
            {
                return new holder(held);
            }
            virtual void print(std::ostream& os)
            {
                os << held;
            }
            virtual double accumulate_pay(double current, int month)
            {
                return current + held.accumulate_pay(month);
            }

        public: // representation

            ValueType held;

        private: // intentionally left unimplemented
            holder & operator=(const holder &);
        };

    private: // representation

        placeholder * content;
    };
}

namespace
{

    struct printable
    {
      virtual ~printable() {}
      virtual void print(std::ostream& os) = 0;
    };

    struct accumable
    {
      virtual ~accumable() {}
      virtual double accumulate_pay(double current, int month) = 0;
    };

    struct T1
    {
        virtual ~T1() {}
        void t(std::ostream& os) {os << "t1,";}
    };
    struct T2
    {
        virtual ~T2() {}
        void t(std::ostream& os) {os << "t2,";}
    };
    struct T3
    {
        virtual ~T3() {}
        void t(std::ostream& os) {os << "t3,";}
    };
    struct T4
    {
        virtual ~T4() {}
        void t(std::ostream& os) {os << "t4,";}
    };
    struct T5
    {
        virtual ~T5() {}
        void t(std::ostream& os) {os << "t5,";}
    };
    struct T6
    {
        virtual ~T6() {}
        void t(std::ostream& os) {os << "t6,";}
    };

typedef any_tell_dont_ask<printable, accumable> Any;
typedef std::vector<Any>::iterator AnyIterator;

class list_employees
{
public:
    list_employees(std::ostream& os)
        : m_os(os)
    {}
    void print(std::vector<Any>& database)
    {
        for( AnyIterator iter = database.begin(); iter != database.end(); ++iter )
        {
            Any& a = *iter;
            a(&printable::print, m_os);
            m_os << " ";
        }
    }
private:
    std::ostream& m_os;
};

class payroll
{
public:
    double calculate_total_pay(std::vector<Any>& database, int month)
    {
        double total = 0.0;
        for( AnyIterator iter = database.begin(); iter != database.end(); ++iter )
        {
            Any& a = *iter;
            total = a(&accumable::accumulate_pay, total, month);
        }
        return total;
    }
};

}

void split_interface_example()
{
    std::vector<Any> database;
    database.push_back(employee("Fred", 100.0));
    database.push_back(chairman("Bert", 200.0, 1000.0));

    {
        payroll pr;
        double result = pr.calculate_total_pay(database, 11);
        std::cout << "Payroll (should be 1300.0): " << result << std::endl;
        // works now
        assert(result == 1300.0);
    }
    {
        std::ostringstream oss;
        list_employees li(oss);
        li.print(database);
        std::string result = oss.str();
        assert(result.find("Fred") != std::string::npos);
        std::cout << "Print (should have 'Fred' & 'Bert'): " << std::endl << result << std::endl;
        // works now
        assert(result.find("Bert") != std::string::npos);
    }
    {
        typedef any_tell_dont_ask<printable> Any2;
        typedef std::vector<Any2>::iterator Any2Iterator;

        std::vector<Any2> database;
        database.push_back(employee("Fred", 100.0));
        database.push_back(chairman("Bert", 200.0, 1000.0));

        std::ostringstream oss;
        for( Any2Iterator iter = database.begin(); iter != database.end(); ++iter )
        {
            Any2& a = *iter;
            a(&printable::print, oss);
            oss << " ";
        }
        std::string result = oss.str();
        assert(result.find("Fred") != std::string::npos);
        std::cout << "Print (should have 'Fred' & 'Bert'): " << std::endl << result << std::endl;
    }
    {
        std::ostringstream oss;
        any_tell_dont_ask<T1> a;
        a(&T1::t, oss);
        std::string result = oss.str();
        assert(result.find("t1") != std::string::npos);
        std::cout << result << std::endl;
    }
    {
        std::ostringstream oss;
        any_tell_dont_ask<T1, T2> a;
        a(&T1::t, oss);
        a(&T2::t, oss);
        std::string result = oss.str();
        assert(result.find("t1") != std::string::npos);
        assert(result.find("t2") != std::string::npos);
        std::cout << result << std::endl;
    }
    {
        std::ostringstream oss;
        any_tell_dont_ask<T1, T2, T3> a;
        a(&T1::t, oss);
        a(&T2::t, oss);
        a(&T3::t, oss);
        std::string result = oss.str();
        assert(result.find("t1") != std::string::npos);
        assert(result.find("t2") != std::string::npos);
        assert(result.find("t3") != std::string::npos);
        std::cout << result << std::endl;
    }
    {
        std::ostringstream oss;
        any_tell_dont_ask<T1, T2, T3, T4> a;
        a(&T1::t, oss);
        a(&T2::t, oss);
        a(&T3::t, oss);
        a(&T4::t, oss);
        std::string result = oss.str();
        assert(result.find("t1") != std::string::npos);
        assert(result.find("t2") != std::string::npos);
        assert(result.find("t3") != std::string::npos);
        assert(result.find("t4") != std::string::npos);
        std::cout << result << std::endl;
    }
    {
        std::ostringstream oss;
        any_tell_dont_ask<T1, T2, T3, T4, T5> a;
        a(&T1::t, oss);
        a(&T2::t, oss);
        a(&T3::t, oss);
        a(&T4::t, oss);
        a(&T5::t, oss);
        std::string result = oss.str();
        assert(result.find("t1") != std::string::npos);
        assert(result.find("t2") != std::string::npos);
        assert(result.find("t3") != std::string::npos);
        assert(result.find("t4") != std::string::npos);
        assert(result.find("t5") != std::string::npos);
        std::cout << result << std::endl;
    }
    {
        std::ostringstream oss;
        any_tell_dont_ask<T1, T2, T3, T4, T5, T6> a;
        a(&T1::t, oss);
        a(&T2::t, oss);
        a(&T3::t, oss);
        a(&T4::t, oss);
        a(&T5::t, oss);
        a(&T6::t, oss);
        std::string result = oss.str();
        assert(result.find("t1") != std::string::npos);
        assert(result.find("t2") != std::string::npos);
        assert(result.find("t3") != std::string::npos);
        assert(result.find("t4") != std::string::npos);
        assert(result.find("t5") != std::string::npos);
        assert(result.find("t6") != std::string::npos);
        std::cout << result << std::endl;
    }
}
