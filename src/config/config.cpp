#include "config/config.h"
#include <fstream>

using namespace std;

/*
 * TODO to improve the XML parsing functionalities
 * we want to be able to get an integer value, string value, handle nested parameters, etc.
 */
string xmlReader_generalString(string l_filename, string opentag, string closetag){

    ifstream l_file;
    l_file.open(l_filename.c_str());

    string l_xmlEntry;
    string l_NWindows = opentag;
    string l__NWindows = closetag;
    string t;
    while(l_file >> l_xmlEntry){

    if(l_xmlEntry.find(l_NWindows)!=string::npos){

        size_t pos = l_xmlEntry.find(l_NWindows);
        if (pos != std::string::npos)
        {
            // If found then erase it from string
            l_xmlEntry.erase(pos, l_NWindows.length());
        }

        pos = l_xmlEntry.find(l__NWindows);
        if (pos != std::string::npos)
        {
            // If found then erase it from string
            l_xmlEntry.erase(pos, l__NWindows.length());
        }


        return l_xmlEntry;
    }

    }
    return "-1";

}

int xmlReader_generalInt(string l_filename, string opentag, string closetag){
	return stoi(xmlReader_generalString(l_filename, opentag, closetag));
}
