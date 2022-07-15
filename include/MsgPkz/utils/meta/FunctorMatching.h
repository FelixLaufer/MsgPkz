#ifndef _FUNCTOR_MATCHING_H_
#define _FUNCTOR_MATCHING_H_

#include "../arx/XPlattform.h"

template <typename T, typename... Args>
struct takes_arguments
{
  using yes = double;
  using no = bool;

  template <typename Functor>
  static yes test(decltype(std::declval<Functor&>()(std::declval<Args>()...))*);

  template <typename Functor>
  static no test(...);

  static constexpr bool value = sizeof(test<T>(nullptr)) == sizeof(yes);
};

template <typename T, typename... Args>
struct takes_arguments<T, std::tuple<Args...>> : takes_arguments<T, Args...>
{};

template <typename T>
struct identity
{
  using type = T;
};

struct type_not_found
{};

template <typename Args, typename Functor, typename... Functors>
struct find_type_helper
{
  using type = typename std::conditional<takes_arguments<Functor, Args>::value, identity<Functor>, find_type_helper<Args, Functors...>>::type::type;
};

template <typename Args>
struct find_type_helper<Args, type_not_found>
{
  using type = type_not_found;
};

template <typename Args, typename... Functors>
struct find_type
{
  using type = typename find_type_helper<Args, Functors..., type_not_found>::type;
};

template <typename Functor>
struct find_functor_helper
{
  template <typename... Functors>
  static const Functor& find()
  {
    return type_not_found();
  }

  template <typename... Functors>
  static const Functor& find(const Functor& arg, Functors&&...)
  {
    return arg;
  }

  template <typename Head, typename... Functors>
  static typename std::enable_if<!std::is_same<Head, Functor>::value, const Functor&>::type find(const Head&, const Functors&... functors)
  {
    return find(functors...);
  }
};

template <typename Functor, typename... Args>
const Functor& find_functor(const Args&... args)
{
  return find_functor_helper<Functor>::find(args...);
}

template <typename Args, typename... Functors>
const typename find_type<Args, Functors...>::type& matching_functor(const Functors&... functors)
{
  using type = typename find_type<Args, Functors...>::type;
  return find_functor<type>(functors...);
}

#endif