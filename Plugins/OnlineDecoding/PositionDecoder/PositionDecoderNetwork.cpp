#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "tensorflow/cc/client/client_session.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/tensor.h"

#include <tensorflow/core/public/session.h>
#include <tensorflow/core/protobuf/meta_graph.pb.h>
#pragma GCC diagnostic pop

#include "PositionDecoderNetwork.h"
#include "PositionDecoderProcessor.h"



PositionDecoderNetwork::PositionDecoderNetwork(std::string path, std::vector<std::vector<uint>> inChannels) :
	channels(inChannels)
{
	std::string ptg = path + ".meta";
	loadConcatenatorGraph(500);
	loadDecoderGraph(&ptg, &path);

	for (uint group=0 ; group<channels.size() ; ++group) {

		sessionInput.push_back({"group"+std::to_string(group)+"-encoder/x:0", 
								tensorflow::Tensor(tensorflow::DT_FLOAT, tensorflow::TensorShape({0, (int) channels.at(group).size(), 32}))});

	}

	sessionOutputTensorName.push_back("bayesianDecoder/positionProba:0");
	sessionOutputTensorName.push_back("bayesianDecoder/positionGuessed:0");
	sessionOutputTensorName.push_back("bayesianDecoder/standardDeviation:0");

	std::cout << "tensorflow graph loaded." << std::endl;
}

void PositionDecoderNetwork::loadDecoderGraph(std::string *pathToGraph, std::string *checkpointPath)
{
	using namespace tensorflow;

	mainSession = NewSession(SessionOptions());
	if (mainSession == nullptr) throw std::runtime_error("Could not create Tensorflow mainSession.");
	

	// Read meta graph info
	MetaGraphDef graph_def;
	status = ReadBinaryProto(Env::Default(), *pathToGraph, &graph_def);
	if (!status.ok()) throw std::runtime_error("Error reading graph : " + status.ToString());

	status = mainSession->Create(graph_def.graph_def());
	if (!status.ok()) throw std::runtime_error("Error creating graph : " + status.ToString());


	// Read weights from the saved model
	Tensor checkpointPathTensor(DT_STRING, TensorShape());
	checkpointPathTensor.scalar<std::string>()() = *checkpointPath;
	status = mainSession->Run(
					      {{ graph_def.saver_def().filename_tensor_name(), checkpointPathTensor },},
					      {},
					      {graph_def.saver_def().restore_op_name()},
					      nullptr);
	if (!status.ok()) throw std::runtime_error("Error loading checkpoint : " + status.ToString());
}

void PositionDecoderNetwork::loadConcatenatorGraph(int nNodes)
{
	using namespace tensorflow;

	assert(nNodes > 1);

	nSpikes = std::vector<int>(channels.size(), 0);

	concatFeedDict.resize(channels.size());
	spikeTensors.resize(channels.size()); // We will fill these tensors each with one spike as they come along
	spikePlaceholders.resize(channels.size()); // The Tensors fill the placeholders in the tensorflow graph
	concatenators.resize(channels.size()); // The concatenators operations will give us one tensor with shape [nSpike, nChannels, 32]

	for (uint group=0 ; group<channels.size() ; ++group) {

		spikeTensors.at(group).resize(nNodes);
		spikePlaceholders.at(group).resize(nNodes);

		for (int node=0 ; node<nNodes ; ++node) {
			spikeTensors     .at(group).at(node) = Tensor(DT_FLOAT, TensorShape({1, (int)channels.at(group).size(), 32}));
			spikePlaceholders.at(group).at(node) = ops::Placeholder(scope, DT_FLOAT);
			concatFeedDict.at(group).insert({spikePlaceholders.at(group).at(node), spikeTensors.at(group).at(node)});
		}

		concatenators.at(group).push_back( ops::Concat(scope, {spikePlaceholders.at(group).at(0), spikePlaceholders.at(group).at(1)}, 0) );
		// First node is already filled, and we need one less concatenator than placeholder
		for (int node=1 ; node<nNodes-1 ; ++node) {
			concatenators.at(group).push_back( ops::Concat(scope, {concatenators.at(group).at(node-1), spikePlaceholders.at(group).at(node+1)}, 0) );
		}
	}

	concatSession = new ClientSession(scope);
}


