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
    struct Cell
    {
        virtual ~Cell() {}
        virtual void displayCellLocation(std::ostream&) const = 0;
    };

    struct Calculation
    {
        virtual ~Calculation() {}
        virtual int calculate() const = 0;
    };

    struct Content
    {
        virtual ~Content() {}
        virtual void update(const std::string& s) = 0;
        virtual void show(std::ostream& os) const = 0;
    };

    ////////////////////////////////////////
    // Cell types...
    //
    static const int cellsPerRow = 16;
    
    // Cell Ids
    //
    class CoordinateCellId
    {
    public:
        CoordinateCellId(int x, int y)
            : m_index(y * cellsPerRow + x)
        {}
        int m_index;
        friend bool operator<(const CoordinateCellId& lhs, const CoordinateCellId& rhs)
        {
            return (lhs.m_index < rhs.m_index);
        }
        virtual void displayCellCoordinates(std::ostream& os) const
        {
            os << (m_index % cellsPerRow) << "," << (m_index / cellsPerRow);
        }
    };

    struct LegacyCellId
    {
        LegacyCellId(int index)
            : m_key(index)
        {}
        int key() const {return m_key;}
        virtual void displayCellLocation(std::ostream& os) const
        {
            os << "[" << m_key << "]";
        }
    private:
        int m_key;
        // operator< not defined
    };

    // Content types
    //
    struct StringCell
    {
        StringCell(const std::string& s) : m_content(s) {}
        std::string m_content;

        int calculate() const
        {
            return 0;
        }
        void update(const std::string& s)
        {
            m_content = s;
        }
        friend bool operator==(const StringCell& lhs, const StringCell& rhs)
        {
            return (lhs.m_content == rhs.m_content);
        }
        friend std::ostream& operator<<(std::ostream& oss, const StringCell&v)
        {
            oss << v.m_content;
            return oss;
        }
    };

    struct ValueCell
    {
        ValueCell(const std::string& s) : m_value(0) {update(s);}
        int m_value;

        int calculate() const
        {
            return m_value;
        }
        void update(const std::string& s)
        {
            std::istringstream iss(s);
            iss >> m_value;
        }
        friend bool operator==(const ValueCell& lhs, const ValueCell& rhs)
        {
            return (lhs.m_value == rhs.m_value);
        }
        friend std::ostream& operator<<(std::ostream& oss, const ValueCell&v)
        {
            oss << v.m_value;
            return oss;
        }
    };

    struct FormulaCell
    {
        FormulaCell(const std::string& s) : m_formula(s) {}
        std::string m_formula; // simple implementation...

        void parseFormula(const std::string& f) {m_formula = f;}
        int calculate() const
        {
            if( m_formula == "50-8" )
                return 42;
            else if( m_formula == "3*9" )
                return 27;
            return 0;
        }
        void streamValue(std::ostream& os) const
        {
            os << calculate();
        }
    };

}

////////////////////////////////////////////////
// Template implementations and specializations
//
namespace any_facade
{
    // Implement less for LegacyCellId
    template<>
    bool less_than_comparable::less<LegacyCellId>(const LegacyCellId& lhs, const LegacyCellId& rhs)
    {
        return (lhs.key() < rhs.key());
    }

    // Implement equals for FormulaCell
    template<>
    bool equality_comparable::equals<FormulaCell>(const FormulaCell& lhs, const FormulaCell& rhs)
    {
        return (lhs.calculate() == rhs.calculate() );
    }

    // Allow any.fn() for Cell interface on CellId keys
    template <>
    class forwarder<any<interfaces<Cell>, less_than_comparable> > : public Cell
    {
    public:
        virtual void displayCellLocation(std::ostream& os) const
        {
            static_cast<const any<interfaces<Cell>, less_than_comparable>*>(this)->content->displayCellLocation(os);
        }
    };

    // Allow any.fn() for Calculation and Content interfaces on cell content
    //
    // Method implemented... deriving from the interface is optional but
    // ensures that changes to the interface will get caught by the compiler
    //
    template <>
    class forwarder<any<interfaces<Calculation, Content>, equality_comparable> > : public Calculation, Content
    {
    public:
        int calculate() const
        {
            return static_cast<const any<interfaces<Calculation, Content>, equality_comparable>*>(this)->content->calculate();
        }

        void update(const std::string& s)
        {
            static_cast<const any<interfaces<Calculation, Content>, equality_comparable>*>(this)->content->update(s);
        }
        void show(std::ostream& os) const
        {
            static_cast<const any<interfaces<Calculation, Content>, equality_comparable>*>(this)->content->show(os);
        }
    };

