//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP___ALGORITHM_COPY_MOVE_COMMON_H
#define _LIBCPP___ALGORITHM_COPY_MOVE_COMMON_H

#include <__algorithm/iterator_operations.h>
#include <__algorithm/unwrap_iter.h>
#include <__algorithm/unwrap_range.h>
#include <__config>
#include <__iterator/iterator_traits.h>
#include <__memory/pointer_traits.h>
#include <__type_traits/enable_if.h>
#include <__type_traits/integral_constant.h>
#include <__type_traits/is_constant_evaluated.h>
#include <__type_traits/is_copy_constructible.h>
#include <__type_traits/is_trivially_assignable.h>
#include <__utility/move.h>
#include <__utility/pair.h>

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCPP_BEGIN_NAMESPACE_STD

// `memmove` algorithms implementation.

template <class _In, class _Out>
_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX14 pair<_In*, _Out*>
__copy_trivial_impl(_In* __first, _In* __last, _Out* __result) {
  const size_t __n = static_cast<size_t>(__last - __first);
  ::__builtin_memmove(__result, __first, __n * sizeof(_Out));

  return std::make_pair(__last, __result + __n);
}

template <class _In, class _Out>
_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX14 pair<_In*, _Out*>
__copy_backward_trivial_impl(_In* __first, _In* __last, _Out* __result) {
  const size_t __n = static_cast<size_t>(__last - __first);
  __result -= __n;

  ::__builtin_memmove(__result, __first, __n * sizeof(_Out));

  return std::make_pair(__last, __result);
}

// Iterator unwrapping and dispatching to the correct overload.

template <class _F1, class _F2>
struct __overload : _F1, _F2 {
  using _F1::operator();
  using _F2::operator();
};

template <class _InIter, class _Sent, class _OutIter, class = void>
struct __can_rewrap : false_type {};

template <class _InIter, class _Sent, class _OutIter>
struct __can_rewrap<_InIter,
                    _Sent,
                    _OutIter,
                    // Note that sentinels are always copy-constructible.
                    __enable_if_t< is_copy_constructible<_InIter>::value &&
                                   is_copy_constructible<_OutIter>::value > > : true_type {};

template <class _Algorithm,
          class _InIter,
          class _Sent,
          class _OutIter,
          __enable_if_t<__can_rewrap<_InIter, _Sent, _OutIter>::value, int> = 0>
_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX17 pair<_InIter, _OutIter>
__unwrap_and_dispatch(_InIter __first, _Sent __last, _OutIter __out_first) {
  auto __range  = std::__unwrap_range(__first, std::move(__last));
  auto __result = _Algorithm()(std::move(__range.first), std::move(__range.second), std::__unwrap_iter(__out_first));
  return std::make_pair(std::__rewrap_range<_Sent>(std::move(__first), std::move(__result.first)),
                                 std::__rewrap_iter(std::move(__out_first), std::move(__result.second)));
}

template <class _Algorithm,
          class _InIter,
          class _Sent,
          class _OutIter,
          __enable_if_t<!__can_rewrap<_InIter, _Sent, _OutIter>::value, int> = 0>
_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX17 pair<_InIter, _OutIter>
__unwrap_and_dispatch(_InIter __first, _Sent __last, _OutIter __out_first) {
  return _Algorithm()(std::move(__first), std::move(__last), std::move(__out_first));
}

template <class _IterOps, class _InValue, class _OutIter, class = void>
struct __can_copy_without_conversion : false_type {};

template <class _IterOps, class _InValue, class _OutIter>
struct __can_copy_without_conversion<
    _IterOps,
    _InValue,
    _OutIter,
    __enable_if_t<is_same<_InValue, typename _IterOps::template __value_type<_OutIter> >::value> > : true_type {};

template <class _AlgPolicy,
          class _NaiveAlgorithm,
          class _OptimizedAlgorithm,
          class _InIter,
          class _Sent,
          class _OutIter>
_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX17 pair<_InIter, _OutIter>
__dispatch_copy_or_move(_InIter __first, _Sent __last, _OutIter __out_first) {
#ifdef _LIBCPP_COMPILER_GCC
  // GCC doesn't support `__builtin_memmove` during constant evaluation.
  if (__libcpp_is_constant_evaluated()) {
    return std::__unwrap_and_dispatch<_NaiveAlgorithm>(std::move(__first), std::move(__last), std::move(__out_first));
  }
#else
  // In Clang, `__builtin_memmove` only supports fully trivially copyable types (just having trivial copy assignment is
  // insufficient). Also, conversions are not supported.
  if (__libcpp_is_constant_evaluated()) {
    using _InValue = typename _IterOps<_AlgPolicy>::template __value_type<_InIter>;
    if (!is_trivially_copyable<_InValue>::value ||
        !__can_copy_without_conversion<_IterOps<_AlgPolicy>, _InValue, _OutIter>::value) {
      return std::__unwrap_and_dispatch<_NaiveAlgorithm>(std::move(__first), std::move(__last), std::move(__out_first));
    }
  }
#endif // _LIBCPP_COMPILER_GCC

  using _Algorithm = __overload<_NaiveAlgorithm, _OptimizedAlgorithm>;
  return std::__unwrap_and_dispatch<_Algorithm>(std::move(__first), std::move(__last), std::move(__out_first));
}

_LIBCPP_END_NAMESPACE_STD

#endif // _LIBCPP___ALGORITHM_COPY_MOVE_COMMON_H
