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
 * SparseHistogram.h
 *
 *  Created on: 29/09/2017
 *      Author: Antonio Augusto Alves Junior
 */

#ifndef SPARSEHISTOGRAM_H_
#define SPARSEHISTOGRAM_H_

#include <hydra/detail/Config.h>
#include <hydra/detail/BackendPolicy.h>
#include <hydra/Types.h>
#include <hydra/detail/Dimensionality.h>
#include <hydra/detail/functors/GetBinCenter.h>
#include <hydra/Range.h>
#include <hydra/Algorithm.h>

#include <type_traits>
#include <utility>
#include <array>


#include <hydra/detail/external/hydra_thrust/iterator/iterator_traits.h>
#include <hydra/detail/external/hydra_thrust/iterator/zip_iterator.h>
#include <hydra/detail/external/hydra_thrust/find.h>

namespace hydra {

/**
 * \ingroup histogram
 */
template<typename T, std::size_t N,  typename BACKEND, typename = typename detail::dimensionality<N>::type,
	typename = typename std::enable_if<std::is_arithmetic<T>::value, void>::type>
class SparseHistogram;

/**
 * \ingroup histogram
 * Class representing multidimensional sparse histogram.
 */
template<typename T, std::size_t N, hydra::detail::Backend BACKEND >
class SparseHistogram<T, N,  detail::BackendPolicy<BACKEND>, detail::multidimensional>
{

	typedef hydra::detail::BackendPolicy<BACKEND>    system_t;

	typedef typename system_t::template container<double> storage_data_t;
	typedef typename system_t::template container<std::size_t> storage_keys_t;

	typedef typename storage_data_t::iterator data_iterator;
	typedef typename storage_data_t::const_iterator data_const_iterator;
	typedef typename storage_data_t::reference data_reference;
	typedef typename storage_data_t::pointer data_pointer;
	typedef typename storage_data_t::value_type data_value_type;

	typedef typename storage_keys_t::iterator keys_iterator;
	typedef typename storage_keys_t::const_iterator keys_const_iterator;
	typedef typename storage_keys_t::reference keys_reference;
	typedef typename storage_keys_t::pointer keys_pointer;
	typedef typename storage_keys_t::value_type keys_value_type;

	typedef hydra_thrust::zip_iterator<
			hydra_thrust::tuple<keys_iterator, data_iterator>> iterator;
	typedef hydra_thrust::zip_iterator<
			hydra_thrust::tuple<keys_const_iterator, data_const_iterator> > const_iterator;
	typedef typename hydra_thrust::iterator_traits<iterator>::reference reference;
	typedef typename hydra_thrust::iterator_traits<iterator>::value_type value_type;
	typedef std::pair<std::size_t*, double*> pointer_pair;

public:

	SparseHistogram()=delete;

	explicit SparseHistogram( std::array<std::size_t , N> const& grid,
			std::array<T, N> const& lowerlimits,   std::array<T, N> const& upperlimits):
				fNBins(1)
	{
		for( std::size_t i=0; i<N; i++){
			fGrid[i]=grid[i];
			fLowerLimits[i]=lowerlimits[i];
			fUpperLimits[i]=upperlimits[i];
			fNBins *=grid[i];
		}

	}

	explicit SparseHistogram( std::size_t (&grid)[N],
			T (&lowerlimits)[N],   T (&upperlimits)[N] ):
				fNBins(1)
	{
		for( std::size_t i=0; i<N; i++){
			fGrid[i]=grid[i];
			fLowerLimits[i]=lowerlimits[i];
			fUpperLimits[i]=upperlimits[i];
			fNBins*=grid[i];
		}

	}

	template<typename Int, typename = typename std::enable_if<std::is_integral<Int>::value, void>::type>
	SparseHistogram( std::array<Int , N> const& grid,
			std::array<T, N> const& lowerlimits,   std::array<T, N> const& upperlimits):
				fNBins(1)
	{
		for( std::size_t i=0; i<N; i++){
			fGrid[i]=grid[i];
			fLowerLimits[i]=lowerlimits[i];
			fUpperLimits[i]=upperlimits[i];
			fNBins *=grid[i];
		}

	}

