#ifndef SRC_CONFIG_CONFIG_H_
#define SRC_CONFIG_CONFIG_H_

#include<string>

/*
 * TODO to improve the XML parsing functionalities
 * we want to be able to get an integer value, string value, handle nested parameters, etc.
 */
std::string xmlReader_generalString(std::string l_filename, std::string opentag, std::string closetag);
int xmlReader_generalInt(std::string l_filename, std::string opentag, std::string closetag);

#endif
