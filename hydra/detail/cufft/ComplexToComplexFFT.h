/*----------------------------------------------------------------------------
 *
 *   Copyright (C) 2016 - 2018 Antonio Augusto Alves Junior
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
 * ComplexToComplexFFT.h
 *
 *  Created on: 16/11/2018
 *      Author: Antonio Augusto Alves Junior
 */

#ifndef COMPLEXTOCOMPLEXFFT_H_
#define COMPLEXTOCOMPLEXFFT_H_


#include <hydra/detail/Config.h>
#include <hydra/detail/BackendPolicy.h>
#include <hydra/Types.h>
#include <hydra/detail/Iterable_traits.h>
#include <hydra/Range.h>
#include <hydra/Tuple.h>

#include <cassert>
#include <memory>
#include <utility>
#include <stdexcept>
#include <type_traits>
//#include <complex.h>

//FFTW3
#include <cufft.h>

//Hydra wrappers
#include<hydra/detail/cufft/Wrappers.h>
#include<hydra/detail/cufft/BaseCuFFT.h>

namespace hydra {

template<typename T,
		typename InputType  = typename std::conditional< std::is_same<double,T>::value, cufftDoubleComplex, cufftComplex>::type,
		typename OutputType = typename std::conditional< std::is_same<double,T>::value, cufftDoubleComplex, cufftComplex>::type,
		typename PlanType   = typename std::conditional< std::is_same<double,T>::value,
		               detail::fftw::_Planner<CUFFT_Z2Z>, detail::fftw::_Planner<CUFFT_C2C> >::type>
class ComplexToComplexCuFFT: public BaseCuFFT<InputType, OutputType, PlanType >
{

public:

	ComplexToComplexCuFFT()=delete;

	ComplexToComplexCuFFT(int logical_size, int sign=1, unsigned flags=FFTW_ESTIMATE):
		BaseCuFFT<InputType, OutputType, PlanType >(logical_size, logical_size, flags, sign)
	{}

	ComplexToComplexCuFFT(ComplexToComplexCuFFT<T,InputType, OutputType, PlanType >&& other):
		BaseCuFFT<InputType, OutputType, PlanType >(
				std::forward<BaseCuFFT<InputType, OutputType, PlanType >&&>(other))
	{}

	ComplexToComplexCuFFT<T,InputType, OutputType, PlanType >&
	operator=(ComplexToComplexCuFFT<T,InputType, OutputType, PlanType >&& other)
	{
		if(this ==&other) return *this;

		BaseFFTW<InputType, OutputType, PlanType >::operator=(other);

		return *this;
	}



	~ComplexToComplexCuFFT(){  }

};

}  // namespace hydra



#endif /* COMPLEXTOCOMPLEXFFT_H_ */
