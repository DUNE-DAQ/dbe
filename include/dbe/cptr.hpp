// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from dbe/cptr.hpp to include/dbe/cptr.hpp).

/************************************************************
 * cptr.hpp                                             *
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
#ifndef LUTILS_CPTR_HPP
#define LUTILS_CPTR_HPP

#include <utility>
#include <iostream>
#include <mutex>
#include <memory>
#include <thread>
#include <chrono>

template<typename T, typename M>
class cptr_proxy;

//-----------------------------------------------------------------------------------------------------
/**
 * cptr models a concurrent pointer with locks.
 *
 * In its current implementation takes advantage of the drill down behaviour of the arrow operator and
 * returns a proxy object. That object is returned only once the associated lock has been obtained.
 * The proxy can be dereferenced and returns the pointer to the actual holded object. The lock is released
 * when the proxy object is destroyed.
 *
 *
 * Usage:
 *
 *
 * Myclass * p = new Myclass;
 *
 * In any thread :
 *
 * cptr<Myclass> cp(p);
 *
 * cp->dosomething();
 *
 *
 */
template<typename T, typename M = std::recursive_mutex, int N = 100> class cptr
{
protected:
  std::shared_ptr<M> llock;

  T * base;

public:
  explicit cptr ( T * p )
    : llock ( new M() ),
      base ( p )
  {
  }

  cptr_proxy<T, M> operator->()
  {
    while ( !llock->try_lock() )
    {
      std::this_thread::sleep_for ( std::chrono::nanoseconds ( N ) );
    }

    return *this;
  }

  cptr_proxy<T, M> operator->() const
  {
    while ( !llock->try_lock() )
    {
      std::this_thread::sleep_for ( std::chrono::nanoseconds ( N ) );
    }

    return *this;
  }

  T * get()
  {
    return base;
  }

  T * get() const
  {
    return base;
  }

  friend class cptr_proxy<T, M> ;
};
//-----------------------------------------------------------------------------------------------------
//template<typename T, typename M, int N> M cptr<T,M,N>::llock;
//-----------------------------------------------------------------------------------------------------
/**
 * This is a proxy class of the concurrent pointer, the object is returned only by the arrow operator of
 * class cptr. This results in a temporary object, which once destroyed release the mutex locked in cptr class
 * by the arrow operator.
 *
 * This class is not supposed to be used directly.
 */
template<typename T, typename M>
class cptr_proxy
{
private:
  T * that;

  std::shared_ptr<M> llock_ptr;

public:
  cptr_proxy ( cptr<T, M> const & cp )
    : that ( cp.base ),
      llock_ptr ( cp.llock )
  {
  }

  T * operator->()
  {
    return that;
  }

  ~cptr_proxy()
  {
    llock_ptr->unlock();
  }
};
//-----------------------------------------------------------------------------------------------------

#endif /*LUTILS_CPTR_HPP*/
