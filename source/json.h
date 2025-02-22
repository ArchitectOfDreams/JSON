/*
Copyright (c) 2025 by Thomas E. Hankin III

Licensed for use under the terms of the MIT license.
See LICENSE.md for details.
 */

/**
 * \file json.h
 *
 * \brief Support for reading, writing and manipulating JSON objects and files
 */

#ifndef JSON_H
#define JSON_H

#if __cplusplus < 201402L
#error "Requires at least C++14"
#endif

#include <functional>
#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <vector>

/**
 * \brief Contains all library declarations
 */
namespace json {



/**
 * \brief Internal types for JSON values
 *
 * Types are categorized as *scalar* or *reference* types. Scalar-typed values
 * store data directly, while reference-typed values store internal pointers to
 * shared data.
 *
 * \sa Value
 * \sa Value::type()
 * \sa Value::isReference()
 */
enum class Type
{
	null,    ///< type for null values
	boolean, ///< type for boolean values
	number,  ///< type for numeric values
	string,  ///< type for string values
	array,   ///< type for array reference values
	object   ///< type for object reference values
};



class Value;

/**
 * \brief A resizable, sequential list of JSON values
 *
 * \sa Type::array
 */
typedef std::vector<Value> Array;

/**
 * \brief A collection of JSON properties represented as name-value pairs
 *
 * \sa Type::object
 */
typedef std::map<std::string, Value> Object;



/**
 * \brief A JSON value
 *
 * A flexible, hierarchical container for various types of data,
 * used as the object model for all the other classes in the library.
 *
 * \sa Type
 * \sa Formatter
 * \sa Parser
 */
class Value
{
	friend bool operator==(const Value& lhs, const Value& rhs);
	friend bool operator!=(const Value& lhs, const Value& rhs);

	friend std::ostream& operator<<(std::ostream& out, const Value& value);
	friend std::istream& operator>>(std::istream& in, Value& value);

public:
	/**
	 * \brief Constructs a null Value
	 *
	 * On return, `type()` returns `Type::null`
	 * and the value contains no data.
	 *
	 * \sa Value::isValid() const
	 */
	Value(std::nullptr_t = 0);

	/**
	 * \brief Constructs a boolean Value
	 *
	 * On return, `type()` returns `Type::boolean`
	 * and `asBoolean()` returns a copy of \p data.
	 *
	 * \param data The initial data
	 *
	 * \sa Value::asBoolean() const
	 */
	Value(bool data);

	/**
	 * \brief Constructs a numeric Value
	 *
	 * On return, `type()` returns `Type::number`
	 * and `asNumber()` returns a copy of \p data.
	 *
	 * \param data The initial data
	 *
	 * \sa Value::asNumber() const
	 */
	Value(double data);

	/**
	 * \brief Constructs a numeric Value
	 *
	 * On return, `type()` returns `Type::number`
	 * and `asNumber()` returns a copy of \p data cast to `double`.
	 *
	 * \param data The initial data
	 *
	 * \sa Value::asNumber() const
	 */
	Value(long data);

	/**
	 * \brief Constructs a numeric Value
	 *
	 * On return, `type()` returns `Type::number`
	 * and `asNumber()` returns a copy of \p data cast to `double`.
	 *
	 * \param data The initial data
	 *
	 * \sa Value::asNumber() const
	 */
	Value(int data);

	/**
	 * \brief Constructs a string Value
	 *
	 * On return, `type()` returns `Type::string`
	 * and `asString()` returns a copy of \p data.
	 *
	 * \param data The initial data
	 *
	 * \sa Value::asString() const
	 */
	Value(std::string data);

	/**
	 * \brief Constructs a string Value
	 *
	 * On return, `type()` returns `Type::string`
	 * and `asString()` returns `std::string(1, data)`.
	 *
	 * \param data The initial data
	 *
	 * \sa Value::asString() const
	 */
	Value(char data);

	/**
	 * \brief Constructs a string Value
	 *
	 * On return, `type()` returns `Type::string`
	 * and `asString()` returns `std::string(data)`.
	 *
	 * \param data The initial data
	 *
	 * \sa Value::asString() const
	 */
	Value(const char* data);

	/**
	 * \brief Constructs an array reference Value
	 *
	 * On return, `type()` returns `Type::array`
	 * and `asArray()` returns a reference to a shared copy of \p data.
	 *
	 * \param data The initial data
	 *
	 * \sa Value::isReference() const
	 * \sa Value::asArray() const
	 */
	Value(Array data);

	/**
	 * \brief Constructs an object reference Value
	 *
	 * On return, `type()` returns `Type::object`
	 * and `asObject()` returns a reference to a shared copy of \p data.
	 *
	 * \param data The initial data
	 *
	 * \sa Value::isReference() const
	 * \sa Value::asObject() const
	 */
	Value(Object data);

