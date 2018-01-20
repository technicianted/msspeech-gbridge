/*

Copyright 2017 technicianted

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

*/

#ifndef __MSSPEECH_GBRIDGE_CLI_H__
#define __MSSPEECH_GBRIDGE_CLI_H__

#include <string>

extern std::string logLevel;
extern std::string subscriptionKey;
extern std::string gstTransformerEndpoint;
extern std::string listenEndpoint;
extern int maxSessions;

int parse_opt(int argc, char **argv);
void usage();

#endif
