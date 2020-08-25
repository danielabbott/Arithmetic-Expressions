#pragma once

#include <optional>
#include "Variable.h"
#include "StringReader.h"
#include <memory>
#include "ArithTypes.h"

namespace Arithmetic {

	struct ArithmeticExpression {
		struct Node {
			enum class Type {
				Add, Sub, Mul, Div, Power, Modulo, Brackets, Literal, Variable
			};
			Type type;

			bool is_operator() {
				return type == Type::Add || type == Type::Sub || type == Type::Mul || type == Type::Div || type == Type::Power || type == Type::Modulo;
			}

			FloatType literal_value = 0.0f;
			std::optional<std::string> variable_name = std::nullopt;

			std::optional<Node*> parent = std::nullopt;

			// Not used for literal
			std::optional<Node*> left_child = std::nullopt;

			// Not used for Brackets or Literal
			std::optional<Node*> right_child = std::nullopt;

			Node(Type type_, std::optional<Node*> parent_) : type(type_), parent(parent_) {}
			Node(FloatType value, std::optional<Node*> parent_) : type(Type::Literal), literal_value(value), parent(parent_) {}
			Node(std::string&& builtin_name, std::optional<Node*> parent_) : type(Type::Variable), variable_name(std::move(builtin_name)), parent(parent_) {}

			std::optional<FloatType> value_if_possible(VariableSet const&);

			~Node();

			Node(const Node& other) = delete;
			Node& operator=(const Node& other) = delete;

			Node(Node&& other) = delete;
			Node& operator=(Node&& other) = delete;
		};

		std::optional<Node*> root = std::nullopt;

		// Attempts to collapse the expression into a single literal
		std::optional<FloatType> value_if_possible(VariableSet const&);

		ArithmeticExpression() : reader(std::string()) {};

		// String pointer is moved forward to point to the character after the arithmetic expression
		ArithmeticExpression(std::string const&);

		auto string_length() {
			return reader.current_position();
		}

		~ArithmeticExpression();

		// TODO: Implement these
		ArithmeticExpression(const ArithmeticExpression& other) = delete;
		ArithmeticExpression& operator=(const ArithmeticExpression& other) = delete;

		ArithmeticExpression(ArithmeticExpression&& other) = default;
		ArithmeticExpression& operator=(ArithmeticExpression&& other) = default;


	private:
		StringReader reader;
		std::optional<Node*> working_node = std::nullopt;
		void add_digit_or_variable(Node* newNode);
		void add_digit(FloatType v);
		void add_variable(std::string&& name);
		void add_operator(Node::Type type);
		void open_brackets();
		void close_brackets();
	};

}