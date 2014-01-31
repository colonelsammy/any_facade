//          Copyright Malcolm Noyes 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef ANY_FACADE_HPP_INCLUDED
#define ANY_FACADE_HPP_INCLUDED

#include <member_function_traits.hpp>
#ifdef ANY_FACADE_USE_RTTI
#include <typeinfo>
#endif
#if __cplusplus > 199711L
#include <type_traits>
#endif

namespace any_facade
{
#if __cplusplus > 199711L
    typedef std::remove_reference remove_reference;
    typedef std::remove_const remove_const;
#else
    template< class T > struct remove_reference      {typedef T type;};
    template< class T > struct remove_reference<T&>  {typedef T type;};
#ifdef _MSC_VER
    template< class T > struct remove_reference<T&&> {typedef T type;}; 
#endif
    template< class T > struct remove_const          {typedef T type;};
    template< class T > struct remove_const<const T>  {typedef T type;};
    template< class T > struct remove_const<const T*>  {typedef T* type;};
#endif

#ifdef ANY_FACADE_USE_RTTI
    template <typename InterfaceClass>
    class type_info
    {
    private:
        explicit type_info(const std::type_info& v)
            : m_value(v)
        {}
        type_info& operator=(const type_info&);
        type_info();

        const std::type_info& m_value;

    public:
        type_info(const type_info&)
            : m_value(rhs.m_value)
        {}

        template <typename T>
        static type_info type_id()
        {
            return type_info(typeid(typename remove_const<typename remove_reference<T>::type>::type));
        }
        bool operator == (const type_info& rhs)
        {
            return m_value.operator==(rhs.m_value);
        }
        bool operator != (const type_info& rhs)
        {
            return !operator==(rhs);
        }
        bool operator < (const type_info& rhs)
        {
            return (m_value.hash_code() < rhs.hash_code());
        }
        size_t hash_code() const {return m_value.hash_code();}
    };

#else // ANY_FACADE_USE_RTTI

    template <typename InterfaceClass>
    class type_info
    {
    private:
        explicit type_info(size_t v)
            : m_value(v)
        {}

        size_t m_value;

        static size_t s_type;

        template <typename T>
        static type_info base_type_id()
        {
            static bool init(false);
            static size_t result;
            if( !init )
            {
                result = ++s_type;
                init = true;
            }
            return type_info(result);
        }
    public:
        template <typename T>
        static type_info type_id()
        {
            return base_type_id<typename remove_const<typename remove_reference<T>::type>::type>();
        }
        bool operator == (const type_info& rhs)
        {
            return (m_value == rhs.m_value);
        }
        bool operator != (const type_info& rhs)
        {
            return !operator==(rhs);
        }
        bool operator < (const type_info& rhs)
        {
            return (m_value < rhs.m_value);
        }
        size_t hash_code() const {return m_value;}
    };

    template <typename InterfaceClass>
    //static
    size_t type_info<InterfaceClass>::s_type = 0;

#endif // ANY_FACADE_USE_RTTI

    //
    // Comparability classes...specialize for specific types as required
    //
    struct not_comparable
    {
        template <typename T>
        struct compare
        {
            virtual ~compare() {}
        };
        template <typename Derived, typename T>
        struct compare2 : public T
        {
        };
    };
    //
    // If your class supports equaility, but doesn't support operator==(), specialize this class...
    //
    struct equality_comparable
    {
        template <typename T>
        struct compare
        {
            virtual ~compare() {}
            virtual bool equals(const T& other) const = 0;
        };
        template <typename Derived, typename T>
        struct compare2 : public T
        {
            virtual bool equals(const T& other) const
            {
                const Derived* otherType = static_cast<const Derived*>(&other);
                return equality_comparable::equals(static_cast<const Derived*>(this)->value(), otherType->value());
            }
        };
        template <typename T>
        static bool equals(const T& lhs, const T& rhs)
        {
            return (lhs == rhs);
        }
    };
    //
    // If your class supports less, but doesn't support operator<(), specialize this class...
    //
    struct less_than_comparable
    {
        template <typename T>
        struct compare
        {
            virtual ~compare() {}
            virtual bool less(const T& other) const = 0;
        };
        template <typename Derived, typename T>
        struct compare2 : public T
        {
            virtual bool less(const T& other) const
            {
                const Derived* otherType = static_cast<const Derived*>(&other);
                return less_than_comparable::less(static_cast<const Derived*>(this)->value(), otherType->value());
            }
        };
        template <typename T>
        static bool less(const T& lhs, const T& rhs)
        {
            return (lhs < rhs);
        }
    };
    //
    // comparability for equals and less...
    //
    struct less_than_equals_comparable : public equality_comparable, public less_than_comparable
    {
        template <typename T>
        struct compare : public equality_comparable::compare<T>, public less_than_comparable::compare<T>
        {
            //virtual ~compare() {}
            //virtual bool equals(const T& other) const = 0;
            //virtual bool less(const T& other) const = 0;
        };
        template <typename Derived, typename T>
        struct compare2 : public T
        //struct compare2 : public equality_comparable::compare2<Derived, T>, public less_than_comparable::compare2<Derived, T>
        {
            virtual bool equals(const T& other) const
            {
                const Derived* otherType = static_cast<const Derived*>(&other);
                return equality_comparable::equals(static_cast<const Derived*>(this)->value(), otherType->value());
            }
            virtual bool less(const T& other) const
            {
                const Derived* otherType = static_cast<const Derived*>(&other);
                return less_than_comparable::less(static_cast<const Derived*>(this)->value(), otherType->value());
            }
        };
    };

