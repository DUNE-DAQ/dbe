// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from dbe/convenience.hpp to include/dbe/convenience.hpp).

/************************************************************
 * convenience.hpp                                             *
 *                                                          *
 *  Created on: 1 Mar 2014                                     *
 *      Author: Leonidas Georgopoulos                                     *
 *
 *  Copyright (C) 2014 Leonidas Georgopoulos
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See accompanying file LICENSE_1_0.txt or
 *  copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 ************************************************************/

#ifndef LUTILS_CONVENIENCE_HPP_
#define LUTILS_CONVENIENCE_HPP_

/*
 * STL inclusions
 */
#include <memory>
#include <string>
#include <typeinfo>

/*
 * G++ demangle
 */
#ifdef __GNUG__
#include <cstdlib>
#include <cxxabi.h>
#endif

namespace tools
{
//------------------------------------------------------------------------------
/**
 * Class gentraits provides information about varying classes
 */
template<typename T> class gentraits
{
public:
	typedef std::string t_is;

	static t_is constexpr isnot();

	static t_is constexpr isa();

#ifdef __GNUG__
	static t_is constexpr filt();
#endif
};
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/**
 * Returns the formatted class name
 * @param a class pointer
 * @return a string type representation as defined by the compiler
 */
template<typename T> typename gentraits<T>::t_is constexpr filt(T *)
{
	return gentraits<T>::filt();
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/**
 * Evaluates if the class isnot
 * @return
 */
template<typename T> typename gentraits<T>::t_is constexpr gentraits<T>::isnot()
{
	return t_is("ISNOT");
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/**
 * Evaluates what the class is
 * @return returns the name of the class in compiler jargon
 */
template<typename T> typename gentraits<T>::t_is constexpr gentraits<T>::isa()
{
	return typeid(T).name();
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
#ifdef __GNUG__
template<typename T> typename gentraits<T>::t_is constexpr gentraits<T>::filt()
{
	int status = -42;
    char *res = abi::__cxa_demangle(typeid(T).name(), NULL, NULL, &status);

    if(status != 0) {
        ::free(res);
        return isnot();
    } else {
        typename gentraits<T>::t_is result =  res;
        ::free(res);
        return result;
    }
}
#endif
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/**
 * Returns appropriately evaluated object if the class is of requested type
 * @param requested type
 * @return the object evaluation
 */
template<typename A, typename B> typename gentraits<A>::t_is const isa(
		B const &)
{
	return typeid(A).name() == typeid(B).name() ? typeid(A).name() :
			gentraits<A>::isnot();
}
//------------------------------------------------------------------------------

}/* namespace tools */

#endif
