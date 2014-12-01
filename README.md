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
It works just like previous example except it returns smart pointers instead of owning raw pointers.

