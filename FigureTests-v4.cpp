#include <gmock/gmock.h>
#include <type_traits>
#include <functional>
#include <memory>

namespace object
{
    template<typename T>
    std::unique_ptr<T> clone(const T& object)
    {
        using base_type = typename T::base_type;
        auto ptr = static_cast<const base_type&>(object).clone();
        return std::unique_ptr<T>(static_cast<T*>(ptr));
    }

    template<typename T>
    auto clone(T* object) -> decltype(clone(*object))
    {
        return clone(*object);
    }
}

struct Figure
{
    using base_type = Figure;

    virtual ~Figure() = default;
    virtual double  area()  const = 0;

protected:
    virtual Figure* clone() const = 0;

    template <typename T>
    friend std::unique_ptr<T> object::clone(const T&);
};

struct Square : Figure
{
    double a = 0;

    Square() = default;
    Square(double a) : a{a}{}


    double  area()  const override
    {
        return a * a;
    }

protected:
    Square* clone() const override     // return type is Square* instead of Figure* (Covariant Return Type)
    {
        return new Square(*this);
    }
};

using namespace ::testing;

TEST(FigureTests_v3, square_clone_method_called_directly_return_pointer_of_type_Square)
{
    auto square = Square{};
    auto figure = object::clone(square);

    ASSERT_TRUE((std::is_same<std::unique_ptr<Square>, decltype(figure)>::value));
}

TEST(FigureTests_v3, square_clone_method_called_via_base_class_return_pointer_of_type_Figure)
{
    auto square = Square{};
    auto square_figure = static_cast<Figure*>(&square);
    auto figure = object::clone(square_figure);

    ASSERT_TRUE((std::is_same<std::unique_ptr<Figure>, decltype(figure)>::value));
}

TEST(FigureTests_v3, cloned_square_via_base_pointer_should_return_same_area)
{
    auto a = 4.;
    auto square = Square{a};
    auto square_figure = static_cast<Figure*>(&square);
    auto figure = object::clone(square_figure);

    ASSERT_THAT( square.area(), Eq(a*a));
    ASSERT_THAT(figure->area(), Eq(square.area()));
}

TEST(FigureTests_v3, clonning_rvalue)
{
    auto square = Square{};
    auto figure = object::clone(Square{});

    ASSERT_TRUE((std::is_same<std::unique_ptr<Square>, decltype(figure)>::value));
}

