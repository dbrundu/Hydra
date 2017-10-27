/*----------------------------------------------------------------------------
 *
 *   Copyright (C) 2016-2017 Antonio Augusto Alves Junior
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
 * multiarray_container.inl
 *
 *  Created on: 27/10/2017
 *      Author: Antonio Augusto Alves Junior
 */

#ifndef MULTIARRAY_CONTAINER_INL_
#define MULTIARRAY_CONTAINER_INL_



#include <iostream>
#include <assert.h>
#include <time.h>
#include <chrono>

#include <hydra/device/System.h>
#include <hydra/host/System.h>

#include <hydra/Vector3R.h>
#include <hydra/Vector4R.h>
#include <hydra/Complex.h>
#include <hydra/Tuple.h>
#include <hydra/multiarray2.h>
#include <hydra/Placeholders.h>

//command line
#include <tclap/CmdLine.h>


using namespace hydra::placeholders;

int main(int argv, char** argc)
{
	size_t nentries = 0;

	try {

		TCLAP::CmdLine cmd("Command line arguments for ", '=');

		TCLAP::ValueArg<size_t> EArg("n", "number-of-events","Number of events", true, 10e6, "size_t");
		cmd.add(EArg);

		// Parse the argv array.
		cmd.parse(argv, argc);

		// Get the value parsed by each arg.
		nentries = EArg.getValue();

	}
	catch (TCLAP::ArgException &e)  {
		std::cerr << "error: " << e.error() << " for arg " << e.argId()
														<< std::endl;
	}


	auto caster = []__host__ __device__ ( hydra::tuple<double,double, double,double, double, double>& entry)
	{
		return hydra::make_tuple(
				hydra::complex<double>( hydra::get<0>(entry),hydra::get<1>(entry) ),
				hydra::Vector4R( hydra::get<2>(entry), hydra::get<3>(entry) , hydra::get<4>(entry),hydra::get<5>(entry)));
	};

	auto reverse_caster = [] __host__ __device__ ( hydra::tuple<hydra::complex<int>, hydra::Vector4R> const& entry)
	{
		hydra::complex<double> cvalue  = hydra::get<0>(entry);
		hydra::Vector4R     vector4 = hydra::get<1>(entry);

		return hydra::make_tuple( cvalue.real(), cvalue.imag(),
				vector4.get(0), vector4.get(1), vector4.get(2), vector4.get(3));
	};

	//device
	{
		std::cout << "=========================================="<<std::endl;
		std::cout << "|            <--- DEVICE --->            |"<<std::endl;
		std::cout << "=========================================="<<std::endl;


		hydra::multiarray2< 6,double , hydra::device::sys_t> mvector_d;

		//push_back tuple
		for(size_t i=0; i<nentries; i++ )
			mvector_d.push_back(hydra::tuple<double,double, double,double, double, double>(i, i, i, i, i, i ));

		//print 10 first elements
		std::cout<< std::endl << "________________________________________________________________________________" << std::endl<< std::endl;
		for(size_t i=0; i<10; i++ )
			std::cout << i << ": "<< mvector_d[i] << std::endl;

		std::cout<< std::endl << " Vector capacity: "
				<< mvector_d.capacity()
				<< "  size: "
				<< mvector_d.size()
				<< std::endl << std::endl;

		//multiply first column by 2 using hydra::begin; hydra::end
		for(auto x=hydra::begin<0>(mvector_d);
				x!=hydra::end<0>(mvector_d); x++ ) *x *=2 ;

		//multiply second column by 4 using placeholders
		for(auto x=mvector_d.begin(_0);
				x!= mvector_d.end(_0); x++ ) *x *=4 ;

		//add third and fourth columns by 1 using placeholders
		for(size_t i=0; i<mvector_d.size(); i++ ) {

			mvector_d[_2][i] +=1.0;
			mvector_d[_3][i] +=1.0;
		}

		//print 10 first elements again
		std::cout<< std::endl << "________________________________________________________________________________" << std::endl<< std::endl;
		std::cout<< std::endl << "(i , j, k, l, m, n ) ->  (2*i , *4j, k+1, l+1, m, n ) " << std::endl<< std::endl;
		for(size_t i=0; i<10; i++ )
			std::cout << i << ": "<< mvector_d[i] << std::endl;

		std::cout<< std::endl << " Vector capacity: "
				<< mvector_d.capacity()
				<< "  size: "
				<< mvector_d.size()
				<< std::endl << std::endl;

		//cast contents to a hydra::tuple<hydra::complex<int>, hydra::Vector4R>


		//print 10 first elements
		std::cout<< std::endl << "________________________________________________________________________________" << std::endl<< std::endl;
		std::cout<< std::endl << "(i , j, k, l, m, n ) ->  ( hydra::complex(i , j), hydra::Vector4R(k, l, m, n) ) " << std::endl<< std::endl;
		for(size_t i=0; i<10; i++ )
			std::cout << i << ": "<< mvector_d[caster][i] << std::endl;

		std::cout<< std::endl << " Vector capacity: "
				<< mvector_d.capacity()
				<< "  size: "
				<< mvector_d.size()
				<< std::endl << std::endl;

		//clear vector and push_back directly hydra::tuple<hydra::complex<int>, hydra::Vector4R> using a caster
		std::cout<< std::endl << "________________________________________________________________________________" << std::endl<< std::endl;
		mvector_d.clear();
		std::cout<< std::endl << " Vector capacity: "
				<< mvector_d.capacity()
				<< "  size: "
				<< mvector_d.size()
				<< std::endl << std::endl;



		for(size_t i=0; i<nentries; i++ ){

			hydra::complex<double> cvalue(i, i);
			hydra::Vector4R     vector4(i, i, i, i );

			mvector_d.push_back( reverse_caster, hydra::make_tuple(cvalue, vector4));
		}

		std::cout<< std::endl << "Print current content  " << std::endl<< std::endl;
		for(size_t i=0; i<10; i++ )
			std::cout << i << ": "<< mvector_d[i] << std::endl;

		std::cout<< std::endl << " Vector capacity: "
				<< mvector_d.capacity()
				<< "  size: "
				<< mvector_d.size()
				<< std::endl << std::endl;
	}//device

	//host
	{

		std::cout << "=========================================="<<std::endl;
		std::cout << "|              <--- HOST --->            |"<<std::endl;
		std::cout << "=========================================="<<std::endl;

		hydra::multiarray2<6, double, hydra::host::sys_t>   mvector_h;

		//push_back tuple
		for(size_t i=0; i<nentries; i++ )
			mvector_h.push_back(hydra::tuple< double,double,double,double, double, double>(i, i, i, i, i, i ));

		//print 10 first elements
		std::cout<< std::endl << "________________________________________________________________________________" << std::endl<< std::endl;
		for(size_t i=0; i<10; i++ )
			std::cout << i << ": "<< mvector_h[i] << std::endl;

		std::cout<< std::endl << " Vector capacity: "
				<< mvector_h.capacity()
				<< "  size: "
				<< mvector_h.size()
				<< std::endl << std::endl;

		//multiply first column by 2 using hydra::begin; hydra::end
		for(auto x=hydra::begin<0>(mvector_h);
				x!=hydra::end<0>(mvector_h); x++ ) *x *=2 ;

		//multiply second column by 4 using placeholders
		for(auto x=mvector_h.begin(_0);
				x!= mvector_h.end(_0); x++ ) *x *=4 ;

		//add third and fourth columns by 1 using placeholders
		for(size_t i=0; i<mvector_h.size(); i++ ) {

			mvector_h[_2][i] +=1.0;
			mvector_h[_3][i] +=1.0;
		}

		//print 10 first elements again
		std::cout<< std::endl << "________________________________________________________________________________" << std::endl<< std::endl;
		std::cout<< std::endl << "(i , j, k, l, m, n ) ->  (2*i , *4j, k+1, l+1, m, n ) " << std::endl<< std::endl;
		for(size_t i=0; i<10; i++ )
			std::cout << i << ": "<< mvector_h[i] << std::endl;

		std::cout<< std::endl << " Vector capacity: "
				<< mvector_h.capacity()
				<< "  size: "
				<< mvector_h.size()
				<< std::endl << std::endl;

		//cast contents to a hydra::tuple<hydra::complex<int>, hydra::Vector4R>

		//print 10 first elements
		std::cout<< std::endl << "________________________________________________________________________________" << std::endl<< std::endl;
		std::cout<< std::endl << "(i , j, k, l, m, n ) ->  ( hydra::complex(i , j), hydra::Vector4R(k, l, m, n) ) " << std::endl<< std::endl;
		for(size_t i=0; i<10; i++ )
			std::cout << i << ": "<< mvector_h[caster][i] << std::endl;

		std::cout<< std::endl << " Vector capacity: "
				<< mvector_h.capacity()
				<< "  size: "
				<< mvector_h.size()
				<< std::endl << std::endl;

		//clear vector and push_back directly hydra::tuple<hydra::complex<int>, hydra::Vector4R> using a caster
		std::cout<< std::endl << "________________________________________________________________________________" << std::endl<< std::endl;
		mvector_h.clear();
		std::cout<< std::endl << " Vector capacity: "
				<< mvector_h.capacity()
				<< "  size: "
				<< mvector_h.size()
				<< std::endl << std::endl;


		for(size_t i=0; i<nentries; i++ ){

			hydra::complex<double> cvalue(i, i);
			hydra::Vector4R     vector4(i, i, i, i );

			mvector_h.push_back( reverse_caster, hydra::make_tuple(cvalue, vector4));
		}

		std::cout<< std::endl << "Print current content  " << std::endl<< std::endl;
		for(size_t i=0; i<10; i++ )
			std::cout << i << ": "<< mvector_h[i] << std::endl;

		std::cout<< std::endl << " Vector capacity: "
				<< mvector_h.capacity()
				<< "  size: "
				<< mvector_h.size()
				<< std::endl << std::endl;
	}//device


	return 0;
}


#endif /* MULTIARRAY_CONTAINER_INL_ */
