#include "catch.hpp"
#include "any_facade.hpp"
#include <map>
#include <vector>
#include <typeinfo>
#include <numeric>  // accumulate

namespace af = any_facade;

namespace
{
    // Interfaces we want to implement
    struct Key
    {
        virtual ~Key() {}
        // nothing else...
    };

    struct Geometry
    {
        virtual ~Geometry() {}
        virtual int area() const = 0;
        virtual void move(int x, int y) = 0;
    };

    struct Printable
    {
        virtual ~Printable() {}
        virtual void print(std::ostream& os) const = 0;
    };

    class SimpleKey
    {
    public:
        SimpleKey(int v)
            : m_id(v)
        {}
        int m_id;
        friend bool operator<(const SimpleKey& lhs, const SimpleKey& rhs)
        {
            return (lhs.m_id < rhs.m_id);
        }
    };

    struct TroublesomeKey
    {
        TroublesomeKey(int v)
            : m_key(v)
        {}
        int key() const {return m_key;}
    private:
        int m_key;
        // no < op
    };

    struct SimpleRectangle
    {
        SimpleRectangle(int x, int y, int width, int height) : m_x(x), m_y(y), m_width(width), m_height(height) {}
        int m_x;
        int m_y;
        int m_width;
        int m_height;

        int area() const {return m_width * m_height;}
        void move(int x, int y) {m_x = x; m_y = y;}
        friend bool operator==(const SimpleRectangle& lhs, const SimpleRectangle& rhs)
        {
            return (
                lhs.m_x == rhs.m_x &&
                lhs.m_y == rhs.m_y &&
                lhs.area() == rhs.area() );
        }
        friend std::ostream& operator<<(std::ostream& oss, const SimpleRectangle&v)
        {
            oss << "Area:" << v.area() << ", x:" << v.m_x << ", y:" << v.m_y;
            return oss;
        }
    };

    struct TroublesomeEllipse
    {
        TroublesomeEllipse(int x, int y, int width, int height) : m_x(x), m_y(y), m_width(width), m_height(height) {}
        int m_x;
        int m_y;
        int m_width;
        int m_height;

        int calcArea() const {return static_cast<int>(3.142159 / 4 * m_width * m_height);}
        void moveTo(int x, int y) {m_x = x; m_y = y;}
        void stream(std::ostream& os) const
        {
            os << "Area:" << calcArea() << ", x:" << m_x << ", y:" << m_y;
        }
    };

}

namespace any_facade
{
    template<>
    bool less_than_comparable::less<TroublesomeKey>(const TroublesomeKey& lhs, const TroublesomeKey& rhs)
    {
            return (lhs.key() < rhs.key());
    }

    template<>
    bool equality_comparable::equals<TroublesomeEllipse>(const TroublesomeEllipse& lhs, const TroublesomeEllipse& rhs)
    {
            return (
                lhs.m_x == rhs.m_x &&
                lhs.m_y == rhs.m_y &&
                lhs.calcArea() == rhs.calcArea() );
    }

    template <>
    class forwarder<any<interfaces<>, less_than_comparable> >
    {
    public:
    };

    //
    // Method implemented... deriving from the interface is optional but
    // ensures that changes to the interface will get caught by the compiler
    //
    template <>
    class forwarder<any<interfaces<Geometry, Printable>, equality_comparable> > : public Geometry, Printable
    {
    public:
        int area() const
        {
            return static_cast<const any<interfaces<Geometry, Printable>, equality_comparable>*>(this)->content->area();
        }
        void move(int x, int y)
        {
            static_cast<any<interfaces<Geometry, Printable>, equality_comparable>*>(this)->content->move(x, y);
        }
        void print(std::ostream& os) const
        {
            static_cast<const any<interfaces<Geometry, Printable>, equality_comparable>*>(this)->content->print(os);
        }
    };

