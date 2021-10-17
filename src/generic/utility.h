#ifndef SRC_GENERIC_UTILITY_H_
#define SRC_GENERIC_UTILITY_H_

#include <string>
#include <fstream>
#include "architecture/constants_typedefs.h"

//TODO clean up this file. remove all unused functions. re-write cleanly the ones that remain.

void showErrorAndExit(std::string errorMessage);
void checkFilePath(char *path, enum std::_Ios_Openmode openMode);
int convertHexToInt(std::string l_hex);

template <typename I> std::string convertNumberToHex(I w, size_t hex_len = sizeof(I)<<1) {
    static const char* digits = "0123456789ABCDEF";
    std::string rc(hex_len,'0');
    for (size_t i=0, j=(hex_len-1)*4 ; i<hex_len; ++i,j-=4)
        rc[i] = digits[(w>>j) & 0x0f];
    return rc;
}

int modifyBit(int n, int p, int b); /* set pth bit of n to b */
int getBit(long int bitStream, int position); //position goes from 0 to 63
int extract(unsigned int instructionWord,int start, int end);
unsigned long int extract_float(unsigned long int bitStream,int start, int end);
void decToBinary(unsigned int n);
int convertBtoL(char *cpuInstruction);
regType convertBigEndianByteArrayToLittleEndianInteger(char *x_array, int x_noOfBytes);
char* convertLittleEndianIntegerToBigEndianByteArray(regType x_value, int x_noOfBytes);
/*float convertBigEndianByteArrayToLittleEndianFloat(char* x_array);
char* convertLittleEndianFloatToBigEndianByteArray(float x_value);
double convertBigEndianByteArrayToLittleEndianDouble(char* x_array);
char* convertLittleEndianDoubleToBigEndianByteArray(double x_value);*/
unsigned int swapEndian(unsigned int inputWord);

/*
 * Returns 1, if sparcRegister is mis-aligned (Odd).
 * Returns 0, if sparcRegister is not mis-aligned (Even).
 * It's useful to prevent loading mis-aligned registers
 * in double word (DWORD) instructions by raising
 * illegal_instruction and invalid_fp_register traps.
 */
int is_register_mis_aligned (unsigned int registerIndex);
void displayWord (char* cpuInstruction, int isInstruction);
std::string GetStdoutFromCommand(std::string cmd);
int float_to_int_steam(float getVal);
unsigned long double_to_long_steam(double getVal);
float int_to_float_steam(int getVal);
long int* long_double_to_long_long_int_steam(long double getVal);
float int_float(int input);
double lint_double(long int input);
long double llint_ldouble(long int msb, long int lsb);
int extract_exponent_int(int input);
int extract_exponent_lint(long int input);
int extract_exponent_llint(long int msb);
int extract_sign_int(int input);
int extract_sign_lint(long int input);
int extract_sign_llint(long int msb);
int extract_fraction_int(int input);
long int extract_fraction_lint(long int input);
long int extract_fraction_llint(long int msb);
int is_infinity_int(int input);
int is_infinity_lint(long int input);
int is_infinity_llint(long int msb, long int lsb);
int is_nan_int (int input);
int is_nan_lint(long int input);
int is_nan_llint(long int msb, long int lsb);
void disp_int(int snd, int lpos, int rpos);
int modifyBits(int snd, int rcv, int l, int r);
int getBits(int snd, int rcv, int l, int r);
int signExtendByteToWord(int byte);
int signExtendHalfWordToWord(int halfword);
#endif
