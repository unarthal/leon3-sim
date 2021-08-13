#ifndef UTILITY_H
#define UTILITY_H

#include <iostream>
#include <fstream>
#include <bitset>
#include <climits>
//#include "header.h"
#include <math.h> 
// #include <cstdio>
#include <stdio.h>
#include <iomanip>

using namespace std;

extern std::ofstream xout;

inline int modifyBit(int n, int p, int b) 
{ 
    int mask = 1 << p; 
    return (n & ~mask) | ((b << p) & mask); 
}

//position goes from 0 to 63
inline int getBit(long int bitStream, int position)
{
	int bit;
	bit = ((unsigned long int)(bitStream & ((unsigned long int)1 << position))) >> position;
	return bit;
}



inline int extract(unsigned int instructionWord,int start, int end)
{
    // start is smaller and end is bigger in value always
    // left shift by (31-end), followed by right shift start 
    // to get the bits between start and end (inclusive)

    // TODO: Make the following code more readable
    return (instructionWord << 31 - end) >> (31 - (end-start));
}

inline unsigned long int extract_float(unsigned long int bitStream,int start, int end)
{
    // start is smaller and end is bigger in value always
    // left shift by (31-end), followed by right shift start 
    // to get the bits between start and end(inclusive)

    // TODO: Make the following code more readable
    return (bitStream << ((sizeof(bitStream)*8 - 1) - end)) >> ((sizeof(bitStream)*8 - 1) - (end-start));
}


// FIXME: Is this conversion to array only for 
// printing purposes?
inline void decToBinary(unsigned int n) 
{ 
    xout<<"Binary: ";
    // array to store binary number 
    int binaryNum[32]; 
  
    // counter for binary array 
    int i = 0; 
    while (n > 0) 
    { 
        // storing remainder in binary array 
        binaryNum[i] = n % 2; 
        n = n / 2; 
        i++; 
    } 
  
    // printing binary array in reverse order 
    for (int j = i - 1; j >= 0; j--) 
    {
        xout << binaryNum[j]; 
    }
    xout << endl;
} 

// FIXME: Check why CPU Instruction 
// is a char* (and not uint32)?
inline int convertBtoL(char *cpuInstruction)
{
    unsigned int hexDigit, instructionWord;
    hexDigit = cpuInstruction[0]; 
    hexDigit = (hexDigit << 24) >> 24; 
    instructionWord = (instructionWord << 8) | hexDigit;

	hexDigit = cpuInstruction[1]; 
    hexDigit = (hexDigit << 24) >> 24; 
    instructionWord = (instructionWord << 8) | hexDigit;

	hexDigit = cpuInstruction[2]; 
    hexDigit = (hexDigit << 24) >> 24; 
    instructionWord = (instructionWord << 8) | hexDigit;

	hexDigit = cpuInstruction[3]; 
    hexDigit = (hexDigit << 24) >> 24; 
    instructionWord = (instructionWord << 8) | hexDigit;

    xout<<endl;
    decToBinary(instructionWord);
    return instructionWord;
}


/*
 * Returns 1, if sparcRegister is mis-aligned (Odd).
 * Returns 0, if sparcRegister is not mis-aligned (Even).
 * It's useful to prevent loading mis-aligned registers
 * in double word (DWORD) instructions by raising
 * illegal_instruction and invalid_fp_register traps.
 */
inline int is_register_mis_aligned (unsigned int registerIndex)
{
    // Is register mis-aligned?
    if(registerIndex % 2)
        return 1;       // Odd register
    else
        return 0;       // Even register
}

