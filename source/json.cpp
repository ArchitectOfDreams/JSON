/*
Copyright (c) 2025 by Thomas E. Hankin III

Licensed for use under the terms of the MIT license.
See LICENSE.txt for details.
 */

#include <iomanip>
#include <iostream>
#include <sstream>
#include <stack>
#include "json.h"

namespace json {



/*
 * TODO: transition to using std::variant or std::any ?
 * (requires C++17)
 */
struct ValueImplBase
{
	virtual ~ValueImplBase() noexcept = default;

	virtual bool equals(const ValueImplBase&) const noexcept
	{
		return false;
	}
};

typedef std::shared_ptr<ValueImplBase> ValueImplOwner;



template<typename T>
struct ValueImpl : ValueImplBase
{
	T value;

	explicit ValueImpl(T&& value):
		value(std::move(value))
	{}

	virtual bool equals(const ValueImplBase& impl) const noexcept override
	{
		auto that = dynamic_cast<const ValueImpl<T>*>(&impl);
		return that && this->value == that->value;
	}
};

template<typename T>
static std::shared_ptr<ValueImpl<T>> makeImpl(T&& value)
{
	return std::make_shared<ValueImpl<T>>(std::move(value));
}

template<typename T>
static ValueImpl<T>& implAs(ValueImplBase& base)
{
	auto impl = dynamic_cast<ValueImpl<T>*>(&base);
	if (!impl) throw TypeError("invalid type conversion");
	return *impl;
}
template<typename T>
static T& implValueAs(ValueImplBase& base)
{
	return implAs<T>(base).value;
}



struct Value::State
{
	Type type;
	ValueImplOwner impl;

