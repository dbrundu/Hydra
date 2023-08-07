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
#include <hydra/detail/external/hydra_thrust/system/detail/generic/temporary_buffer.h>
#include <hydra/detail/external/hydra_thrust/detail/pointer.h>
#include <hydra/detail/external/hydra_thrust/detail/malloc_and_free.h>
#include <hydra/detail/external/hydra_thrust/pair.h>

HYDRA_THRUST_NAMESPACE_BEGIN
namespace system
{
namespace detail
{
namespace generic
{


template<typename T, typename DerivedPolicy>
__host__ __device__
  hydra_thrust::pair<hydra_thrust::pointer<T,DerivedPolicy>, typename hydra_thrust::pointer<T,DerivedPolicy>::difference_type>
    get_temporary_buffer(hydra_thrust::execution_policy<DerivedPolicy> &exec, typename hydra_thrust::pointer<T,DerivedPolicy>::difference_type n)
{
  hydra_thrust::pointer<T,DerivedPolicy> ptr = hydra_thrust::malloc<T>(exec, n);

  // check for a failed malloc
  if(!ptr.get())
  {
    n = 0;
  } // end if

  return hydra_thrust::make_pair(ptr, n);
} // end get_temporary_buffer()


__hydra_thrust_exec_check_disable__
template<typename DerivedPolicy, typename Pointer>
__host__ __device__
  void return_temporary_buffer(hydra_thrust::execution_policy<DerivedPolicy> &exec, Pointer p, std::ptrdiff_t)
{
  // If we are here, no user customization of the three-argument signature with
  // a size parameter of `return_temporary_buffer` was found. There may be an
  // old two-argument signature `return_temporary_buffer` though, so we make
  // another ADL call to try and find one.
  //
  // The interface layer downcast and then did ADL dispatch - there were no
  // matches for DerivedPolicy (aka no one customized the three-argument
  // signature), so this overload got found an implicit upcast to
  // `execution_policy<DerivedPolicy>` was done. Now, we're looking for a
  // customization of the two-argument signature so we need to downcast again.
  return_temporary_buffer(hydra_thrust::detail::derived_cast(hydra_thrust::detail::strip_const(exec)), p);
} // end return_temporary_buffer()


__hydra_thrust_exec_check_disable__
template<typename DerivedPolicy, typename Pointer>
__host__ __device__
  void return_temporary_buffer(hydra_thrust::execution_policy<DerivedPolicy> &exec, Pointer p)
{
  // If we are here, no user customization of either the old two-argument
  // signature or the new three-argument signature with a size parameter of
  // `return_temporary_buffer` was found.
  hydra_thrust::free(exec, p);
} // end return_temporary_buffer()


} // end generic
} // end detail
} // end system
HYDRA_THRUST_NAMESPACE_END

