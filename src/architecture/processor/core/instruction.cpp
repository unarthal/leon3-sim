#include "architecture/processor/core/instruction.h"

instruction::instruction()
{
	instructionWord = -1;
	disp30 = rd = op2 = imm22 = op = a = cond = op3 = rs1 = i = asi = rs2 = opf = -1;
	disp22 = simm13 = imm7 = -1;
}

instruction::~instruction()
{

}

bool instruction::isControlTransferInstruction()
{
	if(op == 1 || (op == 0 && op2 != 4) || (/*jump and link*/op == 2 && op3 == 56)|| (/*RETT*/op == 2 && op3 == 57))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool instruction::isConditionalControlTransferInstruction()
{
	if((/*Bicc*/op == 0 && ((cond >= 1 && cond <= 7) || (cond >= 9 && cond <= 15)))
			|| (/*Ticc*/op == 2 && op3 == 58 && (((rd & 0xf) >= 1 && (rd & 0xf) <= 7) || ((rd & 0xf) >= 9 && (rd & 0xf) <= 15))))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool instruction::isICCWritingInstruction()
{
	if((op == 2
			&& (/*logical*/op3 == 17 || op3 == 21 || op3 == 18 || op3 == 22 || op3 == 19 || op3 == 23
					/*add*/ || op3 == 16 || op3 == 24
					/*tagged add*/ || op3 == 32 || op3 == 34
					/*subtract*/ || op3 == 20 || op3 == 28
					/*tagged subtract*/ || op3 == 33 || op3 == 35
					/*multiply step*/ || op3 == 36
					/*multiply*/ || op3 == 26 || op3 == 27
					/*divide*/ || op3 == 30 || op3 == 31))
			|| (/*WRPSR*/op == 2 && op3 == 49))
	{
		return true;
	}

	return false;
}

bool instruction::isCWPWritingInstruction()
{
	 if((/*save and restore*/op == 2 && (op3 == 60 || op3 == 61)) || (/*RETT*/op == 2 && op3 == 57))
	 {
		 return true;
	 }

	 return false;
}

bool instruction::isYWritingInstruction()
{
	if((op == 2
			&& (/*UMUL*/op3 == 10 || /*SMUL*/op3 == 11 || /*UMULcc*/op3 == 26 || /*SMULcc*/op3 == 27 || /*MULScc*/op3 == 36 || (/*WRY*/op3 == 48 && rs1 == 0))))
	{
		return true;
	}

	return false;
}

bool instruction::isYReadingInstruction()
{
	if((op == 2
			&& (/*UDIV*/op3 == 14 || /*SDIV*/op3 == 15 || /*UDIVcc*/op3 == 30 || /*SDIVcc*/op3 == 31 || /*MULScc*/op3 == 36 || (/*RDY*/op3 == 40 && rs1 == 0))))
	{
		return true;
	}

	return false;
}

bool instruction::isASRWritingInstruction()
{
	if(/*WRASR*/op == 2 && op3 == 48 && rs1 != 0)
	{
		return true;
	}

	return false;
}

bool instruction::isASRReadingInstruction()
{
	if(/*RDASR*/op == 2 && op3 == 40 && rs1 != 0)
	{
		return true;
	}

	return false;
}

bool instruction::isWIMWritingInstruction()
{
	if(/*WRWIM*/op == 2 && op3 == 50)
	{
		return true;
	}

	return false;
}

bool instruction::isWIMReadingInstruction()
{
	if(/*RDWIM*/op == 2 && op3 == 42)
	{
		return true;
	}

	return false;
}
