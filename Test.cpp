#include "Arithmetic.h"
#include "Assert.h"
#include "StringReader.h"
#include <iostream>

using namespace std;
using namespace Arithmetic;


static void do_test(const char* s, FloatType expect) {
	ArithmeticExpression e2(s);
	assert_(e2.string_length() == strlen(s));
	auto v = e2.value_if_possible(VariableSet{});
	assert_(v.has_value() && v.value() == expect);
}

int main()
{
	cout << "sizeof(FloatType) = " << sizeof(FloatType) << '\n';
	{
		// 2+2=4
		ArithmeticExpression e;
		auto plus = new ArithmeticExpression::Node(ArithmeticExpression::Node::Type::Add, nullptr);
		auto n2 = new ArithmeticExpression::Node(2.0f, plus);
		auto n2_ = new ArithmeticExpression::Node(2.0f, plus);

		e.root = plus;
		plus->left_child = n2;
		plus->right_child = n2_;


		auto val = e.value_if_possible(VariableSet{});
		assert_(val.has_value() && val.value() == 4.0);
	}

	do_test("1+1+2+3+2*2*2", 1 + 1 + 2 + 3 + 2 * 2 * 2);
	do_test("5*(2+2) + (3%2)", 5 * (2 + 2) + (3 % 2));
	do_test("2^3 + 1 - 1", 8 + 1 - 1);
	do_test("2^(3 + 1) -   \t\n1", 16 - 1);
	do_test("1+2*2", 1 + 2 * 2);
	do_test("1*(0-1)", -1);
	do_test("-1*(0-1)", 1);
	do_test("-1*(-1)", 1);
	do_test("-1*-1", 1);
	do_test("2^-2", 0.25);
	{
		const char* s = "1+$aa";
		ArithmeticExpression e(s);
		assert_(e.string_length() == strlen(s));
		auto v = e.value_if_possible({
			{string("aa"), 3.0}
			});
		assert_(v.has_value() && v.value() == 4.0);
	}

	{
		const char* s = "1+$content_width";
		ArithmeticExpression e(s);
		assert_(e.string_length() == strlen(s));
		auto v = e.value_if_possible(VariableSet{
			{"content_width", 3.0}
			});
		assert_(v.has_value() && v.value() == 4.0);

		v = e.value_if_possible(VariableSet{
			{"content_width", 4.0 }
			});
		assert_(v.has_value() && v.value() == 5.0);
	}

	{
		ArithmeticExpression e("($var + 3*6) / 2.2");
		std::optional<double> value = e.value_if_possible(VariableSet{
			{"var", 3.0}
			});
		assert_(value.has_value() && abs(value.value() - 9.5454545454545454545454545454545) < 0.0001);
	}

	StringReader r("12");
	assert_(!r.end() && r.next() == '1');
	assert_(!r.end() && r.next() == '2');
	assert_(r.end());


	cout << "Tests completed\n";

	return 0;
}