    template <typename Derived, typename Base>
    class value_type_operations<Derived, Base, SimpleKey> : public Base
    {
    public:
        typedef typename Base::PlaceholderType PlaceholderType;
        virtual bool less(const PlaceholderType& other) const
        {
            if( other.type() == type_info<typename Derived::AnyType>::template type_id<TroublesomeKey>())
            {
                typedef typename Derived::AnyType::template holder<TroublesomeKey> OtherDerived;
                // objects of known type...comparison with members
                const OtherDerived* otherType = static_cast<const OtherDerived*>(&other);
                return (static_cast<const Derived*>(this)->value().m_id < otherType->value().key());
            }
            return Base::less(other);
        }
    };
    template <typename Derived, typename Base>
    class value_type_operations<Derived, Base, TroublesomeKey> : public Base
    {
    public:
        typedef typename Base::PlaceholderType PlaceholderType;
        virtual bool less(const PlaceholderType& other) const
        {
            if( other.type() == type_info<typename Derived::AnyType>::template type_id<SimpleKey>())
            {
                typedef typename Derived::AnyType::template holder<SimpleKey> OtherDerived;
                // objects of known type...comparison with members
                const OtherDerived* otherType = static_cast<const OtherDerived*>(&other);
                return (static_cast<const Derived*>(this)->value().key() < otherType->value().m_id);
            }
            return Base::less(other);
        }
    };

    // Default value type operations - most objects support these...
    //
    template <typename Derived, typename Base, typename ValueType>
    class value_type_operations : public Base
    {
    public:
        // Geometry...
        virtual int area() const
        {
            return static_cast<const Derived*>(this)->held.area();;
        }
        virtual void move(int x, int y)
        {
            static_cast<Derived*>(this)->held.move(x,y);
        }

        // Printable...
        virtual void print(std::ostream& os) const
        {
            os << static_cast<const Derived*>(this)->held;
        }
    };

    // TroublesomeEllipse needs special treatment...
    //
    template <typename Derived, typename Base>
    class value_type_operations<Derived, Base, TroublesomeEllipse> : public Base
    {
    public:
        // Geometry...
        virtual int area() const
        {
            return static_cast<const Derived*>(this)->held.calcArea();;
        }
        virtual void move(int x, int y)
        {
            static_cast<Derived*>(this)->held.moveTo(x,y);
        }

        // Printable...
        virtual void print(std::ostream& os) const
        {
            os << "Ellipse:";
            static_cast<const Derived*>(this)->held.stream(os);
        }
    };
}


namespace AnyMultipleInterfacesUnitTests
{
    typedef af::any<af::interfaces<>, af::less_than_comparable> AnyKey;
    typedef af::any<af::interfaces<Geometry, Printable>, af::equality_comparable> Any;

    struct SumAreas : public std::binary_function<int, std::pair<AnyKey,Any>, int>
    {
        int operator()(int total, const std::pair<AnyKey,Any>& elem) const
        {
            return total + elem.second.area();
        }
    };

    TEST_CASE("Require operations work", "[any]")
    {
        std::map<AnyKey,Any> data;
        
        data.insert( std::make_pair(TroublesomeKey(1), SimpleRectangle(5, 7, 100, 25))     );
        data.insert( std::make_pair(TroublesomeKey(2), SimpleRectangle(11, 13, 100, 25))   );
        data.insert( std::make_pair(SimpleKey(42),TroublesomeEllipse(27, 0, 150, 50)) );

        std::ostringstream oss;
        for( std::map<AnyKey,Any>::const_iterator it = data.begin(); it != data.end(); ++it )
        {
            it->second.print(oss);
            oss << "#";
        }
        REQUIRE(oss.str() == "Area:2500, x:5, y:7#Area:2500, x:11, y:13#Ellipse:Area:5891, x:27, y:0#");
        std::map<AnyKey,Any>::iterator f = data.find(SimpleKey(42));
        REQUIRE(f != data.end());
        f->second.move(341,666);
        REQUIRE(f->second.area() == 5891);

        f = data.find(SimpleKey(1));
        REQUIRE(f != data.end());
        REQUIRE(f->second.area() == 2500);

        f = data.find(TroublesomeKey(42));
        REQUIRE(f != data.end());
        REQUIRE(f->second.area() == 5891);

        oss.str("");
        for( std::map<AnyKey,Any>::const_iterator it = data.begin(); it != data.end(); ++it )
        {
            it->second.print(oss);
            oss << "#";
        }
        REQUIRE(oss.str() == "Area:2500, x:5, y:7#Area:2500, x:11, y:13#Ellipse:Area:5891, x:341, y:666#");

        REQUIRE(std::accumulate( data.begin(), data.end(), 0, SumAreas()) == 10891);
    }
}
