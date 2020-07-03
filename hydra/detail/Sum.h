/*----------------------------------------------------------------------------
 *
 *   Copyright (C) 2016 - 2020 Antonio Augusto Alves Junior
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
 * Sum.h
 *
 *  Created on: 05/05/2016
 *      Author: Antonio Augusto Alves Junior
 */

/**
 * \file
 * \ingroup functor
 */


#ifndef SUM_H_
#define SUM_H_


#include <hydra/detail/Config.h>
#include <hydra/Types.h>
#include <hydra/Function.h>
#include <hydra/detail/FunctorTraits.h>
#include <hydra/detail/utility/Utility_Tuple.h>
#include <hydra/detail/base_functor.h>
#include <hydra/detail/Constant.h>
#include <hydra/Complex.h>
#include <hydra/Parameter.h>
#include <hydra/Parameter.h>
#include <hydra/Tuple.h>
#include <hydra/detail/CompositeBase.h>
#include <type_traits>

namespace hydra {


template<typename F1, typename F2, typename ...Fs>
class Sum: public detail::CompositeBase< F1,F2, Fs...>
{

public:

	typedef typename detail::sum_result<
	 		  typename F1::return_type,
			  typename F2::return_type,
			  typename Fs::return_type... >::type  return_type;

	Sum()=delete;

    Sum(F1 const& f1, F2 const& f2, Fs const&... fs ):
		detail::CompositeBase<F1,F2, Fs...>( f1, f2,fs...)
  	{}

	__hydra_host__ __hydra_device__
	Sum(const Sum<F1,F2, Fs...>& other):
		detail::CompositeBase<F1,F2, Fs...>(  other)
	{};

	__hydra_host__ __hydra_device__
	inline Sum<F1,F2, Fs...>&
	operator=(Sum<F1,F2, Fs...> const& other)
	{
		if(this==&other) return *this;
		detail::CompositeBase<F1,F2, Fs...>::operator=( other);

		return *this;

	}

  	template<typename T1>
  	__hydra_host__ __hydra_device__
  	inline return_type
  	operator()(T1&& t ) const
  	{
  		return detail::accumulate<return_type,T1,F1,F2,Fs...>(std::forward<T1>(t),this->fFtorTuple );

  	}

};

/**
 * operator+ for two functors.
 */
template<typename T1, typename T2>
inline typename std::enable_if<
(detail::is_hydra_functor<T1>::value || detail::is_hydra_lambda<T1>::value) &&
(detail::is_hydra_functor<T2>::value || detail::is_hydra_lambda<T2>::value),
Sum<T1, T2> >::type
operator+(T1 const& F1, T2 const& F2)
{
	return  Sum<T1,T2>(F1, F2);
}

/**
 * operator+ for a value and a functor.
 */
template <typename T, typename U>
inline typename std::enable_if<
(detail::is_hydra_functor<T>::value || detail::is_hydra_lambda<T>::value) &&
(std::is_arithmetic<U>::value),
Sum< Constant<U>, T> >::type
operator+(U const cte, T const& F)
{
	return  Constant<U>(cte)+F;
}

/**
 * operator+ for a value and a functor.
 */
template <typename T, typename U>
inline typename std::enable_if<
(detail::is_hydra_functor<T>::value || detail::is_hydra_lambda<T>::value) &&
(std::is_arithmetic<U>::value),
Sum< Constant<U>, T> >::type
operator+( T const& F, U cte)
{
	return  Constant<U>(cte)+F;
}

/**
 * operator+ for a complex value and a functor.
 */
template <typename T, typename U>
inline typename std::enable_if<
(detail::is_hydra_functor<T>::value || detail::is_hydra_lambda<T>::value) &&
(std::is_arithmetic<U>::value),
Sum< Constant<hydra::complex<U>>, T> >::type
operator+(hydra::complex<U> const& cte, T const& F)
{
	return  Constant<hydra::complex<U> >(cte)+F;
}

/**
 * operator+ for a complex value and a functor.
 */
template <typename T, typename U>
inline typename std::enable_if<
(detail::is_hydra_functor<T>::value || detail::is_hydra_lambda<T>::value) &&
(std::is_arithmetic<U>::value),
Sum< Constant<U>, T> >::type
operator+( T const& F, hydra::complex<U> const& cte)
{
	return  Constant<hydra::complex<U> >(cte)+F;
}


// Convenience function
template <typename F1, typename F2, typename ...Fs>
inline typename std::enable_if<
(detail::is_hydra_functor<F1>::value || detail::is_hydra_lambda<F1>::value) &&
(detail::is_hydra_functor<F2>::value || detail::is_hydra_lambda<F2>::value) &&
detail::all_true<(detail::is_hydra_functor<Fs>::value || detail::is_hydra_lambda<Fs>::value)...>::value,
Sum<F1, F2,Fs...>>::type
sum(F1 const& f1, F2 const& f2, Fs const&... functors )
{
	return  Sum<F1, F2,Fs... >(f1,f2, functors ... );
}


}//namespace hydra


#endif /* SUM_H_ */