    template <typename Derived, typename Base, typename ValueType>
    class value_type_operations;

    template <typename Derived>
    class forwarder;

    template <int N>
    struct null_base { virtual ~null_base() {} };

    template <typename I0 = null_base<0>,
                typename I1 = null_base<1>,
                typename I2 = null_base<2>,
                typename I3 = null_base<3>,
                typename I4 = null_base<4>,
                typename I5 = null_base<5>,
                typename I6 = null_base<6> >
    struct interfaces : public I0, public I1, public I2, public I3, public I4, public I5, public I6
    {};

    //template <typename Interface, typename Comparable>
    //class placeholder2;

    template <typename Interface, typename Comparable>
    class any;
        template <typename Interface, typename Comparable>
    class placeholder;

    template <typename Interface, typename Comparable>
    class placeholder : public Interface, public Comparable::template compare<placeholder<Interface,Comparable> >
    {
    public: // structors
        virtual ~placeholder() {}
    public: // queries
        virtual type_info<any<Interface, Comparable> > type() const  = 0;
        virtual placeholder<Interface, Comparable> * clone() const = 0;
    };

    template <typename Derived, typename Interface, typename Comparable>
    class less_than_equals : public placeholder<Interface, Comparable>
    {
    };

    template <typename Derived, typename Interface>
    class less_than_equals<Derived, Interface, less_than_equals_comparable> : public less_than_equals_comparable::compare2<Derived, placeholder<Interface, less_than_equals_comparable> >
    {
    public:
        /*virtual bool equals(const placeholder<Interface, less_than_equals_comparable>& other) const
        {
            const Derived* otherType = static_cast<const Derived*>(&other);
            return less_than_equals_comparable::equals(static_cast<const Derived*>(this)->held, otherType->held);
        }
        virtual bool less(const placeholder<Interface, less_than_equals_comparable>& other) const
        {
            const Derived* otherType = static_cast<const Derived*>(&other);
            return less_than_equals_comparable::less(static_cast<const Derived*>(this)->held, otherType->held);
        }*/
    };

    template <typename Derived, typename Interface>
    class less_than_equals<Derived, Interface, less_than_comparable> : public less_than_comparable::compare2<Derived, placeholder<Interface, less_than_comparable> >
    {
    public:
        /*virtual bool less(const placeholder<Interface, less_than_comparable>& other) const
        {
            const Derived* otherType = static_cast<const Derived*>(&other);
            return less_than_comparable::less(static_cast<const Derived*>(this)->held, otherType->held);
        }*/
    };

    template <typename Derived, typename Interface>
    class less_than_equals<Derived, Interface, equality_comparable> : public equality_comparable::compare2<Derived, placeholder<Interface, equality_comparable> >
    {
    public:
        /*virtual bool equals(const placeholder<Interface, equality_comparable>& other) const
        {
            const Derived* otherType = static_cast<const Derived*>(&other);
            return equality_comparable::equals(static_cast<const Derived*>(this)->held, otherType->held);
        }*/
    };

