#include "generic/utility.h"

#include <iostream>
#include <fstream>
#include <bitset>
#include <climits>
#include <math.h>
#include <stdio.h>
#include <iomanip>
#include "architecture/constants_typedefs.h"

using namespace std;

void showErrorAndExit(string errorMessage)
{
	cout << errorMessage << endl;
	fflush(stdout);
	fflush(stderr);
	exit(1);
}

void checkFilePath(char *path, enum std::_Ios_Openmode openMode)
{
	fstream f;
	f.open(path, openMode);
	if(!f.is_open())
	{
		string errMessage = "File open failed. File: ";
		errMessage += path;
		errMessage += " ; Mode: ";
		errMessage += to_string(openMode);
		showErrorAndExit(errMessage);
	}
	f.close();
}

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

/* set pth bit of n to b */
int modifyBit(int n, int p, int b)
{
    int mask = 1 << p;
    return (n & ~mask) | ((b << p) & mask);
}

//position goes from 0 to 63
int getBit(long int bitStream, int position)
{
	int bit;
	bit = ((unsigned long int)(bitStream & ((unsigned long int)1 << position))) >> position;
	return bit;
}



int extract(unsigned int instructionWord,int start, int end)
{
    // start is smaller and end is bigger in value always
    // left shift by (31-end), followed by right shift start
    // to get the bits between start and end (inclusive)

    return (instructionWord << (31 - end)) >> (31 - (end-start));
}

unsigned long int extract_float(unsigned long int bitStream,int start, int end)
{
    // start is smaller and end is bigger in value always
    // left shift by (31-end), followed by right shift start
    // to get the bits between start and end(inclusive)

    return (bitStream << ((sizeof(bitStream)*8 - 1) - end)) >> ((sizeof(bitStream)*8 - 1) - (end-start));
}


// FIXME: Is this conversion to array only for
// printing purposes?
void decToBinary(unsigned int n)
{
    cout<<"Binary: ";
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
        cout << binaryNum[j];
    }
    cout << endl;
}

// FIXME: Check why CPU Instruction
// is a char* (and not uint32)?
int convertBtoL(char *cpuInstruction)
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

    cout<<endl;
    decToBinary(instructionWord);
    return instructionWord;
}

regType convertBigEndianByteArrayToLittleEndianInteger(char *x_array, int x_noOfBytes)
{
	regType littleEndian = 0;
	char* tmpArray = (char*)(&littleEndian);
	for(int i = x_noOfBytes-1; i >= 0 ; i--)
	{
		tmpArray[i] = x_array[x_noOfBytes-1-i];
	}
	return littleEndian;
}

char* convertLittleEndianIntegerToBigEndianByteArray(regType x_value, int x_noOfBytes)
{
	char* bytes = new char[x_noOfBytes];
	char* tmpArray = (char*)(&x_value);
	for(int i = x_noOfBytes-1; i >= 0; i--)
	{
		bytes[i] = tmpArray[(x_noOfBytes-1-i)];
	}
	return bytes;
}

/*float convertBigEndianByteArrayToLittleEndianFloat(char* x_array)
{
	float f;
	char* tmp_array = (char*)(&f);
	tmp_array[3] = x_array[0];
	tmp_array[2] = x_array[1];
	tmp_array[1] = x_array[2];
	tmp_array[0] = x_array[3];
	return f;
}

char* convertLittleEndianFloatToBigEndianByteArray(float x_value)
{
	char* byte_array = new char[4];
	char* tmp_array = (char*)(&x_value);
	byte_array[0] = tmp_array[3];
	byte_array[1] = tmp_array[2];
	byte_array[2] = tmp_array[1];
	byte_array[3] = tmp_array[0];
	return byte_array;
}

double convertBigEndianByteArrayToLittleEndianDouble(char* x_array)
{
	double d;
	char* tmp_array = (char*)(&d);
	tmp_array[7] = x_array[0];
	tmp_array[6] = x_array[1];
	tmp_array[5] = x_array[2];
	tmp_array[4] = x_array[3];
	tmp_array[3] = x_array[4];
	tmp_array[2] = x_array[5];
	tmp_array[1] = x_array[6];
	tmp_array[0] = x_array[7];
	return d;
}

char* convertLittleEndianDoubleToBigEndianByteArray(double x_value)
{
	char* byte_array = new char[8];
	char* tmp_array = (char*)(&x_value);
	byte_array[0] = tmp_array[7];
	byte_array[1] = tmp_array[6];
	byte_array[2] = tmp_array[5];
	byte_array[3] = tmp_array[4];
	byte_array[4] = tmp_array[3];
	byte_array[5] = tmp_array[2];
	byte_array[6] = tmp_array[1];
	byte_array[7] = tmp_array[0];
	return byte_array;
}*/



/*
 * converts big endian to little endian
 * and little endian to big endian
 */
unsigned int swapEndian(unsigned int inputWord)
{
	return ((inputWord & 0x000000ff) << 24u) | ((inputWord & 0x0000ff00) << 8u) | ((inputWord & 0x00ff0000) >> 8u) | ((inputWord & 0xff000000) >> 24u);
}


/*
 * Returns 1, if sparcRegister is mis-aligned (Odd).
 * Returns 0, if sparcRegister is not mis-aligned (Even).
 * It's useful to prevent loading mis-aligned registers
 * in double word (DWORD) instructions by raising
 * illegal_instruction and invalid_fp_register traps.
 */
