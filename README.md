# Pedantic-Builder-Template-for-Cpp17
## Intro
Builder pattern is meant to help the user resolve complicated signatures of constructors and functions. Let’s consider the following constructor call for example of such signature:  
SomeObject(3, 5.5, true, 0);  
By this call it is difficult to infer what each value means and it’s easy to confuse the arguments while passing them to the constructor. Builder pattern can solve these problems.  
But ordinary implementations of this pattern raise another problems. It becomes possible to miss an argument of the parameter list; or, in some implementation, more complex constructor calls, such as creation of dynamic objects, become impossible.  
This work contains templates that allow the user to define builder classes in a compact way, that have the following properties:  
- Allow to prepare values of arguments by applying a chain custom-created setter methods;  
- The order of applying of the setters is free;  
- Check if all arguments have been passed at compile time, disallows the user to skip an argument;  
- Support move-semantics;  
- Support defaulted arguments for constructors;  
- Can be applied as a function/constructor parameter or can use special methods to invoke a function/constructor;  
- Virtuality-free. This implementation operates constant templates.  
The goal of this work is also to demonstrate what template metaprogramming can do. Even if you don’t need this template, perhaps you’ll find some patterns used in this work useful for you.
## The two variants of the Template
This work contains two variants of Builder Template that use distinct types of data managers – Cores. I called these variants Shared Builder and Private Builder. Shared Builder assumes using a Core that stores the argument values in a container with static storage duration, thus, if the parameter pack of two or more different Cores are same it will store the values of the arguments at the same location. Such template could be “privatized” though, e.g. by adding some type to the end of the parameter pack. But for stronger guarantees of non-overlapping it is recommended to second variant, Private Builder, which assumes using a Core that uses a pointer (or a smart pointer) to the stored data. It is less convenient to use in general (though after a bunch of improvements of the template the inconvenience has shrunk to the necessity to inherit the base class constructor in the implementation of the Final Builder class), but it should be more convenient to use in the situations when the overlapping of the data storage is unacceptable (like in threads, for instance).
## Defining a Builder class
### The head
The names of the two templates are the following:
```cpp
template<typename Core, bool... Marks> class SharedBuilder;
template<typename Core, bool... Marks> class PrivateBuilder;
```
where  
`Core` is the data manager;  
`Marks` can tell whether or not the corresponding argument has been set.  
The Final Builder class should be a child of one of the two Builder Templates.  
So, the full head of the Final Builder class should look like the following:
```cpp
template<bool c1, bool c2, …, bool cN>
class SomeBuilder : public SharedBuilder<SomeSharedCore<types…>, c1, c2, …, cN>
```
Or
```cpp
template<bool c1, bool c2, …, bool cN>
class SomeBuilder : public PrivateBuilder<SomePrivateCore<types…>, c1, c2, …, cN>
```
### About Cores
A Core has to be a template class whose parameter list contains (or consists of) a pack matching the parameter list of the target constructor/function. The sizes of the parameter pack of the Core and the bool parameter pack must be equal to the size of the parameter list. For example, if we’ve got a constructor like `A(int a, double d, bool b)` the head of the Builder class definition will look like this:
```cpp
template<bool c1, bool c2, bool c3>
class SomeBuilder : public SharedBuilder<SomeSharedCore<int, double, bool>, c1, c2, c3>
```
Shared or Private Cores are used by corresponding Builder Templates to store, manage and get arguments. It is in turn the base class of the hierarchy of the builder class, having a Builder Template of the corresponding type as a direct derived class. Shared Builder cannot use a private Core and vice versa.  
There are two default Cores provided in this work, a shared and a private one. The user can define custom Cores. Such a Core must contain:  
- A storage of the argument values (like `std::tuple`).  
- A template type alias called `ArgType<I>` (like `std::tuple_element`), which is the alias of the type of the I-th argument, accessible for Builder Templates that are children of Cores.  
- A member function `GetArg<I>` (most reasonably public) that returns a universal reference to the I-th argument. It is used by Builder Templates for both getting and setting argument in the store.  
- A Private Core must also have a constructor that takes a pointer to another object of this Core type. This constructor will be used by Final Builder. That is the reason why constructor should be inherited. This constructor of Final Builder must be accessible inside Private Builder Template as a side class. 
See the definition of the default Cores for more information.  
### The body
Inside the body of the of the Final Builder class definition the user only needs to define setters (or passers) that will be used for passing the arguments, and, in case of `PrivateBuilder` usage, to inherit constructor of the base class. A setter should be defined via returning the result of the inner function:
```cpp
auto SetSomething(SomeType value) {
return this->template SetArg<FinalBuilderTempName, N>(value);
}
```
where:
`FinalBuilderTempName` – is the name of the current Builder Template that is being defined;
`N` – the number of the argument in the signature from 0;
`value` – the value of the argument.
It may look nice, but it’s too verbose anyway. It also could be done more concisely using the special macro that is suitable for usage inside setter definitions in both Builder variants:
`return PBT_SET_ARG_INNER(N, value)`;
In this way, an updated Builder will be returned, where the fact of setting of the argument will be counted. The pattern assumes a chain call of setters. If all has been defined correctly, the last setter will return a Builder instance where all bool template parameters are true.
## Using a Builder object
### Step-by-step guideline
**1.** Create an instance of the defined Final Builder class, where all bool template parameters are false.  
**2.** Call a chain of setters.  
**3a.** As it was mentioned above, a proper builder object (that has passed through all setters) can be used as an argument. It is worth to define an overload that takes a fully-set builder instantiation (where all bool template arguments are true), like the following:  
`A(Builder<1, 1, …, 1> builder)`, where the number of 1s is equal to the number of arguments in the signature being resolved.  
For that, both default Cores have a getter `GetArg<size_t I>()`, which returns the stored value of the parameter number I.  
**3b.** Or the chain can be finalized by a call of one of the described below methods that can invoke constructors and functions. These methods pass arguments to constructors/functions, but not necessary all. If a false is met in the bool template parameter pack of the Builder, the passing will be stopped and the arguments will mismatch with the parameter list, what will result in a compiler error. These methods are described in the next section.  
See `examples.cpp` file for more information.
### Public methods of Builder Templates
```cpp
template<typename TargetType, typename Option, size_t… Idx> 
auto Construct() 
```
Constructs an object of type `TargetType`, applying an `Option` to arguments `Idx`, and returns the constructed object.
```cpp
template<typename TargetType, typename Option, size_t… Idx> 
auto ConstructNew() 
```
Constructs an object of type `TargetType` with dynamic storage duration, applying an `Option` to arguments `Idx`,  and returns a pointer to the constructed object.
```cpp
template<typename Option, size_t… Idx, typename Func> 
decltype(auto) Invoke(Func&& func) 
```
Invokes a function `func`, applying an `Option` to arguments `Idx`.  
The `Option` parameter can be one of the following type names: `pbt::none`, `pbt::move`, `pbt::derefer`, `pbt::move_or_derefer`. The `none` option adds no additional specifications to argument passing. The rest three options do the same: they tell that the arguments whose numbers are specified in the following list are to be moved from the store, if they are not pointers (someone unlikely would want to move a pointer). If the arguments are pointers, they are dereferenced instead (it is a safe way of dealing with signatures that take an object by an lvalue-reference). By default (if the index list is empty) move semantics are applied to all arguments (but not dereferencing). For example, if the user specifies e.g. `Construct<Type, pbt::move, 2, 3>()`, the third and the fourth arguments of the parameter list will be moved and the rest will be passed without additional specifications.
### About default arguments
There are ways to invoke something with default arguments. For example, to define an overload that takes a builder object with the template parameters, corresponding to the default arguments are false. 
In case of default arguments the `Construct` methods will construct objects if the arguments with default values have not been set. With invoking functions, however, a problem arises due to the C++ restrictions. The problem is that a function object does not retain the information about default arguments of its function, so, if not all arguments have been set, the arguments will mismatch with the parameter list even if it contains defaulted arguments.
## Usage
Add `pbt_inner` folder and also `shared_builder.hpp` and/or `private_builder.hpp` into your project, if you’d like to use default cores. Include `shared_builder.hpp` or `private_builder.hpp` dependently on which type of Builder Templates you’d like to use.
## Note
Even though calling order of setters is free it is recommended to keep the same order when using instances of the same user-defined builder template, in order to avoid redundant instantiations. Then the number of specializations of the builder template will be N + 1, where N is number of arguments. At the very worst case number of specializations may reach 2N.
