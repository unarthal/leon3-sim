#include "simulator/simulator.h"
#include "config/config.h"
#include <string>

using namespace std;

///sim globals
long long unsigned int nINS = 0;
long long unsigned int nMR = 0;
long long unsigned int nMW = 0;
long long unsigned int nBT;

long long unsigned int clock_cycles = 0;
int SIM_LOOP_SIGNAL = 0;
int MEM_LAT = 0;
int MEMDUMP_MAX=0;
int Quiet=0;
std::ofstream xout;
///globals
char* config_path = "src/config/config.xml";




int convertHexToInt(string l_hex){
    int l_number=0, l_term=0;
    
    for(int i=l_hex.length()-1;i>=0;i--){
        
        int l_tmp = 0;
        
        if(int(l_hex[i])<58){
            l_tmp = int(l_hex[i]) - 48;
            
        }
        else{
            l_tmp = int(l_hex[i]) - 87;
        }
        
        l_term = pow(16,l_hex.length()-1-i)*l_tmp;
        l_number = l_number+l_term;
    
    }
    return l_number;
}

void readDump(string l_filename){
    
    ifstream fchk1(l_filename.c_str());
    if(!fchk1.good()){
        xout<<l_filename.c_str()<<":Disassembly file Path Invalid\n";
        exit(1);
    }


    ifstream l_file;
    l_file.open(l_filename.c_str());
    string l_identify_main[2];
    l_file >> l_identify_main[0];
    
    int l_count = 0;
    while(!l_file.eof()){
        if(l_identify_main[l_count].find("<main>:")!=-1){
            break;
        }
        l_count = 1 - l_count;
        l_file >> l_identify_main[l_count];
    }
    l_count = 1 - l_count;

    Start_addr=convertHexToInt(l_identify_main[l_count]);

    while(!l_file.eof()){
        if(l_identify_main[l_count].find(">:")!=-1){
            break;
        }
        l_count = 1 - l_count;
        l_file >> l_identify_main[l_count];
    }
    l_count = 1 - l_count;

    End_addr=convertHexToInt(l_identify_main[l_count])-4;
}


int xmlReader_general(string l_filename, string opentag, string closetag){
    
    ifstream l_file;
    l_file.open(l_filename.c_str());
    
    string l_xmlEntry;
    string l_NWindows = opentag;
    string l__NWindows = closetag;
    string t;
    while(l_file >> l_xmlEntry){
    
    if(l_xmlEntry.find(l_NWindows)!=-1){
        
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


        return stoi(l_xmlEntry);
    }

    }
    return -1;


}

void usage(){
    cout << "USAGE: {-h:help, -c:config.xml, -e:execfile, -f:write_to_outfile, -q:quiet, -Q:Quiet} \n\n";
    cout << "./SparcSimulator -h    // help" << endl;
    cout << "./SparcSimulator -c <config.xml> -e <exec.out>   // normal operation, verbose" << endl;
    cout << "./SparcSimulator -q -c <config.xml> -e <exec.out>    // quiet: don't display all registers"<< endl;
    cout << "./SparcSimulator -Q -c <config.xml> -e <exec.out>    // Quiet: don't display Anything"<< endl;
    cout << "./SparcSimulator -f -c <config.xml> -e <exec.out>    // send all output to sim_out.txt"<< endl;
                
}



int main(int argc, char* argv[])
{   
    ifstream fchk1;
    int opt;    
    char* OBJ_PTH;

    // std::ofstream xout;
    if(argc < 2){
        usage();
        exit(1);
        } 
   
    while((opt = getopt(argc, argv,"hqQfe:c:")) != -1) 
    { 
        switch(opt) 
        { 
            case 'h': /// help
                usage();
                return -1;
                break;
            case 'q':
                Quiet = 2;
                break;
            case 'Q': /// quiet  
                Quiet = 1;
                xout.open("/dev/null");
                break; 
            case 'f': /// output to file  sim_out.txt
                Quiet = 1;
                xout.open("sim_out.txt");
                break; 
            case 'e': /// exec file
                fchk1.open(optarg);
                if(!fchk1.good()){
                    cout<<optarg<<":Exec File Path Invalid\n";
                    exit(1);
                } else {
                    OBJ_PTH = optarg;
                    fchk1.close();
                }
                break;
            case 'c':
                config_path = optarg;
                break; 
            case ':': 
                cout<< opt <<"option needs a value\n"; 
                break; 
            case '?': 
                cout<<"unknown option: "<<optopt;
                break; 
        } 
    } 
    for(; optind < argc; optind++){     
        cout<<"extra arguments: "<<argv[optind]<<"\n"; 
    }
    // xout<<"exitT\n";
    // exit(0);
    if(Quiet != 1 ){
    /// default xout stream 
    	xout.copyfmt(std::cout);                                  
        xout.clear(std::cout.rdstate());                          
        xout.basic_ios<char>::rdbuf(std::cout.rdbuf());
    }
    // xout<<"test1\ntest2\n";




    //string exec(argv[3]);
    char *cmmd = new char [21+strlen(OBJ_PTH)+strlen("dump")];
    strcpy(cmmd, "sparc-elf-objdump -D ");//len = 21 of string without \0
    strcat(cmmd, OBJ_PTH); // the elf file
    strcat(cmmd, ">");
    strcat(cmmd, "dump");//4 the disassembly to write to 
    system(cmmd);
    readDump("dump");
    xout << Start_addr <<" "<<End_addr<< endl;
    //Call the script from here also is possible
    // xout << argv[1] << endl;

    int NWindows = xmlReader_general(config_path, "<NWindows>", "</NWindows>");
    MEM_LAT = xmlReader_general(config_path, "<MEM_LAT>", "</MEM_LAT>") - 1;

    Simulator *s=new Simulator(OBJ_PTH);
    s->startSimulation(Start_addr, End_addr, NWindows);
    
    xout.close();    
    return 0;

}
