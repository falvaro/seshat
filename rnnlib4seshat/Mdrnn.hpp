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

#ifndef _INCLUDED_Mdrnn_h
#define _INCLUDED_Mdrnn_h

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "InputLayer.hpp"
#include "BiasLayer.hpp"
#include "NetworkOutput.hpp"
#include "NeuronLayer.hpp"
#include "IdentityLayer.hpp"
#include "FullConnection.hpp"
#include "ActivationFunctions.hpp"
#include "LstmLayer.hpp"
#include "BlockLayer.hpp"
#include "NetcdfDataset.hpp"
#include "GatherLayer.hpp"
#include "CollapseLayer.hpp"
#include "CopyConnection.hpp"

typedef pair<const Layer*, Connection*> PLC;
typedef multimap<const Layer*, Connection*>::iterator CONN_IT;
typedef multimap<const Layer*, Connection*>::const_iterator CONST_CONN_IT;
typedef vector<Layer*>::iterator LAYER_IT;
typedef vector<Layer*>::const_iterator CONST_LAYER_IT;
typedef vector<Layer*>::reverse_iterator REVERSE_LAYER_IT;
typedef vector<vector<Layer*> >::iterator LEVEL_IT;

struct Mdrnn {
  // data
  ostream& out;
  multimap<const Layer*, Connection*> connections;
  vector<Layer*> hiddenLayers;
  vector<vector<Layer*> > hiddenLevels;
  InputLayer* inputLayer;
  vector<NetworkOutput*> outputs;
  vector<Layer*> outputLayers;
  vector<bool> bidirectional;
  vector<bool> symmetry;
  vector<size_t> inputBlock;
  Layer* inputBlockLayer;
  BiasLayer *bias;
  vector<Layer*> recurrentLayers;
  map<string, real_t> errors;
  map<string, real_t> normFactors;
  Vector<string> criteria;
  WeightContainer *wc;
  DataExportHandler *DEH;

  // functions
  Mdrnn(ostream& o, ConfigFile& conf, const DataHeader& data, WeightContainer *weight, DataExportHandler *deh):
    out(o), wc(weight), DEH(deh)  {

    inputLayer = new InputLayer("input", data.numDims, data.inputSize, data.inputLabels, wc, DEH);
    bidirectional = conf.get_list<bool>("bidirectional", true, data.numDims);
    symmetry = conf.get_list<bool>("symmetry", false, data.numDims);
    inputBlock = conf.get_list<size_t>("inputBlock", 0, data.numDims);
    inputBlockLayer = in(inputBlock, 0) ? 0 : add_layer(new BlockLayer(inputLayer, inputBlock, wc, deh), false);
    bias = new BiasLayer(weight, deh);
  }

  virtual ~Mdrnn() {
    delete inputLayer;
    delete bias;
    delete_range(hiddenLayers);
    delete_range(outputLayers);
    delete_map(connections);
  }

  size_t num_seq_dims() const {
    return bidirectional.size();
  }

  Layer* get_input_layer() const {
    return inputBlockLayer ?
        reinterpret_cast<Layer*>(inputBlockLayer) :
        reinterpret_cast<Layer*>(inputLayer);
  }

  Connection* add_connection(Connection* conn) {
    connections.insert(make_pair(conn->to, conn));
    return conn;
  }

  FullConnection* connect_layers(
      Layer* from, Layer* to, const vector<int>& delay = empty_list_of<int>()) {
    FullConnection* conn = new FullConnection(from, to, wc, delay);
    add_connection(conn);
    return conn;
  }

  void make_layer_recurrent(Layer* layer) {
    vector<int> delay = empty_list_of<int>().repeat(layer->num_seq_dims(), 0);
    FOR(i, delay.size()) {
      delay[i] = -layer->directions[i];
      connect_layers(layer, layer, delay);
      delay[i] = 0;
    }
  }

  Layer* gather_level(const string& name, int levelNum) {
    return add_layer(new GatherLayer(name, hiddenLevels[levelNum], wc, DEH));
  }

  Layer* collapse_layer(
      Layer* src, Layer* dest,
      const vector<bool>& activeDims = empty_list_of<bool>()) {
    Layer* layer = add_layer(new CollapseLayer (src, dest, wc, DEH, activeDims));
    add_connection(new CopyConnection(layer, dest, wc));
    return layer;
  }

  bool is_recurrent(const Layer* layer) const {
    LOOP(const PLC& c, connections.equal_range(layer)) {
      if (c.second->from == c.second->to) {
        return true;
      }
    }
    return false;
  }

  bool is_mirror(const Layer* layer) {
    FOR(i, symmetry.size()) {
      if (symmetry[i] && (layer->directions[i] < 0)) {
        return true;
      }
    }
    return false;
  }