	/**
	 * \brief Copies a Value
	 *
	 * If \p that is a reference-type value, creates a new reference
	 * to the shared data. Otherwise, copies the data itself.
	 *
	 * On return, the expression `*this == that` evaluates as true.
	 *
	 * \param that The value to copy
	 *
	 * \sa Value::isReference() const
	 */
	Value(const Value& that);

	/**
	 * \brief Moves a Value
	 *
	 * Transfers the data from the temporary value \p that without copying
	 * it. On return, \p that is in a defined but invalid state, and should
	 * not be used in expressions.
	 *
	 * If \p that is a reference-type value, only the reference is
	 * transferred; the referenced data is not affected.
	 *
	 * \param that The value to move
	 *
	 * \sa Value::isReference() const
	 */
	Value(Value&& that);


	/**
	 * \brief Destroys this Value
	 *
	 * If this is a reference-type value, also destroys the referenced data
	 * if this is the last reference to it.
	 *
	 * \sa Value::isReference() const
	 */
	~Value() noexcept;


	/**
	 * \brief Copies data into this Value
	 *
	 * If \p that is a reference-type value, creates a new reference
	 * to the shared data. Otherwise, copies the data itself.
	 *
	 * On return, the expression `*this == that` evaluates as true.
	 *
	 * \param that The value to copy
	 * \return `*this`
	 *
	 * \sa Value::isReference() const
	 */
	Value& operator=(const Value& that);

	/**
	 * \brief Moves data into this Value
	 *
	 * Transfers the data from the temporary value \p that without copying
	 * it. On return, \p that is in a defined but invalid state, and should
	 * not be used in expressions.
	 *
	 * If \p that is a reference-type value, only the reference is
	 * transferred; the referenced data is not affected.
	 *
	 * \param that The value to move
	 * \return `*this`
	 *
	 * \sa Value::isReference() const
	 */
	Value& operator=(Value&& that);


	/**
	 * \brief Reports this Value's type
	 *
	 * \return The internal type descriptor
	 */
	Type type() const noexcept;

	/**
	 * \brief Indicates if this Value is non-null
	 *
	 * \return true if `type()` is not `Type::null`
	 *
	 * \sa Value(std::nullptr_t)
	 */
	bool isValid() const noexcept;

	/**
	 * \brief Synonym for `isValid() const`
	 *
	 * \return `this->isValid()`
	 */
	explicit operator bool() const noexcept;

	/**
	 * \brief Indicates if this Value has reference type
	 *
	 * \return true if `type()` is `Type::array` or `Type::object`.
	 *
	 * \sa Value(Array)
	 * \sa Value(Object)
	 */
	bool isReference() const noexcept;


	/**
	 * \brief Compares this Value to another
	 *
	 * Compares first by `type()`, then by data for scalar-typed values
	 * or by reference for reference-typed values.
	 *
	 * \param that The value to compare with this
	 * \return The result of the comparison
	 *
	 * \sa Value::isReference() const
	 */
	bool equals(const Value& that) const noexcept;


	/**
	 * \brief Accesses this Value's boolean data
	 *
	 * \return The stored data
	 * \exception TypeError if `type()` is not `Type::boolean`
	 *
	 * \sa Value(bool)
	 */
	bool asBoolean() const;

	/**
	 * \brief Accesses this Value's numeric data
	 *
	 * \return The stored data
	 * \exception TypeError if `type()` is not `Type::number`
	 *
	 * \sa Value(double)
	 * \sa Value(long)
	 * \sa Value(int)
	 */
	double asNumber() const;

	/**
	 * \brief Accesses this Value's string data
	 *
	 * \return The stored data
	 * \exception TypeError if `type()` is not `Type::string`
	 *
	 * \sa Value(std::string)
	 * \sa Value(char)
	 * \sa Value(const char*)
	 */
	std::string asString() const;

	/**
	 * \brief Accesses this Value's array data
	 *
	 * \return The referenced data
	 * \exception TypeError if `type()` is not `Type::array`
	 *
	 * \sa Value(Array)
	 * \sa Value::arrayLength() const
	 * \sa Value::element(size_t) const
	 */
	Array& asArray() const;

	/**
	 * \brief Accesses this Value's object data
	 *
	 * \return The referenced data
	 * \exception TypeError if `type()` is not `Type::object`
	 *
	 * \sa Value(Object)
	 * \sa Value::hasProperty(const std::string&) const
	 * \sa Value::property(const std::string&) const
	 */
	Object& asObject() const;


