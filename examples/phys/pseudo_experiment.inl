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
 * pseudo_experiment.inl
 *
 *  Created on: 14/11/2019
 *      Author: Antonio Augusto Alves Junior
 */

#ifndef PSEUDO_EXPERIMENT_INL_
#define PSEUDO_EXPERIMENT_INL_

/**
 * \example pseudo_experiment.inl
 *
 */

/**
 * \brief This example shows how to implement a fast pseudo-experiment chain
 * to estimate the value and uncertainty of observables using SPlots and Booststrapping
 * in a conceptualy consistent way.
 */

#include <iostream>
#include <assert.h>
#include <time.h>
#include <chrono>
#include <random>
#include <future>
#include <random>

//command line
#include <tclap/CmdLine.h>

//this lib
#include <hydra/device/System.h>
#include <hydra/host/System.h>
#include <hydra/Function.h>
#include <hydra/Algorithm.h>
#include <hydra/FunctionWrapper.h>
#include <hydra/Random.h>
#include <hydra/LogLikelihoodFCN.h>
#include <hydra/Parameter.h>
#include <hydra/UserParameters.h>
#include <hydra/Pdf.h>
#include <hydra/AddPdf.h>
#include <hydra/Algorithm.h>
#include <hydra/Filter.h>
#include <hydra/GaussKronrodQuadrature.h>
#include <hydra/SPlot.h>
#include <hydra/DenseHistogram.h>
#include <hydra/SparseHistogram.h>

#include <hydra/functions/Gaussian.h>
#include <hydra/functions/Exponential.h>
#include <hydra/functions/BreitWignerNR.h>
#include <hydra/functions/Exponential.h>
#include <hydra/functions/Gaussian.h>
#include <hydra/functions/Chebychev.h>

#include <hydra/Placeholders.h>

//Minuit2
#include "Minuit2/FunctionMinimum.h"
#include "Minuit2/MnUserParameterState.h"
#include "Minuit2/MnPrint.h"
#include "Minuit2/MnMigrad.h"
#include "Minuit2/MnMinimize.h"
#include "Minuit2/MnMinos.h"
#include "Minuit2/MnContours.h"
#include "Minuit2/CombinedMinimizer.h"
#include "Minuit2/MnPlot.h"
#include "Minuit2/MinosError.h"
#include "Minuit2/ContoursError.h"
#include "Minuit2/VariableMetricMinimizer.h"

 // Include classes from ROOT
#ifdef _ROOT_AVAILABLE_

#include <TROOT.h>
#include <TH1D.h>
#include <TApplication.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TStyle.h>
#endif //_ROOT_AVAILABLE_

using namespace hydra::placeholders;
using namespace ROOT::Minuit2;