    template <typename Interface = interfaces<>, typename Comparable = less_than_equals_comparable>
    class any : public forwarder<Interface>
    {
        // CRTP base class has access to 'content'
        friend class forwarder<Interface>;
    public:
    private:

        //
        // This has to be the most derived class so that 'clone' doesn't slice,
        // so we use CRTP to enforce this condition
        //
        template<typename T>
        class holder : public value_type_operations<holder<T>, less_than_equals<holder<T>, Interface, Comparable>, T>
        //class holder : public value_type_operations<holder<T>, typename Comparable::compare2<holder<T>, placeholder<Interface,Comparable> >, T>
        {
            // CRTP base class has access to 'held'
            friend class value_type_operations<holder<T>, less_than_equals<holder<T>, Interface, Comparable>, T>;
            friend class less_than_equals<holder<T>, Interface, Comparable>;
            //friend class Comparable::compare2<holder<T>, Interface>;
        public: // structors
            typedef T ValueType;

            explicit holder(const ValueType & v)
                : held(v)
            {
            }

            ValueType value() const { return held; }

        public: // queries

            virtual type_info<any> type() const
            {
                return type_info<any>::template type_id<ValueType>();
            }
            virtual placeholder<Interface, Comparable> * clone() const
            {
                return new holder(*this);
            }
            /*virtual bool equals(const placeholder& other) const
            {
                // safe - types match
                const holder<ValueType>* rhs = static_cast<const holder<ValueType>*>(&other);
                return (held == rhs->held);
            }
            virtual bool less(const placeholder& other) const
            {
                // safe - types match
                const holder<ValueType>* rhs = static_cast<const holder<ValueType>*>(&other);
                return (held < rhs->held);
            }*/

        private: // intentionally left unimplemented
            holder & operator=(const holder &);

        private: // representation

            ValueType held;
        };

    public: // structors

        any()
            : content(0)
        {
        }

        template<typename ValueType>
        any(const ValueType & value)
            : content(new holder<ValueType>(value))
        {
        }

        any(const any & other)
            : content(other.content ? other.content->clone() : 0)
        {
        }

        ~any()
        {
            delete content;
        }

    public: // modifiers

        any & swap(any & rhs)
        {
            std::swap(content, rhs.content);
            return *this;
        }

        any & operator=(any rhs)
        {
            any(rhs).swap(*this);
            return *this;
        }

        // interface forwarding, allow up to 10 params
        template <typename Function>
        typename member_function_traits<Function>::result_type call(Function fn)
        {
            return (content->*fn)();
        }

        template <typename Function>
        typename member_function_traits<Function>::result_type call(
            Function fn,
            typename member_function_traits<Function>::arg1_type t1)
        {
            return (content->*fn)(t1);
        }

        template <typename Function>
        typename member_function_traits<Function>::result_type call(
            Function fn,
            typename member_function_traits<Function>::arg1_type t1,
            typename member_function_traits<Function>::arg2_type t2)
        {
            return (content->*fn)(t1, t2);
        }

        template <typename Function>
        typename member_function_traits<Function>::result_type call(
            Function fn,
            typename member_function_traits<Function>::arg1_type t1,
            typename member_function_traits<Function>::arg2_type t2,
            typename member_function_traits<Function>::arg3_type t3)
        {
            return (content->*fn)(t1, t2, t3);
        }

        template <typename Function>
        typename member_function_traits<Function>::result_type call(
            Function fn,
            typename member_function_traits<Function>::arg1_type t1,
            typename member_function_traits<Function>::arg2_type t2,
            typename member_function_traits<Function>::arg3_type t3,
            typename member_function_traits<Function>::arg4_type t4)
        {
            return (content->*fn)(t1, t2, t3, t4);
        }

        template <typename Function>
        typename member_function_traits<Function>::result_type call(
            Function fn,
            typename member_function_traits<Function>::arg1_type t1,
            typename member_function_traits<Function>::arg2_type t2,
            typename member_function_traits<Function>::arg3_type t3,
            typename member_function_traits<Function>::arg4_type t4,
            typename member_function_traits<Function>::arg5_type t5)
        {
            return (content->*fn)(t1, t2, t3, t4, t5);
        }

