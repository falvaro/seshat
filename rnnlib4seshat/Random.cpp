/*Copyright 2009,2010 Alex Graves

This file is part of RNNLIB.

RNNLIB is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RNNLIB is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RNNLIB.  If not, see <http://www.gnu.org/licenses/>.*/

#include "Random.hpp"
#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/lagged_fibonacci.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/local_time/local_time.hpp>

using namespace std;
using namespace boost;
using namespace boost::random;
using namespace boost::posix_time;
using namespace boost::gregorian;

typedef mt19937 BaseGenType;
static BaseGenType generator(42u);

real_t Random::normal()
{
	static variate_generator<BaseGenType&, normal_distribution<real_t> > norm(generator, normal_distribution<real_t>());
	return norm();
}
real_t Random::normal(real_t dev, real_t mean)
{
	return (normal() * dev) + mean;
}
unsigned int Random::set_seed(unsigned int seed)
{
	if (seed == 0)
	{
		time_period p (ptime(date(1970, Jan, 01)), microsec_clock::local_time());
		seed = (unsigned int)p.length().ticks();
	}
	srand(seed);
	generator.seed(seed);
	return seed;
}
real_t Random::uniform(real_t range)
{
	return (uniform()*2*range) - range;
}
real_t Random::uniform()
{
	static variate_generator<BaseGenType&, uniform_real<real_t> > uni(generator, uniform_real<real_t>());
	return uni();
}
