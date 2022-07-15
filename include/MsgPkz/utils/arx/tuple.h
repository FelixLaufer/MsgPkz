#pragma once

#ifndef ARX_TYPE_TRAITS_TUPLE_H
#define ARX_TYPE_TRAITS_TUPLE_H

#if ARX_HAVE_LIBSTDCPLUSPLUS >= 201103L // Have libstdc++11

#include <tuple>

#else // Do not have libstdc++11

namespace arx { namespace stdx {

    // https://theolizer.com/cpp-school2/cpp-school2-15/
    // https://wandbox.org/permlink/C0BWIzjqg4iO3kKZ

    template<typename... Ts>
    struct tuple
    {
        tuple() {}
    };

    template<typename tFirst, typename... tRest>
    class tuple<tFirst, tRest...> : public tuple<tRest...>
    {
        template<size_t N, typename... tTypes> friend struct get_helper;
        template<size_t N, typename... tTypes> friend struct get_helper_c;
        tFirst  mMember;
    public:
        tuple(tFirst const& iFirst, tRest const&... iRest)
        : tuple<tRest...>(iRest...)
        , mMember(iFirst)
        { }
        constexpr tuple() {}
    };
    
    template<size_t N, typename... tTypes>
    struct get_helper { };
    template<typename tFirst, typename... tRest>
    struct get_helper<0, tFirst, tRest...>
    {
        typedef tFirst type;
        static type& get(tuple<tFirst, tRest...>& iTuple)
        {
            return iTuple.mMember;
        }
    };
    template<size_t N, typename tFirst, typename... tRest>
    struct get_helper<N, tFirst, tRest...>
    {
        typedef typename get_helper<N - 1, tRest...>::type type;
        static type& get(tuple<tFirst, tRest...>& iTuple)
        {
            return get_helper<N - 1, tRest...>::get(iTuple);
        }
    };

    template<size_t N, typename... tTypes>
    typename get_helper<N, tTypes...>::type& get(tuple<tTypes...>& iTuple)
    {
        return get_helper<N, tTypes...>::get(iTuple);
    }

    template<size_t N, typename... tTypes>
    struct get_helper_c { };
    template<typename tFirst, typename... tRest>
    struct get_helper_c<0, tFirst, tRest...>
    {
      typedef tFirst type;
      static const type& get(const tuple<tFirst, tRest...>& iTuple)
      {
        return iTuple.mMember;
      }
    };
    template<size_t N, typename tFirst, typename... tRest>
    struct get_helper_c<N, tFirst, tRest...>
    {
      typedef typename get_helper_c<N - 1, tRest...>::type type;
      static const type& get(const tuple<tFirst, tRest...>& iTuple)
      {
        return get_helper_c<N - 1, tRest...>::get(iTuple);
      }
    };

    template<size_t N, typename... tTypes>
    const typename get_helper_c<N, tTypes...>::type& get(const tuple<tTypes...>& iTuple)
    {
      return get_helper_c<N, tTypes...>::get(iTuple);
    }

    template <class T> class tuple_size;
    template <class T> class tuple_size<const T>;
    template <class T> class tuple_size<volatile T>;
    template <class T> class tuple_size<const volatile T>;
    template <class... Types>
    class tuple_size <tuple<Types...>>
    : public integral_constant <size_t, sizeof...(Types)> {};
 
    template <class... Types>
    auto make_tuple(Types&&... args)
    -> std::tuple<typename std::remove_reference<Types>::type...>
    {
        return std::tuple<typename std::remove_reference<Types>::type...>(std::forward<typename std::remove_reference<Types>::type>(args)...);
    }

    template<size_t I, class T>
    struct tuple_element;

    template<size_t I, class Head, class... Tail>
    struct tuple_element<I, std::tuple<Head, Tail...>>
      : std::tuple_element<I - 1, std::tuple<Tail...>>
    { };

    template<class Head, class... Tail>
    struct tuple_element<0, std::tuple<Head, Tail...>>
    {
      using type = Head;
    };

} } // namespace arx::std

#endif // Do not have libstdc++11

#endif // ARX_TYPE_TRAITS_TUPLE_H
