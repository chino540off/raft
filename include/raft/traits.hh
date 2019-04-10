/**
 *  @file traits.hh
 *  @author Olivier Détour (detour.olivier@gmail.com)
 */
#ifndef RAFT_TRAITS_HH_
#define RAFT_TRAITS_HH_

#include <type_traits>

namespace raft
{

template <typename enum_type> // Declare traits type
struct enum_traits
{
}; // Don't need to declare all possible traits

template <typename E> // SFINAE makes function contingent on trait
typename std::enable_if<enum_traits<E>::has_any, bool>::type
any(E e)
{
  return e != E::ok;
}

} /** !raft  */

#endif /** !RAFT_TRAITS_HH_ */
