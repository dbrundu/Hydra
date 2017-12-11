
/*----------------------------------------------------------------------------
 *
 *   Copyright (C) 2016 - 2017 Antonio Augusto Alves Junior
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
 * Gaussian.h
 *
 *  Created on: Dec 11, 2017
 *      Author: Antonio Augusto Alves Junior
 */

#ifndef GAUSSIAN_H_
#define GAUSSIAN_H_


#include <hydra/Function.h>
#include <hydra/Pdf.h>
#include <hydra/detail/Integrator.h>
#include <hydra/Parameter.h>
#include <hydra/Tuple.h>
#include <tuple>


namespace hydra {

template<unsigned int ArgIndex>
class Gaussian: public BaseFunctor<Gaussian<ArgIndex>, double, 2>
{
	Gaussian()=delete;

	Gaussian(Parameter const& mean, Parameter const& sigma ):
		BaseFunctor<Gaussian<ArgIndex>, double, 2>({mean, sigma})
		{}

	__host__ __device__
	Gaussian(Gaussian<ArgIndex> const& other ):
		BaseFunctor<Gaussian<ArgIndex>, double,2>(other)
		{}

	__host__ __device__
	Gaussian<ArgIndex>&
	operator=(Gaussian<ArgIndex> const& other ){
		if(this==&other) return  *this;
		BaseFunctor<Gaussian<ArgIndex>,double, 2>(other);
		return  *this;
	}

	template<typename T>
	__host__ __device__ inline
	double Evaluate(unsigned int n, T*x)
	{
		double m2 = (x[ArgIndex] - _par[0])*(x[ArgIndex] - _par[0] );
		double s2 = _par[1]*_par[1];
		return exp(-m2/(2.0 * s2 ));

	}

	template<typename T>
	__host__ __device__ inline
	double Evaluate(T x)
	{
		double m2 = ( get<ArgIndex>(x) - _par[0])*(get<ArgIndex>(x) - _par[0] );
		double s2 = _par[1]*_par[1];
		return exp(-m2/(2.0 * s2 ));

	}

};

class GaussianAnalyticalIntegral: public Integrator<GaussianAnalyticalIntegral>
{

	GaussianAnalyticalIntegral(double min, double max):
		fLowerLimit(min),
		fUpperLimit(max)
	{ }

	inline GaussianAnalyticalIntegral(GaussianAnalyticalIntegral const& other):
		fLowerLimit(other.GetLowerLimit()),
		fUpperLimit(other.GetUpperLimit())
	{}

	inline GaussianAnalyticalIntegral&
	operator=( GaussianAnalyticalIntegral const& other)
	{
		if(this == &other) return *this;

		this->fLowerLimit = other.GetLowerLimit();
		this->fUpperLimit = other.GetUpperLimit();

		return *this;
	}

	double GetLowerLimit() const {
		return fLowerLimit;
	}

	void SetLowerLimit(double lowerLimit ) {
		fLowerLimit = lowerLimit;
	}

	double GetUpperLimit() const {
		return fUpperLimit;
	}

	void SetUpperLimit(double upperLimit) {
		fUpperLimit = upperLimit;
	}

	template<typename FUNCTOR>	inline
	std::pair<double, double> Integrate(FUNCTOR const& functor){

		double fraction = cumulative(functor[0], functor[1], fUpperLimit)
						 - cumulative(functor[0], functor[1], fLowerLimit);

		double scale = functor[1]*sqrt(2.0*PI);

		return std::make_pair(fraction*scale ,0.0);
	}


private:

	inline double cumulative(const double mean, const double sigma, const double x)
	{
		return 0.5*(1.0 + erf( (x-mean)/( sigma*sqrt(2) ) ) );
	}

	double fLowerLimit;
	double fUpperLimit;

};



}  // namespace hydra


#endif /* GAUSSIAN_H_ */
