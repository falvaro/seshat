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

#ifndef _INCLUDED_NetcdfDataset_h  
#define _INCLUDED_NetcdfDataset_h  

#include <functional>
#include <algorithm>
#include <string>
#include <numeric>
#include <map>
//#include <netcdfcpp.h>
#include <boost/algorithm/string/replace.hpp>
#include "DataSequence.hpp"
#include "Helpers.hpp"

#define SEQ_IT vector<DataSequence*>::iterator
#define CONST_SEQ_IT vector<DataSequence*>::const_iterator


struct DataHeader
{
	//data
	int numDims;
	Vector<string> inputLabels;
	map<string, int> inputLabelCounts;
	Vector<string> targetLabels;
	map<string, int> targetLabelCounts;
	size_t inputSize;
	size_t outputSize;
	size_t numSequences;
	size_t numTimesteps;
	size_t totalTargetStringLength;

	//functions
  DataHeader(): outputSize(0),
		numTimesteps(0),
		totalTargetStringLength(0)
  {}
};

#endif
