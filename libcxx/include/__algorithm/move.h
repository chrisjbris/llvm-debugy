//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP___ALGORITHM_MOVE_H
#define _LIBCPP___ALGORITHM_MOVE_H

#include <__algorithm/copy_move_common.h>
#include <__algorithm/iterator_operations.h>
#include <__config>
#include <__utility/move.h>
#include <__utility/pair.h>

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCPP_BEGIN_NAMESPACE_STD

template <class _AlgPolicy>
struct __move_loop {
  template <class _InIter, class _Sent, class _OutIter>
  _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX14 pair<_InIter, _OutIter>
  operator()(_InIter __first, _Sent __last, _OutIter __result) const {
    while (__first != __last) {
      *__result = _IterOps<_AlgPolicy>::__iter_move(__first);
      ++__first;
      ++__result;
    }
    return std::make_pair(std::move(__first), std::move(__result));
  }
};

struct __move_trivial {
  // At this point, the iterators have been unwrapped so any `contiguous_iterator` has been unwrapped to a pointer.
  template <class _In, class _Out, __enable_if_t< is_trivially_assignable<_Out&, _In&&>::value, int > = 0>
  _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX14 pair<_In*, _Out*>
  operator()(_In* __first, _In* __last, _Out* __result) const {
    return std::__copy_trivial_impl(__first, __last, __result);
  }
};

template <class _AlgPolicy, class _InIter, class _Sent, class _OutIter>
inline _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX14
pair<_InIter, _OutIter>
__move(_InIter __first, _Sent __last, _OutIter __result) {
  return std::__dispatch_copy_or_move<_AlgPolicy, __move_loop<_AlgPolicy>, __move_trivial>(
      std::move(__first), std::move(__last), std::move(__result));
}

template <class _InputIterator, class _OutputIterator>
inline _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX20
_OutputIterator move(_InputIterator __first, _InputIterator __last, _OutputIterator __result) {
  static_assert(is_copy_constructible<_InputIterator>::value, "Iterators has to be copy constructible.");
  static_assert(is_copy_constructible<_OutputIterator>::value, "The output iterator has to be copy constructible.");

  return std::__move<_ClassicAlgPolicy>(
      std::move(__first), std::move(__last), std::move(__result)).second;
}

_LIBCPP_END_NAMESPACE_STD

#endif // _LIBCPP___ALGORITHM_MOVE_H
