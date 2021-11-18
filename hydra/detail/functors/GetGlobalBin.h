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
 * GetGlobalBin.h
 *
 *  Created on: 23/09/2017
 *      Author: Antonio Augusto Alves Junior
 */

#ifndef GETGLOBALBIN_H_
#define GETGLOBALBIN_H_

#include <hydra/detail/external/hydra_thrust/functional.h>
#include <hydra/Tuple.h>
#include <hydra/detail/utility/Utility_Tuple.h>

namespace hydra {

namespace detail {


template<std::size_t N, typename T>
struct GetGlobalBin: public hydra_thrust::unary_function<typename tuple_type<N,T>::type ,std::size_t>
{
	//typedef typename tuple_type<N,T>::type ArgType;

	GetGlobalBin( std::size_t (&grid)[N], T (&lowerlimits)[N], T (&upperlimits)[N])
	{
		fNGlobalBins=1;
		for( std::size_t i=0; i<N; i++){
			fNGlobalBins *=grid[i];
			fGrid[i]=grid[i];
			fLowerLimits[i]=lowerlimits[i];
			fDelta[i]= upperlimits[i] - lowerlimits[i];
		}
	}

	__hydra_host__ __hydra_device__
	GetGlobalBin( GetGlobalBin<N, T> const& other ):
	fNGlobalBins(other.fNGlobalBins)
	{
		for( std::size_t i=0; i<N; i++){
			fGrid[i] = other.fGrid[i];
			fDelta[i] = other.fDelta[i];
			fLowerLimits[i] = other.fLowerLimits[i];
		}
		fNGlobalBins =other.fNGlobalBins;
	}

	__hydra_host__ __hydra_device__
	GetGlobalBin<N, T>&
	operator=( GetGlobalBin<N, T> const& other )
	{
		if(this==&other) return *this;
		for( std::size_t i=0; i<N; i++){
			fGrid[i]= other.fGrid[i];
			fDelta[i] = other.fDelta[i];
			fLowerLimits[i] = other.fLowerLimits[i];

		}
		fNGlobalBins =other.fNGlobalBins;
		return *this;
	}

	//k = i_1*(dim_2*...*dim_n) + i_2*(dim_3*...*dim_n) + ... + i_{n-1}*dim_n + i_n

	template<std::size_t I>
	__hydra_host__ __hydra_device__
	typename hydra_thrust::detail::enable_if< I== N, void>::type
	get_global_bin(const std::size_t (&)[N], std::size_t& ){ }

	template<std::size_t I=0>
	__hydra_host__ __hydra_device__
	typename hydra_thrust::detail::enable_if< (I< N), void>::type
	get_global_bin(const std::size_t (&indexes)[N], std::size_t& index)
	{
	    std::size_t prod =1;
	    for(std::size_t i=N-1; i>I; i--)
	           prod *=fGrid[i];
	    index += prod*indexes[I];

	    get_global_bin<I+1>( indexes, index);
	}



	__hydra_host__ __hydra_device__
	std::size_t get_bin( T (&X)[N]){

		std::size_t indexes[N];
		std::size_t bin=0;
		for(std::size_t i=0; i<N; i++)
			indexes[i]=std::size_t(X[i]);

		get_global_bin(indexes,  bin);


		return bin;
	}

	template<typename ArgType>
	__hydra_host__ __hydra_device__
	std::size_t operator()(ArgType value){

		T X[N];

		tupleToArray(value, X );


		bool is_underflow = false;
		bool is_overflow  = false;

		for(std::size_t i=0; i<N; i++){

			X[i]  = (X[i]-fLowerLimits[i])*fGrid[i]/fDelta[i];

			if( X[i]<0.0 ) is_underflow = true;
            if( X[i]>fGrid[i] ) is_overflow = true;

		}

		std::size_t result = is_underflow ? fNGlobalBins : (is_overflow ? fNGlobalBins+1 : get_bin(X) );


		return result;

	}


	T fLowerLimits[N];
	T fDelta[N];
	std::size_t   fGrid[N];
	std::size_t   fNGlobalBins;



};

//---------------

template<typename T>
struct GetGlobalBin<1,T>: public hydra_thrust::unary_function<T,std::size_t>
{

	GetGlobalBin( std::size_t grid, T lowerlimits, T upperlimits):
		fLowerLimits(lowerlimits),
		fDelta( upperlimits - lowerlimits),
		fGrid(grid),
		fNGlobalBins(grid)
	{ }

	__hydra_host__ __hydra_device__
	GetGlobalBin( GetGlobalBin<1, T> const& other ):
	fNGlobalBins(other.fNGlobalBins),
	fGrid(other.fGrid ),
	fDelta(other.fDelta ),
	fLowerLimits(other.fLowerLimits )
	{}

	__hydra_host__ __hydra_device__
	GetGlobalBin<1, T>&
	operator=( GetGlobalBin<1, T> const& other )
	{
		if(this==&other) return *this;

		fGrid  = other.fGrid;
		fDelta = other.fDelta;
		fLowerLimits = other.fLowerLimits;
		fNGlobalBins = other.fNGlobalBins;

		return *this;
	}

	__hydra_host__ __hydra_device__
	std::size_t get_bin(T X){

		return std::size_t(X) ;
	}

	__hydra_host__ __hydra_device__
 std::size_t	operator()(T& value){

		T X = value;

		bool is_underflow = true;
		bool is_overflow  = true;

		X  = (X-fLowerLimits)*fGrid/fDelta;
		is_underflow =(X<0.0);
		is_overflow  =(X>fGrid);


		return is_underflow ? fNGlobalBins  : (is_overflow ? fNGlobalBins+1 : get_bin(X) );

	}


	T fLowerLimits;
	T fDelta;
	std::size_t   fGrid;
	std::size_t   fNGlobalBins;



};

}//namespace detail

}//namespace hydra

#endif /* GETGLOBALBIN_H_ */
