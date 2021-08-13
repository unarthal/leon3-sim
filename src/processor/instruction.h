#ifndef INSTRUCTION_H
#define INSTRUCTION_H

class instruction
{
	public:
        unsigned int  disp30, rd, op2, imm22, op, a, cond, op3, rs1, i, asi, rs2, opf;
        int  disp22, simm13, imm7;
};

#endif