        template <typename Function>
        typename member_function_traits<Function>::result_type call(
            Function fn,
            typename member_function_traits<Function>::arg1_type t1,
            typename member_function_traits<Function>::arg2_type t2,
            typename member_function_traits<Function>::arg3_type t3,
            typename member_function_traits<Function>::arg4_type t4,
            typename member_function_traits<Function>::arg5_type t5,
            typename member_function_traits<Function>::arg6_type t6)
        {
            return (content->*fn)(t1, t2, t3, t4, t5, t6);
        }

        template <typename Function>
        typename member_function_traits<Function>::result_type call(
            Function fn,
            typename member_function_traits<Function>::arg1_type t1,
            typename member_function_traits<Function>::arg2_type t2,
            typename member_function_traits<Function>::arg3_type t3,
            typename member_function_traits<Function>::arg4_type t4,
            typename member_function_traits<Function>::arg5_type t5,
            typename member_function_traits<Function>::arg6_type t6,
            typename member_function_traits<Function>::arg7_type t7)
        {
            return (content->*fn)(t1, t2, t3, t4, t5, t6, t7);
        }

        template <typename Function>
        typename member_function_traits<Function>::result_type call(
            Function fn,
            typename member_function_traits<Function>::arg1_type t1,
            typename member_function_traits<Function>::arg2_type t2,
            typename member_function_traits<Function>::arg3_type t3,
            typename member_function_traits<Function>::arg4_type t4,
            typename member_function_traits<Function>::arg5_type t5,
            typename member_function_traits<Function>::arg6_type t6,
            typename member_function_traits<Function>::arg7_type t7,
            typename member_function_traits<Function>::arg8_type t8)
        {
            return (content->*fn)(t1, t2, t3, t4, t5, t6, t7, t8);
        }

        template <typename Function>
        typename member_function_traits<Function>::result_type call(
            Function fn,
            typename member_function_traits<Function>::arg1_type t1,
            typename member_function_traits<Function>::arg2_type t2,
            typename member_function_traits<Function>::arg3_type t3,
            typename member_function_traits<Function>::arg4_type t4,
            typename member_function_traits<Function>::arg5_type t5,
            typename member_function_traits<Function>::arg6_type t6,
            typename member_function_traits<Function>::arg7_type t7,
            typename member_function_traits<Function>::arg8_type t8,
            typename member_function_traits<Function>::arg9_type t9)
        {
            return (content->*fn)(t1, t2, t3, t4, t5, t6, t7, t8, t9);
        }

        template <typename Function>
        typename member_function_traits<Function>::result_type call(
            Function fn,
            typename member_function_traits<Function>::arg1_type t1,
            typename member_function_traits<Function>::arg2_type t2,
            typename member_function_traits<Function>::arg3_type t3,
            typename member_function_traits<Function>::arg4_type t4,
            typename member_function_traits<Function>::arg5_type t5,
            typename member_function_traits<Function>::arg6_type t6,
            typename member_function_traits<Function>::arg7_type t7,
            typename member_function_traits<Function>::arg8_type t8,
            typename member_function_traits<Function>::arg9_type t9,
            typename member_function_traits<Function>::arg10_type t10)
        {
            return (content->*fn)(t1, t2, t3, t4, t5, t6, t7, t8, t9, t10);
        }

    public: // queries

        bool empty() const
        {
            return !content;
        }

    public: // comparisons
        // equality
        friend bool operator==(const any& lhs, const any& rhs)
        {
            if( lhs.content->type() == rhs.content->type() )
            {
                // objects of the same type...value type comparison
                return lhs.content->equals(*rhs.content);
            }
            return false;
        }
        friend bool operator!=(const any& lhs, const any& rhs) {return !static_cast<bool>(lhs == rhs);}

        // less than comparable
        friend bool operator<(const any& lhs, const any& rhs)
        {
            if( lhs.content->type() == rhs.content->type() )
            {
                // objects of the same type...value type comparison
                return lhs.content->less(*rhs.content);
            }
            if( lhs.content->type() < rhs.content->type() ) return true;
            else return false;
        }
        friend bool operator>(const any& lhs, const any& rhs)  { return rhs < lhs; }
        friend bool operator<=(const any& lhs, const any& rhs) { return !static_cast<bool>(rhs < lhs); }
        friend bool operator>=(const any& lhs, const any& rhs) { return !static_cast<bool>(lhs < rhs); }

    private: // types

    private: // representation

        placeholder<Interface, Comparable> * content;
    };

}

#endif // ANY_FACADE_HPP_INCLUDED
