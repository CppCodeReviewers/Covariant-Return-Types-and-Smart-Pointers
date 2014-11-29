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
	virtual Square* clone() const      // return type is Square* instead of Figure* (Covariant Return Type)
	{
		return new Square(*this);
	}
};
```

It compiles and works. Return type depends on what type method clone is called - if it is called via pointer or reference to Figure it will return pointer to Figure and if it is called via pointer or reference to Square it will return pointer to Square. Simple and obvious, right?

```
Square* square = new Square{};
Figure* figure = square;

auto f1 = figure->clone();	// f1 has type Figure*
auto f2 = square->clone();	// f2 has type Square*
```