int is_register_mis_aligned (unsigned int registerIndex)
{
    // Is register mis-aligned?
    if(registerIndex % 2)
        return 1;       // Odd register
    else
        return 0;       // Even register
}

void displayWord (char* cpuInstruction, int isInstruction)
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
            cout<<hi;
			if(isInstruction)
				// printf(" ");
                cout<<" ";
		}
	}
}

string GetStdoutFromCommand(string cmd) {

    string data;
    FILE * stream;
    const int max_buffer = 256;
    char buffer[max_buffer];
    cmd.append(" 2>&1");

    stream = popen(cmd.c_str(), "r");
    if (stream) {
		while (!feof(stream))
		{
			if (fgets(buffer, max_buffer, stream) != NULL)
			{
				data.append(buffer);
			}
			pclose(stream);
		}
    }
    return data;
}

int float_to_int_steam(float getVal){
	bitset<sizeof(float) * CHAR_BIT> bits(*(int*)(&getVal));
	int val = (int)bits.to_ulong();
	return val;
}

unsigned long double_to_long_steam(double getVal){
	bitset<sizeof(double) * CHAR_BIT> bits(*(long int*)(&getVal));

    unsigned long val = bits.to_ulong();
	return val;
}

float int_to_float_steam(int getVal){
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

long int* long_double_to_long_long_int_steam(long double getVal){
	long int* ret_Val = new long int [2];

    bitset<sizeof(double) * CHAR_BIT> bits(*(long int*)(&getVal));
	ret_Val[0] = bits.to_ulong();


	return ret_Val;
}//INCOMPLETE



float int_float(int input){//floating point single
	int s, e, f;
	float output;
	s = getBit(input, 31);
	e = extract_float(input, 23, 30);
	f = extract_float(input, 0, 22);
	cout << s << " s" << endl;
	cout << e << " e" << endl;
	cout << f << " f" << endl;
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

double lint_double(long int input){//floating point double
	long int s, e, f;
	double output;
	s = getBit(input, 63);
	e = extract_float(input, 52, 62);
	f = extract_float(input, 0, 51);
	cout << s << " s" << endl;
	cout << e << " e" << endl;
	cout << f << " f" << endl;
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

long double llint_ldouble(long int msb, long int lsb){//floating point quad
	long int s, e, f_msb, f_lsb;
	long double output;
	s = getBit(msb, 63);
	e = extract_float(msb, 47, 62);
	f_msb = extract_float(msb, 0, 51);
    f_lsb = lsb;
	cout << s << " s" << endl;
	cout << e << " e" << endl;
	cout << f_msb << " f_msb" << endl;

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



int extract_exponent_int(int input){
    return extract_float(input, 23, 30);
}

int extract_exponent_lint(long int input){
    return extract_float(input, 52, 62);;
}

int extract_exponent_llint(long int msb){
    return extract_float(msb, 47, 62);
}

int extract_sign_int(int input){
    return getBit(input, 31);
}

int extract_sign_lint(long int input){
    return getBit(input, 31);
}

int extract_sign_llint(long int msb){
    return getBit(msb, 31);
}

int extract_fraction_int(int input){
    return extract_float(input, 0, 22);
}

long int extract_fraction_lint(long int input){
    return extract_float(input, 0, 51);
}

long int extract_fraction_llint(long int msb){
    return extract_float(msb, 0, 51);
}

int is_infinity_int(int input){
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

int is_infinity_lint(long int input){
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

int is_infinity_llint(long int msb, long int lsb){
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

int is_nan_int (int input){
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

int is_nan_lint(long int input){
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

int is_nan_llint(long int msb, long int lsb){
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

void disp_int(int snd, int lpos, int rpos){
    bool bit = 0;
for(int i=lpos;i>=rpos;i--)
    {
        bit = (snd >> i) & 1;
        cout<<bit;

    }
    cout<<"\n";
}

int modifyBits(int snd, int rcv, int l, int r){
    int snd_l_lim_pos = l-r;
    int snd_pos = snd_l_lim_pos;
    int rcv_pos = l;
    while(snd_pos >= 0 ){


        // cout<<((snd >> snd_pos) & 1);

        rcv = ((snd >> snd_pos) & 1) ? (rcv |= 1 << rcv_pos) : (rcv &= ~(1 << rcv_pos));
        // cout<<(t?1:0)<<"LL\n";
        // cout<<rcv_pos<<" "<<snd_pos<<"\n";
        rcv_pos--;
        snd_pos--;
    }
    return rcv;
}


int getBits(int snd, int rcv, int l, int r){
    int snd_l_lim_pos = l-r;
    int snd_pos = l;
    int rcv_pos = snd_l_lim_pos;
    while(rcv_pos >= 0 ){

        rcv = ((snd >> snd_pos) & 1) ? (rcv |= 1 << rcv_pos) : (rcv &= ~(1 << rcv_pos));
        // cout<<(t?1:0)<<"LL\n";
        // cout<<rcv_pos<<" "<<snd_pos<<"\n";
        rcv_pos--;
        snd_pos--;
    }

    return rcv;
}

int signExtendByteToWord(int byte)
{
	if(byte & 0x00000080 == 0x00000080)
	{
		byte |= 0xFFFFFF00;
	}
	return byte;
}

int signExtendHalfWordToWord(int halfword)
{
	if(halfword & 0x00008000 == 0x00008000)
	{
		halfword |= 0xFFFF0000;
	}
	return halfword;
}
