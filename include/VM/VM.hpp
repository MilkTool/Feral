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

#ifndef VM_VM_HPP
#define VM_VM_HPP

#include <deque>
#include <string>
#include <unordered_map>

#include "DyLib.hpp"
#include "SrcFile.hpp"
#include "Vars.hpp"
#include "VMStack.hpp"

#define _STRINGIZE(x) #x
#define STRINGIFY(x) _STRINGIZE(x)

typedef std::vector< var_src_t * > src_stack_t;

typedef std::unordered_map< std::string, var_src_t * > all_srcs_t;

typedef Errors ( * fmod_read_code_fn_t )( const std::string & data, const std::string & src_dir, const std::string & src_path,
					  bcode_t & bc, const size_t & flags, const bool is_main_src,
					  const bool & expr_only, const size_t & begin_idx, const size_t & end_idx );

typedef srcfile_t * ( * fmod_load_fn_t )( const std::string & src_file, const size_t & flags, const bool is_main_src,
					  Errors & err, const size_t & begin_idx, const size_t & end_idx );

typedef bool ( * mod_init_fn_t )( vm_state_t & vm, const size_t src_id, const size_t & idx );
typedef void ( * mod_deinit_fn_t )();
#define INIT_MODULE( name )			\
	extern "C" bool init_##name( vm_state_t & vm, const size_t src_id, const size_t & idx )
#define DEINIT_MODULE( name )			\
	extern "C" void deinit_##name()

template< typename T, typename ... Args > T * make( Args... args )
{
	// 0, 0 for src_id and idx
	T * res = new T( args..., 0, 0 );
	res->dref();
	return res;
}

template< typename T, typename ... Args > T * make_all( Args... args )
{
	// 0, 0 for src_id and idx
	T * res = new T( args... );
	res->dref();
	return res;
}

struct vm_state_t
{
	bool exit_called;
	size_t exit_code;
	size_t exec_flags;
	// each size_t defines if the corresponding exec function can handle vm.fail or not
	size_t except_count;

	src_stack_t src_stack;
	all_srcs_t all_srcs;
	vm_stack_t * vm_stack;

	// globally common variables
	var_base_t * tru;
	var_base_t * fals;
	var_base_t * nil;
	var_base_t * args;

	// this is a pointer since it must be explicitly deleted after everything else
	dyn_lib_t * dlib;

	// arguments for feral source from command line
	var_base_t * src_args;

	vm_state_t( const std::string & self_bin, const std::string & self_base,
		   const std::vector< std::string > & args, const size_t & flags );
	~vm_state_t();

	void push_src( srcfile_t * src, const size_t & idx );
	void push_src( const std::string & src_path );
	void pop_src();

	// modules & imports
	// nmod = native module
	// fmod = feral module
	bool mod_exists( const std::vector< std::string > & locs, std::string & mod, const std::string & ext );
	bool nmod_load( const std::string & mod_str, const size_t & src_id, const size_t & idx );
	// updated mod_str with actual file name (full canonical path)
	int fmod_load( std::string & mod_str, const size_t & src_id, const size_t & idx );
	inline fmod_read_code_fn_t fmod_read_code_fn() { return m_src_read_code_fn; }

	inline void set_fmod_load_fn( fmod_load_fn_t load_fn ) { m_src_load_fn = load_fn; }
	inline void set_fmod_read_code_fn( fmod_read_code_fn_t read_code_fn ) { m_src_read_code_fn = read_code_fn; }

	inline const std::vector< std::string > & inc_locs() const { return m_inc_locs; }
	inline const std::vector< std::string > & dll_locs() const { return m_dll_locs; }

	inline var_src_t * current_source() const { return src_stack.back(); }
	inline srcfile_t * current_source_file() const { return src_stack.back()->src(); }

	void gadd( const std::string & name, var_base_t * val, const bool iref = true );
	var_base_t * gget( const std::string & name );

	template< typename ... T > void register_type( const std::string & name, const size_t & src_id = 0, const size_t & idx = 0 )
	{
		set_typename( type_id< T ... >(), name );
		var_typeid_t * type_var = make_all< var_typeid_t >( type_id< T ... >(), src_id, idx );
		if( src_stack.empty() ) gadd( name + "_t", type_var );
		else src_stack.back()->add_native_var( name + "_t", type_var, true, true );
	}

	void add_typefn( const std::uintptr_t & type, const std::string & name, var_base_t * fn, const bool iref );
	template< typename ... T > void add_native_typefn( const std::string & name, nativefnptr_t fn, const size_t & args_count,
							   const size_t & src_id, const size_t & idx )
	{
		add_typefn( type_id< T ... >(), name,
			    new var_fn_t( src_stack.back()->src()->path(),
					  std::vector< std::string >( args_count, "" ),
					  {}, { .native = fn }, src_id, idx ),
			    false );
	}
	var_base_t * get_typefn( var_base_t * var, const std::string & name );

	// used to convert typeid -> name
	void set_typename( const std::uintptr_t & type, const std::string & name );
	std::string type_name( const std::uintptr_t & type );
	std::string type_name( const var_base_t * val );

	// vm fail handling
	inline void fail_push( var_base_t * obj, const bool & iref = true ) { if( iref ) var_iref( obj ); m_fail_queue.push_back( obj ); }
	inline var_base_t * fail_top() { return m_fail_queue.front(); }
	inline void fail_pop( const bool & dref = true ) { if( dref ) var_dref( m_fail_queue.back() ); m_fail_queue.pop_front(); }

	inline const std::string & self_bin() const { return m_self_bin; }
	inline const std::string & self_base() const { return m_self_base; }

	void fail( const size_t & src_id, const size_t & idx, const char * msg, ... );

	bool load_core_mods();
private:
	// file loading function
	fmod_load_fn_t m_src_load_fn;
	// code loading function
	fmod_read_code_fn_t m_src_read_code_fn;
	// include and module locations - searches in increasing order of vector elements
	std::vector< std::string > m_inc_locs;
	std::vector< std::string > m_dll_locs;
	// global vars/objects that are required
	std::unordered_map< std::string, var_base_t * > m_globals;
	// functions for any and all C++ types
	std::unordered_map< std::uintptr_t, vars_frame_t * > m_typefns;
	// names of types (optional)
	std::unordered_map< std::uintptr_t, std::string > m_typenames;
	// all functions to call before unloading dlls
	std::unordered_map< std::string, mod_deinit_fn_t > m_dll_deinit_fns;
	// vm fail stack
	std::deque< var_base_t * > m_fail_queue;
	// path where feral binary exists (used by sys.self_bin())
	std::string m_self_bin;
	// parent directory of where feral binary exists (used by sys.self_base())
	std::string m_self_base;
};

const char * nmod_ext();
const char * fmod_ext( const bool compiled = false );

namespace vm
{

// end = 0 = till size of bcode
int exec( vm_state_t & vm, const bcode_t * custom_bcode = nullptr, const size_t & begin = 0, const size_t & end = 0 );

}

#endif // VM_VM_HPP