inline void displayWord (char* cpuInstruction, int isInstruction)
{
	if(cpuInstruction != NULL)
	{
		int count; 
		unsigned int hexDigit; 
		for(count = 0; count <= 3; count++)
		{
			char instructionByte = cpuInstruction[count];
			hexDigit = instructionByte;
                        
                       /* hexDigit is left shifted by 24 bits followed by right shifted by 24 bits 
                        * to clear higher order 24 bits, if set by sign extension caused by widening 
                        * of data during auto-casting. Casting takes place because of hexDigit being an
                        * unsigned long (32 bits) while cpuInstruction is an array of type char (8 bits).
                        */
			hexDigit = (hexDigit << 24) >> 24;
			// printf("%02X", hexDigit);
            char hi[32];
            sprintf(hi,"%X",hexDigit );
            xout<<hi;
			if(isInstruction)
				// printf(" ");
                xout<<" ";
		}
	}
}

inline string GetStdoutFromCommand(string cmd) {

    string data;
    FILE * stream;
    const int max_buffer = 256;
    char buffer[max_buffer];
    cmd.append(" 2>&1");

    stream = popen(cmd.c_str(), "r");
    if (stream) {
    while (!feof(stream))
        if (fgets(buffer, max_buffer, stream) != NULL) data.append(buffer);
        pclose(stream);
    }
    return data;
}

inline int float_to_int_steam(float getVal){
	bitset<sizeof(float) * CHAR_BIT> bits(*(int*)(&getVal));
	int val = (int)bits.to_ulong();
	return val;
}

inline unsigned long double_to_long_steam(double getVal){
	bitset<sizeof(double) * CHAR_BIT> bits(*(long int*)(&getVal));
	
    unsigned long val = bits.to_ulong();
	return val;
}

inline float int_to_float_steam(int getVal){
	bitset<sizeof(float) * CHAR_BIT> bits(getVal);
	int exponent = 0;
    exponent = getVal << 1;
    exponent = (((unsigned int)(exponent)) >> 24);
    int fraction = getVal << 9;
    fraction = (((unsigned int)(fraction)) >> 9);
    int digits = 0;
    int tmp_fraction = fraction;
    while (tmp_fraction != 0) 
    {
        tmp_fraction = tmp_fraction / 10;
        digits = digits + 1;
    }
    int sign = (((unsigned int)(getVal)) >> 31);
    float ret_val = 0;
    if(exponent==0){
        ret_val = pow(2,-126)*pow(10,digits)*fraction*pow(-1, sign);
    }
    else{
        ret_val = pow(2,exponent-127)*pow(10,digits)*(fraction*pow(-1, sign) + 1);
    }

	return ret_val;
}

inline long int* long_double_to_long_long_int_steam(long double getVal){
	long int* ret_Val = new long int [2];

    bitset<sizeof(double) * CHAR_BIT> bits(*(long int*)(&getVal));
	ret_Val[0] = bits.to_ulong();


	return ret_Val;
}//INCOMPLETE



inline float int_float(int input){//floating point single
	int s, e, f, u;
	float output;
	s = getBit(input, 31);
	e = extract_float(input, 23, 30);
	f = extract_float(input, 0, 22);
	xout << s << " s" << endl;
	xout << e << " e" << endl;
	xout << f << " f" << endl;
	if(0 < e && e<255){
		output = pow(-1, s)*pow(2,e-127)*(1+pow(10, -23)*f);
	}
	else if(e==0){
		output = pow(-1, s)*pow(2,-126)*(pow(10, -23)*f);
	}
	else{
		output = 0;
	}
	return output;
}

inline double lint_double(long int input){//floating point double
	long int s, e, f, u;
	double output;
	s = getBit(input, 63);
	e = extract_float(input, 52, 62);
	f = extract_float(input, 0, 51);
	xout << s << " s" << endl;
	xout << e << " e" << endl;
	xout << f << " f" << endl;
	if(0 < e && e<2047){
		output = pow(-1, s)*pow(2,e-1023)*(1+pow(10, -52)*f);
	}
	else if(e==0){
		output = pow(-1, s)*pow(2,-1022)*(pow(10, -52)*f);
    }
	else{
		output = 0;
	}
	return output;
}

