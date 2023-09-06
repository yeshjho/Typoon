#pragma once

#include <array>
#include <iostream>
#include <string_view>
#include <tuple>

/*
	Generates class serialization helper for specified type:

	namespace foo {
		struct Bar { int x; float y; bool z; };
	}

	JSON5_CLASS(foo::Bar, x, y, z)
*/
#define JSON5_CLASS(_Name, ...) \
	template <> struct json5::detail::class_wrapper<_Name> { \
		static constexpr const char* names = #__VA_ARGS__; \
		inline static auto make_named_tuple(_Name &out) noexcept { \
			return std::tuple( names, std::tie( _JSON5_CONCAT( _JSON5_PREFIX_OUT, ( __VA_ARGS__ ) ) ) ); \
		} \
		inline static auto make_named_tuple( const _Name &in ) noexcept { \
			return std::tuple( names, std::tie( _JSON5_CONCAT( _JSON5_PREFIX_IN, ( __VA_ARGS__ ) ) ) ); \
		} \
	};

/*
	Generates class serialization helper for specified type with inheritance:

	namespace foo {
		struct Base { std::string name; };
		struct Bar : Base { int x; float y; bool z; };
	}

	JSON5_CLASS(foo::Base, name)
	JSON5_CLASS_INHERIT(foo::Bar, foo::Base, x, y, z)
*/
#define JSON5_CLASS_INHERIT(_Name, _Base, ...) \
	template <> struct json5::detail::class_wrapper<_Name> { \
		static constexpr const char* names = #__VA_ARGS__; \
		inline static auto make_named_tuple(_Name &out) noexcept { \
			return std::tuple_cat( \
			                       json5::detail::class_wrapper<_Base>::make_named_tuple(out), \
			                       std::tuple(names, std::tie( _JSON5_CONCAT(_JSON5_PREFIX_OUT, (__VA_ARGS__)) ))); \
		} \
		inline static auto make_named_tuple(const _Name &in) noexcept { \
			return std::tuple_cat( \
			                       json5::detail::class_wrapper<_Base>::make_named_tuple(in), \
			                       std::tuple(names, std::tie( _JSON5_CONCAT(_JSON5_PREFIX_IN, (__VA_ARGS__)) ))); \
		} \
	};

