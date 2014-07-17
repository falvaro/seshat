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

#ifndef _INCLUDED_NeuronLayer_h  
#define _INCLUDED_NeuronLayer_h  

#include "Layer.hpp"

template <class F> struct NeuronLayer: public FlatLayer
{
  NeuronLayer(const string& name, size_t numDims, size_t size, WeightContainer *weight, DataExportHandler *deh):
    FlatLayer(name, numDims, size, weight, deh)
	{
		init();
	}
  NeuronLayer(const string& name, const vector<int>& directions, size_t size, WeightContainer *weight, DataExportHandler *deh):
    FlatLayer(name, directions, size, weight, deh)
	{
		init();
	}
	~NeuronLayer(){}
	void init()
	{
		display(this->inputActivations, "inputActivations");
		display(this->outputActivations, "outputActivations");
		display(this->inputErrors, "inputErrors");
		display(this->outputErrors, "outputErrors");	
	}
	void feed_forward(const vector<int>& coords)
	{
		transform(this->inputActivations[coords], this->outputActivations[coords], F::fn);
	}
	void feed_back(const vector<int>& coords)
	{
		LOOP(TDDD t, zip(this->inputErrors[coords], this->outputActivations[coords], this->outputErrors[coords]))
		{
			t.get<0>() = F::deriv(t.get<1>()) * t.get<2>();
		}
	}
};

#endif