int main(int argv, char** argc)
{
	size_t nentries = 0;
	size_t nstudies = 0;

	try {

		TCLAP::CmdLine cmd("Command line arguments for ", '=');

		TCLAP::ValueArg<size_t> EArg("n", "number-of-events","Number of events", true, 10e6, "size_t");
		cmd.add(EArg);

		TCLAP::ValueArg<size_t> DArg("m", "number-of-studies","Number of MC studies", true, 1000, "size_t");
		cmd.add(DArg);

		// Parse the argv array.
		cmd.parse(argv, argc);

		// Get the value parsed by each arg.
		nentries = EArg.getValue();
		nstudies = DArg.getValue();

	}
	catch (TCLAP::ArgException &e)  {
		std::cerr << "error: " << e.error() << " for arg " << e.argId()
														<< std::endl;
	}

	/*
     * Dataset layout: the data points are 2 dimensional. Each data point can be
     * represented by hydra::tuple<double, double> object. The values in the dimension <0>
     * represents the discriminant variable, which is distributed following a gaussian + argus.
     * The dimension <1> is the observable, which is distributed following a non-relativistic breit-wigner + exponential
     *
     */

	//======================================================
	// Pseudo-experiment Model
	//======================================================

	//======================================================
	// 1) Gaussian + Exponential model (dimension <0>)
	// data range
    double data_min   =  0.0;
    double data_max   =  10.0;

	//Gaussian

	//parameters
	hydra::Parameter  mean  = hydra::Parameter::Create().Name("mean").Value( 5.0).Error(0.0001).Limits(4.9, 5.1);
	hydra::Parameter  sigma = hydra::Parameter::Create().Name("sigma").Value(0.5).Error(0.0001).Limits(0.4, 0.6);

	//gaussian function evaluating on the first argument
	auto Gaussian_PDF = hydra::make_pdf( hydra::Gaussian<>(mean, sigma),
			hydra::AnalyticalIntegral<hydra::Gaussian<>>(data_min, data_max));

	//-------------------------------------------
	//Exponential
    //parameters
    auto  tau  = hydra::Parameter::Create().Name("tau").Value(-0.2).Error(0.0001).Limits(-0.3, -0.1);

    //Background PDF
    auto Exponential_PDF = hydra::make_pdf(hydra::Exponential<>(tau),
    		 hydra::AnalyticalIntegral<hydra::Exponential<>>(data_min, data_max));

	//------------------

	//yields
	hydra::Parameter N_Exponential("N_Exponential", nentries/2, 100, 100 , nentries) ;
	hydra::Parameter N_Gaussian("N_Gaussian"      , nentries/2, 100, 100 , nentries) ;

	//make model
	auto splot_model = hydra::add_pdfs( {N_Gaussian, N_Exponential}, Gaussian_PDF, Exponential_PDF);
	splot_model.SetExtended(1);

	//======================================================
	// 2) Breit-Wigner  (dimension <1>)
	//-----------------
	// data range
	double obs_min   =  0.0;
	double obs_max   =  15.0;

	//Breit-Wigner

	//parameters
	hydra::Parameter  mass  = hydra::Parameter::Create().Name("mass" ).Value(6.0).Error(0.0001).Limits(5.0,7.0);
	hydra::Parameter  width = hydra::Parameter::Create().Name("width").Value(1.0).Error(0.0001).Limits(0.5,1.5);

	//Breit-Wigner function evaluating on the first argument
	auto BreitWigner_PDF = hydra::make_pdf( hydra::BreitWignerNR<>(mass, width ),
			hydra::AnalyticalIntegral<hydra::BreitWignerNR<>>(obs_min, obs_max));

    //-------------------------------------------


	//======================================================
	// Pseudo-experiment data sample generation
	//======================================================
	//

	//dataset
	hydra::multiarray<double,2, hydra::host::sys_t> dataset(nentries);

	//generate the primary dataset using std random device and multithread facility
	{
		//generator

		//fill Gaussian component in a separated thread
		auto gaussian_handler = std::async(std::launch::async,
				[data_min, data_max, mean, sigma, nentries, &dataset]( ){

			//Standard mersenne_twister_engine seeded with rd()
			std::mt19937 gen( 258 );

			std::normal_distribution<> dist(mean, sigma);

			auto first = dataset.begin(_0);
			auto last  = dataset.begin(_0) + int(nentries/2);

			auto it = first;
			do
			{
				double x= dist(gen);
				if((x > data_min) &&  (x < data_max) )
				{
					*it=x;
					++it;
				}

			}while(it != last);

			return hydra::make_range(first, last);
		} );


		//fill Expontial component in a separated thread
		auto exp_handler = std::async(std::launch::async,
				[data_min, data_max, tau, nentries, &dataset]( ){

			//Standard mersenne_twister_engine seeded with rd()
			std::mt19937 gen( 456 );

			std::exponential_distribution<> dist(-tau);

			auto first = dataset.begin(_0) + int(nentries/2);
			auto last  = dataset.end(_0);

			auto it = first;

			do
			{
				double x= dist(gen);//*( data_max - data_min) + data_min;

				if((x > data_min) &&  (x < data_max) )
				{
					*it=x;
					++it;
				}

			}while(it != last);

			return hydra::make_range(first, last);
		} );



		//fill Breit-Wigner component in a separated thread
		auto breit_wigner_handler = std::async(std::launch::async,
				[obs_min, obs_max, mass, width, nentries, &dataset]( ){

			std::mt19937 gen( 159 ); //Standard mersenne_twister_engine seeded with rd()

			std::uniform_real_distribution<> dist(0.0, 1.0);

			auto first = dataset.begin(_1) ;
			auto last  = dataset.begin(_1) + int(nentries/2);

			auto breit_wigner_dist = [](double mean, double width, double rnd){

				return  mean + 0.5*width*tan(PI*(rnd-0.5));
			};

			auto it = first;
			do
			{
				double rnd = dist(gen);
				double x =breit_wigner_dist(mass, width, rnd);

				if((x > obs_min) &&  (x < obs_max) )
				{
					*it=x;
					++it;
				}

			}while(it != last);

			return hydra::make_range(first, last);
		} );


		//fill noise component in a separated thread
		auto noise_handler = std::async(std::launch::async,
				[obs_min, obs_max, nentries, &dataset]( ){

			std::mt19937 gen( 753 ); //Standard mersenne_twister_engine seeded with rd()

			std::uniform_real_distribution<> dist(obs_min, obs_max);

			auto first = dataset.begin(_1) + int(nentries/2);
			auto last  = dataset.end(_1);

			auto it = first;

			do
			{
				double x= dist(gen);
				*it=x;
				++it;


			}while(it != last);

			return hydra::make_range(first, last);
		} );

		//wait the threads to finish their tasks
		gaussian_handler.wait();
		exp_handler.wait();
		breit_wigner_handler.wait();
		noise_handler.wait();

		//shake the coquetel well
		std::random_shuffle(dataset.begin(), dataset.end());

		std::cout <<  " Dataset " <<  " size:  " << dataset.size() << std::endl;
		for(int i=0; i<100; i++){
			std::cout << i << ") "<< dataset[i] << std::endl;
		}
	}

	//device
	//------------------------
#ifdef _ROOT_AVAILABLE_

	TH1::SetDefaultBufferSize(nstudies);

	TH1D hist_data_sfit("data_sfit", "Control variable", 100, data_min, data_max);
	TH1D hist_data_observable("data_observable", "Observable", 100, obs_min, obs_max);

	TH1D hist_N_Gaussian("N_Gaussian", "N_Gaussian", 100, 1.0, 0.0);
	TH1D hist_N_Exponential("N_Exponential", "N_Exponential", 100, 1.0, 0.0);
	TH1D hist_mean("mean", "mean", 100, 1.0, 0.0);
	TH1D hist_sigma("sigma", "sigma", 100, 1.0, 0.0 );
	TH1D hist_tau("tau", "tau", 100, 1.0, 0.0 );
	TH1D hist_mass("mass", "mass", 100, 1.0, 0.0 );
	TH1D hist_mass_error("mass_error", "mass error", 100, 1.0, 0.0 );
	TH1D hist_width("width", "width", 100, 1.0, 0.0 );
	TH1D hist_width_error("width_error", "width error", 100, 1.0, 0.0 );

	for(auto x: dataset){
		hist_data_sfit.Fill( hydra::get<0>(x) );
		hist_data_observable.Fill( hydra::get<1>(x) );
	}

#endif //_ROOT_AVAILABLE_



/* this scope will run a loop where:
 * 1- a new sample will be produced at same statistical level
 * 2- perform a splot to obtain a background free sample, which will contain negative weights
 * 3- perform a fit and store the results
 * 4- repeat the loop
 */
	hydra::multiarray<double, 9, hydra::host::sys_t> variable_log{};

	{

		for(size_t study=0; study<nstudies; study++)
		{
			//====================================================================
			// PSEUDO-SAMPLE PRODUCTION AND SPLOT
			//====================================================================

			//boost_strapped data (bs-data)
			auto bs_range = hydra::boost_strapped_range( dataset,
					std::chrono::system_clock::now().time_since_epoch().count() + 10*study);

			//bring the bs-data to the device
			hydra::multiarray<double,2, hydra::device::sys_t> dataset_device( bs_range.begin(),
					bs_range.begin() + dataset.size());


			//create fcn for sfit
			auto splot_fcn = hydra::make_loglikehood_fcn(splot_model,
					hydra::columns(dataset_device, _0) );

			//print level
			ROOT::Minuit2::MnPrint::SetLevel(1);
			hydra::Print::SetLevel(hydra::WARNING);

			//minimization strategy
			MnStrategy strategy(2);

			// create Migrad minimizer
			MnMigrad migrad_splot(splot_fcn, splot_fcn.GetParameters().GetMnState(), strategy);

			//std::cout<< splot_fcn.GetParameters().GetMnState() << std::endl;

			// ... Minimize and profile the time

			auto start = std::chrono::high_resolution_clock::now();

			FunctionMinimum minimum_splot =  FunctionMinimum(migrad_splot(1000, 5));

			auto end = std::chrono::high_resolution_clock::now();

			std::chrono::duration<double, std::milli> elapsed = end - start;

			// output
			std::cout<<"SFit minimum: "<< minimum_splot << std::endl;

			//time
			std::cout << "-----------------------------------------"<<std::endl;
			std::cout << "| [Fit Time] (ms) = " << elapsed.count() <<std::endl;
			std::cout << "-----------------------------------------"<<std::endl;

			//--------------------------------------------
			//perform splot for two components
			//allocate memory to hold weights
			hydra::multiarray<double, 2, hydra::device::sys_t> sweigts_device( dataset_device.size() );

			//create splot
			auto splot  = hydra::make_splot( splot_fcn.GetPDF() );

			start = std::chrono::high_resolution_clock::now();

			auto covar = splot.Generate( hydra::columns(dataset_device, _0), sweigts_device);

			end = std::chrono::high_resolution_clock::now();

			elapsed = end - start;

			//time
			std::cout << "-----------------------------------------"<<std::endl;
			std::cout << "| SFit time (ms) ="<< elapsed.count()  <<std::endl;
			std::cout << "-----------------------------------------"<<std::endl;

			std::cout << "SFit covariance matrix: "
					<< std::endl
					<< covar
					<< std::endl;

			std::cout << std::endl
					<< "sWeights ( Gaussian, Exponential):"
					<< std::endl;

			for(size_t i = 0; i< 10; i++)
				std::cout << i << ") :"
				<< sweigts_device[i]
				                  << std::endl;


			//====================================================================
			// MAIN FIT AND OBSERVABLE ESTIMATION
			//====================================================================

			auto fcn = hydra::make_loglikehood_fcn(BreitWigner_PDF,//observable_model,
					hydra::columns(dataset_device, _1),
					hydra::columns(sweigts_device, _0) );

			// create Migrad minimizer
			MnMigrad migrad(fcn, fcn.GetParameters().GetMnState(), strategy);

			//std::cout << fcn.GetParameters().GetMnState() << std::endl;

			// ... Minimize and profile the time

			start = std::chrono::high_resolution_clock::now();

			FunctionMinimum minimum =  FunctionMinimum(migrad(1000, 5));

			end = std::chrono::high_resolution_clock::now();

			elapsed = end - start;
			// output
			std::cout <<"Fit minimum: "
					<< minimum
					<< std::endl
					<< std::endl;

			//time
			std::cout << "-----------------------------------------"<<std::endl;
			std::cout << "| Fit time (ms) = " << elapsed.count()  <<std::endl;
			std::cout << "-----------------------------------------"<<std::endl;

			//add results to log
			// 0  | N_Gaussian
			// 1  | N_Exponential
			// 2  | mean
			// 3  | sigma
			// 4  | tau
			// 5  | mass
			// 6  | width

			std::cout << "----------------------------------------"  << std::endl;
			std::cout << "Logging result #" << study << " "                         << std::endl;
			std::cout << "----------------------------------------"  << std::endl;
			std::cout << "N_Gaussian ..... " << minimum_splot.UserParameters().Value("N_Gaussian") <<std::endl;
			std::cout << "N_Exponential .. " << minimum_splot.UserParameters().Value("N_Exponential") <<std::endl;
			std::cout << "mean ........... " << minimum_splot.UserParameters().Value("mean") <<std::endl;
			std::cout << "sigma .......... " << minimum_splot.UserParameters().Value("sigma")<<std::endl ;
			std::cout << "tau ............ " << minimum_splot.UserParameters().Value("tau") <<std::endl;
			std::cout << "mass ........... " << minimum.UserParameters().Value("mass") <<std::endl;
			std::cout << "mass error ..... " << minimum.UserParameters().Error("mass") <<std::endl;
			std::cout << "width .......... " << minimum.UserParameters().Value("width") <<std::endl;
			std::cout << "width error .... " << minimum.UserParameters().Error("width") <<std::endl;
			std::cout << "----------------------------------------"  << std::endl;

			variable_log.push_back( hydra::make_tuple(
					minimum_splot.UserParameters().Value("N_Gaussian"),
					minimum_splot.UserParameters().Value("N_Exponential"),
					minimum_splot.UserParameters().Value("mean"),
					minimum_splot.UserParameters().Value("sigma"),
					minimum_splot.UserParameters().Value("tau"),
					minimum.UserParameters().Value("mass"),
					minimum.UserParameters().Error("mass"),
					minimum.UserParameters().Value("width"),
					minimum.UserParameters().Error("width")
					));

		}
		std::cout << std::endl << std::endl << std::endl;
		std::cout << "----------------------------------------"  << std::endl;
		std::cout << "Dumping logged studies: "  <<     variable_log.size()            << std::endl;
		std::cout << "----------------------------------------"  << std::endl;

		for(auto x:variable_log)
		{
			std::cout << x << std::endl;

			hist_N_Gaussian.Fill(hydra::get<0>(x));
			hist_N_Exponential.Fill(hydra::get<1>(x));
			hist_mean.Fill(hydra::get<2>(x));
			hist_sigma.Fill(hydra::get<3>(x));
			hist_tau.Fill(hydra::get<4>(x));
			hist_mass.Fill(hydra::get<5>(x));
			hist_mass_error.Fill(hydra::get<6>(x));
			hist_width.Fill(hydra::get<7>(x));
			hist_width_error.Fill(hydra::get<8>(x));

		}


	}

#ifdef _ROOT_AVAILABLE_

	TApplication *myapp=new TApplication("myapp",0,0);

	//draw histograms
	TCanvas canvas_13("canvas_13" ,"Dataset", 1000, 500);
	canvas_13.Divide(2);
    canvas_13.cd(1);
	hist_data_sfit.Draw("hist");
	canvas_13.cd(2);
	hist_data_observable.Draw("hist");

    gStyle->SetOptFit(111);

    TCanvas canvas_0("canvas_0" ,"", 500, 500);
	hist_N_Gaussian.SetLineWidth(2);
	hist_N_Gaussian.Draw("E");
	hist_N_Gaussian.Fit("gaus");
	canvas_0.Update();

	TCanvas canvas_1("canvas_1" ,"", 500, 500);
	hist_N_Exponential.SetLineWidth(2);
	hist_N_Exponential.Draw("E");
	hist_N_Exponential.Fit("gaus");
	canvas_1.Update();

	TCanvas canvas_2("canvas_2" ,"", 500, 500);
	hist_mean.SetLineWidth(2);
	hist_mean.Draw("E");
	hist_mean.Fit("gaus");
	canvas_2.Update();

	TCanvas canvas_3("canvas_3" ,"", 500, 500);
	hist_sigma.SetLineWidth(2);
	hist_sigma.Draw("E");
	hist_sigma.Fit("gaus");
	canvas_3.Update();

	TCanvas canvas_4("canvas_4" ,"", 500, 500);
	hist_tau.SetLineWidth(2);
	hist_tau.Draw("E");
	hist_tau.Fit("gaus");
	canvas_4.Update();

	TCanvas canvas_5("canvas_5" ,"", 500, 500);
	hist_mass.SetLineWidth(2);
	hist_mass.Draw("E");
	hist_mass.Fit("gaus");
	canvas_5.Update();

	TCanvas canvas_6("canvas_6" ,"", 500, 500);
	hist_mass_error.SetLineWidth(2);
	hist_mass_error.Draw("E");
	hist_mass_error.Fit("gaus");
	canvas_6.Update();

	TCanvas canvas_7("canvas_7" ,"", 500, 500);
	hist_width.SetLineWidth(2);
	hist_width.Draw("E");
	hist_width.Fit("gaus");
	canvas_7.Update();

	TCanvas canvas_8("canvas_8" ,"", 500, 500);
	hist_width_error.Draw("hist");
	hist_width_error.SetLineWidth(2);
	hist_width_error.Draw("E");
	hist_width_error.Fit("gaus");
	canvas_8.Update();

	myapp->Run();

#endif //_ROOT_AVAILABLE_

	return 0;



}
#endif /* PSEUDO_EXPERIMENT_INL_ */