	/**
	 * \brief Reports the number of elements in the array
	 * referenced by this Value
	 *
	 * If `type()` is `Type::array`, returns the number of elements
	 * in the referenced array. Otherwise, returns 0.
	 *
	 * \return The number of elements in the array, or 0
	 *
	 * \sa Array
	 * \sa Value(Array)
	 * \sa Value::asArray() const
	 * \sa Value::element(size_t) const
	 */
	size_t arrayLength() const noexcept;

	/**
	 * \brief Accesses an element of the array referenced by this Value
	 *
	 * If \p index is out-of-bounds and less than `SIZE_MAX`, the array
	 * is automatically expanded to length \p index + 1 to accomodate.
	 * The extra elements are default-initialized to null values.
	 *
	 * \param index The index of the requested array element
	 * \return A reference to the array element at \p index
	 * \exception TypeError if `type()` is not `Type::array`
	 * \exception std::length_error if thrown by Array's `resize()` method
	 * \exception std::out_of_range for \p index not less than `SIZE_MAX`
	 *
	 * \sa Array
	 * \sa Value(Array)
	 * \sa Value::asArray() const
	 * \sa Value::arrayLength() const
	 */
	Value& element(size_t index) const;
	/**
	 * \brief Synonym for `element(size_t) const`
	 *
	 * \param index The index of the requested array element
	 * \return `this->element(index)`
	 */
	Value& operator[](size_t index) const;


	/**
	 * \brief Checks if the object referenced by this Value
	 * contains the requested property
	 *
	 * If `type()` is `Type::object`, checks whether the referenced object
	 * contains a property mapped to \p name. Otherwise, returns false.
	 *
	 * \param name The name of the requested property
	 * \return true if the property was found
	 *
	 * \sa Object
	 * \sa Value(Object)
	 * \sa Value::asObject() const
	 * \sa Value::property(const std::string&) const
	 */
	bool hasProperty(const std::string& name) const noexcept;

	/**
	 * \brief Accesses a property of the object referenced by this Value
	 *
	 * If the property specified by \p name does not exist, it is
	 * automatically added to the object and default-initialized to
	 * a null value.
	 *
	 * \param name The name of the requested property.
	 * Names are case-sensitive, and unique within a single object.
	 * An empty string is permitted
	 * \return A reference to the object property mapped to \p name
	 * \exception TypeError if `type()` is not `Type::object`
	 *
	 * \sa Object
	 * \sa Value(Object)
	 * \sa Value::asObject() const
	 * \sa Value::hasProperty(const std::string&) const
	 */
	Value& property(const std::string& name) const;

	/**
	 * \brief Synonym for `property(const std::string&) const`
	 *
	 * \param name The name of the requested property
	 * \return `this->property(name)`
	 */
	Value& operator[](const std::string& name) const;

private:
	struct State;
	std::unique_ptr<State> state;
};



/**
 * \brief Compares two values as equal
 *
 * The comparison is done in terms of `Value::equals(const Value&) const`.
 *
 * \param lhs The first value to compare
 * \param rhs The second value to compare
 * \return `lhs.equals(rhs)`
 */
bool operator==(const Value& lhs, const Value& rhs);

/**
 * \brief Compares two values as inequal
 *
 * The comparison is done in terms of `Value::equals(const Value&) const`.
 *
 * \param lhs The first value to compare
 * \param rhs The second value to compare
 * \return `!lhs.equals(rhs)`
 */
bool operator!=(const Value& lhs, const Value& rhs);


/**
 * \brief Outputs a Value to a stream
 *
 * Prints \p value to \p out using an instance of Formatter.
 *
 * \param out The destination stream
 * \param value The value to print
 * \return \p out
 */
std::ostream& operator<<(std::ostream& out, const Value& value);

/**
 * \brief Inputs a Value from a stream
 *
 * Parses \p in using an instance of Parser and stores the result in \p value.
 *
 * If an error occurs while parsing, sets `std::ios_base::failbit`
 * on the stream.
 *
 * \param in The source stream
 * \param [out] value Receives the parsed object
 * \return \p in
 */
std::istream& operator>>(std::istream& in, Value& value);



/**
 * \brief Formatted printing for JSON values
 *
 * A %Formatter prints values to a character stream using the same
 * representation that is expected by the Parser. Additionally, it
 * provides support for multiline layouts via configuration flags.
 *
 * \sa Value
 * \sa operator<<(std::ostream&, const Value&)
 */
struct Formatter
{
	/**
	 * \brief Flags to configure the behavior of a Formatter
	 */
	enum Flags : unsigned
	{
		multiline = 1, ///< Print across multiple lines
		indented  = 2  ///< Print with left indentation
			/**< Requires `Formatter::multiline`. */
	};


	/**
	 * \brief The configuration flags for this Formatter
	 */
	unsigned flags;

	/**
	 * \brief The recursion level for this Formatter
	 */
	size_t level;


