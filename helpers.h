#ifndef HELPERS_H
#define HELPERS_H

/******************************************************************************
* HELPERS / UTILS
******************************************************************************/
int write_data(int fd, const char* msg);
int read_data(int fd, char* msg);
void exitErr(std::string msg);
void parseClientArgs(int argc, char** argv, char* host, int& port);

#endif
