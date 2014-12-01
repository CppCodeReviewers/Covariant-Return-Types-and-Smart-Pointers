Covariant Return Types and Smart Pointers
=========================

Covariant Return Types is language feature which allows you to change return type of your virtual function to covariant type ie. pointer to derived class instead of pointer to base class - see example below:

```
struct Figure 
{
	virtual ~Figure() = default;	
	virtual Figure* clone() const = 0;
	// ... other methods
};

struct Square : Figure
{
	Square* clone() const override     // return type is Square* instead of Figure* (Covariant Return Type)
	{
		return new Square(*this);
	}
	// ... other methods
};
```

It compiles and works. Return type depends on what type method clone is called - if it is called via pointer or reference to Figure it will return pointer to Figure and if it is called via pointer or reference to Square it will return pointer to Square. Simple and obvious, right?

```
Square  square;
Figure& figure = square;

auto f1 = figure.clone();	// f1 has type Figure*
auto f2 = square.clone();	// f2 has type Square*

auto f3 = Square{}.clone(); // f3 has type Square*

```

# What about Smart Pointers?

Since c++11 we want to avoid owning raw pointers so we would like to introduce interface which use ie. std::unique_ptr<Figure> instead of Figure*.
```
struct Figure 
{
	virtual ~Figure() = default;	
	virtual std::unique_ptr<Figure> clone() const = 0;
	// ... other methods
};

struct Square : Figure
{
	std::unique_ptr<Square> clone() const override
	{
		return new Square(*this);
	}
	// ... other methods
};
```

Unfortunately smart pointers are not treated as covariant types so `virtual std::unique_ptr<Square> clone() override;` will not override `virtual std::unique_ptr<Figure> clone() = 0;`. My compiler (mingw with gcc-4.9) display errors:
```
Covariant-Return-Types-and-Smart-Pointers\FigureTests.cpp:34:29: error: invalid covariant return type for 'virtual std::unique_ptr<Square> Square::clone() const'
     std::unique_ptr<Square> clone() const override     // return type is Square* instead of Figure* (Covariant Return Type)
                             ^
Covariant-Return-Types-and-Smart-Pointers\FigureTests.cpp:24:37: error:   overriding 'virtual std::unique_ptr<Figure> Figure::clone() const'
     virtual std::unique_ptr<Figure> clone() const = 0;
```

Can we somehow introduce smart pointers and make use of covariant return types?

# Clone function

To be able to use polymorphic magic we need to have clone() method inside base class interface but we can also use some helper function which will wrap raw pointers into smart pointers ie.
```
std::unique_ptr<Figure> clone(const Figure& object)
{
	return std::unique_ptr<Figure>(object.clone());
}
```
Please notice that this function will work for every class which inherits Figure interface - it takes argument by reference. We use such method like this:
```
Square  square;
Figure& figure = square;

auto f1 = clone(figure);	// f1 has type std::unique_ptr<Figure>
auto f2 = clone(square);	// f2 has type std::unique_ptr<Figure>

auto f3 = clone(Square{});	// f3 has type std::unique_ptr<Figure>
```
It almost worked like previous solutions but it returns smart pointer to Figure as f2 instead of smart pointer to Square. The problem is that `clone` function takes argumet as reference to Figure which imply that it will call clone method via Figure reference so it will always return pointer to Figure from `object.clone()` call. We need to distinguish type on which clone function is called - let's try templates!
```
template <typename T>
std::unique_ptr<T> clone(const T& object)
{
	return std::unique_ptr<T>(object.clone());
}
```
and now...
```
Square  square;
Figure& figure = square;

auto f1 = clone(figure);	// f1 has type std::unique_ptr<Figure>
auto f2 = clone(square);	// f2 has type std::unique_ptr<Square>

auto f3 = clone(Square{});	// f3 has type std::unique_ptr<Square>
```
It works just like previous example except it returns smart pointers instead of owning raw pointers. The only thing left is how to prevent user of caling Figure::clone function which still returns raw pointer - and in worse scenario make memory leak.

# Preventing use of Figure::clone

Method Figure::clone is public so user is allowed to call it. We can make it protected and then to make function clone able to call Figure::clone we can make it friend.