	/**
	 * \brief Constructs a Formatter state
	 *
	 * \param flags Initial value for `Formatter::flags`
	 * \param level Initial value for `Formatter::level`
	 *
	 * \sa Flags
	 */
	explicit Formatter(
		unsigned flags = 0,
		size_t level = 0
	);


	/**
	 * \brief Prints a JSON value to a stream
	 *
	 * \param out The destination stream
	 * \param value The value to print
	 * \return \p out
	 */
	std::ostream& print(std::ostream& out, const Value& value) const;

	/**
	 * \brief Prints a JSON array to a stream
	 *
	 * \param out The destination stream
	 * \param array The array to print
	 * \return \p out
	 */
	std::ostream& print(std::ostream& out, const Array& array) const;

	/**
	 * \brief Prints a JSON object to a stream
	 *
	 * \param out The destination stream
	 * \param object The object to print
	 * \return \p out
	 */
	std::ostream& print(std::ostream& out, const Object& object) const;
};



/**
 * \brief Stream-based parsing for JSON values
 *
 * See https://json.org/ for a description of the JSON grammar.
 *
 * \sa Value
 * \sa operator>>(std::istream&, Value&)
 * \sa Formatter
 */
class Parser
{
public:
	/**
	 * \brief Constructs a default Parser state
	 */
	Parser();


	/**
	 * \brief Destroys this Parser state
	 */
	~Parser() noexcept;


	/**
	 * \brief Indicates whether logging is turned on for this Parser
	 *
	 * \return The current logging setting
	 *
	 * \sa Parser::setLogging(bool)
	 */
	bool logging() const;

	/**
	 * \brief Turns logging on or off for this Parser
	 *
	 * \param logging The new logging setting
	 *
	 * \sa Parser::logging() const
	 */
	void setLogging(bool logging);


	/**
	 * \brief Parses a JSON object from a stream
	 *
	 * \param in The source stream
	 * \return The parsed object
	 * \exception SyntaxError on encountering missing or incorrect syntax
	 * \exception ConversionError on encountering a malformed
	 * string or number token
	 */
	Value parseObject(std::istream& in);


	/**
	 * \brief Accesses the character buffer for this Parser
	 *
	 * The buffer contains all characters read by the parser since its
	 * initialization or the last call to `clearBuffer()`.
	 *
	 * \return The internal character buffer
	 */
	const std::string& buffer() const;

	/**
	 * \brief Empties the character buffer for this Parser
	 *
	 * \sa Parser::buffer() const
	 */
	void clearBuffer();

	/**
	 * \brief Reports the line count for this Parser
	 *
	 * The line count is the total number of line breaks encountered
	 * by the parser since its initialization.
	 *
	 * \return The internal line count
	 */
	size_t lines() const;

private:
	typedef std::function<bool(Parser&, std::istream&)> Rule;
	typedef std::map<std::string, Rule> RuleMap;
	typedef std::function<bool(int)> CharClass;

	struct State;
	std::unique_ptr<State> state;

	static RuleMap initialRules();

	static bool letter(int);
	static bool digit(int);
	static bool n0digit(int);
	static bool xdigit(int);
	static bool space(int);
	static bool strchar(int);

	void getc(std::istream&);
	bool read(int what, std::istream&);
	bool read(CharClass, std::istream&);

	Rule rule(const std::string& id) const;
	bool parse(const std::string& id, std::istream&);

	Value result() const;
	Value& result();

	Value value() const;
	Value& value();
	void pushValue(Value);
	void popValue();

	std::string name() const;
	std::string& name();
	void pushName(std::string);
	std::string popName();

	bool beginToken(std::istream&);
	bool pushObject(std::istream&);
	bool pushArray(std::istream&);
	bool finishCompound(std::istream&);
	bool compileName(std::istream&);
	bool compileProperty(std::istream&);
	bool compileArrayElement(std::istream&);
	bool compileCompound(std::istream&);
	bool compileString(std::istream&);
	bool compileNumber(std::istream&);
	bool compileSymbol(std::istream&);

	friend class bnf_generators;
};



/**
 * \brief Exception thrown by Value's accessor methods when used on a value
 * with an incompatible type
 */
class TypeError : public std::runtime_error
{
public:
	using runtime_error::runtime_error;
};


/**
 * \brief Parent type for exceptions thrown by the Parser
 */
class ParserError : public std::runtime_error
{
public:
	using runtime_error::runtime_error;
};


/**
 * \brief Exception thrown by the Parser on encountering
 * syntactically invalid source
 */
class SyntaxError : public ParserError
{
public:
	using ParserError::ParserError;
};


/**
 * \brief Exception thrown by the Parser on encountering
 * a malformed number or string
 */
class ConversionError : public ParserError
{
public:
	using ParserError::ParserError;
};



} // namespace json

#endif