  Layer* add_output_layer(NetworkOutput* output, bool addBias = true) {
    Layer* layer = dynamic_cast<Layer*>(output);
    check(layer, "unable to cast network output " + string(
        typeid(*output).name()) + " to layer");
    outputLayers += layer;
    if (addBias) {
      add_bias(layer);
    }
    outputs += output;
    return layer;
  }

  Layer* add_layer(Layer* layer, bool addBias = false, bool recurrent = false) {
    hiddenLayers += layer;
    if (!is_mirror(layer)) {
      if (addBias) {
        add_bias(layer);
      }
      if (recurrent) {
        make_layer_recurrent(layer);
      }
    }
    return layer;
  }

  Layer* add_layer(
      const string& type, const string& name, int size,
      const vector<int>& directions, bool addBias = false,
      bool recurrent = false) {
    Layer* layer;
    if (type == "tanh") {
      layer = new NeuronLayer<Tanh>(name, directions, size, wc, DEH);
    } else if (type == "softsign") {
      layer = new NeuronLayer<Softsign>(name, directions, size, wc, DEH);
    } else if (type == "logistic") {
      layer = new NeuronLayer<Logistic>(name, directions, size, wc, DEH);
    } else if (type == "identity") {
      layer = new IdentityLayer(name, directions, size, wc, DEH);
    } else if (type == "lstm") {
      layer = new LstmLayer<Tanh, Tanh, Logistic>(name, directions, size, wc, DEH, 1);
    } else if (type == "linear_lstm") {
      layer = new LstmLayer<Tanh, Identity, Logistic>(
						      name, directions, size, wc, DEH, 1);
    } else if (type == "softsign_lstm") {
      layer = new LstmLayer<Softsign, Softsign, Logistic>(
							  name, directions, size, wc, DEH, 1);
    } else {
      check(false, "layer type " + type + " unknown");
    }
    return add_layer(layer, addBias, recurrent);
  }

  Layer* add_hidden_layers_to_level(
      const string& type, int size, bool recurrent, const string& name,
      int dim, int levelNum, vector<int> directions, bool addBias = true) {
    if (dim == num_seq_dims()) {
      string fullName = name + "_" + str(hiddenLevels.at(levelNum).size());
      Layer* layer = add_layer(
          type, fullName, size, directions, addBias, recurrent);
      hiddenLevels.at(levelNum).push_back(layer);
      return layer;
    } else {
      if (bidirectional[dim]) {
        directions[dim] = -1;
        add_hidden_layers_to_level(
            type, size, recurrent, name, dim + 1, levelNum, directions,
            addBias);
      }
      directions[dim] = 1;
      return add_hidden_layers_to_level(
          type, size, recurrent, name, dim + 1, levelNum, directions, addBias);
    }
  }

  virtual void build() {
    LOOP(vector<Layer*>& v, hiddenLevels) {
      LOOP(Layer* dest, v) {
        if (is_mirror(dest)) {
          vector<int> sourceDirs(dest->directions.size());
          LOOP(TIBI t, zip(sourceDirs, symmetry, dest->directions)) {
            t.get<0>() = (((t.get<1>() > 0) || t.get<2>()) ? 1 : -1);
          }
          LOOP(Layer* src, v) {
            if (src->directions == sourceDirs) {
              copy_connections(src, dest, true);
              break;
            }
          }
        }
      }
    }
    recurrentLayers.clear();
    LOOP(Layer* l, hiddenLayers) {
      l->build();
      if (is_recurrent(l)) {
        recurrentLayers += l;
      }
    }
    criteria.clear();
    LOOP(Layer* l, outputLayers) {
      l->build();
    }
    LOOP(NetworkOutput* l, outputs) {
      criteria.extend(l->criteria);
    }
  }

  int copy_connections(Layer* src, Layer* dest, bool mirror = false) {
    int numCopied = 0;
    LOOP(PLC c, connections.equal_range(src)) {
      FullConnection* oldConn = dynamic_cast<FullConnection*>(c.second);
      vector<int> delay = oldConn->delay;
      if (mirror) {
        LOOP(int i, indices(delay)) {
          if (src->directions[i] != dest->directions[i]) {
            delay[i] *= -1;
          }
        }
      }
      add_connection(new FullConnection(oldConn->from == oldConn->to ? dest :
                                        oldConn->from, dest, wc, delay, oldConn));
      ++numCopied;
    }
    return numCopied;
  }

  FullConnection* add_bias(Layer* layer) {
    return connect_layers(bias, layer);
  }

  void connect_to_hidden_level(Layer* from, int levelNum) {
    LOOP(int i, indices(hiddenLevels.at(levelNum))) {
      Layer* to = hiddenLevels.at(levelNum)[i];
      if (!is_mirror(to)) {
        connect_layers(from, to);
      }
    }
  }

