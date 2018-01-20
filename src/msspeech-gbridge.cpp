#include <iostream>
#include <fstream>
#include <unistd.h>
#include <grpc++/server_builder.h>
#include <grpc++/server.h>
#include <spdlog/spdlog.h>

#include "googlecloudspeechserviceimpl.h"
#include "msspeechsessionfactory.h"
#include "msspeechsession.h"
#include "msspeech-gbridge-cli.h"

int start(int argc, char **argv)
{
    MSSpeechSessionFactory factory(subscriptionKey, maxSessions);
    factory.start();

    GoogleCloudSpeechServiceImpl serviceImpl(&factory);
    ::grpc::ServerBuilder builder;
    builder.AddListeningPort(listenEndpoint, ::grpc::InsecureServerCredentials());
    builder.RegisterService(&serviceImpl);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << listenEndpoint << std::endl;

    server->Wait();
    return 0;
}

int main(int argc, char **argv)
{
    if (parse_opt(argc, argv) == -1) {
        usage();
    }

    if (listenEndpoint.empty()) {
        std::cerr << "listen endpoint not specified" << std::endl;
        usage();
    }
    if (subscriptionKey.empty()) {
        std::cerr << "subscription key not specified" << std::endl;
        usage();
    }

    for(unsigned int i=0; i<sizeof(spdlog::level::level_names); i++) {
        if (logLevel == spdlog::level::level_names[i]) {
            spdlog::set_level((spdlog::level::level_enum)i);
            break;
        }
    }

    return start(argc, argv);
}