	template<typename Int, typename = typename std::enable_if<std::is_integral<Int>::value, void>::type>
	SparseHistogram( Int (&grid)[N],
			T (&lowerlimits)[N],   T (&upperlimits)[N] ):
				fNBins(1)
	{
		for( std::size_t i=0; i<N; i++){
			fGrid[i]=grid[i];
			fLowerLimits[i]=lowerlimits[i];
			fUpperLimits[i]=upperlimits[i];
			fNBins*=grid[i];
		}

	}

	SparseHistogram<T, N, detail::BackendPolicy<BACKEND>, detail::multidimensional>&
	operator=(SparseHistogram<T, N,  detail::BackendPolicy<BACKEND>, detail::multidimensional> const& other )
	{
		if(this==&other) return *this;

		fContents = other.GetContents();
		fBins = other.GetBins();

		for( std::size_t i=0; i<N; i++){
			fGrid[i] = other.GetGrid(i);
			fLowerLimits[i] = other.GetLowerLimits(i);
			fUpperLimits[i] = other.GetUpperLimits(i);
		}

		fNBins= other.GetNBins();
		return *this;
	}

	SparseHistogram(SparseHistogram<T, N,  detail::BackendPolicy<BACKEND>, detail::multidimensional> const& other ):
			fContents(other.GetContents()),
			fBins(other.GetBins())
		{
			for( std::size_t i=0; i<N; i++){
				fGrid[i] = other.GetGrid(i);
				fLowerLimits[i] = other.GetLowerLimits(i);
				fUpperLimits[i] = other.GetUpperLimits(i);
			}

			fNBins= other.GetNBins();
		}

	template<hydra::detail::Backend BACKEND2>
	SparseHistogram<T, N, detail::BackendPolicy<BACKEND>, detail::multidimensional>&
	operator=(SparseHistogram<T, N,  detail::BackendPolicy<BACKEND2>, detail::multidimensional> const& other )
	{
		if(this==&other) return *this;

		fContents = other.GetContents();
		fBins = other.GetBins();

		for( std::size_t i=0; i<N; i++){
			fGrid[i] = other.GetGrid(i);
			fLowerLimits[i] = other.GetLowerLimits(i);
			fUpperLimits[i] = other.GetUpperLimits(i);
		}

		fNBins= other.GetNBins();
		return *this;
	}

	template<hydra::detail::Backend BACKEND2>
	SparseHistogram(SparseHistogram<T, N,  detail::BackendPolicy<BACKEND2>, detail::multidimensional> const& other ):
		fContents(other.GetContents()),
		fBins(other.GetBins())
	{
		for( std::size_t i=0; i<N; i++){
			fGrid[i] = other.GetGrid(i);
			fLowerLimits[i] = other.GetLowerLimits(i);
			fUpperLimits[i] = other.GetUpperLimits(i);
		}

		fNBins= other.GetNBins();
	}


	inline const storage_data_t& GetContents() const {
		return fContents;
	}

	inline const storage_keys_t& GetBins() const
	{
		return fBins;
	}

	inline void SetBins(storage_keys_t bins)
	{
		fBins = bins;
	}

	inline void SetContents(storage_data_t histogram) {
		fContents = histogram;
	}

	inline std::size_t GetGrid(std::size_t i) const {
		return fGrid[i];
	}

	T GetLowerLimits(std::size_t i) const {
		return fLowerLimits[i];
	}

	inline T GetUpperLimits(std::size_t i) const {
		return fUpperLimits[i];
	}

	inline std::size_t GetNBins() const {
		return fNBins;
	}

	template<typename Int,
			typename = typename std::enable_if<std::is_integral<Int>::value, void>::type>
	inline 	std::size_t GetBin( Int  (&bins)[N]){

		std::size_t bin=0;

		get_global_bin( bins,  bin);

		return bin;
	}

	inline std::size_t GetBin( std::array<std::size_t,N> const&  bins){

		std::size_t bin=0;

		get_global_bin( bins,  bin);

		return bin;
	}


	template<typename Int,
		typename = typename std::enable_if<std::is_integral<Int>::value, void>::type>
	inline std::size_t GetBin( std::array<Int,N> const&  bins){

		std::size_t bin=0;

		get_global_bin( bins,  bin);

		return bin;
	}

	template<typename Int,
			typename = typename std::enable_if<std::is_integral<Int>::value, void>::type>
	inline void GetIndexes(std::size_t globalbin,  Int  (&bins)[N]){

		get_indexes(globalbin, bins);
	}

