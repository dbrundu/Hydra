/*----------------------------------------------------------------------------
 *
 *   Copyright (C) 2016 - 2019 Antonio Augusto Alves Junior
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
 * LogLikelihoodFCN2.inl
 *
 *  Created on: Aug 16, 2017
 *      Author: Antonio Augusto Alves Junior
 */

#ifndef LOGLIKELIHOODFCN2_INL_
#define LOGLIKELIHOODFCN2_INL_


#include <hydra/FCN.h>
#include <hydra/PDFSumExtendable.h>
#include <hydra/detail/functors/LogLikelihood1.h>
#include <hydra/detail/external/thrust/transform_reduce.h>
#include <hydra/detail/external/thrust/inner_product.h>

namespace hydra {

/**
 * \ingroup fit
 * \brief LogLikehood object for composed models represented by hydra::PDFSumExtendable<Pdfs...> objects
 * \tparam Pdfs
 * \tparam IteratorD
 * \tparam IteratorW
 */
template<typename ...Pdfs, typename IteratorD , typename ...IteratorW>
class LogLikelihoodFCN< PDFSumExtendable<Pdfs...>, IteratorD, IteratorW...>: public FCN<LogLikelihoodFCN< PDFSumExtendable<Pdfs...>, IteratorD, IteratorW ...> >
{

public:


	/**
	 * @brief LogLikelihoodFCN constructor for non-cached models.
	 *
	 * @param functor hydra::PDF instance.
	 * @param begin  iterator pointing to the begin of the dataset.
	 * @param end   iterator pointing to the end of the dataset.
	 */
	LogLikelihoodFCN(PDFSumExtendable<Pdfs...> const& functor, IteratorD begin, IteratorD end, IteratorW ...wbegin):
		FCN<LogLikelihoodFCN<PDFSumExtendable<Pdfs...>, IteratorD, IteratorW...>>(functor,begin, end, wbegin...)
		{}

	LogLikelihoodFCN(LogLikelihoodFCN<PDFSumExtendable<Pdfs...>, IteratorD, IteratorW...>const& other):
		FCN<LogLikelihoodFCN<PDFSumExtendable<Pdfs...>, IteratorD, IteratorW...>>(other)
		{}

	LogLikelihoodFCN<PDFSumExtendable<Pdfs...>, IteratorD, IteratorW...>&
	operator=(LogLikelihoodFCN<PDFSumExtendable<Pdfs...>, IteratorD, IteratorW...>const& other)
	{
		if(this==&other) return  *this;
		FCN<LogLikelihoodFCN<PDFSumExtendable<Pdfs...>, IteratorD, IteratorW...>>::operator=(other);
		return  *this;
	}

	template<size_t M = sizeof...(IteratorW)>
	inline typename std::enable_if<(M==0), double >::type
	Eval( const std::vector<double>& parameters ) const{

		static_assert((M==0), ">>>>>>> (M==0) <<<<<<<<");

		using   HYDRA_EXTERNAL_NS::thrust::system::detail::generic::select_system;
		typedef typename HYDRA_EXTERNAL_NS::thrust::iterator_system<IteratorD>::type System;
		typedef typename PDFSumExtendable<Pdfs...>::functor_type functor_type;
		System system;

		// create iterators
		HYDRA_EXTERNAL_NS::thrust::counting_iterator<size_t> first(0);
		HYDRA_EXTERNAL_NS::thrust::counting_iterator<size_t> last = first + this->GetDataSize();

		GReal_t final;
		GReal_t init=0;

		if (INFO >= Print::Level()  )
		{
			std::ostringstream stringStream;
			for(size_t i=0; i< parameters.size(); i++){
				stringStream << "Parameter["<< i<<"] :  " << parameters[i]  << "  ";
			}
			HYDRA_LOG(INFO, stringStream.str().c_str() )
		}

		const_cast< LogLikelihoodFCN<PDFSumExtendable<Pdfs...>, IteratorD, IteratorW...>*  >(this)->GetPDF().SetParameters(parameters);

		auto NLL = detail::LogLikelihood1<functor_type>(this->GetPDF().GetFunctor());

		final = HYDRA_EXTERNAL_NS::thrust::transform_reduce(select_system(system), this->begin(), this->end(),
				NLL, init, HYDRA_EXTERNAL_NS::thrust::plus<GReal_t>());

		GReal_t  r = (GReal_t)this->GetDataSize() + this->GetPDF().IsExtended()*
				( this->GetPDF().GetCoefSum() -	this->GetDataSize()*::log(this->GetPDF().GetCoefSum() ) ) - final;

		return r;

	}

