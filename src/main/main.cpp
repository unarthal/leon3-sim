#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include "generic/utility.h"
#include "architecture/system.h"
#include "elf/elf.h"

using namespace std;
//globals
int verbose=0;

void usage(){
    cout << "USAGE: {-h:help, -c:config.xml, -e:sparcv8 executable, -s:statistics file, -o:standard out file, -q:quiet, -Q:Quiet} \n\n";
    cout << "./leon3-sim -h    // help" << endl;
    cout << "./leon3-sim -c <config.xml> -e <exec.out> -s <statistics.txt>   // normal operation, verbose" << endl;
    cout << "./leon3-sim -q -c <config.xml> -e <exec.out> -s <statistics.txt>    // quiet: minimal display"<< endl;
    cout << "./leon3-sim -Q -c <config.xml> -e <exec.out> -s <statistics.txt>    // Quiet: don't display anything"<< endl;
    cout << "./leon3-sim -c <config.xml> -e <exec.out> -s <statistics.txt> -o <standardout.txt>    // redirect standard out to standardout.txt"<< endl;
}

int main(int argc, char* argv[])
{
	if(argc < 3){
		usage();
		exit(1);
	}

	verbose = 2;
	ofstream stdoutfile;

    string executablePath;
    string configFilePath;
    string statisticsFilePath;

    int opt;
    while((opt = getopt(argc, argv,"hqQs:o:e:c:")) != -1)
    { 
        switch(opt) 
        { 
            case 'h':
                usage();
                return 0;
                break;
            case 'q':
                verbose = 1;
                break;
            case 'Q':
                verbose = 0;
                break; 
            case 'o':
            	checkFilePath(optarg, ios::out);
                stdoutfile.open(optarg, ios::out);
                break; 
            case 'e':
            	checkFilePath(optarg, ios::in);
            	executablePath = optarg;
                break;
            case 'c':
            	checkFilePath(optarg, ios::in);
            	configFilePath = optarg;
                break;
            case 's':
            	checkFilePath(optarg, ios::out);
            	statisticsFilePath = optarg;
                break; 
            case ':': 
                cout<< opt <<"option needs a value\n"; 
                break; 
            case '?': 
                cout<<"unknown option: "<<opt;
                break; 
        } 
    }

    std::streambuf *coutbuf;
    std::streambuf *cerrbuf;
    if(stdoutfile.is_open()){
       	coutbuf = std::cout.rdbuf(); //save old buf
    	std::cout.rdbuf(stdoutfile.rdbuf());
       	cerrbuf = std::cerr.rdbuf(); //save old buf
    	std::cerr.rdbuf(stdoutfile.rdbuf());
    }

    initializeArchitecture(executablePath, configFilePath);

    simulate();

    recordSystemStatistics(statisticsFilePath);

    //cleanup
    if(stdoutfile.is_open())
    {
    	stdoutfile.close();
    	std::cout.rdbuf(coutbuf);
    	std::cout.rdbuf(cerrbuf);
    }
    return 0;

}
