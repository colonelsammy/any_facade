//          Copyright Malcolm Noyes 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef ANY_FACADE_HPP_INCLUDED
#define ANY_FACADE_HPP_INCLUDED

//#include <member_function_traits.hpp>
//#include <vector>
//#include <iostream>
//#include <sstream>
//#include <cassert>
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
        size_t hash_code() const {return m_value;}
    };

    template <typename InterfaceClass>
    //static
    size_t type_info<InterfaceClass>::s_type = 0;

#endif // ANY_FACADE_USE_RTTI

    template <typename Base, typename T>
    class value_type_operations;

    template <typename T>
    class forwarder;

    template <int N>
    struct null_base { virtual ~null_base() {} };

    template <typename I0 = null_base<0>, typename I1 = null_base<1>, typename I2 = null_base<2>, typename I3 = null_base<3>, typename I4 = null_base<4>, typename I5 = null_base<5>, typename I6 = null_base<6> >
    class any : public forwarder<any<I0,I1,I2,I3,I4,I5,I6> >
    {
        friend class forwarder<any<I0,I1,I2,I3,I4,I5,I6> >;
    private:
        class placeholder;

        template <typename T>
        class operation_wrapper : public T
        {
        public:
            template<typename ValueType>
            operation_wrapper(const ValueType & value)
                : T(value)
            {}
            virtual placeholder * clone() const
            {
                return new operation_wrapper(*this);
            }
        private:
            operation_wrapper & operator=(const operation_wrapper &);
        };

    public: // structors

        any()
            : content(0)
        {
        }

        template<typename ValueType>
        any(const ValueType & value)
            : content(new operation_wrapper<value_type_operations<holder<ValueType>, ValueType> >(value))
        {
        }

        /*any(const any & other)
            : content(other.content ? other.content->clone() : 0)
        {
        }*/

        ~any()
        {
            delete content;
        }

    public: // modifiers

        /*any & swap(any & rhs)
        {
            std::swap(content, rhs.content);
            return *this;
        }

        any & operator=(any rhs)
        {
            any(rhs).swap(*this);
            return *this;
        }*/

        // interface forwarding
        /*template <typename Function>
        typename member_function_traits<Function>::result_type operator()(Function fn)
        {
            return (content->*fn)();
        }
        template <typename Function>
        typename member_function_traits<Function>::result_type operator()(Function fn,
            typename member_function_traits<Function>::arg1_type t1)
        {
            return (content->*fn)(t1);
        }
        template <typename Function>
        typename member_function_traits<Function>::result_type operator()(Function fn,
            typename member_function_traits<Function>::arg1_type t1,
            typename member_function_traits<Function>::arg2_type t2)
        {
            return (content->*fn)(t1, t2);
        }*/

    public: // queries

        bool empty() const
        {
            return !content;
        }

        /*bool equals(const any& rhs) const
        {
            if(content && rhs.content)
            {
                return content->equals(*rhs.content);
            }
            return false;
        }*/
    private: // types
        any(const any & other);
        any operator=(const any & other);

        class placeholder : public I0, public I1, public I2, public I3, public I4, public I5, public I6
        {
        public: // structors
            virtual ~placeholder() {}
        public: // queries

            virtual placeholder * clone() const = 0;
            virtual type_info<any> type() const  = 0;
            //virtual bool equals(const placeholder& other) const = 0;
        };

    public:
        template<typename ValueType>
        class holder : public placeholder
        {
        public: // structors

            holder(const ValueType & value)
                : held(value)
            {
            }

        public: // queries

            virtual type_info<any> type() const
            {
                return type_info<any>::template type_id<ValueType>();
            }
            /*virtual bool equals(const placeholder& other) const
            {
                if( type() == other.type() )
                {
                    // safe - types match
                    const holder<ValueType>* rhs = static_cast<const holder<ValueType>*>(&other);
                    return (held == rhs->held);
                }
                return false;
            }*/

        protected: // representation

            ValueType held;

        private: // intentionally left unimplemented
            holder & operator=(const holder &);
        };

    protected: // representation

        placeholder * content;
    };

    template <typename I0, typename I1, typename I2, typename I3, typename I4, typename I5, typename I6>
    bool operator==(const any<I0,I1,I2,I3,I4,I5,I6>& lhs, const any<I0,I1,I2,I3,I4,I5,I6>& rhs)
    {
        return lhs.equals(rhs);
    }

}

#endif // ANY_FACADE_HPP_INCLUDED
