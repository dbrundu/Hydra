/*
 *  Copyright 2008-2021 NVIDIA Corporation
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

/*! \file hydra_thrust/system_error.h
 *  \brief System diagnostics
 */

#pragma once

#include <hydra/detail/external/hydra_thrust/detail/config.h>

HYDRA_THRUST_NAMESPACE_BEGIN

/*! \addtogroup system
 *  \{
 */

/*! \namespace hydra_thrust::system
 *  \brief \p hydra_thrust::system is the namespace which contains specific Thrust
 *         backend systems. It also contains functionality for reporting error
 *         conditions originating from the operating system or other low-level
 *         application program interfaces such as the CUDA runtime. They are
 *         provided in a separate namespace for import convenience but are
 *         also aliased in the top-level \p hydra_thrust namespace for easy access.
 */
namespace system
{
} // end system

/*! \} // end system
 */

HYDRA_THRUST_NAMESPACE_END

#include <hydra/detail/external/hydra_thrust/system/error_code.h>
#include <hydra/detail/external/hydra_thrust/system/system_error.h>
