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

/*! \file simple_defines.h
 *  \brief Primitive macros without dependencies.
 */

#pragma once

#define HYDRA_THRUST_UNKNOWN 0
#define HYDRA_THRUST_FALSE   0
#define HYDRA_THRUST_TRUE    1

#define HYDRA_THRUST_UNUSED_VAR(expr) do { (void)(expr); } while (0)

#define HYDRA_THRUST_PREVENT_MACRO_SUBSTITUTION

