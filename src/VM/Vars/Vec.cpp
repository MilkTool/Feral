/*
	MIT License

	Copyright (c) 2020 Feral Language repositories

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so.
*/

#include "VM/Vars/Base.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////// VAR_VEC //////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

var_vec_t::var_vec_t( const std::vector< var_base_t * > & val, const bool & refs, const size_t & src_id, const size_t & idx )
	: var_base_t( type_id< var_vec_t >(), src_id, idx, false, false ), m_val( val ), m_refs( refs )
{}
var_vec_t::~var_vec_t()
{
	for( auto & v : m_val ) var_dref( v );
}

var_base_t * var_vec_t::copy( const size_t & src_id, const size_t & idx )
{
	std::vector< var_base_t * > new_vec;
	if( m_refs ) {
		for( auto & v : m_val ) {
			var_iref( v );
			new_vec.push_back( v );
		}
	} else {
		for( auto & v : m_val ) {
			new_vec.push_back( v->copy( src_id, idx ) );
		}
	}
	return new var_vec_t( new_vec, m_refs, src_id, idx );
}
std::vector< var_base_t * > & var_vec_t::get() { return m_val; }
bool var_vec_t::is_ref_vec() { return m_refs; }
void var_vec_t::set( var_base_t * from )
{
	for( auto & v : m_val ) {
		var_dref( v );
	}
	m_val.clear();
	for( auto & v : VEC( from )->m_val ) {
		var_iref( v );
	}
	m_val = VEC( from )->m_val;
	m_refs = VEC( from )->m_refs;
}
