#ifndef _FUNCTION_TRAITS_H_
#define _FUNCTION_TRAITS_H_

#include "../arx/XPlattform.h"

template<typename>
struct function_traits;

template<typename Func>
struct function_traits
{
  template <typename>
  struct function_traits_helper;

  template <typename F, typename R, typename... Args>
  struct function_traits_helper<R(F::*)(Args...)>
  {
    using argument_types = std::tuple<Args...>;
    using return_type = R;
  };

  template <typename F, typename R, typename... Args>
  struct function_traits_helper<R(F::*)(Args...) const>
  {
    using argument_types = std::tuple<std::decay_t<Args>...>;
    using return_type = R;
  };

  using return_type = typename function_traits_helper<decltype(&Func::operator())>::return_type;
  using argument_types = typename function_traits_helper<decltype(&Func::operator())>::argument_types;
};

template<typename... Func>
using argument_types_tuple = std::tuple<typename function_traits<Func>::argument_types...>;

#endif