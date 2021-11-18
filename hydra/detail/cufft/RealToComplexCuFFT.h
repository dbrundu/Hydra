/*----------------------------------------------------------------------------
 *
 *   Copyright (C) 2016 - 2021 Antonio Augusto Alves Junior
 *
 *   This file is part of Hydra Data Analysis Framework.
 *
 *   Hydra is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Hydra is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Hydra.  If not, see <http://www.gnu.org/licenses/>.
 *
 *---------------------------------------------------------------------------*/

/*
 * RealToComplexCuFFT.h
 *
 *  Created on: 16/11/2018
 *      Author: Antonio Augusto Alves Junior
 */

#ifndef REALTOCOMPLEXCUFFT_H_
#define REALTOCOMPLEXCUFFT_H_

#include <hydra/detail/Config.h>
#include <hydra/detail/BackendPolicy.h>
#include <hydra/Types.h>
#include <hydra/detail/Iterable_traits.h>
#include <hydra/Range.h>
#include <hydra/Tuple.h>
#include <hydra/Complex.h>

#include <cassert>
#include <memory>
#include <utility>
#include <stdexcept>
#include <type_traits>
//#include <complex.h>

//CuFFT
#include <cufft.h>

//Hydra wrappers
#include<hydra/detail/cufft/WrappersCuFFT.h>
#include<hydra/detail/cufft/BaseCuFFT.h>

namespace hydra {

template<typename T,
        typename InputType  = T,
		typename OutputType = typename std::conditional< std::is_same<double,T>::value, hydra::complex<double>, hydra::complex<float>>::type,
		typename PlanType   = typename std::conditional< std::is_same<double,T>::value,
		               detail::cufft::_Planner<CUFFT_D2Z>, detail::cufft::_Planner<CUFFT_R2C> >::type>
class RealToComplexCuFFT: public BaseCuFFT<InputType, OutputType, PlanType >
{

public:


	RealToComplexCuFFT()=delete;

	RealToComplexCuFFT(int  logical_size):
		BaseCuFFT<InputType, OutputType, PlanType >(logical_size, logical_size/2 +1)
	{ }

	RealToComplexCuFFT(RealToComplexCuFFT<T,InputType, OutputType, PlanType >&& other):
		BaseCuFFT<InputType, OutputType, PlanType >(
				std::forward<BaseCuFFT<InputType, OutputType, PlanType >&&>(other))
	{ }

	RealToComplexCuFFT<T,InputType, OutputType, PlanType>&
	operator=(RealToComplexCuFFT<T,InputType, OutputType, PlanType>&& other)
	{
		if(this ==&other) return *this;

		BaseCuFFT<InputType, OutputType, PlanType >::operator=(other);

		return *this;
	}

	void SetSize(int logical_size){
		this->Reset(logical_size, logical_size/2 + 1 );
	}


	~RealToComplexCuFFT(){ }

};

}  // namespace hydra

#endif /* REALTOCOMPLEXFFT_H_ */