	template<size_t M = sizeof...(IteratorW)>
	inline typename std::enable_if<(M>0), double >::type
	Eval( const std::vector<double>& parameters ) const{

		static_assert((M>0), ">>>>>>> (M>0) <<<<<<<<");

		using   HYDRA_EXTERNAL_NS::thrust::system::detail::generic::select_system;
		typedef typename HYDRA_EXTERNAL_NS::thrust::iterator_system<typename FCN<LogLikelihoodFCN<PDFSumExtendable<Pdfs...>, IteratorD, IteratorW...>>::iterator>::type System;
		typedef typename PDFSumExtendable<Pdfs...>::functor_type functor_type;
		System system;

		// create iterators
		HYDRA_EXTERNAL_NS::thrust::counting_iterator<size_t> first(0);
		HYDRA_EXTERNAL_NS::thrust::counting_iterator<size_t> last = first + this->GetDataSize();

		GReal_t final;
		GReal_t init=0;

		if (INFO >= Print::Level()  )
		{
			std::ostringstream stringStream;
			for(size_t i=0; i< parameters.size(); i++){
				stringStream << "Parameter["<< i<<"] :  " << parameters[i]  << "  ";
			}
			HYDRA_LOG(INFO, stringStream.str().c_str() )
		}

		const_cast< LogLikelihoodFCN<PDFSumExtendable<Pdfs...>, IteratorD, IteratorW...>*  >(this)->GetPDF().SetParameters(parameters);

		auto NLL = detail::LogLikelihood2<functor_type>(this->GetPDF().GetFunctor());

		final = HYDRA_EXTERNAL_NS::thrust::inner_product(select_system(system), this->begin(), this->end(),this->wbegin(),
				 init,HYDRA_EXTERNAL_NS::thrust::plus<GReal_t>(),NLL );

		GReal_t  r = (GReal_t)this->GetDataSize() + this->GetPDF().IsExtended()*
				( this->GetPDF().GetCoefSum() -	this->GetDataSize()*::log(this->GetPDF().GetCoefSum() ) ) - final;

		return r;

	}
};


/**
 * \ingroup fit

 * \brief Convenience function to build up loglikehood fcns
 * @param pdf hydra::Pdf object
 * @param first iteraror pointing to begin of data range
 * @param last iteraror pointing to end of data range
 * @param weights iteraror pointing to begin of weights range
 * @return
 */
template<typename... Pdfs,  typename Iterator, typename ...Iterators >
inline typename std::enable_if< hydra::detail::is_iterator<Iterator>::value  && detail::are_iterators<Iterators...>::value,
LogLikelihoodFCN< PDFSumExtendable<Pdfs...>, Iterator,Iterators...  >>::type
make_loglikehood_fcn(PDFSumExtendable<Pdfs...> const& functor, Iterator first, Iterator last, Iterators... weights )
{
	return LogLikelihoodFCN< PDFSumExtendable<Pdfs...>, Iterator, Iterators...>( functor, first, last, weights...);
}

/**
 * \ingroup fit
 * \brief Convenience function to build up loglikehood fcns
 *
 * @param functor hydra::PDFSumExtendable<Pdfs...> object
 * @param points iterable (range) describing data points
 * @param weights iterable (range) describing weight of data points
 * @return
 */
template<typename ...Pdfs, typename Iterable, typename ...Iterables >
inline typename std::enable_if<   (!hydra::detail::is_hydra_dense_histogram<Iterable>::value) &&
		                          (!hydra::detail::is_hydra_sparse_histogram<Iterable>::value) &&
								  detail::is_iterable<Iterable>::value && detail::are_iterables<Iterables...>::value  ,
LogLikelihoodFCN< PDFSumExtendable<Pdfs...>, decltype(std::declval<Iterable>().begin() ),
                  decltype(std::declval<Iterables>().begin())... > >::type
make_loglikehood_fcn(PDFSumExtendable<Pdfs...> const& functor, Iterable&& points, Iterables&&... weights ){

	return make_loglikehood_fcn( functor,   std::forward<Iterable>(points).begin(),
					std::forward<Iterable>(points).end(),
					std::forward<Iterables>(weights).begin() ...);

}


/**
 *  \ingroup fit
 *  \brief Convenience function to build up loglikehood fcns
 *
 * @param functor PDFSumExtendable<Pdfs...>
 * @param points hydra::DenseHistogram<T,N,BACKEND,D> object
 * @return hydra::LogLikelihoodFCN object
 */
template<typename ...Pdfs, typename T, size_t N, hydra::detail::Backend BACKEND,typename D>
LogLikelihoodFCN<  PDFSumExtendable<Pdfs...>,
                     decltype(std::declval< DenseHistogram<T,N,detail::BackendPolicy<BACKEND>,D>>().GetBinsCenters().begin()),
                     decltype(std::declval< DenseHistogram<T,N,detail::BackendPolicy<BACKEND>,D>>().GetBinsContents().begin()) >
make_loglikehood_fcn(PDFSumExtendable<Pdfs...> const& functor,	DenseHistogram<T,N,detail::BackendPolicy<BACKEND>,D>&& points )
{

	return make_loglikehood_fcn( functor, std::forward<DenseHistogram<T,N,detail::BackendPolicy<BACKEND>,D>>(points).GetBinsCenters().begin(),
			std::forward<DenseHistogram<T,N,detail::BackendPolicy<BACKEND>,D>>(points).GetBinsCenters().end(),
			std::forward<DenseHistogram<T,N,detail::BackendPolicy<BACKEND>,D>>(points).GetBinsContents().begin());
}

/**
 * \ingroup fit
 * \brief Convenience function to build up loglikehood fcns
 *
 * @param functor functor PDFSumExtendable<Pdfs...>
 * @param points SparseHistogram<T,N,BACKEND,D> object
 * @return hydra::LogLikelihoodFCN
 */
template<typename ...Pdfs, typename T, size_t N, hydra::detail::Backend BACKEND,typename D>
LogLikelihoodFCN<  PDFSumExtendable<Pdfs...>,
                     decltype(std::declval<SparseHistogram<T,N,detail::BackendPolicy<BACKEND>,D>>().GetBinsCenters().begin()),
                     decltype(std::declval<SparseHistogram<T,N,detail::BackendPolicy<BACKEND>,D>>().GetBinsContents().begin()) >
make_loglikehood_fcn(PDFSumExtendable<Pdfs...> const& functor,	SparseHistogram<T,N,detail::BackendPolicy<BACKEND>,D>const& points)
{

	return make_loglikehood_fcn( functor, points.GetBinsCenters().begin(),
			points.GetBinsCenters().end(),
			points.GetBinsContents().begin());
}

}  // namespace hydra






#endif /* LOGLIKELIHOODFCN2_INL_ */