```
struct Figure 
{
	virtual ~Figure() = default;	
	
	// ... other methods
	
protected:
	virtual std::unique_ptr<Figure> clone() const = 0;
		
    template <typename T>
    friend std::unique_ptr<T> clone(const T&);
};

struct Square : Figure
{
	// ... other methods
	
protected:
	std::unique_ptr<Square> clone() const override
	{
		return new Square(*this);
	}
};
```
In such case we get such results:
```
Square  square;
Figure& figure = square;

auto f1 = clone(figure);	// f1 has type std::unique_ptr<Figure>
auto f2 = clone(square);	// Error! Square::clone is protected in this context

auto f3 = clone(Square{});	// Error! Square::clone is protected in this context
```
When we call `clone` function on `Figure` reference it works as expected but when called on `Square` it fails to work. We can put friend declaration to every class which inherits from `Figure` but this is not the best what we can do. Lets try to use fact that `clone` function works on Figure reference/pointer. We can do something like that:
```
template <typename T>
std::unique_ptr<T> clone(const T& object)
{
	using base_type = Figure;
	auto ptr = static_cast<const base_type&>(object).clone();
	return std::unique_ptr<T>(static_cast<T*>(ptr));
}
```
And now everything works just like before. Unfortunately this code is not so generic - it can only work on classes which inherit Figure class. Lets make it more generic.
```
template <typename T>
std::unique_ptr<T> clone(const T& object)
{
	using base_type = typename T::base_type;
	auto ptr = static_cast<const base_type&>(object).clone();
	return std::unique_ptr<T>(static_cast<T*>(ptr));
}
```
and change definition of `Figure` class to:
```
struct Figure 
{
	using base_type = Figure;

	virtual ~Figure() = default;	
	
	// ... other methods
	
protected:
	virtual std::unique_ptr<Figure> clone() const = 0;
	
    template <typename T>
    friend std::unique_ptr<T> object::clone(const T&);
};
```
Now `clone` function works on every polymorphic type which has defined `base_type` attribute and virtual `clone()` method. Type T has to inherits T::base_type - we can make this restriction explicit by putting `static_assert` into the code:
```
template <typename T>
std::unique_ptr<T> clone(const T& object)
{
	using base_type = typename T::base_type;
    static_assert(std::is_base_of<base_type, T>::value, "T object has to derived from T::base_type");	
	auto ptr = static_cast<const base_type&>(object).clone();
	return std::unique_ptr<T>(static_cast<T*>(ptr));
}
```
Can we do better? Yes, we can introduce class which can be inherited to make your type cloneable.

# Generic solution - cloneable type

Lets pack all requirements for `Figure` interface to separate class - lets call it `cloneable` - and lets put there everything which need to be defined to make class cloneable. Then `Figure` class can inherit it and rest should work the same.

```
struct Figure;

struct cloneable
{
	using base_type = Figure;

	virtual ~cloneable() = default;	
protected:
	virtual std::unique_ptr<Figure> clone() const = 0;
	
    template <typename T>
    friend std::unique_ptr<T> object::clone(const T&);
}

struct Figure : cloneable
{	
	// ... other methods
};
```
Everything works! Again solution does not look generic - we can eliminate dependency to `Figure` class by using template and [Curiously recurring template pattern](http://en.wikipedia.org/wiki/Curiously_recurring_template_pattern)

```
template <typename Type>
struct cloneable
{
	using base_type = Type;

	virtual ~cloneable() = default;	
protected:
	virtual std::unique_ptr<Type> clone() const = 0;
	
    template <typename T>
    friend std::unique_ptr<T> object::clone(const T&);
}

struct Figure : cloneable<Figure> // curiously recurring template pattern
{	
	// ... other methods
};
```
and that's it! We can add helper method to be able to clone object via pointer instead of reference:
```
template<typename T>
auto clone(T* object) -> decltype(clone(*object))
{
	return clone(*object);
}
```
and we can put everything in one namespace ie. `object` then entire solution will look like this:
```
namespace object
{
    template<typename T>
    std::unique_ptr<T> clone(const T& object)
    {
        using base_type = typename T::base_type;
        static_assert(std::is_base_of<base_type, T>::value, "T object has to derived from T::base_type");
        auto ptr = static_cast<const base_type&>(object).clone();
        return std::unique_ptr<T>(static_cast<T*>(ptr));
    }

    template<typename T>
    auto clone(T* object) -> decltype(clone(*object))
    {
        return clone(*object);
    }

    template<typename T>
    struct cloneable
    {
        using base_type = T;

        virtual ~cloneable() = default;
    protected:
        virtual T* clone() const = 0;

        template <typename X>
        friend std::unique_ptr<X> object::clone(const X&);
    };
}

struct Figure : object::cloneable<Figure>
{
	// ... other methods
};

struct Square : Figure
{
	// ... other methods
	
protected:
	std::unique_ptr<Square> clone() const override
	{
		return new Square(*this);
	}
};
```

# Expert question

Can we adjust this solution to take advantage of move semantics when `clone` function is called on rvalue objects?