inline long double llint_ldouble(long int msb, long int lsb){//floating point quad
	long int s, e, f_msb, f_lsb, u;
	long double output;
	s = getBit(msb, 63);
	e = extract_float(msb, 47, 62);
	f_msb = extract_float(msb, 0, 51);
    f_lsb = lsb;
	xout << s << " s" << endl;
	xout << e << " e" << endl;
	xout << f_msb << " f_msb" << endl;
	
	if(0 < e && e<32767){
		output = pow(-1, s)*pow(2,e-16383)*(1+pow(10, -112)*f_lsb+pow(10, -48)*f_msb);
	}
	else if(e==0){
		output = pow(-1, s)*pow(2,-1022)*(pow(10, -112)*f_lsb+pow(10, -48)*f_msb);
	}
	else{
		output = 0;
	}
	return output;
}



inline int extract_exponent_int(int input){
    return extract_float(input, 23, 30);
}

inline int extract_exponent_lint(long int input){
    return extract_float(input, 52, 62);;
}

inline int extract_exponent_llint(long int msb){
    return extract_float(msb, 47, 62);
}

inline int extract_sign_int(int input){
    return getBit(input, 31);
}

inline int extract_sign_lint(long int input){
    return getBit(input, 31);
}

inline int extract_sign_llint(long int msb){
    return getBit(msb, 31);
}

inline int extract_fraction_int(int input){
    return extract_float(input, 0, 22);
}

inline long int extract_fraction_lint(long int input){
    return extract_float(input, 0, 51);
}

inline long int extract_fraction_llint(long int msb){
    return extract_float(msb, 0, 51);
}

inline int is_infinity_int(int input){
    int s = extract_sign_int(input);
    int e = extract_exponent_int(input);
    int f = extract_fraction_int(input);
    if(s == 1 && e == 255 && f == 0){
        return -1;
    }
    else if (s == 0 && e == 255 && f == 0)
    {
        return 1;
    }
    else{
        return 0;
    }
}

inline int is_infinity_lint(long int input){
    int s = extract_sign_lint(input);
    int e = extract_exponent_lint(input);
    long int f = extract_fraction_lint(input);
    if(s == 1 && e == 2047 && f == 0){
        return -1;
    }
    else if (s == 0 && e == 2047 && f == 0)
    {
        return 1;
    }
    else{
        return 0;
    }
}

inline int is_infinity_llint(long int msb, long int lsb){
    int s = extract_sign_llint(msb);
    int e = extract_exponent_llint(msb);
    long int f_msb = extract_fraction_llint(msb);
    if(s == 1 && e == 32767 && f_msb == 0 && lsb==0){
        return -1;
    }
    else if (s == 0 && e == 32767 && f_msb == 0 && lsb==0)
    {
        return 1;
    }
    else{
        return 0;
    }
}

inline int is_nan_int (int input){
    int s = extract_sign_int(input);
    int e = extract_exponent_int(input);
    int f = extract_fraction_int(input);
    if(e == 255 && f == 1){
        return 1;//signalling nan
    }
    else if (e == 255 && f != 0 && f!=1)
    {
        return -1;//quiet nan
    }
    else{
        return 0;
    }
}

inline int is_nan_lint(long int input){
    int s = extract_sign_lint(input);
    int e = extract_exponent_lint(input);
    long int f = extract_fraction_lint(input);
    if(e == 2047 && f == 1){
        return 1;
    }
    else if (e == 2047 && f != 0 && f!=1)
    {
        return -1;
    }
    else{
        return 0;
    }
}

inline int is_nan_llint(long int msb, long int lsb){
    int s = extract_sign_llint(msb);
    int e = extract_exponent_llint(msb);
    long int f_msb = extract_fraction_llint(msb);
    if(e == 32767 && f_msb == 0 && lsb==1){
        return 1;
    }
    else if (e == 32767 && f_msb != 0 && lsb==0)
    {
        return -1;
    }
    else{
        return 0;
    }
}

inline void disp_int(int snd, int lpos, int rpos){
    bool bit = 0;
for(int i=lpos;i>=rpos;i--)
    {
        bit = (snd >> i) & 1;
        xout<<bit;
        
    }
    xout<<"\n";
}

