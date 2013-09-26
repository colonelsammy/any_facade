Boost.any is a very clever way of turning heterogeneous types into homogeneous value types and when it first appeared it was a pioneering example of the use of type erasure.  It was written by Kevlin Henney about 13 years ago as part of the Boost library although the idea behind it appeared before that; the earliest similar published work I could find was an article on external polymorphism published in 1996.

When Kevlin first wrote it he foresaw that it could be adapted for other uses and boost::function, any_iterator and many others can trace their heritage back to it. This article describes another variation that I've called any_facade after the design pattern used to 

This is my article. This is a test [[1]](#Link1). 
And this is another [[2]](#Link2). 


3. There are a couple of problems with boost::any though; one to do with rtti and the other with the way we use it. To illustrate this I'll use the simple data classes shown on the left.... and you'll recall the example from the first slide our example of using employee and chairman, shown on the right; now the first problem is that any relies on rtti.

4. Embedded programmers will tell you that using rtti is always bad and cite space and speed issues, and sometimes they might be right, but here we're  specifically concerned with the typeid function, which booost::any uses to do its magic. Here we can see where the problem comes.  Although the chairman class is derived from employee, as far as the boost::any typeid comparision is concerned it's not the same thing as an employee.  So the first any_cast works for employee but fails for chairman.  As we expect, the second cast works for chairman and fails for employee, but since the first one failed we've failed to add the chairman's pay...

5. The second problem is one of evolution in our use of programming idioms. Some of us have moved towards 'Tell, don't Ask'; for example Steve Freeman and Nat Price's work; if you look at how we extract values from an any at the bottom you'll see the mother of all 'getters' dressed up as a cast! Now, there are quite a few people here who know Kevlin and they will know that he is one of those annoying people who are almost always right...

6. ...so obviously I'm not going to pass up the opportunity to point some inconsistency...

7. ... however the real point is that the problem with working by 'asking' is that it leads to simple errors.  Here we can see that our goal is to print a list of employees but our programmer has forgotten to deal with the chairman type (by now our chairman might have a complex...).  So what would we like to do?  

8. Well I would like to be able to tell any 'any' to print itself to an ostream, or tell to add itself to the payroll calculation;  that way each object gets to do its thing and we don't forget any 'anys'.  As a side effect, if we don't need any_cast then maybe we could do away with the need for rtti as well.  As we'll see, we have to compromise on some 'spelling' but we can get surprisingly close to that goal. To start, we have to see how any works....

9. (I'll keep our goal here on the left...) In principle any is very easy.  We have a simple 'placeholder'  interface that describes the desired operations and a pointer 'content' to manage it. Then we have a concrete implementation....  

10. ...that allows any type but hides the actual type.  A constructor on the outer class converts the supplied value type when an any is constructed, places the value in a holder and then the type is gone, except that...

11. ...we also have a friend 'any_cast' function that allows us to 'get' back the value after checking to see if the value is of the 'correct' type. We can manually adapt this mechanism for our "tell don't ask" class...

12. ...we can enhance the placeholder and the holder with the desired functions and add a forwarding function to the outer class.  With some minor adaptations to our original data classes, the original requirement can now be met, but this isn't very generic; we would have to alter all instances of our "tell don't ask" class whenever we wanted to support different operations. What we'd like is something that implements our 'print' operation...

13. ...so we can extract the interface, like this... This still leaves us with a problem though; our outer class still needs a forwarding function and that still leaves us with duplication every time we want a slightly different type of "tell don't ask". Sadly, C++ doesn't have reflection, so to fix this, we need to change the spelling of our call...

14. ...and our forwarding function. If we make the first parameter a function pointer template parameter, then we can pass any call to any named function automatically...as long as the signature of the function matches.  Notice that our 'any' doesn't itself derive from an interface; that's still hidden in the depths of the placeholder.  We can only do this because we've also effectively used the forwarding function to hide the function name; it *is* possible to allow derivation from the interface and then our original calling convention would work, but that requires some rather scary macros to implement the named functions. Of course, this only works if the function signature matches...

15. ...but in general it won't. It would be nice if we could do something about this.  Well, if you go poking around in the depths of Boost, you'll find in type_traits a header called function_traits.hpp.  In amongst the details, the interesting thing about this is that, given a function pointer, it can extract the function return type and parameter types using partial template specialization. It turns out that a little adaptation allows the same technique to be used for member function traits, which is exactly what we want.

16. Now we can re-write our forwarding function like this... this is a bit of a mouthful but it just extracts the types and provides a unified interface to whatever type of object is contained within the any...for that reason I've called it any_facade.  Now at least in C++03 we need a function for each distinct number of parameters that we want to support, but for the simple version that's it.  There are other things that can be done with the type of object that is used to construct the 'any' (if we wanted different types to do something different for example) and if we add in a simpler form of type info (based around the interface type) then we can also test for equality.

17. References - any_facade can be found on GitHub (the code used in this example and the more complicated versions).

"Rather than working with glorified C structs dressed as classes with assembleresque getters and setters, develop classes that have rich, intentional public interfaces."

The PfA Papers: Deglobalisation, Overload Journal #83 - Feb 2008 + Design of applications and programs   Author: Kevlin Henney (http://accu.org/index.php/journals/1470)

"This means that getters that return state are a liability - don't ask an object for information to work with. Instead, ask the object to do the work with the information it already has."

97 Things Every Programmer Should Know: Coding with Reason    Author: Yechiel Kimchi

"The boost::any class (based on the class of the same name described in "Valued Conversions" by Kevlin Henney, C++ Report 12(7), July/August 2000) is a variant value type based on [discriminated types that contain values of different types but do not attempt conversion between them]. It supports copying of any value type and safe checked extraction of that value strictly against its type. A similar design, offering more appropriate operators, can be used for a generalized function adaptor, any_function, a generalized iterator adaptor, any_iterator, and other object types that need uniform runtime treatment but support only compile-time template parameter conformance."

Chapter 2. Boost.Any    Kevlin Henney    Copyright © 2001 Kevlin Henney (http://www.boost.org/doc/libs/1_54_0/doc/html/any.html)

Valued Conversions   by Kevlin Henney, C++ Report 12(7), July/August 2000 (http://www.two-sdg.demon.co.uk/curbralan/papers/ValuedConversions.pdf)

Kevlin Henney. Substitutability. Principles, Idioms and Techniques for C++ (PDF). Presented at JaCC, Oxford, 16th September 1999. (http://www.two-sdg.demon.co.uk/curbralan/papers/accu/Substitutability.pdf#page=60)
Kevlin Henney. Idioms. Breaking the Language Barrier (PDF). Presented at the ACCU's C and C++ European Developers Forum, the Oxford Union, Oxford, UK, 12th September 1998. (http://www.two-sdg.demon.co.uk/curbralan/papers/accu/Idioms.pdf#page=32)


Thomas Becker On the Tension Between Object-Oriented and Generic Programming in C++ and What Type Erasure Can Do About It (http://www.artima.com/cppsource/type_erasure2.html)

Twisting the RTTI System for Safe Dynamic Casts of void* in C++    Cassio Neri   DrDobbs April 05, 2011

(http://www.drdobbs.com/cpp/twisting-the-rtti-system-for-safe-dynami/229401004)

External Polymorphism    Chris Cleeland, Douglas C. Schmidt and Timothy H. Harrison  Proceedings of the 3rd Pattern Languages of Programming Conference, Allerton Park, Illinois, September 4–6, 1996. (http://www.cs.wustl.edu/~schmidt/PDF/External-Polymorphism.pdf)


### Footnotes

1. <a name="Link1"></a>Google: [http://google.com](http://google.com)
2. <a name="Link2"></a>Yahoo: [http://yahoo.com](http://google.com)

--------------------------------------------------------------------------------
