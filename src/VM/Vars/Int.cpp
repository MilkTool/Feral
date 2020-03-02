/*
	Copyright (c) 2020, Electrux
	All rights reserved.
	Using the BSD 3-Clause license for the project,
	main LICENSE file resides in project's root directory.
	Please read that file and understand the license terms
	before using or altering the project.
*/

#include "Base.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////// SOME EXTRAS ///////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::unordered_map< std::string, var_base_t * > * var_int_base()
{
	static std::unordered_map< std::string, var_base_t * > v;
	return & v;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////// VAR_INT //////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

var_int_t::var_int_t( const mpz_class & val, const size_t & idx )
	: var_base_t( VT_INT, idx, 1 ), m_val( val ) { fuse( VT_INT, var_int_base() ); }

var_base_t * var_int_t::copy( const size_t & idx ) { return new var_int_t( m_val, idx ); }
mpz_class & var_int_t::get() { return m_val; }
void var_int_t::set( var_base_t * from )
{
	m_val = INT( from )->get();
}