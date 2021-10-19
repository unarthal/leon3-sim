#ifndef INSTRUCTION_H
#define INSTRUCTION_H

class instruction
{
	public:
		int instructionWord;
        int  disp30, rd, op2, imm22, op, a, cond, op3, rs1, i, asi, rs2, opf;
        int  disp22, simm13, imm7;

        instruction();
        ~instruction();

        bool isControlTransferInstruction();
        bool isConditionalControlTransferInstruction();
        bool isICCWritingInstruction();
        bool isCWPWritingInstruction();
        bool isYWritingInstruction();
        bool isYReadingInstruction();
        bool isASRWritingInstruction();
        bool isASRReadingInstruction();
        bool isWIMWritingInstruction();
        bool isWIMReadingInstruction();
};

#endif