	explicit State(
		Type type,
		ValueImplOwner&& impl
	):
		type(type),
		impl(std::move(impl))
	{}
};

Value::Value(std::nullptr_t):
	state(std::make_unique<State>(
		Type::null,
		std::make_shared<ValueImplBase>()))
{}
Value::Value(bool value):
	state(std::make_unique<State>(
		Type::boolean,
		makeImpl(std::move(value))))
{}
Value::Value(double value):
	state(std::make_unique<State>(
		Type::number,
		makeImpl(std::move(value))))
{}
Value::Value(long value):
	Value(double(value))
{}
Value::Value(int value):
	Value(double(value))
{}
Value::Value(std::string value):
	state(std::make_unique<State>(
		Type::string,
		makeImpl(std::move(value))))
{}
Value::Value(char value):
	Value(std::string(1, value))
{}
Value::Value(const char* value):
	Value(std::string(value))
{}
Value::Value(Array value):
	state(std::make_unique<State>(
		Type::array,
		makeImpl(std::make_unique<Array>(std::move(value)))))
{}
Value::Value(Object value):
	state(std::make_unique<State>(
		Type::object,
		makeImpl(std::make_unique<Object>(std::move(value)))))
{}
Value::Value(const Value& that):
	state(std::make_unique<State>(*that.state))
{}
Value::Value(Value&&) = default;

Value::~Value() noexcept = default;

Value& Value::operator=(const Value& that)
{
	return *this = Value(that);
}
Value& Value::operator=(Value&&) = default;

Type Value::type() const noexcept
{
	return state->type;
}
bool Value::isValid() const noexcept
{
	return type() != Type::null;
}
Value::operator bool() const noexcept
{
	return isValid();
}
bool Value::isReference() const noexcept
{
	return type() == Type::array || type() == Type::object;
}

bool Value::equals(const Value& that) const noexcept
{
	return
		(this->type() == Type::null && that.type() == Type::null)
		|| (this->type() == that.type() && this->state->impl->equals(*that.state->impl));
}

bool Value::asBoolean() const
{
	return implValueAs<bool>(*state->impl);
}
double Value::asNumber() const
{
	return implValueAs<double>(*state->impl);
}
std::string Value::asString() const
{
	return implValueAs<std::string>(*state->impl);
}
Array& Value::asArray() const
{
	return *implValueAs<std::unique_ptr<Array>>(*state->impl);
}
Object& Value::asObject() const
{
	return *implValueAs<std::unique_ptr<Object>>(*state->impl);
}

size_t Value::arrayLength() const noexcept
{
	return (type() == Type::array) ? asArray().size() : 0;
}
Value& Value::element(size_t index) const
{
	if (index >= SIZE_MAX)
		throw std::out_of_range("index >= SIZE_MAX");
	Array &array = asArray();
	if (index >= array.size())
		array.resize(index + 1);
	return array[index];
}
Value& Value::operator[](size_t index) const
{
	return element(index);
}

bool Value::hasProperty(const std::string& name) const noexcept
{
	return type() == Type::object && asObject().count(name);
}
Value& Value::property(const std::string& name) const
{
	return asObject()[name];
}
Value& Value::operator[](const std::string& name) const
{
	return property(name);
}

bool operator==(const Value& lhs, const Value& rhs)
{
	return lhs.equals(rhs);
}
bool operator!=(const Value& lhs, const Value& rhs)
{
	return !lhs.equals(rhs);
}

std::ostream& operator<<(std::ostream& out, const Value& value)
{
	return Formatter().print(out, value);
}
std::istream& operator>>(std::istream& in, Value& value)
{
	try
	{
		value = Parser().parseObject(in);
	}
	catch (ParserError&)
	{
		value = {};
		in.setstate(std::ios_base::failbit);
	}
	return in;
}



Formatter::Formatter(unsigned flags, size_t level):
	flags(flags),
	level(level)
{}

std::ostream& Formatter::print(std::ostream& out, const Value& value) const
{
	switch (value.type())
	{
	default:
	case Type::null:
		return out << "null";
	case Type::boolean:
		return out << std::boolalpha << value.asBoolean();
	case Type::number:
		return out << value.asNumber();
	case Type::string:
		return out << std::quoted(value.asString());
	case Type::array:
		return print(out, value.asArray());
	case Type::object:
		return print(out, value.asObject());
	}
}
std::ostream& Formatter::print(std::ostream& out, const Array& array) const
{
	bool multiline = (flags & Formatter::multiline);
	bool indented = multiline && (flags & Formatter::indented);
	out << '[';
	if (multiline) out << '\n';
	for (const Value &value : array)
	{
		if (&value != &array[0])
			out << ',' << (multiline ? '\n' : ' ');
		if (indented) out << std::string(level + 1, '\t');
		Formatter(flags, level + 1).print(out, value);
	}
	if (multiline) out << '\n';
	if (indented) out << std::string(level, '\t');
	return out << ']';
}
std::ostream& Formatter::print(std::ostream& out, const Object& object) const
{
	bool multiline = (flags & Formatter::multiline);
	bool indented = multiline && (flags & Formatter::indented);
	out << '{';
	if (multiline) out << '\n';
	for (const auto &entry : object)
	{
		if (entry.first != object.begin()->first)
			out << ',' << (multiline ? '\n' : ' ');
		if (indented) out << std::string(level + 1, '\t');
		out << std::quoted(entry.first) << ':' << ' ';
		Formatter(flags, level + 1).print(out, entry.second);
	}
	if (multiline) out << '\n';
	if (indented) out << std::string(level, '\t');
	return out << '}';
}



class bnf_generators
{
public:
	typedef Parser::Rule Rule;
	typedef Parser::CharClass CharClass;

