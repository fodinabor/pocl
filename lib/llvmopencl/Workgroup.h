// Header for Workgroup.cc module pass.
//
// Copyright (c) 2011 Universidad Rey Juan Carlos
//               2011-2018 Pekka Jääskeläinen
//               2023 Pekka Jääskeläinen / Intel Finland Oy
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#ifndef _POCL_WORKGROUP_H
#define _POCL_WORKGROUP_H

#include "config.h"
#include "LLVMUtils.h"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/Pass.h>

namespace pocl {
  class Workgroup : public llvm::ModulePass {
  public:
    static char ID;

    Workgroup() : ModulePass(ID) {}

    virtual bool runOnModule(llvm::Module &M);

    static bool isKernelToProcess(const llvm::Function &F);
    static bool hasWorkgroupBarriers(const llvm::Function &F);
  private:
    llvm::Function *createWrapper(
      llvm::Function *F, FunctionMapping &printfCache);

    void createGridLauncher(
      llvm::Function *KernFunc, llvm::Function *WGFunc, std::string KernName);

    llvm::Function*
      createArgBufferWorkgroupLauncher(llvm::Function *Func,
                                       std::string KernName);

    void createDefaultWorkgroupLauncher(llvm::Function *F);
    void createFastWorkgroupLauncher(llvm::Function *F);

    std::vector<llvm::Value*>
      globalHandlesToContextStructLoads(
        llvm::IRBuilder<> &Builder,
        const std::vector<std::string> &&GlobalHandleNames,
        int StructFieldIndex);

    void addPlaceHolder(llvm::IRBuilder<> &Builder, llvm::Value *Value,
                        const std::string TypeStr);

    void privatizeGlobals(llvm::Function *F, llvm::IRBuilder<> &Builder,
                          const std::vector<std::string> &&GlobalHandleNames,
                          std::vector<llvm::Value*> PrivateValues);

    void privatizeContext(llvm::Function *F);

    llvm::Value *createLoadFromContext(
      llvm::IRBuilder<> &Builder, int StructFieldIndex, int FieldIndex);

    void addGEPs(llvm::IRBuilder<> &Builder, int StructFieldIndex,
                 const char* FormatStr);

    void addRangeMetadataForPCField(llvm::Instruction *Instr,
                                    int StructFieldIndex, int FieldIndex = -1);

    LLVMValueRef createAllocaMemcpyForStruct(LLVMModuleRef M,
                                             LLVMBuilderRef Builder,
                                             llvm::Argument &Arg,
                                             LLVMValueRef ArgByteOffset);

    LLVMValueRef createArgBufferLoad (LLVMBuilderRef Builder,
                                      LLVMValueRef ArgBufferPtr,
                                      uint64_t *ArgBufferOffsets,
                                      LLVMContextRef Ctx, LLVMValueRef F,
                                      unsigned ParamIndex);

    llvm::Value *getRequiredSubgroupSize(llvm::Function &F);

    llvm::Module *M;
    llvm::LLVMContext *C;

    // Set to the hidden context argument.
    llvm::Argument *ContextArg;

    // Set to the hidden group_id_* kernel args.
    std::vector<llvm::Value*> GroupIdArgs;

    // Number of hidden args added to the work-group function.
    unsigned HiddenArgs = 0;

    // The width of the size_t data type in the current target.
    int SizeTWidth = 64;
    llvm::Type *SizeT = nullptr;
    llvm::Type *PoclContextT = nullptr;
    llvm::FunctionType *LauncherFuncT = nullptr;

    // Copies of compilation parameters
    std::string KernelName;
    uint64_t address_bits;
    bool WGAssumeZeroGlobalOffset;
    bool WGDynamicLocalSize;
    bool DeviceUsingArgBufferLauncher;
    bool DeviceUsingGridLauncher;
    bool DeviceIsSPMD;
    uint64_t WGLocalSizeX;
    uint64_t WGLocalSizeY;
    uint64_t WGLocalSizeZ;
    uint64_t WGMaxGridDimWidth;

    uint64_t DeviceGlobalASid;
    uint64_t DeviceLocalASid;
    uint64_t DeviceConstantASid;
    uint64_t DeviceContextASid;
    uint64_t DeviceArgsASid;
    bool DeviceSidePrintf;
    bool DeviceAllocaLocals;
    uint64_t DeviceMaxWItemDim;
    uint64_t DeviceMaxWItemSizes[3];
  };
}

#endif
