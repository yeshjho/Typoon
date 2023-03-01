#pragma once

#include "json5.hpp"

#include <functional>

namespace json5 {

//
template <typename Func> void filter( const json5::value &in, std::string_view pattern, Func &&func );

//
std::vector<json5::value> filter( const json5::value &in, std::string_view pattern );

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
template <typename Func>
inline void filter( const json5::value &in, std::string_view pattern, Func &&func )
{
	if ( pattern.empty() )
	{
		func( in );
		return;
	}

	std::string_view head = pattern;
	std::string_view tail = pattern;

	if ( auto slash = pattern.find( "/" ); slash != std::string_view::npos )
	{
		head = pattern.substr( 0, slash );
		tail = pattern.substr( slash + 1 );
	}
	else
		tail = std::string_view();

	// Trim whitespace
	{
		while ( !head.empty() && isspace( head.front() ) ) head.remove_prefix( 1 );
		while ( !head.empty() && isspace( head.back() ) ) head.remove_suffix( 1 );
	}

	if ( head == "*" )
	{
		if ( in.is_object() )
		{
			for ( auto kvp : object_view( in ) )
				filter( kvp.second, tail, std::forward<Func>( func ) );
		}
		else if ( in.is_array() )
		{
			for ( auto v : array_view( in ) )
				filter( v, tail, std::forward<Func>( func ) );
		}
		else
			filter( in, std::string_view(), std::forward<Func>( func ) );
	}
	else if ( head == "**" )
	{
		if ( in.is_object() )
		{
			filter( in, tail, std::forward<Func>( func ) );

			for ( auto kvp : object_view( in ) )
			{
				filter( kvp.second, tail, std::forward<Func>( func ) );
				filter( kvp.second, pattern, std::forward<Func>( func ) );
			}
		}
		else if ( in.is_array() )
		{
			for ( auto v : array_view( in ) )
			{
				filter( v, tail, std::forward<Func>( func ) );
				filter( v, pattern, std::forward<Func>( func ) );
			}
		}
	}
	else
	{
		if ( in.is_object() )
		{
			// Remove string quotes
			if ( head.size() >= 2 )
			{
				auto first = head.front();
				if ( ( first == '\'' || first == '"' ) && head.back() == first )
					head = head.substr( 1, head.size() - 2 );
			}

			for ( auto kvp : object_view( in ) )
			{
				if ( head == kvp.first )
					filter( kvp.second, tail, std::forward<Func>( func ) );
			}
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
inline std::vector<json5::value> filter( const json5::value &in, std::string_view pattern )
{
	std::vector<value> result;
	filter( in, pattern, [&result]( const value & v ) { result.push_back( v ); } );
	return result;
}

} // namespace json5