	static Rule terminal(char);
	static Rule terminal(CharClass);
	static Rule nonterminal(std::string);
	static Rule require(Rule);
	static Rule optional(Rule);
	static Rule multiple(Rule);
	static Rule both(Rule, Rule);
	static Rule either(Rule, Rule);
};

bnf_generators::Rule bnf_generators::terminal(char what)
{
	return [what](Parser& parser, std::istream& in) -> bool
	{
		return parser.read(what, in);
	};
}
bnf_generators::Rule bnf_generators::terminal(CharClass filter)
{
	return [filter](Parser& parser, std::istream& in) -> bool
	{
		return parser.read(filter, in);
	};
}
bnf_generators::Rule bnf_generators::nonterminal(std::string id)
{
	return [id](Parser& parser, std::istream& in) -> bool
	{
		return parser.parse(id, in);
	};
}
bnf_generators::Rule bnf_generators::require(Rule rule)
{
	return rule ?
		[rule](Parser& parser, std::istream& in) -> bool
		{
			return rule(parser, in) || (throw SyntaxError("syntax error"), false);
		}
		: rule;
}
bnf_generators::Rule bnf_generators::optional(Rule rule)
{
	return rule ?
		[rule](Parser& parser, std::istream& in) -> bool
		{
			return rule(parser, in) || true;
		}
		: rule;
}
bnf_generators::Rule bnf_generators::multiple(Rule rule)
{
	return rule ?
		[rule](Parser& parser, std::istream& in) -> bool
		{
			while (in && rule(parser, in))
			{}
			return true;
		}
		: rule;
}
bnf_generators::Rule bnf_generators::both(Rule first, Rule second)
{
	return (first && second) ?
		[first, second](Parser& parser, std::istream& in) -> bool
		{
			return first(parser, in) && second(parser, in);
		}
		: first;
}
bnf_generators::Rule bnf_generators::either(Rule first, Rule second)
{
	return (first && second) ?
		[first, second](Parser& parser, std::istream& in) -> bool
		{
			return first(parser, in) || second(parser, in);
		}
		: first;
}

namespace bnf_symbols {

typedef bnf_generators::Rule Rule;

static Rule operator""_t(char what)
{
	return bnf_generators::terminal(what);
}
static Rule operator""_nt(const char* str, size_t len)
{
	return bnf_generators::nonterminal(std::string(str, len));
}
static Rule operator~(Rule rule)
{
	return bnf_generators::optional(rule);
}
static Rule operator*(Rule rule)
{
	return bnf_generators::multiple(rule);
}
static Rule operator+(Rule first, Rule second)
{
	return bnf_generators::both(first, second);
}
static Rule operator&(Rule first, Rule second)
{
	return bnf_generators::both(first, bnf_generators::require(second));
}
static Rule operator|(Rule first, Rule second)
{
	return bnf_generators::either(first, second);
}

} // namespace bnf_symbols



struct status_t { const Parser &parser; };
static status_t status(const Parser& parser)
{
	return {parser};
}
static std::ostream& operator<<(std::ostream& out, const status_t& status)
{
	const Parser &parser = status.parser;
	return out << "line:" << (parser.lines() + 1)
		<< ", buffer:" << std::quoted(parser.buffer());
}



struct Parser::State
{
	RuleMap rules;
	std::string buffer;
	size_t lines;
	Value result;
	std::stack<std::string> nameStack;
	std::stack<Value> valueStack;
	bool logging;

