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
        using base_type = typename details::decay_type<T>::base_type;
        auto ptr = details::ref<base_type>(object).clone();
        return unique_ptr_t<T>(static_cast<typename unique_ptr_t<T>::pointer>(ptr));
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
    friend object::unique_ptr_t<T> object::clone(T&&);
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
