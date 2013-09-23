//          Copyright Malcolm Noyes 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EMPLOYEE_BOOST_ANY_EXAMPLE_H
#define EMPLOYEE_BOOST_ANY_EXAMPLE_H

#include <string>

class employee
{
public:
    employee(const std::string& name, double salary)
        : m_name(name), m_salary(salary)
    {}
    std::string get_name() const
    {
        return m_name;
    }
    double get_pay() const {return m_salary;}
private:
    std::string m_name;
    double m_salary;
};

class chairman : public employee
{
public:
    chairman(const std::string& name, double salary, double bonus)
        : employee(name, salary), m_bonus(bonus)
    {}
    double get_bonus() const {return m_bonus;}
private:
    double m_bonus;
};

#endif
