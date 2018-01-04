#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sstream>
#include <grpc++/server_builder.h>
#include <grpc++/server.h>

#include "googlecloudspeechserviceimpl.h"
#include "msspeechsessionfactory.h"
#include "msspeechsession.h"

class MSSpeechGBridge
{
public:
    int Start(int argc, char **argv)
    {
        std::string listen_address = "0.0.0.0";
        int listen_port = 8080;
        std::ostringstream stringStream;
        stringStream << listen_address << ":" << listen_port;
        std::string server_address = stringStream.str();
        
        MSSpeechSessionFactory factory(argv[1], 2);
        factory.start();

        GoogleCloudSpeechServiceImpl serviceImpl(&factory);
        ::grpc::ServerBuilder builder;
        builder.AddListeningPort(server_address, ::grpc::InsecureServerCredentials());
        builder.RegisterService(&serviceImpl);
        std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
        std::cout << "Server listening on " << server_address << std::endl;

        server->Wait();
        return 0;
    }
};

int main(int argc, char **argv)
{
    MSSpeechGBridge bridge;
    bridge.Start(argc, argv);

    return 0;
}
