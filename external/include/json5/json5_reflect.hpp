#pragma once

#include "json5_builder.hpp"

#include <array>
#include <fstream>
#include <map>
#include <unordered_map>

namespace json5 {

//
template <typename T> void to_document( document &doc, const T &in, const writer_params &wp = writer_params() );

//
template <typename T> void to_stream( std::ostream &os, const T &in, const writer_params &wp = writer_params() );

//
template <typename T> void to_string( std::string &str, const T &in, const writer_params &wp = writer_params() );

//
template <typename T> std::string to_string( const T &in, const writer_params &wp = writer_params() );

//
template <typename T> bool to_file( std::string_view fileName, const T &in, const writer_params &wp = writer_params() );

//
template <typename T> error from_document( const document &doc, T &out );

//
template <typename T> error from_string( std::string_view str, T &out );

//
template <typename T> error from_file( std::string_view fileName, T &out );

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace detail {

// Pre-declare so compiler knows it exists before it is attempted to be used
template <typename T> error read(const json5::value& in, T& out);

class writer final : public builder
{
public:
	writer( document &doc, const writer_params &wp ): builder( doc ), _params( wp ) { }

	const writer_params &params() const noexcept { return _params; }

private:
	writer_params _params;
};

//---------------------------------------------------------------------------------------------------------------------
inline std::string_view get_name_slice( const char *names, size_t index )
{
	size_t numCommas = index;
	while ( numCommas > 0 && *names )
		if ( *names++ == ',' )
			--numCommas;

	while ( *names && *names <= 32 )
		++names;

	size_t length = 0;
	while ( names[length] > 32 && names[length] != ',' )
		++length;

	return std::string_view( names, length );
}

/* Forward declarations */
template <typename T> json5::value write( writer &w, const T &in );

//---------------------------------------------------------------------------------------------------------------------
inline json5::value write( writer &w, bool in ) { return json5::value( in ); }
inline json5::value write( writer &w, int in ) { return json5::value( double( in ) ); }
inline json5::value write( writer &w, unsigned in ) { return json5::value( double( in ) ); }
inline json5::value write( writer &w, float in ) { return json5::value( double( in ) ); }
inline json5::value write( writer &w, double in ) { return json5::value( in ); }
inline json5::value write( writer &w, const char *in ) { return w.new_string( in ); }
inline json5::value write( writer &w, const std::string &in ) { return w.new_string( in ); }

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
inline json5::value write_array( writer &w, const T *in, size_t numItems )
{
	w.push_array();

	for ( size_t i = 0; i < numItems; ++i )
		w += write( w, in[i] );

	return w.pop();
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T, typename A>
inline json5::value write( writer &w, const std::vector<T, A> &in ) { return write_array( w, in.data(), in.size() ); }

//---------------------------------------------------------------------------------------------------------------------
template <typename T, size_t N>
inline json5::value write( writer &w, const T( &in )[N] ) { return write_array( w, in, N ); }

//---------------------------------------------------------------------------------------------------------------------
template <typename T, size_t N>
inline json5::value write( writer &w, const std::array<T, N> &in ) { return write_array( w, in.data(), N ); }

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
inline json5::value write_map( writer &w, const T &in )
{
	w.push_object();

	for ( const auto &kvp : in )
		w[kvp.first] = write( w, kvp.second );

	return w.pop();
}

//---------------------------------------------------------------------------------------------------------------------
template <typename K, typename T, typename P, typename A>
inline json5::value write( writer &w, const std::map<K, T, P, A> &in ) { return write_map( w, in ); }

template <typename K, typename T, typename H, typename EQ, typename A>
inline json5::value write( writer &w, const std::unordered_map<K, T, H, EQ, A> &in ) { return write_map( w, in ); }

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
inline json5::value write_enum( writer &w, T in )
{
	size_t index = 0;
	const auto *names = enum_table<T>::names;
	const auto *values = enum_table<T>::values;

	while ( true )
	{
		auto name = get_name_slice( names, index );

		// Underlying value fallback
		if ( name.empty() )
			return write( w, std::underlying_type_t<T>( in ) );

		if ( in == values[index] )
			return w.new_string( name );

		++index;
	}

	return json5::value();
}

//---------------------------------------------------------------------------------------------------------------------
template <size_t Index = 0, typename... Types>
inline void write_tuple( writer &w, const char *names, const std::tuple<Types...> &t )
{
	const auto &in = std::get<Index>( t );
	using Type = std::remove_const_t<std::remove_reference_t<decltype( in )>>;

	if ( auto name = get_name_slice( names, Index ); !name.empty() )
	{
		if constexpr ( std::is_enum_v<Type> )
		{
			if constexpr ( enum_table<Type>() )
				w[name] = write_enum( w, in );
			else
				w[name] = write( w, std::underlying_type_t<Type>( in ) );
		}
		else
			w[name] = write( w, in );
	}

	if constexpr ( Index + 1 != sizeof...( Types ) )
		write_tuple < Index + 1 > ( w, names, t );
}

//---------------------------------------------------------------------------------------------------------------------
template <size_t Index = 0, typename... Types>
inline void write_named_tuple( writer &w, const std::tuple<Types...> &t )
{
	write_tuple( w, std::get<Index>( t ), std::get < Index + 1 > ( t ) );

	if constexpr ( Index + 2 != sizeof...( Types ) )
		write_named_tuple < Index + 2 > ( w, t );
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
inline json5::value write( writer &w, const T &in )
{
	w.push_object();
	write_named_tuple( w, class_wrapper<T>::make_named_tuple( in ) );
	return w.pop();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Forward declarations */
template <typename T> error read( const json5::value &in, T &out );

//---------------------------------------------------------------------------------------------------------------------
inline error read( const json5::value &in, bool &out )
{
	if ( !in.is_boolean() )
		return { error::number_expected };

	out = in.get_bool();
	return { error::none };
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
inline error read_number( const json5::value &in, T &out )
{
	return in.try_get( out ) ? error() : error{ error::number_expected };
}

//---------------------------------------------------------------------------------------------------------------------
inline error read( const json5::value &in, int &out ) { return read_number( in, out ); }
inline error read( const json5::value &in, unsigned &out ) { return read_number( in, out ); }
inline error read( const json5::value &in, float &out ) { return read_number( in, out ); }
inline error read( const json5::value &in, double &out ) { return read_number( in, out ); }

//---------------------------------------------------------------------------------------------------------------------
inline error read( const json5::value &in, const char *&out )
{
	if ( !in.is_string() )
		return { error::string_expected };

	out = in.get_c_str();
	return { error::none };
}

//---------------------------------------------------------------------------------------------------------------------
inline error read( const json5::value &in, std::string &out )
{
	if ( !in.is_string() )
		return { error::string_expected };

	out = in.get_c_str();
	return { error::none };
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
inline error read_array( const json5::value &in, T *out, size_t numItems )
{
	if ( !in.is_array() )
		return { error::array_expected };

	auto arr = json5::array_view( in );
	if ( arr.size() != numItems )
		return { error::wrong_array_size };

	for ( size_t i = 0; i < numItems; ++i )
		if ( auto err = read( arr[i], out[i] ) )
			return err;

	return { error::none };
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T, size_t N>
inline error read( const json5::value &in, T( &out )[N] ) { return read_array( in, out, N ); }

//---------------------------------------------------------------------------------------------------------------------
template <typename T, size_t N>
inline error read( const json5::value &in, std::array<T, N> &out ) { return read_array( in, out.data(), N ); }

//---------------------------------------------------------------------------------------------------------------------
template <typename T, typename A>
inline error read( const json5::value &in, std::vector<T, A> &out )
{
	if ( !in.is_array() && !in.is_null() )
		return { error::array_expected };

	auto arr = json5::array_view( in );

	out.clear();
	out.reserve( arr.size() );
	for ( const auto &i : arr )
		if ( auto err = read( i, out.emplace_back() ) )
			return err;

	return { error::none };
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
inline error read_map( const json5::value &in, T &out )
{
	if ( !in.is_object() && !in.is_null() )
		return { error::object_expected };

	std::pair<typename T::key_type, typename T::mapped_type> kvp;

	out.clear();
	for ( auto jsKV : json5::object_view( in ) )
	{
		kvp.first = jsKV.first;

		if ( auto err = read( jsKV.second, kvp.second ) )
			return err;

		out.emplace( std::move( kvp ) );
	}

	return { error::none };
}

//---------------------------------------------------------------------------------------------------------------------
template <typename K, typename T, typename P, typename A>
inline error read( const json5::value &in, std::map<K, T, P, A> &out ) { return read_map( in, out ); }

template <typename K, typename T, typename H, typename EQ, typename A>
inline error read( const json5::value &in, std::unordered_map<K, T, H, EQ, A> &out ) { return read_map( in, out ); }

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
inline error read_enum( const json5::value &in, T &out )
{
	if ( !in.is_string() && !in.is_number() )
		return { error::string_expected };

	size_t index = 0;
	const auto *names = enum_table<T>::names;
	const auto *values = enum_table<T>::values;

	while ( true )
	{
		auto name = get_name_slice( names, index );

		if ( name.empty() )
			break;

		if ( in.is_string() && name == in.get_c_str() )
		{
			out = values[index];
			return { error::none };
		}
		else if ( in.is_number() && in.get<int>() == int( values[index] ) )
		{
			out = values[index];
			return { error::none };
		}

		++index;
	}

	return { error::invalid_enum };
}

//---------------------------------------------------------------------------------------------------------------------
template <size_t Index = 0, typename... Types>
inline error read_tuple( const json5::object_view &obj, const char *names, std::tuple<Types...> &t )
{
	auto &out = std::get<Index>( t );
	using Type = std::remove_reference_t<decltype( out )>;

	auto name = get_name_slice( names, Index );

	auto iter = obj.find( name );
	if ( iter != obj.end() )
	{
		//if constexpr ( std::is_enum_v<Type> )
		//{
		//	if constexpr ( enum_table<Type>() )
		//	{
		//		if ( auto err = read_enum( ( *iter ).second, out ) )
		//			return err;
		//	}
		//	else
		//	{
		//		std::underlying_type_t<Type> temp;
		//		if ( auto err = read( ( *iter ).second, temp ) )
		//			return err;

		//		out = Type( temp );
		//	}
		//}
		if constexpr (std::is_enum_v<Type> && enum_table<Type>())
		{
			if (auto err = read_enum((*iter).second, out))
				return err;
		}
		else
		{
			if ( auto err = read( ( *iter ).second, out ) )
				return err;
		}
	}

	if constexpr ( Index + 1 != sizeof...( Types ) )
	{
		if ( auto err = read_tuple < Index + 1 > ( obj, names, t ) )
			return err;
	}

	return { error::none };
}

//---------------------------------------------------------------------------------------------------------------------
template <size_t Index = 0, typename Tuple>
inline error read_named_tuple( const json5::object_view &obj, Tuple &t )
{
	return read_tuple( obj, std::get<Index>( t ), std::get < Index + 1 > ( t ) );
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
inline error read( const json5::value &in, T &out )
{
	if ( !in.is_object() )
		return { error::object_expected };

	auto namedTuple = class_wrapper<T>::make_named_tuple( out );
	return read_named_tuple( json5::object_view( in ), namedTuple );
}

//---------------------------------------------------------------------------------------------------------------------
template <size_t Index = 0, typename Head, typename... Tail>
inline error read( const json5::array_view &arr, Head &out, Tail &... tail )
{
	if constexpr ( Index == 0 )
	{
		if ( !arr.is_valid() )
			return { error::array_expected };

		if ( arr.size() != ( 1 + sizeof...( Tail ) ) )
			return { error::wrong_array_size };
	}

	if ( auto err = read( arr[Index], out ) )
		return err;

	if constexpr ( sizeof...( Tail ) > 0 )
		return read < Index + 1 > ( arr, tail... );

	return { error::none };
}

} // namespace detail

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
inline void to_document( document &doc, const T &in, const writer_params &wp )
{
	detail::writer w( doc, wp );
	detail::write( w, in );
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
inline void to_stream( std::ostream &os, const T &in, const writer_params &wp )
{
	document doc;
	to_document( doc, in, wp );
	to_stream( os, doc, wp );
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
inline void to_string( std::string &str, const T &in, const writer_params &wp )
{
	document doc;
	to_document( doc, in );
	to_string( str, doc, wp );
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
inline std::string to_string( const T &in, const writer_params &wp )
{
	std::string result;
	to_string( result, in, wp );
	return result;
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
inline bool to_file( std::string_view fileName, const T &in, const writer_params &wp )
{
	std::ofstream ofs( std::string( fileName ).c_str() );
	to_stream( ofs, in, wp );
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
inline error from_document( const document &doc, T &out )
{
	return detail::read( doc, out );
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
inline error from_string( std::string_view str, T &out )
{
	document doc;
	if ( auto err = from_string( str, doc ) )
		return err;

	return from_document( doc, out );
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
inline error from_file( std::string_view fileName, T &out )
{
	document doc;
	if ( auto err = from_file( fileName, doc ) )
		return err;

	return from_document( doc, out );
}

} // json5