void PositionDecoderNetwork::addConcatenatorNode(int graph)
{
	using namespace tensorflow;
	int nNodes = spikeTensors.at(graph).size();
	spikeTensors     .at(graph).push_back( Tensor(DT_FLOAT, TensorShape({1, (int)channels.at(graph).size(), 32})) );
	spikePlaceholders.at(graph).push_back( ops::Placeholder(scope, DT_FLOAT) );
	concatFeedDict   .at(graph).insert(    {spikePlaceholders.at(graph).back(), spikeTensors.at(graph).back()} );
	concatenators    .at(graph).push_back( ops::Concat(scope, {concatenators.at(graph).at(nNodes-2), spikePlaceholders.at(graph).back()}, 0) );
	concatSession = new ClientSession(scope);
}

void PositionDecoderNetwork::clearOutput()
{
	for_each(nSpikes.begin(), nSpikes.end(), [&](int& i)->void{i=0;});
}












void PositionDecoderNetwork::addSpike(std::vector<std::vector<float>>& data, int inGroup)
{
	assert(data.size() == channels.at(inGroup).size());

	for (uint group=0; group<channels.size(); ++group) {
		while (nSpikes.at(group) >= (int) spikeTensors.at(group).size()) addConcatenatorNode(group);

		for (uint chnl=0; chnl<channels.at(group).size(); ++chnl) {
			for (uint spl=0; spl<32; ++spl) {
				spikeTensors.at(group).at(nSpikes.at(group)).tensor<float,3>()(0, chnl, spl) = inGroup==group ? data.at(chnl).at(spl) : .0f;
			}
		}

		nSpikes.at(group) += 1;
	}
}

OnlineDecoding::DecodingResults* PositionDecoderNetwork::inferPosition()
{
	// Concatenate spikes if needed
	for (uint group=0; group<channels.size(); ++group) {

		if (nSpikes.at(group) > 1) {
			status = concatSession->Run(concatFeedDict.at(group), {concatenators.at(group).at(nSpikes.at(group)-2)}, &sessionOutput);
			if (!status.ok()) throw std::runtime_error("Error when concatenating : " + status.ToString());
			sessionInput.at(group).second = sessionOutput.at(0);
		} else if (nSpikes.at(group) == 1) {
			sessionInput.at(group).second = spikeTensors.at(group).at(0);
		} else {
			sessionInput.at(group).second = tensorflow::Tensor(
				tensorflow::DT_FLOAT, 
				tensorflow::TensorShape({0, (int) channels.at(group).size(), 32}));
		}
	}

	// Infer position
	status = mainSession->Run(sessionInput, sessionOutputTensorName, {}, &sessionOutput);
	if (!status.ok()) throw std::runtime_error("Error when evaluationg : " + status.ToString());

	// extract results
	OnlineDecoding::DecodingResults* results = new OnlineDecoding::DecodingResults(&sessionOutput);

	clearOutput();
	return results;
}




OnlineDecoding::DecodingResults::DecodingResults(float& inX, float& inY, float& inStd)
	: X(inX), Y(inY), stdDev(inStd), positionProba(positionProbaAsArray(nullptr))
{
	values[0] = X;
	values[1] = Y;
	values[2] = stdDev;
}

OnlineDecoding::DecodingResults::DecodingResults(std::vector<tensorflow::Tensor>* networkOutput)
	: X(networkOutput->at(1).tensor<float,1>()(0)),
	Y(networkOutput->at(1).tensor<float,1>()(1)),
	stdDev(networkOutput->at(2).tensor<float,1>()(0)),
	positionProba(positionProbaAsArray(networkOutput))
{
	values[0] = X;
	values[1] = Y;
	values[2] = stdDev;
}

std::vector<std::vector<float>> OnlineDecoding::DecodingResults::positionProbaAsArray(std::vector<tensorflow::Tensor>* networkOutput)
{
	if (networkOutput == nullptr) return std::vector<std::vector<float>>(45, std::vector<float>(45, 0.));
	else {
		std::vector<std::vector<float>> res;
		res.resize(45);
		for (int x=0; x<45; ++x) {
			res[x].resize(45);
			for (int y=0; y<45; ++y) {
				res[x][y] = networkOutput->at(0).tensor<float,2>()(x,y);
			}
		}
		return res;
	}
}
