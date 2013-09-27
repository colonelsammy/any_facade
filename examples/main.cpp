//          Copyright Malcolm Noyes 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

//
// You will *not* need boost to build this example.

#include "employee.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <cassert>

void moving_towards_any_facade_example();
void hoisting_out_an_interface_example();
void moving_to_generic_calling_mechanism_example();
void using_function_traits_example();
void split_interface_example();
void split_implementation_example();

int main()
{
    moving_towards_any_facade_example();
    hoisting_out_an_interface_example();
    moving_to_generic_calling_mechanism_example();
    using_function_traits_example();
    split_interface_example();
    split_implementation_example();
    return 0;
}
