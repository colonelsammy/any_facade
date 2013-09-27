//          Copyright Malcolm Noyes 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "employee.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <cassert>

// This example shows how to manually adapt boost::any to allow 
// (some) of the operations to be implemented with a "tell don't ask" idiom.
// In this example we will extract the interface from the placeholder.
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
    struct printable
    {
      virtual ~printable() {}
      virtual void print(std::ostream& os) = 0;
    };

    struct accumable : public printable
    {
      virtual double accumulate_pay(double current, int month) = 0;
    };

    template <typename Interface>
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

        // interface
        void print(std::ostream& os)
        {
            content->print(os);
        }
        double accumulate_pay(double current, int month)
        {
            return content->accumulate_pay(current, month);
        }

    public: // queries

        bool empty() const
        {
            return !content;
        }

    private: // types

        class placeholder : public Interface
        {
        public: // structors
            virtual ~placeholder() {}
        public: // queries

            //virtual const std::type_info & type() const = 0;

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

            /*virtual const std::type_info & type() const
            {
                return typeid(ValueType);
            }*/

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

        /*template<typename ValueType>
        friend ValueType * any_cast(any *);*/

        placeholder * content;
    };
}

/*template<typename ValueType>
ValueType * any_cast(any * operand)
{
    return operand && 
        operand->type() == typeid(ValueType)
        ? &static_cast<any::holder<ValueType> *>(operand->content)->held
        : 0;
}*/

namespace
{

typedef any_tell_dont_ask<accumable> Any;
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
            a.print(m_os);
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
            total = a.accumulate_pay(total, month);
        }
        return total;
    }
};

}

void hoisting_out_an_interface_example()
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
}
