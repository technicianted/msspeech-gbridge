
#include <getopt.h>
#include <stdlib.h>
#include <string>
#include <iostream>

std::string logLevel = "info";
std::string subscriptionKey;
std::string gstTransformerEndpoint;
std::string listenEndpoint;
int maxSessions = 0;

int parse_opt(int argc, char **argv)
{
    char *value = getenv("MSSPEECH_SUBSCRIPTION_KEY");
    if (value)
        subscriptionKey = value;
    value = getenv("MSSPEECH_GSTTRANSFORMER_ENDPOINT");
    if (value)
        gstTransformerEndpoint = value;
    value = getenv("MSSPEECH_LOG_LEVEL");
    if (value)
        logLevel = value;
    value = getenv("MSSPEECH_LISTEN_ENDPOINT");
    if (value)
        listenEndpoint = value;

	int key;
	while ((key = getopt(argc, argv, "+k:t:d:m:")) != -1) {
		switch (key) {
			case 'k':
                subscriptionKey = optarg;
				break;
			case 'd':
                logLevel = optarg;
                break;
			case 't':
                gstTransformerEndpoint = optarg;
                break;
            case 'm':
                maxSessions = strtoul(optarg, NULL, 10);
                break;

            default:
                std::cerr << "Unknown option" << std::endl;
                return -1;
		}
	}

    if (listenEndpoint.empty()) {
        if ((argc - optind) != 1)
            return -1;
        
        listenEndpoint = argv[optind];
    }

	return 0;
}

void usage()
{
	std::cerr << "Usage: msspeech-gbridge [OPTION...] [<listen endpoint>]" << std::endl;
    std::cerr << "  -k KEY\tMicrosoft speech API subscription key." << std::endl;
    std::cerr << "     env: MSSPEECH_SUBSCRIPTION_KEY" << std::endl;
    std::cerr << "  -t ENDPOINT\tgsttransformer endpoint to handle audio codecs and streaming." << std::endl;
    std::cerr << "     env: MSSPEECH_GSTTRANSFORMER_ENDPOINT" << std::endl;
	std::cerr << "  -d LEVEL\tDebug level {trace|debug|info|notice|error}." << std::endl;
    std::cerr << "     env: MSSPEECH_LOG_LEVEL" << std::endl;
    std::cerr << "endpoint: grpc style endpoint" << std::endl;
    std::cerr << "  env: MSSPEECH_LISTEN_ENDPOINT" << std::endl;

    exit(1);
}