	template<typename Int,
				typename = typename std::enable_if<std::is_integral<Int>::value, void>::type>
	inline void GetIndexes(std::size_t globalbin, std::array<Int,N>&  bins){

		get_indexes(globalbin, bins);
	}

	template<typename Int,
				typename = typename std::enable_if<std::is_integral<Int>::value, void>::type>
	inline double GetBinContent( Int  (&bins)[N]){

		std::size_t bin=0;

		get_global_bin( bins,  bin);

		std::size_t index = std::distance(fBins.begin(),
								std::find(fBins.begin(),fBins.end(), bin));

				return  (index < fBins.size() ) ?
								fContents.begin()[index] : 0.0;
	}

	inline double GetBinContent(std::array<std::size_t, N> const& bins){

		std::size_t bin=0;

		get_global_bin( bins,  bin);

		std::size_t index = hydra_thrust::distance(fBins.begin(),
				hydra_thrust::find(fSystem , fBins.begin(),fBins.end(), bin));

		return (index < fBins.size() ) ? fContents.begin()[index] : 0.0;
	}


	template<typename Int,
					typename = typename std::enable_if<std::is_integral<Int>::value, void>::type>
	inline double GetBinContent(std::array<Int, N> const& bins){

			std::size_t bin=0;

			get_global_bin( bins,  bin);

			std::size_t index = hydra_thrust::distance(fBins.begin(),
					hydra_thrust::find(fSystem , fBins.begin(),fBins.end(), bin));

			return (index < fBins.size() ) ? fContents.begin()[index] : 0.0;
		}



	inline double GetBinContent( std::size_t  bin){

		std::size_t index = std::distance(fBins.begin(),
				std::find(fBins.begin(),fBins.end(), bin));

		return  (index < fBins.size() ) ?
				fContents.begin()[index] : 0.0;
	}

	inline Range<data_iterator> GetBinsContents() const {

		return make_range( fContents.begin(), fContents.begin() + fNBins);
	}

	inline Range< hydra_thrust::transform_iterator<detail::GetBinCenter<T,N>, keys_iterator> >
	GetBinsCenters() {

		hydra_thrust::transform_iterator<detail::GetBinCenter<T,N>, keys_iterator> first( fBins.begin(),
				detail::GetBinCenter<T,N>( fGrid, fLowerLimits, fUpperLimits) );

		return make_range( first , first + fNBins);
	}


	//stl interface

	pointer_pair data(){
		return std::make_pair(fBins.data() , fContents.data());
	}

	iterator begin(){
		return hydra_thrust::make_zip_iterator(
						hydra_thrust::make_tuple(fBins.begin() ,  fContents.begin()) );
	}

	iterator end(){
		return hydra_thrust::make_zip_iterator(
				hydra_thrust::make_tuple(fBins.end() ,  fContents.end() ));
	}

	const_iterator begin() const {
		return hydra_thrust::make_zip_iterator(
				hydra_thrust::make_tuple(fBins.cbegin() ,  fContents.cbegin() ) );
	}

	const_iterator end() const {
		return hydra_thrust::make_zip_iterator(
				hydra_thrust::make_tuple(fBins.end() ,  fContents.end() ));
	}

	reference operator[](std::size_t i) {
		return *(begin()+i);
	}

    value_type operator[](std::size_t i) const {
		return begin()[i];
	}

	std::size_t size() const	{

		return  hydra_thrust::distance(begin(), end() );
	}

	template<typename Iterator>
	SparseHistogram<T, N, detail::BackendPolicy<BACKEND>, detail::multidimensional>&
	Fill(Iterator begin, Iterator end);

	template<typename Iterator1, typename Iterator2>
	SparseHistogram<T, N, detail::BackendPolicy<BACKEND>, detail::multidimensional>&
	Fill(Iterator1 begin, Iterator1 end, Iterator2 wbegin);


	template<typename Iterable>
	inline typename std::enable_if< hydra::detail::is_iterable<Iterable>::value,
	SparseHistogram<T, N, detail::BackendPolicy<BACKEND>, detail::multidimensional>& >::type
	Fill(Iterable&& container){
		return this->Fill( std::forward<Iterable>(container).begin(),
				std::forward<Iterable>(container).end());
	}