  void connect_from_hidden_level(int levelNum, Layer* to) {
    LOOP(int i, indices(hiddenLevels.at(levelNum))) {
      Layer* from = hiddenLevels.at(levelNum)[i];
      connect_layers(from, to);
    }
  }

  int add_hidden_level(
      const string& type, int size, bool recurrent = true,
      const string& name = "hidden", bool addBias = true) {
    int levelNum = hiddenLevels.size();
    hiddenLevels.resize(levelNum + 1);
    add_hidden_layers_to_level(
        type, size, recurrent, name, 0, levelNum,
        empty_list_of<bool>().repeat(num_seq_dims(), true), addBias);
    return levelNum;
  }

  void feed_forward_layer(Layer* layer) {
    layer->start_sequence();
    pair<CONN_IT, CONN_IT> connRange = connections.equal_range(layer);
    for (SeqIterator it = layer->input_seq_begin(); !it.end; ++it) {
      LOOP(PLC c, connRange) {
        c.second->feed_forward(*it);
      }
      layer->feed_forward(*it);
    }
  }

  void feed_back_layer(Layer* layer) {
    pair<CONN_IT, CONN_IT> connRange = connections.equal_range(layer);
    for (SeqIterator it = layer->input_seq_rbegin(); !it.end; ++it) {
      layer->feed_back(*it);
      LOOP(PLC c, connRange) {
        c.second->feed_back(*it);
      }
    }
    for (SeqIterator it = layer->input_seq_rbegin(); !it.end; ++it) {
      layer->update_derivs(*it);
      LOOP(PLC c, connRange) {
        c.second->update_derivs(*it);
      }
    }
  }
  virtual void feed_forward(const DataSequence& seq) {
    check(seq.inputs.size(), "empty inputs in sequence\n" + str(seq));
    errors.clear();
    inputLayer->copy_inputs(seq.inputs);
    LOOP(Layer* layer, hiddenLayers) {
      feed_forward_layer(layer);
    }
    LOOP(Layer* layer, outputLayers) {
      feed_forward_layer(layer);
    }
  }
  virtual real_t calculate_output_errors(const DataSequence& seq) {
    real_t error = 0;
    errors.clear();
    if (outputs.size() == 1) {
      NetworkOutput* l = outputs.front();
      error = l->calculate_errors(seq);
      errors = l->errorMap;
      normFactors = l->normFactors;
    } else {
      normFactors.clear();
      LOOP(NetworkOutput* l, outputs) {
        error += l->calculate_errors(seq);
        string layerPrefix = dynamic_cast<Named*>(l)->name + '_';
        LOOP(const PSD& p, l->errorMap) {
          errors[p.first] += p.second;
          errors[layerPrefix + p.first] = p.second;
        }
        LOOP(const PSD& p, l->normFactors) {
          normFactors[p.first] += p.second;
          normFactors[layerPrefix + p.first] = p.second;
        }
      }
    }
    return error;
  }

  virtual real_t calculate_errors(const DataSequence& seq) {
    feed_forward(seq);
    return calculate_output_errors(seq);
  }

  virtual void feed_back() {
    LOOP_BACK(Layer* layer, outputLayers) {
      feed_back_layer(layer);
    }
    LOOP_BACK(Layer* layer, hiddenLayers) {
      feed_back_layer(layer);
    }
  }

  virtual real_t train(const DataSequence& seq) {
    real_t error = calculate_errors(seq);
    feed_back();
    return error;
  }

  virtual void print(ostream& out = cout) const {
    out << typeid(*this).name() << endl;
    prt_line(out);
    out << hiddenLayers.size() + 2 << " layers:" << endl;
    out << (Layer&)*(inputLayer) << endl;
    LOOP(const Layer* layer, hiddenLayers) {
      if (is_recurrent(layer)) {
        out << "(R) ";
      }
      out << *layer << endl;
    }
    LOOP(const Layer* layer, outputLayers) {
      out << *layer << endl;
    }
    prt_line(out);
    out << connections.size() << " connections:" << endl;
    for (CONST_CONN_IT it = connections.begin(); it != connections.end();
         ++it) {
      out << *it->second << endl;
    }
    prt_line(out);
    PRINT(bidirectional, out);
    PRINT(symmetry, out);
    if (inputBlockLayer) {
      PRINT(inputBlock, out);
    }
  }

  virtual void print_output_shape(ostream& out = cout) const {
    LOOP(const Layer* l, outputLayers) {
      out << l->name << " shape = (" << l->outputActivations.shape << ")"
          << endl;
    }
  }
};

ostream& operator << (ostream& out, const Mdrnn& net);

#endif
