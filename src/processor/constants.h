#ifndef CONSTANTS_H
#define CONSTANTS_H

#define ILLEGAL_INSTRUCTION		          0x02
#define PRIVILEGED_INSTRUCTION		      0x03
#define FP_DISABLED                       0x04
#define WINDOW_OVERFLOW                   0x05
#define WINDOW_UNDERFLOW		          0x06
#define MEM_ADDRESS_NOT_ALIGNED		      0x07
#define FP_EXCEPTION    		          0x08
#define TAG_OVERFLOW                      0x0A
#define UNIMPLEMENTED_FLUSH		          0x25
#define DIVISION_BY_ZERO		          0x2A
#define DESIGN_UNIMP    		          0x60

#define UNFINISHED_FPOP    		          2
#define UNIMPLEMENTED_FPOP    		      3
#define INVALID_FP_REGISTER    		      6

#define NXC                               0
#define DZC                               1
#define UFC                               2
#define OFC                               3
#define NVC                               4

#define NXA                               5
#define DZA                               6
#define UFA                               7
#define OFA                               8
#define NVA                               9

#define NXM                              23
#define DZM                              24
#define UFM                              25
#define OFM                              26
#define NVM                              27

#define HALFWORD_ALIGN                    2
#define WORD_ALIGN                        4
#define DOUBLEWORD_ALIGN                  8


#define RET_QUIT     					 0
#define RET_FAILURE                     -1
#define RET_SUCCESS  					 1
#define RET_NOTACOMMAND 				 2
#define RET_TRAP                         5


#define MAX_INPUT_LENGTH 				50


#define ELF_FILE_DOES_NOT_EXIST_ERROR           	-1
#define ELF_BINARY_OUT_OF_DATE				        -2
#define ELF_POINTER_INITIALIZATION_ERROR            -3
#define ELF_SECTION_LOAD_ERROR				        -4

#define FILE_DOES_NOT_EXIST_ERROR			        -2


#define CODE_SECTION				                 1
#define DATA_SECTION				                 2
#define UNINITIALIZED_DATA_SECTION	                 3

#define ELF_FILE_DOES_NOT_EXIST_ERROR           	-1
#define ELF_BINARY_OUT_OF_DATE				        -2
#define ELF_POINTER_INITIALIZATION_ERROR            -3
#define ELF_SECTION_LOAD_ERROR				        -4


#define ICC_CARRY 				                    20
#define ICC_OVERFLOW                                21
#define ICC_ZERO				                    22
#define ICC_NEGATIVE                                23
#define SIGN_BIT				                    31
#define FLOAT_EQUAL                                  0
#define FLOAT_LESSER                                 1
#define FLOAT_GREATER                                2
#define FLOAT_UNORDERED                              3
#define MAX_MEM                                     4294967296 //256 MB
#define OFFSET                                      1073741824


#define INVALID_FP_REGISTER                         6
typedef int regType;
typedef unsigned int addrType;

//#define reset
#define data_store_error                            0x2B
#define instruction_access_MMU_miss                 0x3C
#define instruction_access_error                    0x21
#define r_register_access_error                     0x20
#define instruction_access_exception                0x01
#define privileged_instruction                      0x03
#define illegal_instruction                         0x02
#define fp_disabled                                 0x04
#define cp_disabled                                 0x24
#define unimplemented_FLUSH                         0x25
#define watchpoint_detected                         0x0B
#define window_overflow                             0x05                     
#define window_underflow                            0x06                      
#define mem_address_not_aligned                     0x07                                
#define fp_exception                                0x08                     
#define cp_exception                                0x28                     
#define data_access_error                           0x29                          
#define data_access_MMU_miss                        0x2C                             
#define data_access_exception                       0x09                              
#define tag_overflow                                0x0A                     
#define division_by_zero                            0x2A                         


#define error_mode                                  -1

#endif