	template<typename Iterable1, typename Iterable2>
	inline typename std::enable_if<
	  hydra::detail::is_iterable<Iterable1>::value &&
	  hydra::detail::is_iterable<Iterable2>::value,
	SparseHistogram<T, N, detail::BackendPolicy<BACKEND>, detail::multidimensional>& >::type
	Fill(Iterable1&& container, Iterable2&& wbegin){
		return this->Fill( std::forward<Iterable1>(container).begin(),std::forward<Iterable1>(container).end(),
				std::forward<Iterable2>(wbegin).begin());
	}


	template<hydra::detail::Backend BACKEND2,typename Iterator>
	inline SparseHistogram<T, N, detail::BackendPolicy<BACKEND>, detail::multidimensional>&
	Fill(detail::BackendPolicy<BACKEND2> const& exec_policy, Iterator begin, Iterator end);

	template<hydra::detail::Backend BACKEND2,typename Iterator1, typename Iterator2>
	inline SparseHistogram<T, N, detail::BackendPolicy<BACKEND>, detail::multidimensional>&
	Fill(detail::BackendPolicy<BACKEND2> const& exec_policy, Iterator1 begin, Iterator1 end, Iterator2 wbegin);



private:

	//k = i_1*(dim_2*...*dim_n) + i_2*(dim_3*...*dim_n) + ... + i_{n-1}*dim_n + i_n

	template<typename Int,std::size_t I>
	typename hydra_thrust::detail::enable_if< (I== N) && std::is_integral<Int>::value, void>::type
	get_global_bin(const Int (&)[N], std::size_t& ){ }

	template<typename Int,std::size_t I=0>
	typename hydra_thrust::detail::enable_if< (I< N) && std::is_integral<Int>::value, void>::type
	get_global_bin(const Int (&indexes)[N], std::size_t& index)
	{
		std::size_t prod =1;
		for(std::size_t i=N-1; i>I; i--)
			prod *=fGrid[i];
		index += prod*indexes[I];

		get_global_bin<Int,I+1>( indexes, index);
	}

	template<typename Int,std::size_t I>
	typename hydra_thrust::detail::enable_if< (I== N) && std::is_integral<Int>::value, void>::type
	get_global_bin( std::array<Int,N> const& , std::size_t& ){ }

	template<typename Int,std::size_t I=0>
	typename hydra_thrust::detail::enable_if< (I< N) && std::is_integral<Int>::value, void>::type
	get_global_bin( std::array<Int,N> const& indexes, std::size_t& index)
	{
		std::size_t prod =1;

		for(std::size_t i=N-1; i>I; i--)
			prod *=fGrid[i];

		index += prod*indexes[I];

		get_global_bin<Int, I+1>( indexes, index);
	}
	/*
	 *  conversion of one-dimensional index to multidimensional one
	 * ____________________________________________________________
	 */

	//----------------------------------------
	// multiply  std::array elements
	//----------------------------------------
	template<std::size_t I>
	typename std::enable_if< (I==N), void  >::type
	multiply( std::array<std::size_t, N> const& , std::size_t&  )
	{ }

	template<std::size_t I=0>
	typename std::enable_if< (I<N), void  >::type
	multiply( std::array<std::size_t, N> const&  obj, std::size_t& result )
	{
		result = I==0? 1.0: result;
		result *= obj[I];
		multiply<I+1>( obj, result );
	}

	//----------------------------------------
	// multiply static array elements
	//----------------------------------------
	template< std::size_t I>
	typename std::enable_if< (I==N), void  >::type
	multiply( std::size_t (&)[N] , std::size_t&  )
	{ }

	template<std::size_t I=0>
	typename std::enable_if< (I<N), void  >::type
	multiply( std::size_t (&obj)[N], std::size_t& result )
	{
		result = I==0? 1.0: result;
		result *= obj[I];
		multiply<I+1>( obj, result );
	}


	//-------------------------
	// std::array version
	//-------------------------
	//end of recursion
	template<typename Int, std::size_t I,
	typename = typename std::enable_if<std::is_integral<Int>::value, void>::type>
	typename std::enable_if< (I==N), void  >::type
	get_indexes(std::size_t,  std::array<Int,N>& )
	{}

