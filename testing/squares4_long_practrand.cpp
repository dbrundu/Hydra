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
 * squares4_long_practrand.cpp
 *
 *  Created on: 06/10/2020
 *      Author: Antonio Augusto Alves Junior
 */


#include <cstdio>
#include <cstdint>

//hydra
#include <hydra/Random.h>

//command line
#define TCLAP_SETBASE_ZERO 1
#include <tclap/CmdLine.h>

//set a global seed
static const uint64_t default_seed= 0x548c9decbce65295  ;

int main(int argv, char** argc)
{
	uint64_t seed = default_seed;
	bool     jump = 0;

	try {

		TCLAP::CmdLine cmd("Command line arguments for ", '=');

		TCLAP::ValueArg<uint64_t> SeedArg("s", "seed", "RNG seed.", false, default_seed, "uint64_t");
		cmd.add(SeedArg);

		TCLAP::SwitchArg JumpArg("j", "jump", "Discard first 2^{64} (half period).", false) ;
		cmd.add(JumpArg);

		// Parse the argv array.
		cmd.parse(argv, argc);

		// Get the value parsed by each arg.
		seed    = SeedArg.getValue();
        jump    = JumpArg.getValue();
	}
	catch (TCLAP::ArgException &e)  {
		std::cerr << "error: " << e.error() << " for arg " << e.argId()	<< std::endl;
	}

	hydra::squares4_long RNG(seed);

	if( jump ) RNG.discard(RNG.max);

	constexpr size_t BUFFER_SIZE = 1024 * 1024 / sizeof(uint64_t);
		static uint64_t  buffer[BUFFER_SIZE];

	while (1) {

		for(size_t i = 0; i < BUFFER_SIZE; ++i)
			buffer[i] = RNG();

		fwrite((void*) buffer, sizeof(buffer[0]), BUFFER_SIZE, stdout);
	}
}
