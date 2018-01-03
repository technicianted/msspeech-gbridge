# msspeech-gbridge

Service to allow using of unmodified [Google Speech](https://cloud.google.com/speech/docs/reference/libraries) client [SDKs](https://cloud.google.com/speech/docs/reference/libraries) to access Microsoft [Cognitive Services speech APIs](https://azure.microsoft.com/en-us/services/cognitive-services/speech/).

## Why is it needed

* Richer client support

Microsoft [Cognitive Services speech APIs](https://azure.microsoft.com/en-us/services/cognitive-services/speech/) do not provide any client SDKs. Instead, you must rely on community built or build your own. For example, if you are building a C/C++ based application, you can use [libmsspeech](https://github.com/technicianted/libmsspeech) open source client library.

On the other hande, Google provides an extensive set of client libraries for multiple platform. At the time of this writing, they support C#, Go, Java, Node.js, PHP, Python and Ruby.

With `msspeech-gbridge` service, you can directly use unmodified Google client libraries to access Microsoft APIs.

* Get union of features

Use the APIs interchangeably to get the features you need. For example, language support, text normalization, etc.

* Speech provider agnostic

You can build your applications using Google client SDKs but still be able to choose between Google Speech APIs and Microsoft Cognitive Services Speech APIs depending on your scenario without any code changes.

## Status

This is the very first version of the service. It only works. 

### Unsupported features and TODOs

* Microsoft Speech APIs do not support word level timing. `msspeech-gbridge` will not error when it is requested, but it will not return them.
* Microsoft subscription key has to be supplied at the service, not by the client. WiP to supply it by clients.
* Microsoft Speech APIs support `conversational` mode, which is not available in Google APIs.
* Differences in underlying speech recognition parametesr should be evaluated. For example, timeouts and segmentation.
* Support text to speech APIs.
* Implement the two remaining Google Speech APIs: `LongRunningRecognize`.
* Support Google speech audio codecs that are not supported by Microsoft Speech APIs: `FLAC`, `MULAW`, `AMR_WB`, `OGG_OPUS`, `SPEEX_WITH_HEADER_BYTE`.
* Better command line arguments.
* Support TLS.
* Lots of documentation!

## Using

Example usage using various Google SDKs can be found [here](https://github.com/technicianted/msspeech-gbridge/tree/master/examples/):
* [C#](https://github.com/technicianted/msspeech-gbridge/tree/master/examples/csharp)
* [Go](https://github.com/technicianted/msspeech-gbridge/tree/master/examples/go)
* [Python](https://github.com/technicianted/msspeech-gbridge/tree/master/examples/python) 

Rest of platform SDKs should work but have not been tested.

### As a container

#### Using docker locally

```
docker run --rm -t -p 8080:8080 technicianted/msspeech-gbridge:experimental /run.sh <your microsoft speech subscription key>
```

#### Using Azure Container Instance

If you have an Azure subscription, you can quickly bring up a container running `msspeech-gbridge` as a service:

```
az group create --name gbridge --location eastus
az container create --resource-group gbridge --name msspeech-gbridge --image technicianted/msspeech-gbridge --ip-address public --ports 8080
```

Then you can use the assigned public IP address as an endpoint when using the clients.

### As a standalone service

To use, you will need to build from source and run:

```
./msspeech-gbridge <your microsoft speech API subscription key>
```

## Implications

There are some implications when using `msspeech-gbridge` for your speech applications.

### Latency

Depending on where/how you run `msspeech-gbridge`, some latency might be incurred espcially for shorter audio. With each request, `msspeech-gbridge` attempts to reuse existing connection. If there isn't any, it connects to Microsoft Speech Service and caches the new connection. In the latter case, the time it takes to establish the connection is added to the leading latency.

However, in most cases, Microsoft Speech Service would catch up and make up for the leading latency. In such cases, no trailing latency is added.

1. Running locally with client (same local network or same box):

No latency as client to `msspeech-gbridge` connection establishment is very fast.

1. Running in same region as Microsoft Speech Service:

When running in Azure as a container, in most cases `msspeech-gbridge` would hit a Microsoft Speech Service instanced that is located within the same datacenter. In which case, minimal or no latency is added.

1. Running remotely:

Latency might be incurred due to connection establishment time if audio is very short. Otherwise, Microsoft Speech Service would catch up.

## Building from source

Currently you have to build from source.

### Dependencies

* [`libmsspeech`](https://github.com/technicianted/libmsspeech)
* [`gRPC`](https://grpc.io)
* [`fmtlib`](https://github.com/fmtlib/fmt)

### Building

* Update Google Cloud APIs:

```
git submodule update
```

* Generete Google Cloud APIs:
```
cd googleapis
make LANGUAGE=c++ OUTPUT=output/
```

* Build `msspeech-bridge`:
```
mkdir build
cd build
cmake ../
make
```