	//begin of the recursion
	template<typename Int, std::size_t I=0,
			typename = typename std::enable_if<std::is_integral<Int>::value, void>::type>
	typename std::enable_if< (I<N), void  >::type
	get_indexes(std::size_t index, std::array<Int,N>& indexes)
	{
		std::size_t factor    =  1;
		multiply<I+1>(fGrid, factor );
		indexes[I]  =  index/factor;
		std::size_t next_index =  index%factor;
		get_indexes< Int,I+1>(next_index,indexes );
	}

	//-------------------------
	// static array version
	//-------------------------
	//end of recursion
	template<typename Int, std::size_t I,
	typename = typename std::enable_if<std::is_integral<Int>::value, void>::type>
	typename std::enable_if< (I==N), void  >::type
	get_indexes(std::size_t , Int (&)[N])
	{}

	//begin of the recursion
	template<typename Int, std::size_t I=0,
			typename = typename std::enable_if<std::is_integral<Int>::value, void>::type>
	typename std::enable_if< (I<N), void  >::type
	get_indexes(std::size_t index, Int (&indexes)[N] )
	{
		std::size_t factor    =  1;
		multiply<I+1>(fGrid, factor );
		indexes[I]  =  index/factor;
		std::size_t next_index =  index%factor;
		get_indexes< Int, I+1>(next_index, indexes );
	}


	T fUpperLimits[N];
	T fLowerLimits[N];
	std::size_t   fGrid[N];
	std::size_t   fNBins;
	storage_data_t fContents;
	storage_keys_t fBins;
	system_t fSystem;

};

/**
 * \ingroup histogram
 * Class representing one-dimensional sparse histogram.
 */
template< typename T, hydra::detail::Backend BACKEND >
class SparseHistogram<T,1, detail::BackendPolicy<BACKEND>,  detail::unidimensional >{

	typedef hydra::detail::BackendPolicy<BACKEND>    system_t;

	typedef std::vector<double> storage_data_t;
	typedef std::vector<std::size_t> storage_keys_t;

	typedef typename storage_data_t::iterator data_iterator;
	typedef typename storage_data_t::const_iterator data_const_iterator;
	typedef typename storage_data_t::reference data_reference;
	typedef typename storage_data_t::pointer data_pointer;
	typedef typename storage_data_t::value_type data_value_type;

	typedef typename storage_keys_t::iterator keys_iterator;
	typedef typename storage_keys_t::const_iterator keys_const_iterator;
	typedef typename storage_keys_t::reference keys_reference;
	typedef typename storage_keys_t::pointer keys_pointer;
	typedef typename storage_keys_t::value_type keys_value_type;


	typedef hydra_thrust::zip_iterator<
			hydra_thrust::tuple<keys_iterator, data_iterator>> iterator;
	typedef hydra_thrust::zip_iterator<
			hydra_thrust::tuple<keys_const_iterator, data_const_iterator> > const_iterator;
	typedef typename hydra_thrust::iterator_traits<iterator>::reference reference;
	typedef typename hydra_thrust::iterator_traits<iterator>::value_type value_type;
	typedef std::pair<std::size_t*, double*> pointer_pair;

public:

	SparseHistogram()=delete;


	SparseHistogram( std::size_t grid, T lowerlimits, T upperlimits):
		fGrid(grid),
		fLowerLimits(lowerlimits),
		fUpperLimits(upperlimits),
		fNBins(grid)
	{}


	SparseHistogram(SparseHistogram<T,1, detail::BackendPolicy<BACKEND>,detail::unidimensional > const& other ):
		fContents(other.GetContents()),
		fGrid(other.GetGrid()),
		fLowerLimits(other.GetLowerLimits()),
		fUpperLimits(other.GetUpperLimits()),
		fNBins(other.GetNBins())
	{}

	SparseHistogram<T,1, detail::BackendPolicy<BACKEND>,detail::unidimensional >&
	operator=(SparseHistogram<T,1, detail::BackendPolicy<BACKEND>,detail::unidimensional > const& other )
	{
		if(this==&other) return *this;
		fContents = other.GetContents();
		fGrid = other.GetGrid();
		fLowerLimits = other.GetLowerLimits();
		fUpperLimits = other.GetUpperLimits();
		fNBins = other.GetNBins();
		return *this;
	}

