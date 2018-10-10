/*Copyright 2014 Francisco Alvaro

This file is part of SESHAT.

  SESHAT is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  SESHAT is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with SESHAT.  If not, see <http://www.gnu.org/licenses/>.


This file is a modification of the RNNLIB original software covered by
the following copyright and permission notice:

*/
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

#include "WeightContainer.hpp"

void perturb_weight(real_t& weight, real_t stdDev, bool additive)
{
	weight += Random::normal(fabs(additive ? stdDev : stdDev * weight));
}


template <class R> void perturb_weights(R& weights, real_t stdDev, bool additive)
{
	LOOP(real_t& w, weights)
	{
		perturb_weight(w, stdDev, additive);
	}
}
template <class R> void perturb_weights(R& weights, R& stdDevs, bool additive)
{
	assert(boost::size(weights) == boost::size(stdDevs));
	LOOP(int i, indices(weights))
	{
		perturb_weight(weights[i], stdDevs[i], additive);
	}
}
