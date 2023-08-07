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
#include <hydra/detail/external/hydra_thrust/system/detail/generic/tag.h>

HYDRA_THRUST_NAMESPACE_BEGIN
namespace system
{
namespace detail
{
namespace generic
{


template<typename ExecutionPolicy,
         typename InputIterator,
         typename OutputIterator>
__host__ __device__
  OutputIterator inclusive_scan(hydra_thrust::execution_policy<ExecutionPolicy> &exec,
                                InputIterator first,
                                InputIterator last,
                                OutputIterator result);


// XXX it is an error to call this function; it has no implementation 
template<typename ExecutionPolicy,
         typename InputIterator,
         typename OutputIterator,
         typename BinaryFunction>
__host__ __device__
  OutputIterator inclusive_scan(hydra_thrust::execution_policy<ExecutionPolicy> &exec,
                                InputIterator first,
                                InputIterator last,
                                OutputIterator result,
                                BinaryFunction binary_op);


template<typename ExecutionPolicy,
         typename InputIterator,
         typename OutputIterator>
__host__ __device__
  OutputIterator exclusive_scan(hydra_thrust::execution_policy<ExecutionPolicy> &exec,
                                InputIterator first,
                                InputIterator last,
                                OutputIterator result);


template<typename ExecutionPolicy,
         typename InputIterator,
         typename OutputIterator,
         typename T>
__host__ __device__
  OutputIterator exclusive_scan(hydra_thrust::execution_policy<ExecutionPolicy> &exec,
                                InputIterator first,
                                InputIterator last,
                                OutputIterator result,
                                T init);


// XXX it is an error to call this function; it has no implementation 
template<typename ExecutionPolicy,
         typename InputIterator,
         typename OutputIterator,
         typename T,
         typename BinaryFunction>
__host__ __device__
  OutputIterator exclusive_scan(hydra_thrust::execution_policy<ExecutionPolicy> &exec,
                                InputIterator first,
                                InputIterator last,
                                OutputIterator result,
                                T init,
                                BinaryFunction binary_op);


} // end namespace generic
} // end namespace detail
} // end namespace system
HYDRA_THRUST_NAMESPACE_END

#include <hydra/detail/external/hydra_thrust/system/detail/generic/scan.inl>

