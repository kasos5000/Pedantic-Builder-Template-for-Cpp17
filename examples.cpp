#include <iostream>
#include "private_builder.hpp"
#include "shared_builder.hpp"

using namespace std;

//Creating a class that will report copying and moving
struct Str {
	std::string s;
	Str() = default;
	Str(const std::string& str) :
	    s(str) {
	}
	Str(const Str& other) :
	    s(other.s) {
		cout << "Copy constr" << endl;
	}
	Str(Str&& other) :
	    s(move(other.s)) {
		cout << "Move constr" << endl;
	}
	void operator=(const Str& other) {
		s = other.s;
		cout << "Copy assing" << endl;
	}
	void operator=(Str&& other) {
		s = move(other.s);
		cout << "Move assing" << endl;
	}
};

//Defining a builder class with a shared core
template<bool c1 = false, bool c2 = false, bool c3 = false, bool c4 = false>
class BuilderSh : public SharedBuilder<FullySharedCore<int, double, Str, bool>, c1, c2, c3, c4> {
public:
	auto SetInt(int i) {
		return PBT_SET_ARG_INNER(0, i);
	}
	auto SetDouble(double d) {
		return PBT_SET_ARG_INNER(1, d);
	}
	auto SetString(Str&& s) {
		return PBT_SET_ARG_INNER(2, move(s));
	}
	auto SetBool(bool b) {
		return PBT_SET_ARG_INNER(3, b);
	}
};

//Defining a builder class with a private core
template<bool c1, bool c2, bool c3, bool c4>
using BuilderPrT = PrivateBuilder<SimplePrivateCore<int, double, Str, bool>, c1, c2, c3, c4>;

template<bool c1 = false, bool c2 = false, bool c3 = false, bool c4 = false>
class BuilderPr : public BuilderPrT<c1, c2, c3, c4> {
public:
	using BuilderPrT<c1, c2, c3, c4>::BuilderPrT; //inheriting constructor
	auto SetInt(int i) {
		return PBT_SET_ARG_INNER(0, i);
	}
	auto SetDouble(double d) {
		return PBT_SET_ARG_INNER(1, d);
	}
	auto SetString(const std::string& s) {
		return PBT_SET_ARG_INNER(2, s);
	}
	auto SetBool(bool b) {
		return PBT_SET_ARG_INNER(3, b);
	}
};

//Creating a target class and functions with same parameter lists (not considering references), for which the defined builder classes are suitable
struct A {
	A(int ii, double dd, Str&& s, bool bb = true) :
	    i(ii), d(dd), str(move(s)), b(bb) {
	}
	explicit A(BuilderPr<1, 1, 1, 1> builder) :
	    A(builder.Construct<A, pbt::move, 2>()) {
	}
	int i;
	double d;
	Str str;
	bool b;
};

ostream& operator<<(ostream& os, const A& a) {
	os << a.str.s << ' ' << a.i << ' ' << a.d << ' ' << a.b;
	return os;
}

void Foo1(int i, double d, const Str& s, bool b) {
	cout << i << ' ' << d << ' ' << s.s << ' ' << b << endl;
}

double Foo2(int i, double d, const Str& s, bool b) {
	cout << i << ' ' << d << ' ' << s.s << ' ' << b << endl;
	return i + d;
}

int main() {
	A a1(BuilderPr<>()
	         .SetInt(0)
	         .SetDouble(2.5)
	         .SetString("Passing builder as an argument")
	         .SetBool(false));
	cout << a1 << endl;

	A a2 = BuilderSh()
	           .SetInt(1)
	           .SetDouble(2.55)
	           .SetString({"Object constructed"})
	           .Construct<A, pbt::move, 2>(); //The bool has default value
	cout << a2 << endl;

	double d = BuilderPr()
	               .SetInt(2)
	               .SetDouble(3.5)
	               .SetString("Non-void function invoked")
	               .SetBool(true)
	               .Invoke(Foo2);
	cout << d << endl;

	BuilderSh()
	    .SetInt(3)
	    .SetDouble(4.5)
	    .SetString({"Void function invoked"})
	    .SetBool(true)
	    .Invoke(Foo1);

	unique_ptr<A> a_ptr(BuilderPr<>()
	                         .SetInt(4)
	                         .SetDouble(5.32)
	                         .SetString({"Dynamic object constructed"})
	                         .SetBool(false)
	                         .ConstructNew<A, pbt::move, 2>());
	cout << *a_ptr << endl;

	a_ptr = BuilderSh()
	             .SetInt(5)
	             .SetDouble(3.1)
	             .SetString({"Dynamic object constructed using make_unique"})
	             .Invoke<pbt::move>(make_unique<A,int,double,Str>);
	cout << *a_ptr << endl;
	cout << "Done!";
	return 0;
}
