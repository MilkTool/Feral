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

#include "std/thread_type.hpp"

static size_t thread_id = 1;

var_thread_t::var_thread_t( std::thread * thread, std::shared_future< int > * res,
			    const size_t & src_id, const size_t & idx, const bool & owner )
	: var_base_t( type_id< var_thread_t >(), src_id, idx, false, false ),
	  m_thread( thread ), m_res( res ), m_id( thread_id++ ), m_owner( owner ) {}
var_thread_t::var_thread_t( std::thread * thread, std::shared_future< int > * res, const size_t & id,
			    const size_t & src_id, const size_t & idx, const bool & owner )
	: var_base_t( type_id< var_thread_t >(), src_id, idx, false, false ),
	  m_thread( thread ), m_res( res ), m_id( id ), m_owner( owner ) {}
var_thread_t::~var_thread_t()
{
	if( m_owner ) {
		if( m_thread != nullptr ) { m_thread->join(); delete m_thread; }
		if( m_res != nullptr ) delete m_res;
	}
}

var_base_t * var_thread_t::copy( const size_t & src_id, const size_t & idx )
{
	return new var_thread_t( m_thread, m_res, m_id, src_id, idx, false );
}
void var_thread_t::set( var_base_t * from )
{
	var_thread_t * t = THREAD( from );
	m_owner = false;
	m_thread = t->m_thread;
	m_res = t->m_res;
	m_id = t->m_id;
}