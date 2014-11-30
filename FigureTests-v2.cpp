#include <gmock/gmock.h>
#include <type_traits>
#include <functional>
#include <memory>

namespace object
{

    namespace details
    {
        template<typename T>
        constexpr T& ref(T& object)
        {
            return object;
        }

        template<typename T>
        constexpr T& ref(T* object)
        {
            return *object;
        }

        template <typename T>
        using decay_type = typename std::remove_pointer<typename std::decay<T>::type>::type;
    }

    template <typename T>
    using unique_ptr_t = std::unique_ptr<details::decay_type<T>>;

    template<typename T>
    unique_ptr_t<T> clone(T&& object)
    {
        return unique_ptr_t<T>(details::ref(object).clone());
    }
}

struct Figure
{
    using base_type = Figure;

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

TEST(FigureTests_v2, square_clone_method_called_directly_return_pointer_of_type_Square)
{
    auto square = Square{};
    auto figure = object::clone(square);

    ASSERT_TRUE((std::is_same<std::unique_ptr<Square>, decltype(figure)>::value));
}

TEST(FigureTests_v2, square_clone_method_called_via_base_class_return_pointer_of_type_Figure)
{
    auto square = Square{};
    auto square_figure = static_cast<Figure*>(&square);
    auto figure = object::clone(square_figure);

    ASSERT_TRUE((std::is_same<std::unique_ptr<Figure>, decltype(figure)>::value));
}

TEST(FigureTests_v2, cloned_square_via_base_pointer_should_return_same_area)
{
    auto a = 4.;
    auto square = Square{a};
    auto square_figure = static_cast<Figure*>(&square);
    auto figure = object::clone(square_figure);

    ASSERT_THAT( square.area(), Eq(a*a));
    ASSERT_THAT(figure->area(), Eq(square.area()));
}

