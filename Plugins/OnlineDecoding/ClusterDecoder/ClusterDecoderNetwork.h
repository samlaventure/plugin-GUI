#ifndef MOBSNEURALNETS_H
#define MOBSNEURALNETS_H

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "tensorflow/cc/client/client_session.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/tensor.h"

#include <tensorflow/core/public/session.h>
#include <tensorflow/core/protobuf/meta_graph.pb.h>
#pragma GCC diagnostic pop

#include <ProcessorHeaders.h>

class ClusterDecoderNetwork
{
private:
	void loadDecoderGraph(std::string *pathToGraph, std::string *checkpointPath);

	tensorflow::Scope                                 scope = tensorflow::Scope::NewRootScope();
	tensorflow::Status                                status;
	ScopedPointer<tensorflow::Session>                mainSession;

	tensorflow::Tensor                                spikeTensors;
	std::vector<tensorflow::Tensor>                   sessionOutput;

public:
	ClusterDecoderNetwork(std::string path);
	void setSpikeShape(uint nChannels);
	int getCluster(ScopedPointer<SpikeEvent>& newSpike, const uint& group, const double& threshold=0);

	uint nChannels = 0;
};

#endif // MOBSNEURALNETS_H