/*
	Generates members serialization helper inside class:

	namespace foo {
		struct Bar {
			int x; float y; bool z;
			JSON5_MEMBERS(x, y, z)
		};
	}
*/
#define JSON5_MEMBERS(...) \
	constexpr auto make_named_tuple() noexcept { \
		return std::tuple((const char*)#__VA_ARGS__, std::tie( __VA_ARGS__ )); } \
	constexpr auto make_named_tuple() const noexcept { \
		return std::tuple((const char*)#__VA_ARGS__, std::tie( __VA_ARGS__ )); }

/*
	Generates members serialzation helper inside class with inheritance:

	namespace foo {
		struct Base {
			std::string name;
			JSON5_MEMBERS(name)
		};

		struct Bar : Base {
			int x; float y; bool z;
			JSON5_MEMBERS_INHERIT(Base, x, y, z)
		};
	}
*/
#define JSON5_MEMBERS_INHERIT(_Base, ...) \
	inline auto make_named_tuple() noexcept { \
		constexpr _Base base; \
        static constexpr std::string_view baseMemberNames = std::get<0>(json5::detail::class_wrapper<_Base>::make_named_tuple(base)); \
		static constexpr std::string_view comma = ","; \
		static constexpr std::string_view memberNames = #__VA_ARGS__; \
		static constexpr std::string_view names = json5::detail::join_v<baseMemberNames, comma, memberNames>; \
        return std::tuple{ names.data(), std::tuple_cat( \
            std::get<1>(json5::detail::class_wrapper<_Base>::make_named_tuple(*this)), \
            std::tie(__VA_ARGS__)) }; } \
	inline auto make_named_tuple() const noexcept { \
		constexpr _Base base; \
        static constexpr std::string_view baseMemberNames = std::get<0>(json5::detail::class_wrapper<_Base>::make_named_tuple(base)); \
		static constexpr std::string_view comma = ","; \
		static constexpr std::string_view memberNames = #__VA_ARGS__; \
		static constexpr std::string_view names = json5::detail::join_v<baseMemberNames, comma, memberNames>; \
        return std::tuple{ names.data(), std::tuple_cat( \
            std::get<1>(json5::detail::class_wrapper<_Base>::make_named_tuple(*this)), \
            std::tie(__VA_ARGS__)) }; } \

/*
	Generates enum wrapper:

	enum class MyEnum {
		One, Two, Three
	};

	JSON5_ENUM(MyEnum, One, Two, Three)
*/
#define JSON5_ENUM(_Name, ...) \
	template <> struct json5::detail::enum_table<_Name> : std::true_type { \
		using enum _Name; \
		static constexpr const char* names = #__VA_ARGS__; \
		static constexpr const _Name values[] = { __VA_ARGS__ }; };

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace json5 {

/* Forward declarations */
class builder;
class document;
class parser;
class value;

//---------------------------------------------------------------------------------------------------------------------
struct error final
{
	enum
	{
		none,               // no error
		invalid_root,       // document root is not an object or array
		unexpected_end,     // unexpected end of JSON data (end of stream, string or file)
		syntax_error,       // general parsing error
		invalid_literal,    // invalid literal, only "true", "false", "null" allowed
		invalid_escape_seq, // invalid or unsupported string escape \ sequence
		comma_expected,     // expected comma ','
		colon_expected,     // expected color ':'
		boolean_expected,   // expected boolean literal "true" or "false"
		number_expected,    // expected number
		string_expected,    // expected string "..."
		object_expected,    // expected object { ... }
		array_expected,     // expected array [ ... ]
		wrong_array_size,   // invalid number of array elements
		invalid_enum,       // invalid enum value or string (conversion failed)
		could_not_open,     // stream is not open
	};

	static constexpr const char *type_string[] =
	{
		"none", "invalid root", "unexpected end", "syntax error", "invalid literal",
		"invalid escape sequence", "comma expected", "colon expected", "boolean expected",
		"number expected", "string expected", "object expected", "array expected",
		"wrong array size", "invalid enum", "could not open stream",
	};
	
	int type = none;
	int line = 0;
	int column = 0;

	operator int() const noexcept { return type; }
};

//---------------------------------------------------------------------------------------------------------------------
struct writer_params
{
	// One level of indentation
	const char *indentation = "  ";

	// End of line string
	const char *eol = "\n";

	// Write all on single line, omit extra spaces
	bool compact = false;

	// Write regular JSON (don't use any JSON5 features)
	bool json_compatible = false;

	// Escape unicode characters in strings
	bool escape_unicode = false;

	// Custom user data pointer
	void *user_data = nullptr;
};

//---------------------------------------------------------------------------------------------------------------------
enum class value_type { null = 0, boolean, number, array, string, object };

} // namespace json5

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace json5::detail {

using string_offset = unsigned;

template <typename T> struct class_wrapper
{
	constexpr static auto make_named_tuple( T &in ) noexcept { return in.make_named_tuple(); }
	constexpr static auto make_named_tuple( const T &in ) noexcept { return in.make_named_tuple(); }
};

template <typename T> struct enum_table : std::false_type { };

class char_source
{
public:
	virtual ~char_source() = default;

	virtual int next() = 0;
	virtual int peek() = 0;
	virtual bool eof() const = 0;

	error make_error( int type ) const noexcept { return error{ type, _line, _column }; }

protected:
	int _line = 1;
	int _column = 1;
};

// Copied from https://stackoverflow.com/a/62823211
template <std::string_view const&... Strs>
struct join
{
	// Join all strings into a single std::array of chars
	static constexpr auto impl() noexcept
	{
		constexpr std::size_t len = (Strs.size() + ... + 0);
		std::array<char, len + 1> arr{};
		auto append = [i = 0, &arr](auto const& s) mutable {
			for (auto c : s) arr[i++] = c;
		};
		(append(Strs), ...);
		arr[len] = 0;
		return arr;
	}
	// Give the joined string static storage
	static constexpr auto arr = impl();
	// View as a std::string_view
	static constexpr std::string_view value {arr.data(), arr.size() - 1};
};
// Helper to get the value out
template <std::string_view const&... Strs>
static constexpr auto join_v = join<Strs...>::value;

} // namespace json5::detail

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Here be dragons... */

#define _JSON5_EXPAND(...) __VA_ARGS__
#define _JSON5_JOIN(X, Y) _JSON5_JOIN2(X, Y)
#define _JSON5_JOIN2(X, Y) X##Y
#define _JSON5_COUNT(...) _JSON5_EXPAND(_JSON5_COUNT2(__VA_ARGS__, \
                                        16, 15, 14, 13, 12, 11, 10, 9, \
                                        8, 7, 6, 5, 4, 3, 2, 1, ))

#define _JSON5_COUNT2(_, \
                      _16, _15, _14, _13, _12, _11, _10, _9, \
                      _8, _7, _6, _5, _4, _3, _2, _X, ...) _X

#define _JSON5_FIRST(...) _JSON5_EXPAND(_JSON5_FIRST2(__VA_ARGS__, ))
#define _JSON5_FIRST2(X,...) X
#define _JSON5_TAIL(...) _JSON5_EXPAND(_JSON5_TAIL2(__VA_ARGS__))
#define _JSON5_TAIL2(X,...) (__VA_ARGS__)

#define _JSON5_CONCAT(   _Prefix, _Args) _JSON5_JOIN(_JSON5_CONCAT_,_JSON5_COUNT _Args)(_Prefix, _Args)
#define _JSON5_CONCAT_1( _Prefix, _Args) _Prefix _Args
#define _JSON5_CONCAT_2( _Prefix, _Args) _Prefix(_JSON5_FIRST _Args),_JSON5_CONCAT_1( _Prefix,_JSON5_TAIL _Args)
#define _JSON5_CONCAT_3( _Prefix, _Args) _Prefix(_JSON5_FIRST _Args),_JSON5_CONCAT_2( _Prefix,_JSON5_TAIL _Args)
#define _JSON5_CONCAT_4( _Prefix, _Args) _Prefix(_JSON5_FIRST _Args),_JSON5_CONCAT_3( _Prefix,_JSON5_TAIL _Args)
#define _JSON5_CONCAT_5( _Prefix, _Args) _Prefix(_JSON5_FIRST _Args),_JSON5_CONCAT_4( _Prefix,_JSON5_TAIL _Args)
#define _JSON5_CONCAT_6( _Prefix, _Args) _Prefix(_JSON5_FIRST _Args),_JSON5_CONCAT_5( _Prefix,_JSON5_TAIL _Args)
#define _JSON5_CONCAT_7( _Prefix, _Args) _Prefix(_JSON5_FIRST _Args),_JSON5_CONCAT_6( _Prefix,_JSON5_TAIL _Args)
#define _JSON5_CONCAT_8( _Prefix, _Args) _Prefix(_JSON5_FIRST _Args),_JSON5_CONCAT_7( _Prefix,_JSON5_TAIL _Args)
#define _JSON5_CONCAT_9( _Prefix, _Args) _Prefix(_JSON5_FIRST _Args),_JSON5_CONCAT_8( _Prefix,_JSON5_TAIL _Args)
#define _JSON5_CONCAT_10(_Prefix, _Args) _Prefix(_JSON5_FIRST _Args),_JSON5_CONCAT_9( _Prefix,_JSON5_TAIL _Args)
#define _JSON5_CONCAT_11(_Prefix, _Args) _Prefix(_JSON5_FIRST _Args),_JSON5_CONCAT_10(_Prefix,_JSON5_TAIL _Args)
#define _JSON5_CONCAT_12(_Prefix, _Args) _Prefix(_JSON5_FIRST _Args),_JSON5_CONCAT_11(_Prefix,_JSON5_TAIL _Args)
#define _JSON5_CONCAT_13(_Prefix, _Args) _Prefix(_JSON5_FIRST _Args),_JSON5_CONCAT_12(_Prefix,_JSON5_TAIL _Args)
#define _JSON5_CONCAT_14(_Prefix, _Args) _Prefix(_JSON5_FIRST _Args),_JSON5_CONCAT_13(_Prefix,_JSON5_TAIL _Args)
#define _JSON5_CONCAT_15(_Prefix, _Args) _Prefix(_JSON5_FIRST _Args),_JSON5_CONCAT_14(_Prefix,_JSON5_TAIL _Args)
#define _JSON5_CONCAT_16(_Prefix, _Args) _Prefix(_JSON5_FIRST _Args),_JSON5_CONCAT_15(_Prefix,_JSON5_TAIL _Args)

#define _JSON5_PREFIX_IN(_X) in. _X
#define _JSON5_PREFIX_OUT(_X) out. _X