    // Implementation of operations on CoordinateCellId value type
    template <typename Derived, typename Base>
    class value_type_operations<Derived, Base, CoordinateCellId> : public Base
    {
    public:
        typedef typename Base::PlaceholderType PlaceholderType;
        virtual bool less(const PlaceholderType& other) const
        {
            if( other.type() == type_info<typename Derived::AnyType>::template type_id<LegacyCellId>())
            {
                typedef typename Derived::AnyType::template holder<LegacyCellId> OtherDerived;
                // objects of known type...comparison with members
                const OtherDerived* otherType = static_cast<const OtherDerived*>(&other);
                return (static_cast<const Derived*>(this)->value().m_index < otherType->value().key());
            }
            return Base::less(other);
        }
        virtual void displayCellLocation(std::ostream& os) const
        {
            static_cast<const Derived*>(this)->held.displayCellCoordinates(os);
        }
    };

    // Implementation of operations on LegacyCellId value type
    template <typename Derived, typename Base>
    class value_type_operations<Derived, Base, LegacyCellId> : public Base
    {
    public:
        typedef typename Base::PlaceholderType PlaceholderType;
        virtual bool less(const PlaceholderType& other) const
        {
            if( other.type() == type_info<typename Derived::AnyType>::template type_id<CoordinateCellId>())
            {
                typedef typename Derived::AnyType::template holder<CoordinateCellId> OtherDerived;
                // objects of known type...comparison with members
                const OtherDerived* otherType = static_cast<const OtherDerived*>(&other);
                return (static_cast<const Derived*>(this)->value().key() < otherType->value().m_index);
            }
            return Base::less(other);
        }
        virtual void displayCellLocation(std::ostream& os) const
        {
            static_cast<const Derived*>(this)->held.displayCellLocation(os);
        }
    };

    // Implementation of operations on StringCell value type
    // Default value type operations - StringCell...
    //
    template <typename Derived, typename Base, typename ValueType>
    class value_type_operations : public Base
    {
    public:
        typedef typename Base::PlaceholderType PlaceholderType;
        virtual bool equals(const PlaceholderType& other) const
        {
            if( other.type() == type_info<typename Derived::AnyType>::template type_id<FormulaCell>())
            {
                typedef typename Derived::AnyType::template holder<FormulaCell> OtherDerived;
                // objects of known type...comparison with calculated value
                const OtherDerived* otherType = static_cast<const OtherDerived*>(&other);
                std::ostringstream oss;
                oss << otherType->calculate();
                return (static_cast<const Derived*>(this)->value().m_content == oss.str());
            }
            else if( other.type() == type_info<typename Derived::AnyType>::template type_id<ValueCell>())
            {
                typedef typename Derived::AnyType::template holder<ValueCell> OtherDerived;
                // objects of known type...comparison with calculated value
                const OtherDerived* otherType = static_cast<const OtherDerived*>(&other);
                std::ostringstream oss;
                oss << otherType->calculate();
                return (static_cast<const Derived*>(this)->value().m_content == oss.str());
            }
            return Base::equals(other);
        }

        // Calculation...
        virtual int calculate() const
        {
            return static_cast<const Derived*>(this)->held.calculate();
        }

        // Content...
        virtual void update(const std::string& s)
        {
            static_cast<Derived*>(this)->held.update(s);;
        }
        virtual void show(std::ostream& os) const
        {
            os << static_cast<const Derived*>(this)->held;
        }
    };

    // Implementation of operations on ValueCell value type
    // valueCell needs special treatment...
    //
    template <typename Derived, typename Base>
    class value_type_operations<Derived, Base, ValueCell> : public Base
    {
    public:
        typedef typename Base::PlaceholderType PlaceholderType;
        virtual bool equals(const PlaceholderType& other) const
        {
            if( other.type() == type_info<typename Derived::AnyType>::template type_id<FormulaCell>())
            {
                typedef typename Derived::AnyType::template holder<FormulaCell> OtherDerived;
                // objects of known type...comparison with calculated value
                const OtherDerived* otherType = static_cast<const OtherDerived*>(&other);
                return (static_cast<const Derived*>(this)->value().m_value == otherType->calculate());
            }
            else if( other.type() == type_info<typename Derived::AnyType>::template type_id<StringCell>())
            {
                typedef typename Derived::AnyType::template holder<StringCell> OtherDerived;
                // objects of known type...comparison with calculated value
                const OtherDerived* otherType = static_cast<const OtherDerived*>(&other);
                std::ostringstream oss;
                oss << calculate();
                return (oss.str() == otherType->value().m_content);
            }
            return Base::equals(other);
        }

        // Calculation...
        virtual int calculate() const
        {
            return static_cast<const Derived*>(this)->held.calculate();
        }

        // Content...
        virtual void update(const std::string& s)
        {
            static_cast<Derived*>(this)->held.update(s);;
        }
        virtual void show(std::ostream& os) const
        {
            os << static_cast<const Derived*>(this)->held;
        }
    };

