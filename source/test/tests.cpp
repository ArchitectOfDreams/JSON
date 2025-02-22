/*
Copyright (c) 2025 by Thomas E. Hankin III

Licensed for use under the terms of the MIT license.
See LICENSE.md for details.
 */

#undef NDEBUG

#include <cassert>
#include <sstream>
#include <string>
#include "json.h"

using namespace json;



static std::string to_string(const Value& value)
{
	std::stringstream ss;
	Formatter().print(ss, value);
	return ss.str();
}

static Value parseString(const std::string& source)
{
	std::stringstream ss(source);
	Parser parser;
	// parser.setLogging(true);
	return parser.parseObject(ss);
}



int main()
{
	const std::string s("ABC");
	const char *s2 = "xyz";
	const Array a = {"dog", "cat", "bird", "fish"};
	const Object o = {
		{"first_name", "John"},
		{"last_name", "Smith"},
		{"age", 25},
	};
	Value test, test2;

	// Initialization
	assert(Value(false).asBoolean() == false);
	assert(Value(true).asBoolean() == true);
	assert((int)Value(-1).asNumber() == -1);
	assert((long)Value(123456l).asNumber() == 123456l);
	assert(Value(0).asNumber() == 0);
	assert(Value(0.01).asNumber() == 0.01);
	assert(Value(3.14).asNumber() == 3.14);
	assert(Value(s).asString() == s);
	assert(Value('?').asString() == std::string(1, '?'));
	assert(Value(s2).asString() == std::string(s2));
	assert(Value(a).asArray() == a);
	assert(Value(o).asObject() == o);
	assert(!Value(nullptr));

	// Assignment
	assert((test = false).asBoolean() == false);
	assert((test = true).asBoolean() == true);
	assert((int)(test = -1).asNumber() == -1);
	assert((long)(test = 123456l).asNumber() == 123456l);
	assert((test = 0).asNumber() == 0);
	assert((test = 0.01).asNumber() == 0.01);
	assert((test = 3.14).asNumber() == 3.14);
	assert((test = s).asString() == s);
	assert((test = '?').asString() == std::string(1, '?'));
	assert((test = s2).asString() == std::string(s2));
	assert((test = a).asArray() == a);
	assert((test = o).asObject() == o);
	assert(!(test = nullptr));

	// Comparison
	assert(((test = false, test2 = test), test == test2 && test2 == test));
	assert(((test = false, test2 = true), !(test == test2 || test2 == test)));
	assert(((test = false, test2 = true), test != test2 && test2 != test));
	assert(((test = false, test2 = test), !(test != test2 || test2 != test)));
	assert(((test = 1, test2 = test), test == test2 && test2 == test));
	assert(((test = 1, test2 = 1.1), !(test == test2 || test2 == test)));
	assert(((test = 1, test2 = 1.1), test != test2 && test2 != test));
	assert(((test = 1, test2 = test), !(test != test2 || test2 != test)));
	assert(((test = s, test2 = test), test == test2 && test2 == test));
	assert(((test = s, test2 = s2), !(test == test2 || test2 == test)));
	assert(((test = s, test2 = s2), test != test2 && test2 != test));
	assert(((test = s, test2 = test), !(test != test2 || test2 != test)));
	assert(((test = a, test2 = test), test == test2 && test2 == test));
	assert(((test = a, test2 = a), !(test == test2 || test2 == test)));
	assert(((test = a, test2 = a), test != test2 && test2 != test));
	assert(((test = a, test2 = test), !(test != test2 || test2 != test)));
	assert(((test = o, test2 = test), test == test2 && test2 == test));
	assert(((test = o, test2 = o), !(test == test2 || test2 == test)));
	assert(((test = o, test2 = o), test != test2 && test2 != test));
	assert(((test = o, test2 = test), !(test != test2 || test2 != test)));
	assert(((test = nullptr, test2 = test), test == test2 && test2 == test));
	assert(((test = nullptr, test2 = 0), !(test == test2 || test2 == test)));
	assert(((test = nullptr, test2 = 0), test != test2 && test2 != test));
	assert(((test = nullptr, test2 = test), !(test != test2 || test2 != test)));

	// Formatting
	assert(to_string(Value(nullptr)) == std::string("null"));
	assert(to_string(Value(true)) == std::string("true"));
	assert(to_string(Value(false)) == std::string("false"));
	assert(to_string(Value(0)) == std::string("0"));
	assert(to_string(Value(3.14)) == std::string("3.14"));
	assert(to_string(Value("test")) == std::string("\"test\""));
	assert(to_string(Value(Array{})) == std::string("[]"));
	assert(to_string(Value(Array{1, 2, 3})) == std::string("[1, 2, 3]"));
	assert(to_string(Value(Object{})) == std::string("{}"));
	assert(to_string(Value(Object{{"test", true}})) == std::string("{\"test\": true}"));

	// Parsing
	assert(parseString("{ \"null_prop\": null }")["null_prop"] == Value(nullptr));
	assert(parseString("{ \"boolean_prop\": false }")["boolean_prop"] == Value(false));
	assert(parseString("{ \"numeric_prop\": 3.14 }")["numeric_prop"] == Value(3.14));
	assert(parseString("{ \"string_prop\": \"line 1\\nline 2\" }")["string_prop"] == Value("line 1\nline 2"));
	assert(parseString("{ \"array_prop\": [0, -5, -10] }")["array_prop"][2] == Value(-10));
	assert(parseString("{ \"object_prop\": {\"test\": true} }")["object_prop"]["test"] == Value(true));

	return 0;
}
