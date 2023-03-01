#pragma once

#include "json5.hpp"

namespace json5 {

class builder
{
public:
	builder( document &doc ) : _doc( doc ) { }

	const document &doc() const noexcept { return _doc; }

	detail::string_offset string_buffer_offset() const noexcept;
	detail::string_offset string_buffer_add( std::string_view str );
	void string_buffer_add( char ch ) { _doc._strings.push_back( ch ); }
	void string_buffer_add_utf8( uint32_t ch );

	value new_string( detail::string_offset stringOffset ) { return value( value_type::string, stringOffset ); }
	value new_string( std::string_view str ) { return new_string( string_buffer_add( str ) ); }

	void push_object();
	void push_array();
	value pop();

	builder &operator+=( value v );
	value &operator[]( detail::string_offset keyOffset );
	value &operator[]( std::string_view key ) { return ( *this )[string_buffer_add( key )]; }

protected:
	void reset() noexcept;

	document &_doc;
	std::vector<value> _stack;
	std::vector<value> _values;
	std::vector<size_t> _counts;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
inline detail::string_offset builder::string_buffer_offset() const noexcept
{
	return detail::string_offset( _doc._strings.size() );
}

//---------------------------------------------------------------------------------------------------------------------
inline detail::string_offset builder::string_buffer_add( std::string_view str )
{
	auto offset = string_buffer_offset();
	_doc._strings += str;
	_doc._strings.push_back( 0 );
	return offset;
}

//---------------------------------------------------------------------------------------------------------------------
inline void builder::string_buffer_add_utf8( uint32_t ch )
{
	auto &s = _doc._strings;

	if ( 0 <= ch && ch <= 0x7f )
	{
		s += char( ch );
	}
	else if ( 0x80 <= ch && ch <= 0x7ff )
	{
		s += char( 0xc0 | ( ch >> 6 ) );
		s += char( 0x80 | ( ch & 0x3f ) );
	}
	else if ( 0x800 <= ch && ch <= 0xffff )
	{
		s += char( 0xe0 | ( ch >> 12 ) );
		s += char( 0x80 | ( ( ch >> 6 ) & 0x3f ) );
		s += char( 0x80 | ( ch & 0x3f ) );
	}
	else if ( 0x10000 <= ch && ch <= 0x1fffff )
	{
		s += char( 0xf0 | ( ch >> 18 ) );
		s += char( 0x80 | ( ( ch >> 12 ) & 0x3f ) );
		s += char( 0x80 | ( ( ch >> 6 ) & 0x3f ) );
		s += char( 0x80 | ( ch & 0x3f ) );
	}
	else if ( 0x200000 <= ch && ch <= 0x3ffffff )
	{
		s += char( 0xf8 | ( ch >> 24 ) );
		s += char( 0x80 | ( ( ch >> 18 ) & 0x3f ) );
		s += char( 0x80 | ( ( ch >> 12 ) & 0x3f ) );
		s += char( 0x80 | ( ( ch >> 6 ) & 0x3f ) );
		s += char( 0x80 | ( ch & 0x3f ) );
	}
	else if ( 0x4000000 <= ch && ch <= 0x7fffffff )
	{
		s += char( 0xfc | ( ch >> 30 ) );
		s += char( 0x80 | ( ( ch >> 24 ) & 0x3f ) );
		s += char( 0x80 | ( ( ch >> 18 ) & 0x3f ) );
		s += char( 0x80 | ( ( ch >> 12 ) & 0x3f ) );
		s += char( 0x80 | ( ( ch >> 6 ) & 0x3f ) );
		s += char( 0x80 | ( ch & 0x3f ) );
	}
}

//---------------------------------------------------------------------------------------------------------------------
inline void builder::push_object()
{
	auto v = value( value_type::object, nullptr );
	_stack.emplace_back( v );
	_counts.push_back( 0 );
}

//---------------------------------------------------------------------------------------------------------------------
inline void builder::push_array()
{
	auto v = value( value_type::array, nullptr );
	_stack.emplace_back( v );
	_counts.push_back( 0 );
}

//---------------------------------------------------------------------------------------------------------------------
inline value builder::pop()
{
	auto result = _stack.back();
	auto count = _counts.back();

	result.payload( _doc._values.size() );

	_doc._values.push_back( value( double( count ) ) );

	auto startIndex = _values.size() - count;
	for ( size_t i = startIndex, S = _values.size(); i < S; ++i )
		_doc._values.push_back( _values[i] );

	_values.resize( _values.size() - count );

	_stack.pop_back();
	_counts.pop_back();

	if ( _stack.empty() )
	{
		_doc.assign_root( result );
		result = _doc;
	}

	return result;
}

//---------------------------------------------------------------------------------------------------------------------
inline builder &builder::operator+=( value v )
{
	_values.push_back( v );
	_counts.back() += 1;
	return *this;
}

//---------------------------------------------------------------------------------------------------------------------
inline value &builder::operator[]( detail::string_offset keyOffset )
{
	_values.push_back( new_string( keyOffset ) );
	_counts.back() += 2;
	return _values.emplace_back();
}

//---------------------------------------------------------------------------------------------------------------------
inline void builder::reset() noexcept
{
	_doc._data = value::type_null;
	_doc._values.clear();
	_doc._strings.clear();
	_doc._strings.push_back( 0 );
}

} // namespace json5
