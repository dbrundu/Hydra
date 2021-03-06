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
#include <hydra/detail/external/hydra_thrust/detail/preprocessor.h>

// TODO: Enable this or remove this file once nvGRAPH/CUSP migrates off of it.
//#if HYDRA_THRUST_HOST_COMPILER == HYDRA_THRUST_HOST_COMPILER_MSVC
//  #pragma message("warning: The functionality in this header is unsafe, deprecated, and will soon be removed. Use C++11 or C11 atomics instead.")
//#else
//  #warning The functionality in this header is unsafe, deprecated, and will soon be removed. Use C++11 or C11 atomics instead.
//#endif

// msvc case
#if HYDRA_THRUST_HOST_COMPILER == HYDRA_THRUST_HOST_COMPILER_MSVC

#ifndef _DEBUG

#include <intrin.h>
#pragma intrinsic(_ReadWriteBarrier)
#define __hydra_thrust_compiler_fence() _ReadWriteBarrier()
#else

#define __hydra_thrust_compiler_fence() do {} while (0)

#endif // _DEBUG

// gcc case
#elif HYDRA_THRUST_HOST_COMPILER == HYDRA_THRUST_HOST_COMPILER_GCC

#if HYDRA_THRUST_GCC_VERSION >= 40200 // atomic built-ins were introduced ~4.2
#define __hydra_thrust_compiler_fence() __sync_synchronize()
#else
// allow the code to compile without any guarantees
#define __hydra_thrust_compiler_fence() do {} while (0)
#endif // HYDRA_THRUST_GCC_VERSION

// unknown case
#elif HYDRA_THRUST_HOST_COMPILER == HYDRA_THRUST_HOST_COMPILER_CLANG
#define __hydra_thrust_compiler_fence() __sync_synchronize()
#elif HYDRA_THRUST_HOST_COMPILER == HYDRA_THRUST_HOST_COMPILER_UNKNOWN

// allow the code to compile without any guarantees
#define __hydra_thrust_compiler_fence() do {} while (0)

#endif

