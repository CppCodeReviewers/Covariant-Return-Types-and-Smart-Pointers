#include <gmock/gmock.h>
#include <type_traits>
#include <functional>
#include <memory>


struct Figure
{
    virtual ~Figure() = default;
    virtual double  area()  const = 0;
    virtual Figure* clone() const = 0;
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

    Square* clone() const override     // return type is Square* instead of Figure* (Covariant Return Type)
    {
        return new Square(*this);
    }
};

using namespace ::testing;

TEST(FigureTests, square_clone_method_called_directly_return_pointer_of_type_Square)
{
    auto square = Square{};
    auto figure = square.clone();

    ASSERT_TRUE((std::is_same<Square*, decltype(figure)>::value));
}

TEST(FigureTests, square_clone_method_called_via_base_class_return_pointer_of_type_Figure)
{
    auto square = Square{};
    auto square_figure = static_cast<Figure*>(&square);
    auto figure = square_figure->clone();

    ASSERT_TRUE((std::is_same<Figure*, decltype(figure)>::value));
}

TEST(FigureTests, cloned_square_via_base_pointer_should_return_same_area)
{
    auto a = 4.;
    auto square = Square{a};
    auto square_figure = static_cast<Figure*>(&square);

    auto figure = square_figure->clone();

    ASSERT_THAT( square.area(), Eq(a*a));
    ASSERT_THAT(figure->area(), Eq(square.area()));
}
