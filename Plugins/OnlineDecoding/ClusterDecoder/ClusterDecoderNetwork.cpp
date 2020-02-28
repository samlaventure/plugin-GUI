#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "tensorflow/cc/client/client_session.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/tensor.h"

#include <tensorflow/core/public/session.h>
#include <tensorflow/core/protobuf/meta_graph.pb.h>
#pragma GCC diagnostic pop

#include "ClusterDecoderNetwork.h"



ClusterDecoderNetwork::ClusterDecoderNetwork(std::string path)
{
	std::string ptg = path + ".meta";
	loadDecoderGraph(&ptg, &path);

	std::cout << "tensorflow graph loaded." << std::endl;
}

void ClusterDecoderNetwork::loadDecoderGraph(std::string *pathToGraph, std::string *checkpointPath)
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

void ClusterDecoderNetwork::setSpikeShape(uint nChannelsNew)
{
	nChannels = nChannelsNew;
	spikeTensors = tensorflow::Tensor(tensorflow::DT_FLOAT, tensorflow::TensorShape({1, nChannels, 32}));
}

int ClusterDecoderNetwork::getCluster(ScopedPointer<SpikeEvent>& newSpike, const uint& group, const double& threshold)
{
	// Check shape of spike, compatible with the shape of selected group
	if (newSpike->getChannelInfo()->getNumChannels() != nChannels) {
		return -1;
	}

	// Copy spike into a tensorflow tensor
	for (uint chan=0; chan<nChannels; ++chan) {
		for (uint spl=0; spl<32; ++spl) {
			spikeTensors.tensor<float,3>()(0,chan,spl) = newSpike->getDataPointer(chan)[spl];
		}
	}

	// Run CNN network
	status = mainSession->Run(
		{ {"group"+std::to_string(group)+"-encoder/x:0", spikeTensors} },
		{"group"+std::to_string(group)+"-evaluator/sumProbas:0"},
		{},
		&sessionOutput);
	if (!status.ok()) throw std::runtime_error("Error when evaluating : " + status.ToString());

	// Find most probable unit
	auto readable_output = sessionOutput[0].tensor<float,1>();
	uint mostProbableUnit = 0;
	float mostProbableUnitProba = 0.;
	for (uint i=0; i<readable_output.size(); ++i) {
		if (readable_output(i) > mostProbableUnitProba) {
			mostProbableUnit = i;
			mostProbableUnitProba = readable_output(i);
		}
	}

	if (mostProbableUnitProba > threshold) return mostProbableUnit;
	else return -1;
}