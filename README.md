# msspeech-gbridge

Service to allow using of unmodified [Google Speech](https://cloud.google.com/speech/docs/reference/libraries) client [SDKs](https://cloud.google.com/speech/docs/reference/libraries) to access Microsoft [Cognitive Services speech APIs](https://azure.microsoft.com/en-us/services/cognitive-services/speech/).

## Why is it needed

* Richer client support

Microsoft [Cognitive Services speech APIs](https://azure.microsoft.com/en-us/services/cognitive-services/speech/) do not provide any client SDKs. Instead, you must rely on community built or build your own. For example, if you are building a C/C++ based application, you can use [this](https://github.com/technicianted/libmsspeech) open source client library.

On the other hande, Google provides an extensive set of client libraries for multiple platform. At the time of this writing, they support C#, Go, Java, Node.js, PHP, Python and Ruby.

With `msspeech-gbridge` service, you can directly use unmodified Google client libraries to access Microsoft APIs.

* Speech provider agnostic

You can build your applications using Google client SDKs but still be able to choose between Google Speech APIs and Microsoft Cognitive Services Speech APIs depending on your scenario. For example if you want a feature or language that is supported in one API but not the other.

## Status

This is the very first version of the service. It only works. 

### Unsupported features and TODOs

* Microsoft Speech APIs do not support word level timing. `msspeech-gbridge` will not error when it is requested, but it will not return them.
* Microsoft subscription key has to be supplied at the service, not by the client. WiP to supply it by clients.
* Microsoft Speech APIs support `conversational` mode, which is not available in Google APIs.
* Lots of documentation!

## Building from source

Currently you have to build from source.

### Dependencies

* [`libmsspeech`](https://github.com/technicianted/libmsspeech).
* [`gRPC`](https://grpc.io).

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

## Using

Example usage using varios Google SDKs can be found [here](https://github.com/technicianted/msspeech-gbridge/examples/). 

### As a standalone service

To use, you will need to build from source and run:

```
./msspeech-gbridge <your microsoft speech API subscription key>
```

### As a container

Coming soon.
