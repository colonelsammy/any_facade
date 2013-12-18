//          Copyright Malcolm Noyes 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

//
// This example demonstrates the use of boost::any.
//
// You will need boost to build this example.  You will probably need to change
// the location (by default C:\Malcolm\downloads\Boost\boost_1_54_0\boost_1_54_0 in
// Visual Studio and /c/Malcolm/downloads/Boost/boost_1_54_0/boost_1_54_0 for makefile).
//

#include "employee.h"
#include <boost/any.hpp>
#include <vector>
#include <iostream>
#include <sstream>
#include <cassert>

class list_employees
{
public:
    list_employees(std::ostream& os)
        : m_os(os)
    {}
    void print(std::vector<boost::any>& database)
    {
        for( std::vector<boost::any>::iterator iter = database.begin(); iter != database.end(); ++iter )
        {
            boost::any& a = *iter;
            employee * emp = boost::any_cast<employee>(&a);
            if(emp != NULL )
            {
                m_os << emp->get_name() << ", " << emp->get_pay() << std::endl;
            }
            // Ooops!, we forgot the chairman...
        }
    }
private:
    std::ostream& m_os;
};

class payroll
{
public:
    double calculate_total_pay(std::vector<boost::any>& database, int month)
    {
        double total = 0.0;
        for( std::vector<boost::any>::iterator iter = database.begin(); iter != database.end(); ++iter )
        {
            boost::any& a = *iter;
            // works for 'Fred', fails for 'Bert'
            employee * emp = boost::any_cast<employee>(&a);
            if(emp != NULL )
            {
                total += emp->get_pay();
            }
            // works for 'Bert', as expected
            chairman * chair = boost::any_cast<chairman>(&a);
            if(chair != NULL && month == 11)
            {
                total += chair->get_bonus();
            }
        }
        return total;
    }
};

int main()
{
    //boost::any a("42");

    std::vector<boost::any> database;
    database.push_back(employee("Fred", 100.0));
    database.push_back(chairman("Bert", 200.0, 1000.0));

    {
        payroll pr;
        double result = pr.calculate_total_pay(database, 11);
        std::cout << "Payroll (should be 1300.0): " << result << std::endl;
        // fails
        //assert(result == 1300.0);
    }
    {
        std::ostringstream oss;
        list_employees li(oss);
        li.print(database);
        std::string result = oss.str();
        assert(result.find("Fred") != std::string::npos);
        std::cout << "Print (should have 'Fred' & 'Bert'): " << std::endl << result << std::endl;
        // fails
        //assert(result.find("Bert") != std::string::npos);
    }
    return 0;
}