	template<hydra::detail::Backend BACKEND2>
	SparseHistogram(SparseHistogram<T,1, detail::BackendPolicy<BACKEND2>,detail::unidimensional > const& other ):
		fContents(other.GetContents()),
		fGrid(other.GetGrid()),
		fLowerLimits(other.GetLowerLimits()),
		fUpperLimits(other.GetUpperLimits()),
		fNBins(other.GetNBins())
	{}

	template<hydra::detail::Backend BACKEND2>
	SparseHistogram<T,1, detail::BackendPolicy<BACKEND>,detail::unidimensional >&
	operator=(SparseHistogram<T,1, detail::BackendPolicy<BACKEND2>,detail::unidimensional > const& other )
	{
		if(this==&other) return *this;
		fContents = other.GetContents();
		fGrid = other.GetGrid();
		fLowerLimits = other.GetLowerLimits();
		fUpperLimits = other.GetUpperLimits();
		fNBins = other.GetNBins();
		return *this;
	}

	const storage_data_t& GetContents()const  {
		return fContents;
	}

	void SetContents(storage_data_t histogram) {
		fContents = histogram;
	}

	const storage_keys_t& GetBins() const
	{
		return fBins;
	}

	void SetBins(storage_keys_t bins)
	{
		fBins = bins;
	}

	std::size_t GetGrid() const {
		return fGrid;
	}

	T GetLowerLimits() const {
		return fLowerLimits;
	}

	T GetUpperLimits() const {
		return fUpperLimits;
	}

	std::size_t GetNBins() const {
		return fNBins;
	}


	double GetBinContent( std::size_t  bin) {

		std::size_t index = std::distance(fBins.begin(),
				std::find(fBins.begin(),fBins.end(), bin));

		return  ( bin< fBins.size() ) ?
				fContents.begin()[bin] : 0.0;
	}

	inline Range<hydra_thrust::transform_iterator<detail::GetBinCenter<T,1>, keys_iterator> >
	GetBinsCenters() {

		hydra_thrust::transform_iterator<detail::GetBinCenter<T,1>, keys_iterator >
		first( fBins.begin(), detail::GetBinCenter<T,1>( fGrid, fLowerLimits, fUpperLimits) );

		return make_range( first , first+fNBins);
	}

	inline Range<iterator> GetBinsContents()  {

	  	return make_range(begin(),begin()+fNBins );
	}

	//stl interface

	pointer_pair data(){
		return std::make_pair(fBins.data() , fContents.data());
	}

	iterator begin(){
		return hydra_thrust::make_zip_iterator(
						hydra_thrust::make_tuple(fBins.begin() ,  fContents.begin()) );
	}

	iterator end(){
		return hydra_thrust::make_zip_iterator(
				hydra_thrust::make_tuple(fBins.end() ,  fContents.end() ));
	}

	const_iterator begin() const {
		return hydra_thrust::make_zip_iterator(
				hydra_thrust::make_tuple(fBins.cbegin() ,  fContents.cbegin() ) );
	}

	const_iterator end() const {
		return hydra_thrust::make_zip_iterator(
				hydra_thrust::make_tuple(fBins.end() ,  fContents.end() ));
	}

	reference operator[](std::size_t i) {
		return *(begin()+i);
	}

    value_type operator[](std::size_t i) const {
		return begin()[i];
	}

	std::size_t size() const	{

		return  hydra_thrust::distance(begin(), end() );
	}

	template<typename Iterator>
	SparseHistogram<T,1, detail::BackendPolicy<BACKEND>,detail::unidimensional >&
	Fill(Iterator begin, Iterator end);

	template<typename Iterator1, typename Iterator2>
	SparseHistogram<T,1, detail::BackendPolicy<BACKEND>,detail::unidimensional >&
	Fill(Iterator1 begin, Iterator1 end, Iterator2 wbegin);


	template<typename Iterable>
	inline typename std::enable_if< hydra::detail::is_iterable<Iterable>::value,
	SparseHistogram<T,1, detail::BackendPolicy<BACKEND>,detail::unidimensional >& >::type
	Fill(Iterable&& container){
		return this->Fill( std::forward<Iterable>(container).begin(),
				std::forward<Iterable>(container).end());
	}

