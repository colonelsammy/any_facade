//          Copyright Malcolm Noyes 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EMPLOYEE_EXAMPLE_H
#define EMPLOYEE_EXAMPLE_H

#include <string>
#include <ostream>

class employee
{
    friend bool operator==(const employee& lhs, const employee& rhs);
public:
    employee(const std::string& name, double salary)
        : m_name(name), m_salary(salary)
    {}
    void print(std::ostream& os) const
    {
        os << m_name;
    }
    virtual double accumulate_pay(int month) const
    {
        return m_salary;
    }
protected:
    std::string m_name;
    double m_salary;
};

inline bool operator==(const employee& lhs, const employee& rhs)
{
    return (lhs.m_name == rhs.m_name);
}

class chairman : public employee
{
public:
    chairman(const std::string& name, double salary, double bonus)
        : employee(name, salary), m_bonus(bonus)
    {}
    virtual double accumulate_pay(int month) const
    {
        if( month == 11 )
        {
            return m_salary + m_bonus;
        }
        return employee::accumulate_pay(month);
    }
private:
    double m_bonus;
};

inline std::ostream& operator<<(std::ostream& os, const employee& v)
{
    v.print(os);
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const chairman& v)
{
    v.print(os);
    return os;
}

#endif