inline int modifyBits(int snd, int rcv, int l, int r){
    int snd_l_lim_pos = l-r;
    int snd_pos = snd_l_lim_pos;
    int rcv_pos = l;
    int t=0;
    while(snd_pos >= 0 ){
        
        
        // xout<<((snd >> snd_pos) & 1);
        
        rcv = ((snd >> snd_pos) & 1) ? (rcv |= 1 << rcv_pos) : (rcv &= ~(1 << rcv_pos)); 
        // xout<<(t?1:0)<<"LL\n";
        // xout<<rcv_pos<<" "<<snd_pos<<"\n";
        rcv_pos--;
        snd_pos--;
    }    
    return rcv;
}


inline int getBits(int snd, int rcv, int l, int r){
    int snd_l_lim_pos = l-r;
    int snd_pos = l;
    int rcv_pos = snd_l_lim_pos;
    int t=0;
    while(rcv_pos >= 0 ){
        
        rcv = ((snd >> snd_pos) & 1) ? (rcv |= 1 << rcv_pos) : (rcv &= ~(1 << rcv_pos)); 
        // xout<<(t?1:0)<<"LL\n";
        // xout<<rcv_pos<<" "<<snd_pos<<"\n";
        rcv_pos--;
        snd_pos--;
    }    
    
    return rcv;
}


//// STATICTICS
extern long long unsigned int clock_cycles;
extern long long unsigned int nINS; 
extern long long unsigned int nMR; 
extern long long unsigned int nMW;
extern long long unsigned int nBT;

inline void generateStats(){
    ofstream ostat;

    ostat.open("sim_stats.txt");
    ostat << "SIMULATOR STATS:\n\n";
    
    ostat << "#CYCLES TAKEN: " << clock_cycles << "\n";
    ostat << "#INSTRUCTIONS EXECUTED: " << nINS << "\n";
    ostat << "#IPC: " << (nINS/(float)clock_cycles) << "\n";
    ostat << "#MEMORY READS: " << nMR << "\n";
    ostat << "#MEMORY WRITES: " << nMW << "\n";
    ostat << "#BRANCH TAKEN: " << nBT << "\n";

    ostat.close();
}


/// PSR ranges l--left r--right
#define PSR_impl_l  31
#define PSR_impl_r  28

#define PSR_ver_l  27
#define PSR_ver_r  24

#define PSR_icc_l  23
#define PSR_icc_r  20

#define PSR_res_l  19
#define PSR_res_r  14

#define PSR_EC_l  13
#define PSR_EC_r  13

#define PSR_EF_l  12
#define PSR_EF_r  12

#define PSR_PIL_l  11
#define PSR_PIL_r  8

#define PSR_S_l  7
#define PSR_S_r  7

#define PSR_PS_l  6
#define PSR_PS_r  6

#define PSR_ET_l  5
#define PSR_ET_r  5

#define PSR_CWP_l  4
#define PSR_CWP_r  0

/// FSR ranges l--left r--right
#define FSR_RD_l  31
#define FSR_RD_r  30

#define FSR_uhigh_l  29
#define FSR_uhigh_r  28

#define FSR_TEM_l  27
#define FSR_TEM_r  23

#define FSR_NS_l  22
#define FSR_NS_r  22

#define FSR_res_l  21
#define FSR_res_r  20

#define FSR_ver_l  19
#define FSR_ver_r  17

#define FSR_ftt_l  16
#define FSR_ftt_r  14

#define FSR_qne_l  13
#define FSR_qne_r  13

#define FSR_ulow_l  12
#define FSR_ulow_r  12

#define FSR_fcc_l  11
#define FSR_fcc_r  10

#define FSR_aexc_l  9
#define FSR_aexc_r  5

#define FSR_cexc_l  4
#define FSR_cexc_r  0

/// TBR ranges l--left r--right
#define TBR_TBA_l  31
#define TBR_TBA_r  12

#define TBR_TT_l  11
#define TBR_TT_r  4

#define TBR_zero_l  3
#define TBR_zero_r  0

#endif
