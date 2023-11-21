/*
 *  Copyright 2008-2013 NVIDIA Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#pragma once

#include <hydra/detail/external/hydra_thrust/detail/config.h>
#include <hydra/detail/external/hydra_thrust/system/tbb/detail/scan.h>
#include <hydra/detail/external/hydra_thrust/distance.h>
#include <hydra/detail/external/hydra_thrust/advance.h>
#include <hydra/detail/external/hydra_thrust/iterator/iterator_traits.h>
#include <hydra/detail/external/hydra_thrust/detail/function.h>
#include <hydra/detail/external/hydra_thrust/detail/type_traits.h>
#include <hydra/detail/external/hydra_thrust/detail/type_traits/function_traits.h>
#include <hydra/detail/external/hydra_thrust/detail/type_traits/iterator/is_output_iterator.h>
#include <tbb/blocked_range.h>
#include <tbb/parallel_scan.h>

HYDRA_THRUST_NAMESPACE_BEGIN
namespace system
{
namespace tbb
{
namespace detail
{
namespace scan_detail
{

template<typename InputIterator,
         typename OutputIterator,
         typename BinaryFunction,
         typename ValueType>
struct inclusive_body
{
  InputIterator input;
  OutputIterator output;
  hydra_thrust::detail::wrapped_function<BinaryFunction,ValueType> binary_op;
  ValueType sum;
  bool first_call;

  inclusive_body(InputIterator input, OutputIterator output, BinaryFunction binary_op, ValueType dummy)
    : input(input), output(output), binary_op(binary_op), sum(dummy), first_call(true)
  {}
    
  inclusive_body(inclusive_body& b, ::tbb::split)
    : input(b.input), output(b.output), binary_op(b.binary_op), sum(b.sum), first_call(true)
  {}

  template<typename Size> 
  void operator()(const ::tbb::blocked_range<Size>& r, ::tbb::pre_scan_tag)
  {
    InputIterator iter = input + r.begin();
 
    ValueType temp = *iter;

    ++iter;

    for (Size i = r.begin() + 1; i != r.end(); ++i, ++iter)
      temp = binary_op(temp, *iter);

    if (first_call)
      sum = temp;
    else
      sum = binary_op(sum, temp);
      
    first_call = false;
  }
  
  template<typename Size> 
  void operator()(const ::tbb::blocked_range<Size>& r, ::tbb::final_scan_tag)
  {
    InputIterator  iter1 = input  + r.begin();
    OutputIterator iter2 = output + r.begin();

    if (first_call)
    {
      *iter2 = sum = *iter1;
      ++iter1;
      ++iter2;
      for (Size i = r.begin() + 1; i != r.end(); ++i, ++iter1, ++iter2)
        *iter2 = sum = binary_op(sum, *iter1);
    }
    else
    {
      for (Size i = r.begin(); i != r.end(); ++i, ++iter1, ++iter2)
        *iter2 = sum = binary_op(sum, *iter1);
    }

    first_call = false;
  }

  void reverse_join(inclusive_body& b)
  {
    // Only accumulate this functor's partial sum if this functor has been
    // called at least once -- otherwise we'll over-count the initial value.
    if (!first_call)
    {
      sum = binary_op(b.sum, sum);
    }
  } 

  void assign(inclusive_body& b)
  {
    sum = b.sum;
  } 
};


template<typename InputIterator,
         typename OutputIterator,
         typename BinaryFunction,
         typename ValueType>
struct exclusive_body
{
  InputIterator input;
  OutputIterator output;
  hydra_thrust::detail::wrapped_function<BinaryFunction,ValueType> binary_op;
  ValueType sum;
  bool first_call;

  exclusive_body(InputIterator input, OutputIterator output, BinaryFunction binary_op, ValueType init)
    : input(input), output(output), binary_op(binary_op), sum(init), first_call(true)
  {}
    
  exclusive_body(exclusive_body& b, ::tbb::split)
    : input(b.input), output(b.output), binary_op(b.binary_op), sum(b.sum), first_call(true)
  {}

  template<typename Size> 
  void operator()(const ::tbb::blocked_range<Size>& r, ::tbb::pre_scan_tag)
  {
    InputIterator iter = input + r.begin();
 
    ValueType temp = *iter;

    ++iter;

    for (Size i = r.begin() + 1; i != r.end(); ++i, ++iter)
      temp = binary_op(temp, *iter);

    if (first_call && r.begin() > 0)
      sum = temp;
    else
      sum = binary_op(sum, temp);
      
    first_call = false;
  }
  
  template<typename Size> 
  void operator()(const ::tbb::blocked_range<Size>& r, ::tbb::final_scan_tag)
  {
    InputIterator  iter1 = input  + r.begin();
    OutputIterator iter2 = output + r.begin();

    for (Size i = r.begin(); i != r.end(); ++i, ++iter1, ++iter2)
    {
      ValueType temp = binary_op(sum, *iter1);
      *iter2 = sum;
      sum = temp;
    }
    
    first_call = false;
  }

  void reverse_join(exclusive_body& b)
  {
    // Only accumulate this functor's partial sum if this functor has been
    // called at least once -- otherwise we'll over-count the initial value.
    if (!first_call)
    {
      sum = binary_op(b.sum, sum);
    }
  }

  void assign(exclusive_body& b)
  {
    sum = b.sum;
  } 
};

} // end scan_detail

template<typename InputIterator,
         typename OutputIterator,
         typename BinaryFunction>
  OutputIterator inclusive_scan(tag,
                                InputIterator first,
                                InputIterator last,
                                OutputIterator result,
                                BinaryFunction binary_op)
{
  using namespace hydra_thrust::detail;

  // Use the input iterator's value type per https://wg21.link/P0571
  using ValueType = typename hydra_thrust::iterator_value<InputIterator>::type;

  using Size = typename hydra_thrust::iterator_difference<InputIterator>::type;
  Size n = hydra_thrust::distance(first, last);

  if (n != 0)
  {
    typedef typename scan_detail::inclusive_body<InputIterator,OutputIterator,BinaryFunction,ValueType> Body;
    Body scan_body(first, result, binary_op, *first);
    ::tbb::parallel_scan(::tbb::blocked_range<Size>(0,n), scan_body);
  }

  hydra_thrust::advance(result, n);

  return result;
}

template<typename InputIterator,
         typename OutputIterator,
         typename InitialValueType,
         typename BinaryFunction>
  OutputIterator exclusive_scan(tag,
                                InputIterator first,
                                InputIterator last,
                                OutputIterator result,
                                InitialValueType init,
                                BinaryFunction binary_op)
{
  using namespace hydra_thrust::detail;

  // Use the initial value type per https://wg21.link/P0571
  using ValueType = InitialValueType;

  using Size = typename hydra_thrust::iterator_difference<InputIterator>::type;
  Size n = hydra_thrust::distance(first, last);

  if (n != 0)
  {
    typedef typename scan_detail::exclusive_body<InputIterator,OutputIterator,BinaryFunction,ValueType> Body;
    Body scan_body(first, result, binary_op, init);
    ::tbb::parallel_scan(::tbb::blocked_range<Size>(0,n), scan_body);
  }

  hydra_thrust::advance(result, n);

  return result;
} 

} // end namespace detail
} // end namespace tbb
} // end namespace system
HYDRA_THRUST_NAMESPACE_END
