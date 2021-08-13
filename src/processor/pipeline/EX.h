#ifndef EX_H
#define EX_H

#include "EX_ME_latch.h"
#include "RA_EX_latch.h"
#include "interface/Element.h"
#include "interface/sim_globals.h"


class EX : public Element 
{
        RA_EX_latch* raex_latch;
        EX_ME_latch* exme_latch;
    public:
        EX( RA_EX_latch* raex, EX_ME_latch* exme );
        inline int addOverflow(regType regRS1, regType reg_or_imm, regType regRD);
        inline int subtractOverflow(regType regRS1, regType reg_or_imm, regType regRD);
        inline int addCarry(regType regRS1, regType reg_or_imm, regType regRD);
        inline int subtractCarry(regType regRS1, regType reg_or_imm, regType regRD);
        int LoadIntegerInstructions(instruction* ins);
        int LoadFloatingPointInstructions(instruction* ins);
        int StoreIntegerInstructions(instruction* ins);
        int StoreFloatInstructions(instruction* ins);
        int AtomicLoadStoreUnsignedByte(instruction* ins);
        int SWAP(instruction* ins);
        void SethiNop(instruction* ins);
        void LogicalInstructions(instruction* ins);
        void ShiftInstructions(instruction* ins);
        void AddInstructions(instruction* ins);
        int TaggedAddInstructions(instruction* ins);
        void SubtractInstructions(instruction* ins);
        int TaggedSubtractInstructions(instruction* ins);
        void MultiplyStepInstruction(instruction* ins);
        void MultiplyInstructions(instruction* ins);
        int DivideInstructions(instruction* ins);
        int SaveAndRestoreInstructions(instruction* ins);
        void BranchIntegerInstructions(instruction* ins);
        int BranchFloatInstructions(instruction* ins);
        void CallInstruction(instruction* ins);
        int JumpAndLink(instruction* ins);
        int RETT(instruction* ins);
        int TrapOnICC(instruction* ins);
        int ReadStateRegisterInstructions(instruction* ins);
        int WriteStateRegisterInstructions(instruction* ins);
        int FpopInstructions(instruction* ins);
        void updateICCAddSubtract(regType regRS1, regType reg_or_imm, regType regRD, int op);
        int taggedAddSubtract(regType regRS1, regType reg_or_imm, regType regRD, int isTVOpcode, int op);
        void updateICCMulLogical(regType regRD);
        void updateICCDiv(regType regRD, int isOverflow);
        void perform();
        void handleEvent();
};

#endif