	explicit State(RuleMap&& rules, bool logging = false):
		rules(rules),
		lines(0),
		logging(logging)
	{}
};

Parser::Parser():
	state(std::make_unique<State>(initialRules()))
{}

Parser::~Parser() noexcept = default;

bool Parser::logging() const
{
	return state->logging;
}
void Parser::setLogging(bool logging)
{
	state->logging = logging;
}

Value Parser::parseObject(std::istream& in)
{
	result() = {};
	Rule rule = bnf_generators::require(bnf_generators::nonterminal("object"));
	rule(*this, in);
	return result();
}

const std::string& Parser::buffer() const
{
	return state->buffer;
}
void Parser::clearBuffer()
{
	state->buffer.clear();
}
size_t Parser::lines() const
{
	return state->lines;
}

Parser::RuleMap Parser::initialRules()
{
	using namespace bnf_symbols;

	return {
		{"object",
			Rule('{'_t & ("property_list"_nt | "whitespace"_nt) & '}'_t)},
		{"property_list",
			std::mem_fn(Parser::pushObject)
			+ Rule("property"_nt & *(','_t & "property"_nt))
			+ std::mem_fn(Parser::compileCompound)
			+ std::mem_fn(Parser::finishCompound)},
		{"property",
			Rule("name"_nt & ':'_t & "value"_nt)
			+ std::mem_fn(Parser::compileProperty)},
		{"name",
			Rule("whitespace"_nt + "string"_nt + "whitespace"_nt)
			+ std::mem_fn(Parser::compileName)},
		{"array",
			Rule('['_t & ("array_element_list"_nt | "whitespace"_nt) & ']'_t)},
		{"array_element_list",
			std::mem_fn(Parser::pushArray)
			+ Rule("array_element"_nt & *(','_t & "array_element"_nt))
			+ std::mem_fn(Parser::compileCompound)
			+ std::mem_fn(Parser::finishCompound)},
		{"array_element",
			Rule("value"_nt)
			+ std::mem_fn(Parser::compileArrayElement)},
		{"value",
			Rule("whitespace"_nt + (
				"object"_nt | "array"_nt | "string"_nt | "number"_nt | "symbol"_nt
			) + "whitespace"_nt)},
		{"symbol",
			std::mem_fn(Parser::beginToken)
			+ Rule("symbol_char"_nt & *("symbol_char"_nt))
			+ std::mem_fn(Parser::compileSymbol)},
		{"string",
			std::mem_fn(Parser::beginToken)
			+ Rule('"'_t & *("string_content"_nt) & '"'_t)
			+ std::mem_fn(Parser::compileString)},
		{"string_content",
			Rule("escape"_nt | "string_char"_nt)},
		{"escape",
			Rule('\\'_t & "escape_sequence"_nt)},
		{"escape_sequence",
			Rule('"'_t | '\\'_t | '/'_t | 'b'_t | 'f'_t | 'n'_t | 'r'_t | 't'_t | (
				'u'_t & "xdigit"_nt & "xdigit"_nt & "xdigit"_nt & "xdigit"_nt))},
		{"number",
			std::mem_fn(Parser::beginToken)
			+ Rule((
				('-'_t & "digit_sequence"_nt) | "digit_sequence"_nt
			) & ~("fraction"_nt) & ~("exponent"_nt))
			+ std::mem_fn(Parser::compileNumber)},
		{"digit_sequence",
			Rule('0'_t | "nonzero_sequence"_nt)},
		{"nonzero_sequence",
			Rule("nonzero_digit"_nt & *("digit"_nt))},
		{"fraction",
			Rule('.'_t & "digit"_nt & *("digit"_nt))},
		{"exponent",
			Rule(('E'_t | 'e'_t) & ~('+'_t | '-'_t) & "digit"_nt & *("digit"_nt))},
		{"whitespace",
			Rule(*("whitespace_char"_nt))},
		{"symbol_char",
			bnf_generators::terminal(Parser::letter)},
		{"digit",
			bnf_generators::terminal(Parser::digit)},
		{"nonzero_digit",
			bnf_generators::terminal(Parser::n0digit)},
		{"xdigit",
			bnf_generators::terminal(Parser::xdigit)},
		{"whitespace_char",
			bnf_generators::terminal(Parser::space)},
		{"string_char",
			bnf_generators::terminal(Parser::strchar)},
	};
}

bool Parser::letter(int c)
{
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}
bool Parser::digit(int c)
{
	return c >= '0' && c <= '9';
}
bool Parser::n0digit(int c)
{
	return c >= '1' && c <= '9';
}
bool Parser::xdigit(int c)
{
	return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}
bool Parser::space(int c)
{
	return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}
bool Parser::strchar(int c)
{
	return c >= 0x20 && c < 0x7f && c != '"' && c != '\\';
}

void Parser::getc(std::istream& in)
{
	int c = in.get();
	if (c < 0) return;
	state->buffer.push_back(char(c));
	if (c == '\n' || c == '\r')
		state->lines++;
}
bool Parser::read(int what, std::istream& in)
{
	bool success = (in.peek() == what);
	if (success) getc(in);
	return success;
}
bool Parser::read(CharClass filter, std::istream& in)
{
	bool success = filter && filter(in.peek());
	if (success) getc(in);
	return success;
}

Parser::Rule Parser::rule(const std::string& id) const
{
	return state->rules.at(id);
}
bool Parser::parse(const std::string& id, std::istream& in)
{
	if (logging())
	{
		std::clog << "parser[" << status(*this) << "]: "
			<< std::quoted(id) << "..."
			<< std::endl;
	}
	bool success = rule(id)(*this, in);
	if (logging())
	{
		std::clog << "parser[" << status(*this) << "]: "
			<< std::quoted(id) << ": " << std::boolalpha << success
			<< std::endl;
	}
	return success;
}

Value Parser::result() const
{
	return state->result;
}
Value& Parser::result()
{
	return state->result;
}

Value Parser::value() const
{
	return state->valueStack.top();
}
Value& Parser::value()
{
	return state->valueStack.top();
}
void Parser::pushValue(Value value)
{
	state->valueStack.push(std::move(value));
}
void Parser::popValue()
{
	state->valueStack.pop();
}

std::string Parser::name() const
{
	return state->nameStack.top();
}
std::string& Parser::name()
{
	return state->nameStack.top();
}
void Parser::pushName(std::string name)
{
	state->nameStack.push(std::move(name));
}
std::string Parser::popName()
{
	std::string name = std::move(state->nameStack.top());
	state->nameStack.pop();
	return std::move(name);
}

bool Parser::beginToken(std::istream&)
{
	clearBuffer();
	return true;
}
bool Parser::pushObject(std::istream&)
{
	pushValue(Object());
	return true;
}
bool Parser::pushArray(std::istream&)
{
	pushValue(Array());
	return true;
}
bool Parser::finishCompound(std::istream&)
{
	popValue();
	return true;
}
bool Parser::compileName(std::istream&)
{
	pushName(result().asString());
	return true;
}
bool Parser::compileProperty(std::istream&)
{
	value().asObject()[popName()] = result();
	return true;
}
bool Parser::compileArrayElement(std::istream&)
{
	value().asArray().push_back(result());
	return true;
}
bool Parser::compileCompound(std::istream&)
{
	result() = value();
	return true;
}
bool Parser::compileString(std::istream&)
{
	std::string source = buffer();
	size_t length = source.length();
	if (!(length >= 2 && source[0] == '"' && source[length - 1] == '"'))
		throw ConversionError("missing delimiter(s)");
	source = source.substr(1, length -= 2);
	std::string s;
	for (size_t i = 0; i < length; ++i)
	{
		char c = source[i];
		if (c == '\\')
		{
			switch (source[i + 1])
			{
			case '"' :
			case '\\':
			case '/' : c = source[++i]; break;

			case 'b': c = '\b'; ++i; break;
			case 'f': c = '\f'; ++i; break;
			case 'n': c = '\n'; ++i; break;
			case 'r': c = '\r'; ++i; break;
			case 't': c = '\t'; ++i; break;

			case 'u': break;

			default: throw ConversionError("invalid escape");
			}
		}
		s.push_back(c);
	}
	result() = std::move(s);
	return true;
}
bool Parser::compileNumber(std::istream&)
{
	try
	{
		result() = std::stod(buffer());
	}
	catch (std::invalid_argument& ex)
	{
		throw ConversionError(ex.what());
	}
	return true;
}
bool Parser::compileSymbol(std::istream&)
{
	const std::string &symbol = buffer();
	if (symbol == "true")
		result() = true;
	else if (symbol == "false")
		result() = false;
	else if (symbol == "null")
		result() = nullptr;
	else
		throw SyntaxError("unrecognized symbol '" + symbol + "'");
	return true;
}



} // namespace json
