/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: yes (from dbe/macro.h to include/dbe/macro.hpp).
 */

/************************************************************
 * macro.h                                             *
 *                                                          *
 *  Created on: Sep 14, 2014                                     *
 *      Author: Leonidas Georgopoulos                                     *
 *
 *  Copyright (C) 2014 Leonidas Georgopoulos
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See accompanying file LICENSE_1_0.txt or
 *  copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 ************************************************************/

#ifndef LUTILS_MACRO_H_
#define LUTILS_MACRO_H_

#include <string>
#include "dbe/convenience.hpp"

//------------------------------------------------------------------------------
// Messaging macros
//------------------------------------------------------------------------------
/**
 * Define a templated function name, needed to display
 * informative messages
 */
#define HERE_TMPL_DEF(T,funname) \
    typedef typename ::tools::gentraits<T> t_gentraits_T; \
    static typename t_gentraits_T::t_is const TYPEA = t_gentraits_T::filt(); \
    static typename t_gentraits_T::t_is const HERE = #funname + "< " + TYPEA +" >"; \

/**
 * Automatically define the function name, needed to display
 * informative messages
 */
#define HERE_AUTO_DEF(funname) \
    static auto TYPEA = ::tools::filt(this); \
    static auto HERE = std::string("< ") + TYPEA \
              + std::string(" >::") + std::string(#funname); \
/**
 * Define the class member method, which is needed
 * to display informative messages
 */
#define HERE_DEF(cn,fn) static char const * const HERE = "< "#cn" >:: "#fn" ";

/**
 * Define the name of the current function,
 * which is needed to display informative messages
 */
#define HERE_FUN_DEF(fn) static char const * const HERE = #fn" ";

/**
 * Define messages not present in RELEASE  compilation
 */
#ifndef RELEASE

#define ERROR_NOREL(MSG) ERROR(MSG)
#define WARN_NOREL(MSG) WARN(MSG)
#define INFO_NOREL(MSG) INFO(MSG)
#define DEBUG_NOREL(MSG) DEBUG(MSG)

#else
#define ERROR_NOREL(MSG)
#define INFO_NOREL(MSG)
#define DEBUG_NOREL(MSG)
#endif

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Concurrency macros
//------------------------------------------------------------------------------
/**
 * CONDITION_STD_SET provides safe concurrent setting of a shared variable
 */
#define CONDITION_STD_SET(cond,lock,var,val) \
    { \
    { \
      std::lock_guard<std::mutex> \
      __LUTILS_CONDITIONAL_SET_GUARD_##cond_##lock_##var__(lock); \
      var = val; \
    } \
    cond.notify_all(); \
    } \

/**
 * CONDITION_BOOL_WAIT provides safe concurrent wait on a shared variable
 *
 * Upon any exception thrown it will break an enclosing loop
 */
#define CONDITION_BOOL_WAIT(cond,lock,var) \
      \
      std::unique_lock<std::mutex> \
__LUTILS_CONDITIONAL_BOOL_WAIT_GUARD_##cond_##lock_##var__(lock); \
      try { \
cond.wait(__LUTILS_CONDITIONAL_BOOL_WAIT_GUARD_##cond_##lock_##var__, [this]() \
          { return var;}); \
      } catch (...) {\
        break;\
      }\

/**
 * CONDITION_BOOL_WAIT_TRYFUN provides safe concurrent waiting on a shared variable
 * and call a provided function in case of and exception.
 *
 * It is advisable that tryfun does not throw exceptions
 */
#define CONDITION_BOOL_WAIT_TRYFUN(cond,lock,var,tryfun) \
      \
      std::unique_lock<std::mutex> \
__LUTILS_CONDITIONAL_BOOL_WAIT_TRYFUN_GUARD_##cond_##lock_##var__(lock); \
      try { \
cond.wait(__LUTILS_CONDITIONAL_BOOL_WAIT_TRYFUN_GUARD_##cond_##lock_##var__, [this]() \
          { return var;}); \
      } catch (...) {\
        break;\
      }\

/**
 * CONDITION_BOOL_WAIT_FUN provides safe concurrent waiting on
 * a shared variable, which is exposed through a function (fun)
 *
 * Upon any exception thrown it will break an enclosing loop
 */
#define CONDITION_BOOL_WAIT_FUN(cond,lock,fun) \
      \
      std::unique_lock<std::mutex> \
    __LUTILS_CONDITIONAL_BOOL_WAIT_FUN_GUARD_##cond_##lock_(lock); \
      try { \
cond.wait(__LUTILS_CONDITIONAL_BOOL_WAIT_FUN_GUARD_##cond_##lock_, fun ); \
      } catch(...) {\
        break; \
      }\

/**
 * CONDITION_BOOL_WAIT_FUN_TRYFUN provides safe concurrent waiting on
 * a shared variable, which is exposed through a function (fun)
 *
 * It is advisable that tryfun does not throw exceptions
 */
#define CONDITION_BOOL_WAIT_FUN_TRYFUN(cond,lock,fun,tryfun) \
      \
      std::unique_lock<std::mutex> \
    __LUTILS_CONDITIONAL_BOOL_WAIT_FUN_TRYFUN_GUARD_##cond_##lock_(lock); \
      try { \
cond.wait(__LUTILS_CONDITIONAL_BOOL_WAIT_FUN_TRYFUN_GUARD_##cond_##lock_, fun ); \
      } catch(...) {\
        tryfun(); \
      }\
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  IDE COMPAT MACROS
//------------------------------------------------------------------------------
// A workaround for eclipse formatter bug when using decltype
#define DECLTYPE(x) decltype(x)
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  VARIABLE ARGUMENTS MACRO COMPAT MACROS
//------------------------------------------------------------------------------
#define CAT( A, B ) A ## B
#define SELECT( NAME, NUM ) CAT( NAME ## _, NUM )

#define GET_COUNT( _1, _2, _3, _4, _5, _6 , _7, _8, _9, COUNT, ... ) COUNT
#define VA_SIZE( ... ) GET_COUNT( __VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1 )

#define VA_SELECT( NAME, ... ) SELECT( NAME, VA_SIZE(__VA_ARGS__) )(__VA_ARGS__)
//------------------------------------------------------------------------------



#endif /* LUTILS_MACRO_H_ */