    // Implementation of operations on FormulaCell value type
    // FormulaCell needs special treatment...
    //
    template <typename Derived, typename Base>
    class value_type_operations<Derived, Base, FormulaCell> : public Base
    {
    public:
        typedef typename Base::PlaceholderType PlaceholderType;
        virtual bool equals(const PlaceholderType& other) const
        {
            if( other.type() == type_info<typename Derived::AnyType>::template type_id<StringCell>())
            {
                typedef typename Derived::AnyType::template holder<StringCell> OtherDerived;
                // objects of known type...comparison with calculated value
                const OtherDerived* otherType = static_cast<const OtherDerived*>(&other);
                std::ostringstream oss;
                oss << calculate();
                return (oss.str() == otherType->value().m_content);
            }
            else if( other.type() == type_info<typename Derived::AnyType>::template type_id<ValueCell>())
            {
                typedef typename Derived::AnyType::template holder<ValueCell> OtherDerived;
                // objects of known type...comparison with calculated value
                const OtherDerived* otherType = static_cast<const OtherDerived*>(&other);
                return (static_cast<const Derived*>(this)->value().calculate() == otherType->value().m_value);
            }
            return Base::equals(other);
        }

        // Calculation...
        virtual int calculate() const
        {
            return static_cast<const Derived*>(this)->held.calculate();
        }

        // Content...
        virtual void update(const std::string& s)
        {
            static_cast<Derived*>(this)->held.parseFormula(s);;
        }
        virtual void show(std::ostream& os) const
        {
            os << "f():";
            static_cast<const Derived*>(this)->held.streamValue(os);
        }
    };
}

///////////////////////////////////////////////////////
// Tests...

namespace AnyMultipleInterfacesUnitTests
{
    typedef af::any<af::interfaces<Cell>, af::less_than_comparable> CellId;
    typedef af::any<af::interfaces<Calculation, Content>, af::equality_comparable> Any;

    struct SumValues : public std::binary_function<int, std::pair<CellId,Any>, int>
    {
        int operator()(int total, const std::pair<CellId,Any>& elem) const
        {
            return total + elem.second.calculate();
        }
    };

    struct CompareContent : public std::unary_function<std::pair<CellId,Any>, bool>
    {
        explicit CompareContent(const std::pair<CellId,Any>& f)
            : data(f)
        {}
        bool operator()(const std::pair<CellId,Any>& rhs) const
        {
            return (data.second == rhs.second);
        }
        std::pair<CellId,Any> data;
    };

    TEST_CASE("Require 'spreadsheet' operations work", "[any]")
    {
        std::map<CellId,Any> data;
        
        data.insert( std::make_pair(LegacyCellId(1), StringCell("first cell"))     );
        data.insert( std::make_pair(LegacyCellId(2), FormulaCell("50-8"))   );
        data.insert( std::make_pair(LegacyCellId(3), ValueCell("80"))   );
        data.insert( std::make_pair(CoordinateCellId(10,2),FormulaCell("")) );

        std::ostringstream oss;
        for( std::map<CellId,Any>::const_iterator it = data.begin(); it != data.end(); ++it )
        {
            it->first.displayCellLocation(oss);
            oss << "#";
        }
        REQUIRE(oss.str() == "[1]#[2]#[3]#10,2#");
        std::map<CellId,Any>::iterator f = data.find(CoordinateCellId(2,0));
        REQUIRE(f != data.end());
        REQUIRE(f->second.calculate() == 42);

        f = data.find(CoordinateCellId(1,0));
        REQUIRE(f != data.end());
        f->second.update("3412");
        REQUIRE(f->second.calculate() == 0);

        f = data.find(LegacyCellId(42));
        REQUIRE(f != data.end());
        f->second.update("3*9");
        REQUIRE(f->second.calculate() == 27);

        oss.str("");
        for( std::map<CellId,Any>::const_iterator it = data.begin(); it != data.end(); ++it )
        {
            it->second.show(oss);
            oss << "#";
        }
        REQUIRE(oss.str() == "3412#f():42#80#f():27#");

        REQUIRE(std::accumulate( data.begin(), data.end(), 0, SumValues()) == 149);

        oss.str("");
        f = std::find_if( data.begin(), data.end(), CompareContent(std::make_pair(CoordinateCellId(0,0),StringCell("27"))) );
        REQUIRE(f != data.end());
        f->second.show(oss);
        REQUIRE(oss.str() == "f():27");

        oss.str("");
        f = std::find_if( data.begin(), data.end(), CompareContent(std::make_pair(CoordinateCellId(0,0),ValueCell("42"))) );
        REQUIRE(f != data.end());
        f->second.show(oss);
        REQUIRE(oss.str() == "f():42");

        oss.str("");
        f = std::find_if( data.begin(), data.end(), CompareContent(std::make_pair(CoordinateCellId(0,0),ValueCell("3412"))) );
        REQUIRE(f != data.end());
        f->second.show(oss);
        REQUIRE(oss.str() == "3412");

        f = std::find_if( data.begin(), data.end(), CompareContent(std::make_pair(CoordinateCellId(0,0),StringCell("0"))) );
        REQUIRE(f == data.end());
    }
}