	template<typename Iterable1, typename Iterable2>
	inline typename std::enable_if< hydra::detail::is_iterable<Iterable1>::value
	&&  hydra::detail::is_iterable<Iterable2>::value,
	SparseHistogram<T,1, detail::BackendPolicy<BACKEND>,detail::unidimensional >& >::type
	Fill(Iterable1&& container, Iterable2&& wbegin){
		return this->Fill( std::forward<Iterable1>(container).begin(),
				std::forward<Iterable1>(container).end(), std::forward<Iterable2>(wbegin).begin());
	}


	template<hydra::detail::Backend BACKEND2,typename Iterator>
	SparseHistogram<T,1, detail::BackendPolicy<BACKEND>,detail::unidimensional >&
	Fill(detail::BackendPolicy<BACKEND2> const& exec_policy, Iterator begin, Iterator end);

	template<hydra::detail::Backend BACKEND2,typename Iterator1, typename Iterator2>
	SparseHistogram<T,1, detail::BackendPolicy<BACKEND>,detail::unidimensional >&
	Fill(detail::BackendPolicy<BACKEND2> const& exec_policy, Iterator1 begin, Iterator1 end, Iterator2 wbegin);



private:



	T fUpperLimits;
	T fLowerLimits;
	std::size_t   fGrid;
	std::size_t   fNBins;
	storage_data_t fContents;
	storage_keys_t fBins;
	system_t fSystem;
};

/**
 * \ingroup histogram
 * \brief Function to make a N-dimensional sparse histogram.
 *
 * @param backend
 * @param grid  std::array storing the bins per dimension.
 * @param lowerlimits std::array storing the lower limits per dimension.
 * @param upperlimits  std::array storing the upper limits per dimension.
 * @param first Iterator pointing to the begin of the data range.
 * @param end Iterator pointing to the end of the data range.
 * @return
 */
template<typename Iterator, typename T, std::size_t N , hydra::detail::Backend BACKEND>
SparseHistogram< T, N,  detail::BackendPolicy<BACKEND>, detail::multidimensional>
make_sparse_histogram( detail::BackendPolicy<BACKEND> backend, std::array<std::size_t, N> grid,
		std::array<T, N> const& lowerlimits,   std::array<T, N> const& upperlimits,
		Iterator first, Iterator end);
/**
 * \ingroup histogram
 * \brief Function to make a N-dimensional sparse histogram.
 *
 * @param backend
 * @param grid  std::array storing the bins per dimension.
 * @param lowerlimits std::array storing the lower limits per dimension.
 * @param upperlimits  std::array storing the upper limits per dimension.
 * @param first Iterator pointing to the begin of the data range.
 * @param end Iterator pointing to the end of the data range.
 * @param wfirst Iterator pointing to the begin of the weights range.
 * @return
 */
template<typename Iterator1,typename Iterator2, typename T, std::size_t N , hydra::detail::Backend BACKEND>
SparseHistogram< T, N,  detail::BackendPolicy<BACKEND>, detail::multidimensional>
make_sparse_histogram( detail::BackendPolicy<BACKEND>, std::array<std::size_t, N> grid,
		std::array<T, N> const& lowerlimits,   std::array<T, N> const& upperlimits,
		Iterator1 first, Iterator1 end, Iterator2 wfirst);

/**
 * \ingroup histogram
 * \brief Function to make a N-dimensional sparse histogram.
 *
 * @param backend
 * @param grid  std::array storing the bins per dimension.
 * @param lowerlimits std::array storing the lower limits per dimension.
 * @param upperlimits  std::array storing the upper limits per dimension.
 * @param data Iterable storing the data to histogram.
 * @return
 */
template< typename T, std::size_t N , hydra::detail::Backend BACKEND, typename Iterable>
inline typename std::enable_if< hydra::detail::is_iterable<Iterable>::value,
SparseHistogram< T, N,  detail::BackendPolicy<BACKEND>, detail::multidimensional>>::type
make_sparse_histogram( detail::BackendPolicy<BACKEND> backend, std::array<std::size_t, N>const& grid,
		std::array<T, N>const&lowerlimits,   std::array<T, N>const& upperlimits,	Iterable&& data);


/**
 * \ingroup histogram
 * \brief Function to make a N-dimensional sparse histogram.
 *
 * @param backend
 * @param grid  std::array storing the bins per dimension.
 * @param lowerlimits std::array storing the lower limits per dimension.
 * @param upperlimits  std::array storing the upper limits per dimension.
 * @param data Iterable storing the data to histogram.
 * @param weights Iterable storing the weights to data.
 * @return
 */
template< typename T, std::size_t N , hydra::detail::Backend BACKEND, typename Iterable1,typename Iterable2 >
inline typename std::enable_if< hydra::detail::is_iterable<Iterable1>::value&&
hydra::detail::is_iterable<Iterable2>::value,
SparseHistogram< T, N,  detail::BackendPolicy<BACKEND>, detail::multidimensional>>::type
make_sparse_histogram( detail::BackendPolicy<BACKEND> backend, std::array<std::size_t, N>const& grid,
		std::array<T, N>const&lowerlimits,   std::array<T, N>const& upperlimits,
		Iterable1&& data, Iterable2&& weights);



/**
 * \ingroup histogram
 * \brief Function to make a 1-dimensional sparse histogram.
 *
 * @param backend
 * @param nbins the number of bins.
 * @param lowerlimit the lower limit.
 * @param upperlimit  the upper limits.
 * @param first Iterator pointing to the begin of the data range.
 * @param end Iterator pointing to the end of the data range.
 * @return
 */
template<typename Iterator, typename T, hydra::detail::Backend BACKEND>
SparseHistogram< T, 1,  detail::BackendPolicy<BACKEND>, detail::multidimensional>
make_sparse_histogram( detail::BackendPolicy<BACKEND> backend, std::size_t nbins, T lowerlimit, T upperlimit,
		Iterator first, Iterator end);

/**
 * \ingroup histogram
 * \brief Function to make a 1-dimensional sparse histogram.
 *
 * @param backend
 * @param nbins number of bins.
 * @param lowerlimit the lower limit.
 * @param upperlimit the upper limit.
 * @param first iterator pointing to the begin of the data range.
 * @param end iterator pointing to the end of the data range.
 * @param wfirst Iterator pointing to the begin of the weights range.
 * @return
 */
template<typename Iterator1, typename Iterator2, typename T, hydra::detail::Backend BACKEND>
SparseHistogram< T, 1,  detail::BackendPolicy<BACKEND>, detail::unidimensional>
make_sparse_histogram( detail::BackendPolicy<BACKEND>, std::size_t nbins, T lowerlimit,  T upperlimit,
		Iterator1 first, Iterator1 end, Iterator2 wfirst);

/**
 * \ingroup histogram
 * \brief Function to make a 1-dimensional sparse histogram.
 *
 * @param backend
 * @param nbins   number of bins.
 * @param lowerlimit the lower limit.
 * @param upperlimit the upper limit.
 * @param data Iterable storing the data to histogram.
 * @return
 */
template< typename T, hydra::detail::Backend BACKEND, typename Iterable>
inline typename std::enable_if< hydra::detail::is_iterable<Iterable>::value,
SparseHistogram< T, 1,  detail::BackendPolicy<BACKEND>, detail::unidimensional>>::type
make_sparse_histogram( detail::BackendPolicy<BACKEND> backend, std::size_t nbins,
		T lowerlimit,  T upperlimit,	Iterable&& data);

/**
 * \ingroup histogram
 * \brief Function to make a 1-dimensional sparse histogram.
 *
 * @param backend
 * @param nbins   number of bins.
 * @param lowerlimit the lower limit.
 * @param upperlimit the upper limit.
 * @param data Iterable storing the data to histogram.
 * @param weight Iterable storing the weights to data.
 * @return
 */
template< typename T, hydra::detail::Backend BACKEND, typename Iterable1,typename Iterable2 >
inline typename std::enable_if< hydra::detail::is_iterable<Iterable1>::value&&
hydra::detail::is_iterable<Iterable2>::value,
SparseHistogram< T, 1,  detail::BackendPolicy<BACKEND>, detail::unidimensional>>::type
make_sparse_histogram( detail::BackendPolicy<BACKEND> backend, std::size_t nbins,
		T lowerlimit, T upperlimit,	Iterable1&& data, Iterable2&& weights);

}  // namespace hydra

#include <hydra/detail/SparseHistogram.inl>

#endif /* SPARSEHISTOGRAM_H_ */
