#include "Arithmetic.h"
#include <cassert>
#include <iostream>
#include <stdexcept>
#include "Assert.h"

using namespace std;

namespace Arithmetic {

	optional<FloatType> ArithmeticExpression::Node::value_if_possible(VariableSet const& variables)
	{
		if (type == Type::Literal) {
			return literal_value;
		}

		if (type == Type::Variable) {
			return find_var(variables, variable_name.value());
		}

		std::optional<FloatType> lval = nullopt, rval = nullopt;

		if (!left_child.has_value()) {
			assert(false);
			return nullopt;
		}

		lval = left_child.value()->value_if_possible(variables);

		if (type == Type::Brackets) {
			return lval;
		}

		if (!right_child.has_value()) {
			assert(false);
			return nullopt;
		}

		rval = right_child.value()->value_if_possible(variables);

		if (!lval || !rval) {
			return nullopt;
		}

		if (type == Type::Add) {
			return optional<FloatType>(lval.value() + rval.value());
		}
		else if (type == Type::Sub) {
			return optional<FloatType>(lval.value() - rval.value());
		}
		else if (type == Type::Div) {
			return optional<FloatType>(lval.value() / rval.value());
		}
		else if (type == Type::Mul) {
			return optional<FloatType>(lval.value() * rval.value());
		}
		else if (type == Type::Modulo) {
			return optional<FloatType>(fmod(lval.value(), rval.value()));
		}
		else if (type == Type::Power) {
			return optional<FloatType>(pow(lval.value(), rval.value()));
		}

		assert(false);

		return nullopt;
	}

	std::optional<FloatType> ArithmeticExpression::value_if_possible(VariableSet const& vars)
	{
		if (root.has_value()) {
			return root.value()->value_if_possible(vars);
		}
		else {
			return nullopt;
		}
	}

	void ArithmeticExpression::add_digit_or_variable(Node* n) {
		if (!root) {
			root = working_node = n;
		}
		else {
			assert_(working_node);
			// working node is either an operator + - * / ^ % with a left child and no right child
			// or is a bracket with no left or right child

			n->parent = working_node;

			if (working_node.value()->type == Node::Type::Brackets) {
				assert_(!working_node.value()->left_child && !working_node.value()->right_child);
				working_node.value()->left_child = n;
			}
			else if (working_node.value()->is_operator()) {
				assert_(working_node.value()->left_child && !working_node.value()->right_child);
				working_node.value()->right_child = n;
			}
			else {
				panic_();
			}
			working_node = n;
		}
	}

	void ArithmeticExpression::add_digit(FloatType val) {
		// TODO: Potential memory leak if exception is thrown
		add_digit_or_variable(new Node(val, nullopt));
	}

	void ArithmeticExpression::add_variable(string&& name)
	{
		add_digit_or_variable(new Node(std::move(name), nullopt));
	}

	void ArithmeticExpression::add_operator(Node::Type type) {

		assert_(root && working_node);
		assert__(working_node.value()->type == Node::Type::Literal || working_node.value()->type == Node::Type::Variable || working_node.value()->type == Node::Type::Brackets, "+ - * / ^ must follow a number or (");

		if (root == working_node) {
			auto n = new Node(type, nullopt);
			root = n;
			n->left_child = working_node;
			working_node = n;
			n->left_child.value()->parent = n;
		}
		else {
			if ((type == Node::Type::Add || type == Node::Type::Sub) && working_node.value()->parent.value()->type != Node::Type::Add && working_node.value()->parent.value()->type != Node::Type::Sub && working_node.value()->parent.value()->type != Node::Type::Brackets) {
				// Need to add the new node higher up in the tree because of order of operations: * / come before + -

				working_node = working_node.value()->parent.value();
				if (!working_node.value()->parent.has_value()) {
					root = new Node(type, nullopt);
					root.value()->left_child = working_node;
					working_node = root.value();
					return;
				}
			}

			auto n = new Node(type, working_node.value()->parent.value());
			if (working_node.value()->parent.value()->left_child.value() == working_node) {
				working_node.value()->parent.value()->left_child = n;
			}
			else {
				working_node.value()->parent.value()->right_child = n;
			}
			n->left_child = working_node;
			working_node = n;
		}
	}

	void ArithmeticExpression::open_brackets() {
		if (root.has_value()) {
			if (working_node.value()->is_operator()) {
				assert_(working_node.value()->left_child.has_value() && !working_node.value()->right_child.has_value());
				working_node.value()->right_child = new Node(Node::Type::Brackets, working_node);
				working_node = working_node.value()->right_child.value();
			}
			else if (working_node.value()->type == Node::Type::Brackets) {
				assert_(!working_node.value()->left_child.has_value() && !working_node.value()->right_child.has_value());
				working_node.value()->left_child = new Node(Node::Type::Brackets, working_node);
				working_node = working_node.value()->left_child.value();
			}
			else {
				panic__("Unexpected (");
			}



		}
		else {
			// Expression is surrounded with (redundant brackets)
			// Still need to add a node so we don't get confused at the end )
			auto n = new Node(Node::Type::Brackets, nullopt);
			root = working_node = n;
		}
	}

	void ArithmeticExpression::close_brackets() {
		assert_(root && working_node);
		do {
			working_node = working_node.value()->parent.value();
		} while (working_node && working_node.value()->type != Node::Type::Brackets);
		return;
	}

	ArithmeticExpression::ArithmeticExpression(string const& str) : reader(str)
	{

		while (!reader.end()) {
			char c = reader.next();
			if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
				continue;
			}

			optional<Node::Type> type = nullopt;
			FloatType val;

			if (c >= '0' && c <= '9' ||
				(c == '-' && working_node.has_value() && working_node.value()->type == Node::Type::Brackets && !working_node.value()->left_child.has_value()) ||
				(c == '-' && working_node.has_value() && working_node.value()->is_operator() && working_node.value()->left_child.has_value() && !working_node.value()->right_child.has_value()) ||
				(c == '-' && !root.has_value()))
			{
				val = reader.get_float();
				add_digit(val);
			}
			else if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '^') {
				Node::Type type;
				if (c == '+') {
					type = Node::Type::Add;
				}
				else if (c == '-') {
					type = Node::Type::Sub;
				}
				else if (c == '/') {
					type = Node::Type::Div;
				}
				else if (c == '*') {
					type = Node::Type::Mul;
				}
				else if (c == '%') {
					type = Node::Type::Modulo;
				}
				else if (c == '^') {
					type = Node::Type::Power;
				}
				add_operator(type);
			}
			else if (c == '(') {
				open_brackets();
			}
			else if (c == ')') {
				close_brackets();
			}
			else if (c == '$') {

				auto var_name_len = get_var_name_len(reader.c_str_from_cur_pos());
				add_variable(string(reader.c_str_from_cur_pos(), var_name_len));
				reader.advance(var_name_len);
			}
			else {
				cerr << "Invalid symbol: " << c << "\n";
				throw runtime_error("Invalid symbol");
			}
		}
	}

	ArithmeticExpression::~ArithmeticExpression() {
		if (root.has_value()) {
			delete root.value();
		}
	}

	ArithmeticExpression::Node::~Node() {
		if (left_child.has_value()) {
			delete left_child.value();
		}
		if (right_child.has_value()) {
			delete right_child.value();
		}
	}

}