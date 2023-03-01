#pragma once

#include "json5.hpp"

#include <iomanip>
#include <fstream>
#include <sstream>

namespace json5 {

// Writes json5::document into stream
void to_stream( std::ostream &os, const document &doc, const writer_params &wp = writer_params() );

// Converts json5::document to string
void to_string( std::string &str, const document &doc, const writer_params &wp = writer_params() );

// Returns json5::document converted to string
std::string to_string( const document &doc, const writer_params &wp = writer_params() );

// Write json5::document into file, returns 'true' on success
bool to_file( const std::string &fileName, const document &doc, const writer_params &wp = writer_params() );

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
inline void to_stream( std::ostream &os, const char *str, char quotes, bool escapeUnicode )
{
	if ( quotes )
		os << quotes;

	while ( *str )
	{
		bool advance = true;

		if ( str[0] == '\n' )
			os << "\\n";
		else if ( str[0] == '\r' )
			os << "\\r";
		else if ( str[0] == '\t' )
			os << "\\t";
		else if ( str[0] == '"' && quotes == '"' )
			os << "\\\"";
		else if ( str[0] == '\'' && quotes == '\'' )
			os << "\\'";
		else if ( str[0] == '\\' )
			os << "\\\\";
		else if ( uint8_t( str[0] ) >= 128 && escapeUnicode )
		{
			uint32_t ch = 0;

			if ( ( *str & 0b1110'0000u ) == 0b1100'0000u )
			{
				ch |= ( ( *str++ ) & 0b0001'1111u ) << 6;
				ch |= ( ( *str++ ) & 0b0011'1111u );
			}
			else if ( ( *str & 0b1111'0000u ) == 0b1110'0000u )
			{
				ch |= ( ( *str++ ) & 0b0000'1111u ) << 12;
				ch |= ( ( *str++ ) & 0b0011'1111u ) << 6;
				ch |= ( ( *str++ ) & 0b0011'1111u );
			}
			else if ( ( *str & 0b1111'1000u ) == 0b1111'0000u )
			{
				ch |= ( ( *str++ ) & 0b0000'0111u ) << 18;
				ch |= ( ( *str++ ) & 0b0011'1111u ) << 12;
				ch |= ( ( *str++ ) & 0b0011'1111u ) << 6;
				ch |= ( ( *str++ ) & 0b0011'1111u );
			}
			else if ( ( *str & 0b1111'1100u ) == 0b1111'1000u )
			{
				ch |= ( ( *str++ ) & 0b0000'0011u ) << 24;
				ch |= ( ( *str++ ) & 0b0011'1111u ) << 18;
				ch |= ( ( *str++ ) & 0b0011'1111u ) << 12;
				ch |= ( ( *str++ ) & 0b0011'1111u ) << 6;
				ch |= ( ( *str++ ) & 0b0011'1111u );
			}
			else if ( ( *str & 0b1111'1110u ) == 0b1111'1100u )
			{
				ch |= ( ( *str++ ) & 0b0000'0001u ) << 30;
				ch |= ( ( *str++ ) & 0b0011'1111u ) << 24;
				ch |= ( ( *str++ ) & 0b0011'1111u ) << 18;
				ch |= ( ( *str++ ) & 0b0011'1111u ) << 12;
				ch |= ( ( *str++ ) & 0b0011'1111u ) << 6;
				ch |= ( ( *str++ ) & 0b0011'1111u );
			}

			if ( ch <= std::numeric_limits<uint16_t>::max() )
			{
				os << "\\u" << std::hex << std::setfill( '0' ) << std::setw( 4 ) << ch;
			}
			else
				os << "?"; // JSON can't encode Unicode chars > 65535 (emojis)

			advance = false;
		}
		else
			os << *str;

		if ( advance )
			++str;
	}

	if ( quotes )
		os << quotes;
}

//---------------------------------------------------------------------------------------------------------------------
inline void to_stream( std::ostream &os, const value &v, const writer_params &wp, int depth )
{
	const char *kvSeparator = ": ";
	const char *eol = wp.eol;

	if ( wp.compact )
	{
		depth = -1;
		kvSeparator = ":";
		eol = "";
	}

	if ( v.is_null() )
		os << "null";
	else if ( v.is_boolean() )
		os << ( v.get_bool() ? "true" : "false" );
	else if ( v.is_number() )
	{
		if ( double _, d = v.get<double>(); modf( d, &_ ) == 0.0 )
			os << v.get<int64_t>();
		else
			os << d;
	}
	else if ( v.is_string() )
	{
		to_stream( os, v.get_c_str(), '"', wp.escape_unicode );
	}
	else if ( v.is_array() )
	{
		if ( auto av = json5::array_view( v ); !av.empty() )
		{
			os << "[" << eol;
			for ( size_t i = 0, S = av.size(); i < S; ++i )
			{
				for ( int i = 0; i <= depth; ++i ) os << wp.indentation;
				to_stream( os, av[i], wp, depth + 1 );
				if ( i < S - 1 ) os << ",";
				os << eol;
			}

			for ( int i = 0; i < depth; ++i ) os << wp.indentation;
			os << "]";
		}
		else
			os << "[]";
	}
	else if ( v.is_object() )
	{
		if ( auto ov = json5::object_view( v ); !ov.empty() )
		{
			os << "{" << eol;
			size_t count = ov.size();
			for ( auto kvp : ov )
			{
				for ( int i = 0; i <= depth; ++i ) os << wp.indentation;

				if ( wp.json_compatible )
					os << "\"" << kvp.first << "\"" << kvSeparator;
				else
					os << kvp.first << kvSeparator;

				to_stream( os, kvp.second, wp, depth + 1 );
				if ( --count ) os << ",";
				os << eol;
			}

			for ( int i = 0; i < depth; ++i ) os << wp.indentation;
			os << "}";
		}
		else
			os << "{}";
	}

	if ( !depth )
		os << eol;
}

//---------------------------------------------------------------------------------------------------------------------
inline void to_stream( std::ostream &os, const document &doc, const writer_params &wp )
{
	to_stream( os, doc, wp, 0 );
}

//---------------------------------------------------------------------------------------------------------------------
inline void to_string( std::string &str, const document &doc, const writer_params &wp )
{
	std::ostringstream os;
	to_stream( os, doc, wp );
	str = os.str();
}

//---------------------------------------------------------------------------------------------------------------------
inline std::string to_string( const document &doc, const writer_params &wp )
{
	std::string result;
	to_string( result, doc, wp );
	return result;
}

//---------------------------------------------------------------------------------------------------------------------
inline bool to_file( const std::string &fileName, const document &doc, const writer_params &wp )
{
	std::ofstream ofs( fileName );
	if (!ofs.is_open())
		return false;

	to_stream( ofs, doc, wp );
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
inline void to_stream( std::ostream &os, const error &err )
{
	os << err.type_string[err.type] << " at " << err.line << ":" << err.column;
}

//---------------------------------------------------------------------------------------------------------------------
inline std::string to_string( const error &err )
{
	std::ostringstream os;
	to_stream( os, err );
	return os.str();
}

} // namespace json5
