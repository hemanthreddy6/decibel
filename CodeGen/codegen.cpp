// Codegen/codegen.cpp

// #include "codegen.h"

// Include LLVM headers
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"
#include <cstdio>
#include <iostream>
// #include <llvm-14/llvm/ADT/STLFunctionalExtras.h>
// #include <llvm-14/llvm/IR/Constants.h>
// #include <llvm-14/llvm/IR/Instructions.h>
// #include <llvm-14/llvm/Support/Casting.h>
// #include <llvm-14/llvm/IR/Instructions.h>
#include <math.h>
#include <string>
#include <vector>

#define CODEGEN 1

#include "../Semantic/semantic_analyser.cpp"

using namespace llvm;
using namespace std;

// LLVM Global Variables
static LLVMContext TheContext;
static IRBuilder<> Builder(TheContext);
static unique_ptr<Module> TheModule;

// Audio type
StructType *AudioType = nullptr;

// Symbol Table for code generation
static vector<unordered_map<string, AllocaInst *>> NamedValues;
int num_funcs = 0;
vector<pair<Stype *, Function *>> function_nodes;

// Function Declarations
Value *codegen(Stype *node);
Type *getLLVMType(DataType *dtype);
Value *convertToString(Value *Val);
Value *createCast(Value *Val, Type *DestType);
Function *getFunction(const string &name);
AllocaInst *CreateEntryBlockAlloca(Function *TheFunction, const string &VarName, Type *varType);
void pushSymbolTable();
void popSymbolTable();
void declarePrintFunction();
void declareAudioFunctions();

// External function declarations
Function *loadAudioFunction = nullptr;
Function *playAudioFunction = nullptr;
Function *saveAudioFunction = nullptr;
Function *freeAudioFunction = nullptr;
Function *concatAudioFunction = nullptr;
Function *sliceAudioFunction = nullptr;
Function *repeatAudioFunction = nullptr;
Function *superimposeAudioFunction = nullptr;
Function *scaleAudioFunctionStatic = nullptr;
Function *scaleAudioFunctionDynamic = nullptr;
Function *generateAudioFunctionStatic = nullptr;
Function *generateAudioFunctionDynamic = nullptr;
Function *printFunction = nullptr;

// Push a new symbol table onto the stack
void pushSymbolTable() { NamedValues.emplace_back(); }

// Pop the current symbol table off the stack
void popSymbolTable() { NamedValues.pop_back(); }

// Look up a variable in the symbol table
AllocaInst *findVariable(const string &name) {
    // for (auto it = NamedValues.rbegin(); it != NamedValues.rend(); ++it) {
    //     auto varIter = it->find(name);
    //     if (varIter != it->end())
    //         return varIter->second;
    // }
    for (int i = NamedValues.size() - 1; i >= 0; i--) {
        auto varIter = NamedValues[i].find(name);
        if (varIter != NamedValues[i].end())
            return varIter->second;
    }
    cerr << "Could not find " << name << endl;
    return nullptr;
}

// Declare external audio functions
void declareAudioFunctions() {
    if (!loadAudioFunction) {
        // struct Audio load_audio(char * filename);
        FunctionType *LoadAudioType = FunctionType::get(AudioType, {Type::getInt8Ty(TheContext)->getPointerTo()}, false);
        loadAudioFunction = Function::Create(LoadAudioType, Function::ExternalLinkage, "load_audio", TheModule.get());

        // void play_audio(struct Audio audio_var);
        FunctionType *PlayAudioType = FunctionType::get(Type::getVoidTy(TheContext), {AudioType}, false);
        playAudioFunction = Function::Create(PlayAudioType, Function::ExternalLinkage, "play_audio", TheModule.get());

        // void save_audio(struct Audio audio_var, char* filename);
        FunctionType *SaveAudioType = FunctionType::get(Type::getVoidTy(TheContext), {AudioType, Type::getInt8Ty(TheContext)->getPointerTo()}, false);
        saveAudioFunction = Function::Create(SaveAudioType, Function::ExternalLinkage, "save_audio", TheModule.get());

        // void free_audio(struct Audio audio_var);
        FunctionType *FreeAudioType = FunctionType::get(Type::getVoidTy(TheContext), {AudioType}, false);
        freeAudioFunction = Function::Create(FreeAudioType, Function::ExternalLinkage, "free_audio", TheModule.get());

        // struct Audio concat_audio(struct Audio audio_var1, struct Audio audio_var2);
        FunctionType *ConcatAudioType = FunctionType::get(AudioType, {AudioType, AudioType}, false);
        concatAudioFunction = Function::Create(ConcatAudioType, Function::ExternalLinkage, "concat_audio", TheModule.get());

        // struct Audio slice_audio(struct Audio audio_var, double start_time_seconds, double end_time_seconds);
        // returns an audio file by trimming the first <start_time> seconds of the file, until <end_time>. If start_time is negative, it adds extra padding
        // in the front with no sound(zeros), and if end_time exceeds the file duration, adds padding at the end
        FunctionType *SliceAudioType = FunctionType::get(AudioType, {AudioType, Type::getDoubleTy(TheContext), Type::getDoubleTy(TheContext)}, false);
        sliceAudioFunction = Function::Create(SliceAudioType, Function::ExternalLinkage, "slice_audio", TheModule.get());

        // struct Audio repeat_audio(struct Audio audio_var, double times);
        // repeats an audio file <times> times. If <times> is a fraction, it will partially repeat the audio
        FunctionType *RepeatAudioType = FunctionType::get(AudioType, {AudioType, Type::getDoubleTy(TheContext)}, false);
        repeatAudioFunction = Function::Create(RepeatAudioType, Function::ExternalLinkage, "repeat_audio", TheModule.get());

        // struct Audio superimpose_audio(struct Audio audio_var1, struct Audio audio_var2);
        // superimposes one audio file over the other. The resulting file's length will be the max length of the two files
        FunctionType *SuperimposeAudioType = FunctionType::get(AudioType, {AudioType, Type::getDoubleTy(TheContext)}, false);
        superimposeAudioFunction = Function::Create(SuperimposeAudioType, Function::ExternalLinkage, "superimpose_audio", TheModule.get());

        FunctionType *ScaleAudioStatic = FunctionType::get(AudioType, {AudioType, Type::getDoubleTy(TheContext)}, false);
        scaleAudioFunctionStatic = Function::Create(ScaleAudioStatic, Function::ExternalLinkage, "scale_audio_static", TheModule.get());

        FunctionType *ScaleAudioDynamic = FunctionType::get(
            AudioType, {AudioType, PointerType::getUnqual(FunctionType::get(Type::getDoubleTy(TheContext), {Type::getDoubleTy(TheContext)}, false))}, false);
        scaleAudioFunctionDynamic = Function::Create(ScaleAudioDynamic, Function::ExternalLinkage, "scale_audio_dynamic", TheModule.get());

        // struct Audio generate_audio_static(short(*wav_function)(double), double frequency, double length_seconds);
        // generates one audio file over the other. The resulting file's length will be the max length of the two files
        FunctionType *WavFuncType = FunctionType::get(Type::getInt16Ty(TheContext), {Type::getDoubleTy(TheContext)}, false);
        FunctionType *GenerateAudioTypeStatic =
            FunctionType::get(AudioType, {WavFuncType->getPointerTo(), Type::getDoubleTy(TheContext), Type::getDoubleTy(TheContext)}, false);
        generateAudioFunctionStatic = Function::Create(GenerateAudioTypeStatic, Function::ExternalLinkage, "generate_audio_static", TheModule.get());

        // struct Audio generate_audio_static(short(*wav_function)(double), double frequency, double length_seconds);
        // generates one audio file over the other. The resulting file's length will be the max length of the two files
        FunctionType *TimeDepFunctionType = FunctionType::get(Type::getDoubleTy(TheContext), {Type::getDoubleTy(TheContext)}, false);
        FunctionType *GenerateAudioTypeDynamic =
            FunctionType::get(AudioType, {WavFuncType->getPointerTo(), TimeDepFunctionType->getPointerTo(), Type::getDoubleTy(TheContext)}, false);
        generateAudioFunctionStatic = Function::Create(GenerateAudioTypeStatic, Function::ExternalLinkage, "generate_audio_dynamic", TheModule.get());
    }
}

// Declare the printf function
void declarePrintFunction() {
    if (!printFunction) {
        std::vector<Type *> PrintArgs;
        PrintArgs.push_back(Type::getInt8Ty(TheContext)->getPointerTo()); // char*
        FunctionType *PrintType = FunctionType::get(Type::getInt32Ty(TheContext), PrintArgs, true);
        printFunction = Function::Create(PrintType, Function::ExternalLinkage, "printf", TheModule.get());
        // cerr << "Declared Print function yay!!" << endl;
    }
}

// Main code generation function
Value *codegen(Stype *node) {
    switch (node->node_type) {
    case NODE_ROOT: {
        cerr << "Root node" << endl;
        auto val = codegen(node->children[0]);
        if (node->children[1]->children.size()) {
            codegen(node->children[1]->children[1]);
            // main block processed last
            val = codegen(node->children[1]);
        }
        return val;
    }
    case NODE_MAIN_BLOCK: {
        cerr << "Main block node" << endl;
        // before traversing the statements node, do stuff to make the scope be
        // main block and stuff
        // symbol_table[current_scope_table].push_back(unordered_map<string, StEntry>());
        // current_scope = 1;
        return codegen(node->children[0]);
        // current_scope = 0;
    }
    // Literals and Identifiers
    case NODE_INT_LITERAL: {
        cerr << "NODE_INT_LITERAL" << endl;
        int64_t val = std::stoll(node->text);
        return ConstantInt::get(TheContext, APInt(64, val));
    }
    case NODE_FLOAT_LITERAL: {
        cerr << "NODE_FLOAT_LITERAL" << endl;
        double val = 0;
        if (node->text.size() >= 2) {
            if (node->text[node->text.size() - 2] == 'h' || node->text[node->text.size() - 2] == 'H') {
                node->text.pop_back();
                node->text.pop_back();
                val = std::stod(node->text);
                val /= 440;
                if (val <= 0)
                    // TODO: this is an arbitrary largee negative value, decide on a proper value later
                    val = -1e30;
                else
                    val = 12 * log2(val);
            } else if (node->text[node->text.size() - 2] == 'm' || node->text[node->text.size() - 2] == 'M') {
                node->text.pop_back();
                node->text.pop_back();
                val = std::stod(node->text);
                val /= 1000;
            } else if (node->text[node->text.size() - 1] == 's' || node->text[node->text.size() - 1] == 'S') {
                node->text.pop_back();
                val = std::stod(node->text);
            } else {
                val = std::stod(node->text);
            }
        } else
            val = std::stod(node->text);
        return ConstantFP::get(TheContext, APFloat(val));
    }
    case NODE_BOOL_LITERAL: {
        cerr << "NODE_BOOL_LITERAL" << endl;
        bool val = (node->text == "true");
        return ConstantInt::get(TheContext, APInt(1, val));
    }
    case NODE_STRING_LITERAL: {
        cerr << "NODE_STRING_LITERAL" << endl;
        // remove the quotes around the string
        node->text = node->text.substr(1, node->text.size() - 2);
        return Builder.CreateGlobalStringPtr(node->text);
    }
    case NODE_IDENTIFIER: {
        cerr << "NODE_IDENTIFIER" << endl;
        AllocaInst *var = findVariable(node->text);
        // cerr << "found?" << endl;
        return Builder.CreateLoad(var->getAllocatedType(), var, node->text.c_str());
    }
    case NODE_ASSIGNABLE_VALUE: {
        cerr << "NODE_ASSIGNABLE_VALUE" << endl;
        Value *IdValue = codegen(node->children[0]);
        // TODO: Need to handle vectors and slices here!!
        if (IdValue->getType() == AudioType) {
            for (int i = 1; i < node->children.size(); i++) {
                cerr << "hello?-------------------------------------------------------" << endl;
                Value *start_time = codegen(node->children[i]->children[0]);
                Value *end_time = codegen(node->children[i]->children[1]);
                start_time = createCast(start_time, Type::getDoubleTy(TheContext));
                end_time = createCast(end_time, Type::getDoubleTy(TheContext));
                auto SliceFunction = getFunction("slice_audio");
                auto NewIdVal = Builder.CreateCall(SliceFunction, {IdValue, start_time, end_time}, "sliceaudio");
                Function *AudioFreeFunc = TheModule->getFunction("free_audio");
                if (i > 1){
                    Builder.CreateCall(AudioFreeFunc, {IdValue});
                }
                IdValue = NewIdVal;
            }
        }
        return IdValue;

        // return codegen(node->children[0]);
    }
    // Expressions
    case NODE_PLUS_EXPR: {
        cerr << "NODE_PLUS_EXPR" << endl;
        Value *LHS = codegen(node->children[0]);
        Value *RHS = codegen(node->children[1]);
        Type *LHSType = LHS->getType();
        Type *RHSType = RHS->getType();

        if (LHSType->isFloatingPointTy() || RHSType->isFloatingPointTy()) {
            if (LHSType->isIntegerTy())
                LHS = Builder.CreateSIToFP(LHS, Type::getDoubleTy(TheContext), "int_to_float");
            if (RHSType->isIntegerTy())
                RHS = Builder.CreateSIToFP(RHS, Type::getDoubleTy(TheContext), "int_to_float");
            return Builder.CreateFAdd(LHS, RHS, "addtmp");
        }

        if (LHSType->isIntegerTy() && RHSType->isIntegerTy()) {
            return Builder.CreateAdd(LHS, RHS, "addtmp");
        }
        // String addition
        if (LHSType->isPointerTy() && LHSType->getContainedType(0)->isIntegerTy(8)) {
            Function *StrCatFunc = TheModule->getFunction("string_concat");
            if (!StrCatFunc) {
                FunctionType *StrCatType =
                    FunctionType::get(Type::getInt8Ty(TheContext)->getPointerTo(),                                                // Return type: char*
                                      {Type::getInt8Ty(TheContext)->getPointerTo(), Type::getInt8Ty(TheContext)->getPointerTo()}, // Arguments: char*, char*
                                      false);
                StrCatFunc = Function::Create(StrCatType, Function::ExternalLinkage, "string_concat", TheModule.get());
            }
            return Builder.CreateCall(StrCatFunc, {LHS, RHS}, "concat");
        }

        // Audio addition
        if (LHSType->isStructTy() && RHSType->isStructTy() && LHSType->getStructName() == "struct.Audio" && RHSType->getStructName() == "struct.Audio") {
            Function *AudioAddFunc = TheModule->getFunction("concat_audio");
            Function *AudioFreeFunc = TheModule->getFunction("free_audio");
            if (!AudioAddFunc) {
                declareAudioFunctions();
                AudioAddFunc = TheModule->getFunction("concat_audio");
            }
            auto Val = Builder.CreateCall(AudioAddFunc, {LHS, RHS}, "audioadd");
            if (node->children[0]->can_free) {
                Builder.CreateCall(AudioFreeFunc, {LHS});
            }
            if (node->children[1]->can_free) {
                Builder.CreateCall(AudioFreeFunc, {RHS});
            }
            return Val;
        }

        // handling addition of two function
        if (LHSType->isPointerTy() && LHSType->getContainedType(0)->isFunctionTy()) {
            // TODO
        }
    }
    case NODE_MINUS_EXPR: {
        cerr << "NODE_MINUS_EXPR" << endl;
        Value *LHS = codegen(node->children[0]);
        Value *RHS = codegen(node->children[1]);
        Type *LHSType = LHS->getType();
        Type *RHSType = RHS->getType();
        if (LHSType->isFloatingPointTy() || RHSType->isFloatingPointTy()) {
            LHS = createCast(LHS, Type::getDoubleTy(TheContext));
            RHS = createCast(RHS, Type::getDoubleTy(TheContext));
            return Builder.CreateFSub(LHS, RHS, "addtmp");
        }

        if (LHSType->isIntegerTy() && RHSType->isIntegerTy()) {
            LHS = createCast(LHS, Type::getInt64Ty(TheContext));
            RHS = createCast(RHS, Type::getInt64Ty(TheContext));
            return Builder.CreateSub(LHS, RHS, "addtmp");
        }
    }
    case NODE_MULT_EXPR: {
        cerr << "NODE_MULT_EXPR" << endl;
        Value *LHS = codegen(node->children[0]);
        Value *RHS = codegen(node->children[1]);
        Type *LHSType = LHS->getType();
        Type *RHSType = RHS->getType();

        bool LHSisAudio = LHSType->isStructTy() && LHSType->getStructName() == "struct.Audio";
        bool RHSisAudio = RHSType->isStructTy() && RHSType->getStructName() == "struct.Audio";
        bool LHSisScalar = LHSType->isFloatingPointTy() || LHSType->isIntegerTy();
        bool RHSisScalar = RHSType->isFloatingPointTy() || RHSType->isIntegerTy();

        // Audio Scaling
        if ((LHSisAudio && RHSisScalar) || (RHSisAudio && LHSisScalar)) {
            if (RHSisAudio) {
                Value *temp = LHS;
                LHS = RHS;
                RHS = temp;
                node->children[0]->can_free = node->children[1]->can_free;
            }
            LHSType = AudioType;
            RHS = createCast(RHS, Type::getDoubleTy(TheContext));
            RHSType = RHS->getType();
            Function *ScaleAudioFunc = TheModule->getFunction("scale_audio_static");
            auto Val = Builder.CreateCall(ScaleAudioFunc, {LHS, RHS}, "scaleaudio");
            Function *AudioFreeFunc = TheModule->getFunction("free_audio");
            if (node->children[0]->can_free) {
                Builder.CreateCall(AudioFreeFunc, {LHS});
            }
            return Val;
        } else if (LHSType->isFloatingPointTy() || RHSType->isFloatingPointTy()) {
            LHS = createCast(LHS, Type::getDoubleTy(TheContext));
            RHS = createCast(RHS, Type::getDoubleTy(TheContext));
            return Builder.CreateFMul(LHS, RHS, "multmp");
        } else

            if (LHSType->isIntegerTy() && RHSType->isIntegerTy()) {
            LHS = createCast(LHS, Type::getInt64Ty(TheContext));
            RHS = createCast(RHS, Type::getInt64Ty(TheContext));
            return Builder.CreateMul(LHS, RHS, "addtmp");
        } else if (LHSisAudio || RHSisAudio) {
            Type *arg_function_type;
            if (RHSisAudio) {
                Value *temp = LHS;
                LHS = RHS;
                RHS = temp;
                arg_function_type = getLLVMType(node->children[0]->data_type);
                node->children[0]->can_free = node->children[1]->can_free;
            }
            arg_function_type = getLLVMType(node->children[1]->data_type);
            LHSType = AudioType;

            Function *ScaleAudioFuncDyn = TheModule->getFunction("scale_audio_dynamic");
            // return Builder.CreateCall(ScaleAudioFuncDyn, {LHS, RHS}, "scaleaudiodyn");
            auto Val = Builder.CreateCall(ScaleAudioFuncDyn, {LHS, RHS}, "scaleaudiodyn");
            Function *AudioFreeFunc = TheModule->getFunction("free_audio");
            if (node->children[0]->can_free) {
                Builder.CreateCall(AudioFreeFunc, {LHS});
            }
            return Val;
            // return Builder.CreateCall(getFunction("_func_0")->getFunctionType(), temp1, ArgsV);
            // return Builder.CreateCall(functype, temp1, ArgsV);
        }
    }
    case NODE_DIVIDE_EXPR: {
        cerr << "NODE_DIVIDE_EXPR" << endl;
        Value *LHS = codegen(node->children[0]);
        Value *RHS = codegen(node->children[1]);
        if (LHS->getType()->isFloatingPointTy() || RHS->getType()->isFloatingPointTy()) {
            LHS = createCast(LHS, Type::getDoubleTy(TheContext));
            RHS = createCast(RHS, Type::getDoubleTy(TheContext));
            return Builder.CreateFDiv(LHS, RHS, "divtmp");
        } else {
            LHS = createCast(LHS, Type::getInt64Ty(TheContext));
            RHS = createCast(RHS, Type::getInt64Ty(TheContext));
            return Builder.CreateSDiv(LHS, RHS, "divtmp");
        }
    }
    case NODE_MOD_EXPR: {
        cerr << "NODE_MOD_EXPR" << endl;
        Value *LHS = codegen(node->children[0]);
        Value *RHS = codegen(node->children[1]);
        if (LHS->getType()->isFloatingPointTy() || RHS->getType()->isFloatingPointTy()) {
            LHS = createCast(LHS, Type::getDoubleTy(TheContext));
            RHS = createCast(RHS, Type::getDoubleTy(TheContext));
            Value *Div = Builder.CreateFDiv(LHS, RHS, "div");
            Value *TruncDiv = Builder.CreateIntrinsic(Intrinsic::trunc, {LHS->getType()}, {Div}, nullptr, "trunc_div");
            Value *Mul = Builder.CreateFMul(TruncDiv, RHS, "mul");
            return Builder.CreateFSub(LHS, Mul, "mod_float");
        } else {
            LHS = createCast(LHS, Type::getInt64Ty(TheContext));
            RHS = createCast(RHS, Type::getInt64Ty(TheContext));
            return Builder.CreateSRem(LHS, RHS, "mod");
        }
    }
    case NODE_SUPERPOSITION_EXPR: {
        cerr << "NODE_SUPERPOSITION_EXPR" << endl;
        // Implement power operation using llvm.pow function
        Value *LHS = codegen(node->children[0]);
        Value *RHS = codegen(node->children[1]);
        Type *LHSType = LHS->getType();
        Type *RHSType = RHS->getType();

        Function *AudioSuperimpose = TheModule->getFunction("superimpose_audio");
        if (!AudioSuperimpose) {
            declareAudioFunctions();
            AudioSuperimpose = TheModule->getFunction("superimpose_audio");
        }
        auto Val = Builder.CreateCall(AudioSuperimpose, {LHS, RHS}, "superimpose");
        Function *AudioFreeFunc = TheModule->getFunction("free_audio");
        if (node->children[0]->can_free) {
            Builder.CreateCall(AudioFreeFunc, {LHS});
        }
        if (node->children[1]->can_free) {
            Builder.CreateCall(AudioFreeFunc, {RHS});
        }
        return Val;
    }
    case NODE_POWER_EXPR: {
        cerr << "NODE_POWER_EXPR" << endl;
        // Implement power operation using llvm.pow function
        Value *LHS = codegen(node->children[0]);
        Value *RHS = codegen(node->children[1]);
        Type *LHSType = LHS->getType();
        Type *RHSType = RHS->getType();

        bool LHSisString = LHSType->isPointerTy() && LHSType->getContainedType(0)->isIntegerTy(8);
        bool LHSisAudio = LHSType->isStructTy() && LHSType->getStructName() == "struct.Audio";
        bool RHSisScalar = RHSType->isFloatingPointTy() || RHSType->isIntegerTy();

        if (!(LHSisAudio || LHSisString)) {
            Function *PowFunc = Intrinsic::getDeclaration(TheModule.get(), Intrinsic::pow, {Type::getDoubleTy(TheContext)});
            LHS = createCast(LHS, Type::getDoubleTy(TheContext));
            RHS = createCast(RHS, Type::getDoubleTy(TheContext));
            return Builder.CreateCall(PowFunc, {LHS, RHS}, "powtmp");
        }

        RHS = createCast(RHS, Type::getDoubleTy(TheContext));
        RHSType = RHS->getType();
        // String Repeating
        if (LHSisString && RHSisScalar) {
            Function *StrRepeat = TheModule->getFunction("string_repeat");
            if (!StrRepeat) {
                FunctionType *StrRepeatType = FunctionType::get(Type::getInt8Ty(TheContext)->getPointerTo(),            // Return type: char*
                                                                {Type::getInt8Ty(TheContext)->getPointerTo(), RHSType}, // Arguments: char*, scalar
                                                                false);
                auto StrRepeat = Function::Create(StrRepeatType, Function::ExternalLinkage, "string_repeat", TheModule.get());
            }
            return Builder.CreateCall(StrRepeat, {LHS, RHS}, "stringrepeat");
        }

        // Audio Repeating
        if (LHSisAudio && RHSisScalar) {
            Function *AudioRepeat = TheModule->getFunction("repeat_audio");
            RHS = createCast(RHS, Type::getDoubleTy(TheContext));
            RHSType = RHS->getType();
            if (!AudioRepeat) {
                FunctionType *AudioRepeatType = FunctionType::get(AudioType, {LHSType, RHSType}, false);
                auto AudioRepeat = Function::Create(AudioRepeatType, Function::ExternalLinkage, "repeat_audio", TheModule.get());
            }
            auto Val = Builder.CreateCall(AudioRepeat, {LHS, RHS}, "audiorepeat");
            Function *AudioFreeFunc = TheModule->getFunction("free_audio");
            if (node->children[0]->can_free) {
                Builder.CreateCall(AudioFreeFunc, {LHS});
            }
            return Val;
        }
    }
    case NODE_EQUALS_EXPR: {
        cerr << "NODE_EQUALS_EXPR" << endl;
        Value *LHS = codegen(node->children[0]);
        Value *RHS = codegen(node->children[1]);
        auto LHS_TYPE = LHS->getType();
        auto RHS_TYPE = RHS->getType();
        if (LHS_TYPE->isIntegerTy() && RHS_TYPE->isIntegerTy()) {
            LHS = createCast(LHS, Type::getInt64Ty(TheContext));
            RHS = createCast(RHS, Type::getInt64Ty(TheContext));
            return Builder.CreateICmpEQ(LHS, RHS, "eqtmp");
        } else if (LHS_TYPE->isFloatingPointTy() || RHS_TYPE->isFloatingPointTy()) {
            LHS = createCast(LHS, Type::getDoubleTy(TheContext));
            RHS = createCast(RHS, Type::getDoubleTy(TheContext));
            return Builder.CreateFCmp(llvm::FCmpInst::FCMP_UEQ, LHS, RHS, "eqtmp");
        } else {
            // TODO: string equality check
        }
    }
    case NODE_NOT_EQUALS_EXPR: {
        cerr << "NODE_NOT_EQUALS_EXPR" << endl;
        Value *LHS = codegen(node->children[0]);
        Value *RHS = codegen(node->children[1]);
        auto LHS_TYPE = LHS->getType();
        auto RHS_TYPE = RHS->getType();
        if (LHS_TYPE->isIntegerTy() && RHS_TYPE->isIntegerTy()) {
            LHS = createCast(LHS, Type::getInt64Ty(TheContext));
            RHS = createCast(RHS, Type::getInt64Ty(TheContext));
            return Builder.CreateICmpNE(LHS, RHS, "eqtmp");
        } else if (LHS_TYPE->isFloatingPointTy() || RHS_TYPE->isFloatingPointTy()) {
            LHS = createCast(LHS, Type::getDoubleTy(TheContext));
            RHS = createCast(RHS, Type::getDoubleTy(TheContext));
            return Builder.CreateFCmp(llvm::FCmpInst::FCMP_UNE, LHS, RHS, "eqtmp");
        } else {
            // TODO: string non-equality check
        }
    }
    case NODE_LEQ_EXPR: {
        cerr << "NODE_LEQ_EXPR" << endl;
        Value *LHS = codegen(node->children[0]);
        Value *RHS = codegen(node->children[1]);
        auto LHS_TYPE = LHS->getType();
        auto RHS_TYPE = RHS->getType();
        if (LHS_TYPE->isIntegerTy() && RHS_TYPE->isIntegerTy()) {
            LHS = createCast(LHS, Type::getInt64Ty(TheContext));
            RHS = createCast(RHS, Type::getInt64Ty(TheContext));
            return Builder.CreateICmpSLE(LHS, RHS, "eqtmp");
        } else if (LHS_TYPE->isFloatingPointTy() || RHS_TYPE->isFloatingPointTy()) {
            LHS = createCast(LHS, Type::getDoubleTy(TheContext));
            RHS = createCast(RHS, Type::getDoubleTy(TheContext));
            return Builder.CreateFCmp(llvm::FCmpInst::FCMP_ULE, LHS, RHS, "eqtmp");
        }
    }
    case NODE_GEQ_EXPR: {
        cerr << "NODE_GEQ_EXPR" << endl;
        Value *LHS = codegen(node->children[0]);
        Value *RHS = codegen(node->children[1]);
        auto LHS_TYPE = LHS->getType();
        auto RHS_TYPE = RHS->getType();
        if (LHS_TYPE->isIntegerTy() && RHS_TYPE->isIntegerTy()) {
            LHS = createCast(LHS, Type::getInt64Ty(TheContext));
            RHS = createCast(RHS, Type::getInt64Ty(TheContext));
            return Builder.CreateICmpSGE(LHS, RHS, "eqtmp");
        } else if (LHS_TYPE->isFloatingPointTy() || RHS_TYPE->isFloatingPointTy()) {
            LHS = createCast(LHS, Type::getDoubleTy(TheContext));
            RHS = createCast(RHS, Type::getDoubleTy(TheContext));
            return Builder.CreateFCmp(llvm::FCmpInst::FCMP_UGE, LHS, RHS, "eqtmp");
        }
    }
    case NODE_LE_EXPR: {
        cerr << "NODE_LE_EXPR" << endl;
        Value *LHS = codegen(node->children[0]);
        Value *RHS = codegen(node->children[1]);
        auto LHS_TYPE = LHS->getType();
        auto RHS_TYPE = RHS->getType();
        if (LHS_TYPE->isIntegerTy() && RHS_TYPE->isIntegerTy()) {
            LHS = createCast(LHS, Type::getInt64Ty(TheContext));
            RHS = createCast(RHS, Type::getInt64Ty(TheContext));
            return Builder.CreateICmpSLT(LHS, RHS, "eqtmp");
        } else if (LHS_TYPE->isFloatingPointTy() || RHS_TYPE->isFloatingPointTy()) {
            LHS = createCast(LHS, Type::getDoubleTy(TheContext));
            RHS = createCast(RHS, Type::getDoubleTy(TheContext));
            return Builder.CreateFCmp(llvm::FCmpInst::FCMP_ULT, LHS, RHS, "eqtmp");
        }
    }
    case NODE_GE_EXPR: {
        cerr << "NODE_GE_EXPR" << endl;
        Value *LHS = codegen(node->children[0]);
        Value *RHS = codegen(node->children[1]);
        auto LHS_TYPE = LHS->getType();
        auto RHS_TYPE = RHS->getType();
        if (LHS_TYPE->isIntegerTy() && RHS_TYPE->isIntegerTy()) {
            LHS = createCast(LHS, Type::getInt64Ty(TheContext));
            RHS = createCast(RHS, Type::getInt64Ty(TheContext));
            return Builder.CreateICmpSGT(LHS, RHS, "eqtmp");
        } else if (LHS_TYPE->isFloatingPointTy() || RHS_TYPE->isFloatingPointTy()) {
            LHS = createCast(LHS, Type::getDoubleTy(TheContext));
            RHS = createCast(RHS, Type::getDoubleTy(TheContext));
            return Builder.CreateFCmp(llvm::FCmpInst::FCMP_UGT, LHS, RHS, "eqtmp");
        }
    }
    case NODE_LOGICAL_AND_EXPR: {
        cerr << "NODE_LOGICAL_AND_EXPR" << endl;
        Value *LHS = codegen(node->children[0]);
        Value *RHS = codegen(node->children[1]);
        LHS = createCast(LHS, Type::getInt1Ty(TheContext));
        RHS = createCast(RHS, Type::getInt1Ty(TheContext));
        return Builder.CreateAnd(LHS, RHS, "andtmp");
    }
    case NODE_LOGICAL_OR_EXPR: {
        cerr << "NODE_LOGICAL_OR_EXPR" << endl;
        Value *LHS = codegen(node->children[0]);
        Value *RHS = codegen(node->children[1]);
        LHS = createCast(LHS, Type::getInt1Ty(TheContext));
        RHS = createCast(RHS, Type::getInt1Ty(TheContext));
        return Builder.CreateOr(LHS, RHS, "ortmp");
    }
    case NODE_UNARY_INVERSE_EXPR: {
        cerr << "NODE_UNARY_INVERSE_EXPR" << endl;
        Value *Operand = codegen(node->children[0]);
        Operand = createCast(Operand, Type::getInt1Ty(TheContext));
        return Builder.CreateNot(Operand, "invtmp");
    }
    case NODE_UNARY_LOGICAL_NOT_EXPR: {
        cerr << "NODE_UNARY_LOGICAL_NOT_EXPR" << endl;
        Value *Operand = codegen(node->children[0]);
        Operand = createCast(Operand, Type::getInt1Ty(TheContext));
        return Builder.CreateNot(Operand, "nottmp");
    }
    case NODE_UNARY_PLUS_EXPR: {
        cerr << "NODE_UNARY_PLUS_EXPR" << endl;
        return codegen(node->children[0]);
        // Value *Operand = codegen(node->children[0]);
        // return Builder.CreateAdd(Operand, ConstantInt::get(Operand->getType(), 0), "plustmp");
    }
    case NODE_UNARY_MINUS_EXPR: {
        cerr << "NODE_UNARY_MINUS_EXPR" << endl;
        Value *Operand = codegen(node->children[0]);
        if (Operand->getType()->isIntegerTy()) {
            return Builder.CreateSub(ConstantInt::get(Operand->getType(), 0), Operand, "negtmp");
        } else {
            return Builder.CreateFSub(ConstantFP::get(Operand->getType(), 0), Operand, "negtmp");
        }
    }
    // case NODE_NEGATE_EXPR: {cerr << "NODE_NEGATE_EXPR" << endl;
    //     Value* Operand = codegen(node->children[0]);
    //     if (Operand->getType()->isFloatingPointTy()) {
    //         return Builder.CreateFNeg(Operand, "negtmp");
    //     } else {
    //         return Builder.CreateNeg(Operand, "negtmp");
    //     }
    // }
    case NODE_FUNCTION_CALL: {
        cerr << "NODE_FUNCTION_CALL" << endl;
        string FuncName = node->children[0]->text;
        // Function *CalleeF = getFunction(FuncName);
        auto CalleeF = findVariable(FuncName);
        // auto functype = CalleeF->getType();
        FunctionType *functype = (FunctionType *)getLLVMType(node->children[0]->data_type);

        PointerType *FuncPointerType = PointerType::getUnqual(functype);

        // Type *tp = ConstantExpr::getBitCast(getFunction("_func_0"), FuncPointerType)->getType();
        // Type *tp = ;

        // Value *FuncPtr = Builder.CreateLoad(tp, CalleeF);
        vector<Value *> ArgsV;
        Stype *FuncArgsNode = node->children[1];
        int ind = 0;
        for (Stype *arg : FuncArgsNode->children[0]->children) {
            Value *ArgVal = codegen(arg);
            // TODO: Have to typecast to the required type if they are of different types
            ArgVal = createCast(ArgVal, getLLVMType(node->children[0]->data_type->parameters[ind]));
            ArgsV.push_back(ArgVal);
            ind++;
        }

        Value *p = Builder.CreateAlloca(CalleeF->getType(), nullptr, "p");

        Builder.CreateStore(CalleeF, p, false);
        Value *temp = Builder.CreateLoad(FuncPointerType, p);
        Value *temp1 = Builder.CreateLoad(FuncPointerType, temp);

        // return Builder.CreateCall(getFunction("_func_0")->getFunctionType(), temp1, ArgsV);
        return Builder.CreateCall(functype, temp1, ArgsV);

        // LoadInst *loadFPtr = Builder.CreateLoad(tp, CalleeF, "loadedFuncPtr");
        //
        // return Builder.CreateCall(cast<Function>(loadFPtr), ArgsV, "calltmp");
    }

    // case NODE_CAST_EXPR: {cerr << "NODE_CAST_EXPR" << endl;
    //     // Implement type casting
    //     Value* ExprVal = codegen(node->children[0]);
    //     DataType* TargetType = node->data_type;
    //     Type* DestType = getLLVMType(TargetType);
    //
    //     if (ExprVal->getType()->isIntegerTy() && DestType->isFloatingPointTy()) {
    //         return Builder.CreateSIToFP(ExprVal, DestType, "casttmp");
    //     } else if (ExprVal->getType()->isFloatingPointTy() && DestType->isIntegerTy()) {
    //         return Builder.CreateFPToSI(ExprVal, DestType, "casttmp");
    //     } else {
    //         return Builder.CreateBitCast(ExprVal, DestType, "casttmp");
    //     }
    // }
    // case NODE_VECTOR_ACCESS: {cerr << "NODE_VECTOR_ACCESS" << endl;
    //     // Access an element from a vector
    //     Value* VecVal = codegen(node->children[0]);
    //     Value* IndexVal = codegen(node->children[1]);
    //     return Builder.CreateExtractElement(VecVal, IndexVal, "vectorelem");
    // }
    // Statements
    case NODE_NORMAL_ASSIGNMENT_STATEMENT:
    case NODE_PLUS_EQUALS_ASSIGNMENT_STATEMENT:
    case NODE_MINUS_EQUALS_ASSIGNMENT_STATEMENT:
    case NODE_MULT_EQUALS_ASSIGNMENT_STATEMENT:
    case NODE_DIVIDE_EQUALS_ASSIGNMENT_STATEMENT:
    case NODE_POWER_EQUALS_ASSIGNMENT_STATEMENT:
    case NODE_MOD_EQUALS_ASSIGNMENT_STATEMENT: {
        cerr << "NODE_MOD_EQUALS_ASSIGNMENT_STATEMENT" << endl;
        // Handle compound assignments
        Stype *LHSNode = node->children[0];
        Stype *RHSNode = node->children[1];
        Value *RHSVal = codegen(RHSNode);
        AllocaInst *Var = findVariable(LHSNode->children[0]->text);
        Value *LHSVal = Builder.CreateLoad(Var->getAllocatedType(), Var);

        Value *Result;
        switch (node->node_type) {
        case NODE_PLUS_EQUALS_ASSIGNMENT_STATEMENT:
            Result = Builder.CreateAdd(LHSVal, RHSVal, "addassigntmp");
            break;
        case NODE_MINUS_EQUALS_ASSIGNMENT_STATEMENT:
            Result = Builder.CreateSub(LHSVal, RHSVal, "subassigntmp");
            break;
        case NODE_MULT_EQUALS_ASSIGNMENT_STATEMENT:
            Result = Builder.CreateMul(LHSVal, RHSVal, "mulassigntmp");
            break;
        case NODE_DIVIDE_EQUALS_ASSIGNMENT_STATEMENT:
            Result = Builder.CreateSDiv(LHSVal, RHSVal, "divassigntmp");
            break;
        case NODE_POWER_EQUALS_ASSIGNMENT_STATEMENT: {
            // Implement power assignment
            Function *PowFunc = Intrinsic::getDeclaration(TheModule.get(), Intrinsic::pow, {Type::getDoubleTy(TheContext)});
            LHSVal = Builder.CreateSIToFP(LHSVal, Type::getDoubleTy(TheContext));
            RHSVal = Builder.CreateSIToFP(RHSVal, Type::getDoubleTy(TheContext));
            Result = Builder.CreateCall(PowFunc, {LHSVal, RHSVal}, "powassigntmp");
            break;
        }
        case NODE_MOD_EQUALS_ASSIGNMENT_STATEMENT:
            Result = Builder.CreateSRem(LHSVal, RHSVal, "modassigntmp");
            break;
        default:
            Result = RHSVal;
            break;
        }

        Builder.CreateStore(Result, Var);
        return Result;
    }
    case NODE_DECLARATION_STATEMENT:
    case NODE_DECLARATION_STATEMENT_WITH_TYPE: {
        cerr << "NODE_DECLARATION_STATEMENT_WITH_TYPE" << endl;
        // Declaration: IDENTIFIER '<-' expr
        string VarName = node->children[0]->text;
        cerr << VarName << endl;
        Stype *ExprNode = node->children[1];

        Value *InitVal = codegen(ExprNode);

        // Get the function we are currently in.
        Function *TheFunction = Builder.GetInsertBlock()->getParent();
        cerr << TheFunction << endl;
        cerr << "Done with decl stmt" << endl;

        Type *VarType;
        if (node->node_type == NODE_DECLARATION_STATEMENT_WITH_TYPE) {
            DataType *DType = node->children[2]->data_type;
            VarType = getLLVMType(DType);
        } else {
            VarType = InitVal->getType();
        }

        // Create an alloca in the entry block.
        AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, VarName, VarType);

        // Store the initial value into the alloca.
        Builder.CreateStore(InitVal, Alloca);

        // Add the variable to the symbol table.
        NamedValues.back()[VarName] = Alloca;
        return InitVal;
    }
    case NODE_IF_STATEMENT: {
        cerr << "NODE_IF_STATEMENT" << endl;
        // IF expr '{' statements '}'
        Value *CondV = codegen(node->children[0]);

        // Convert condition to a bool by comparing non-equal to zero
        if (CondV->getType()->isIntegerTy())
            CondV = Builder.CreateICmpNE(CondV, ConstantInt::get(CondV->getType(), 0), "ifcond");
        else if (CondV->getType()->isFloatingPointTy())
            CondV = Builder.CreateFCmpONE(CondV, ConstantFP::get(TheContext, APFloat(0.0)), "ifcond");

        Function *TheFunction = Builder.GetInsertBlock()->getParent();

        // Create blocks for the then and else cases.
        BasicBlock *ThenBB = BasicBlock::Create(TheContext, "then", TheFunction);
        BasicBlock *OrBB = BasicBlock::Create(TheContext, "or", TheFunction);
        BasicBlock *ElseBB = BasicBlock::Create(TheContext, "else", TheFunction);
        BasicBlock *MergeBB = BasicBlock::Create(TheContext, "ifcont", TheFunction);

        Builder.CreateCondBr(CondV, ThenBB, OrBB);

        // Emit then block.
        Builder.SetInsertPoint(ThenBB);
        pushSymbolTable();
        codegen(node->children[1]); // Then statements
        popSymbolTable();
        Builder.CreateBr(MergeBB);

        Builder.SetInsertPoint(OrBB);

        vector<BasicBlock *> OrBBS;
        vector<BasicBlock *> OrStatementsBBS;

        for (int i = 0; i < node->children[2]->children.size(); i++) {
            BasicBlock *InnerOrBB = BasicBlock::Create(TheContext, "inner_or", TheFunction);
            BasicBlock *InnerOrStatementsBB = BasicBlock::Create(TheContext, "inner_or_statements", TheFunction);
            OrBBS.push_back(InnerOrBB);
            OrStatementsBBS.push_back(InnerOrStatementsBB);
        }
        OrBBS.push_back(ElseBB);

        Builder.CreateBr(OrBBS[0]);

        for (int i = 0; i < node->children[2]->children.size(); i++) {
            Builder.SetInsertPoint(OrBBS[i]);
            Value *OrCondV = codegen(node->children[2]->children[i]->children[0]);
            if (OrCondV->getType()->isIntegerTy())
                OrCondV = Builder.CreateICmpNE(OrCondV, ConstantInt::get(OrCondV->getType(), 0), "ifcond");
            else if (CondV->getType()->isFloatingPointTy())
                OrCondV = Builder.CreateFCmpONE(OrCondV, ConstantFP::get(TheContext, APFloat(0.0)), "ifcond");

            Builder.CreateCondBr(OrCondV, OrStatementsBBS[i], OrBBS[i + 1]);
            Builder.SetInsertPoint(OrStatementsBBS[i]);
            pushSymbolTable();
            codegen(node->children[2]->children[i]->children[1]); // Then statements
            popSymbolTable();
            Builder.CreateBr(MergeBB);
        }

        // Emit else block.
        // TheFunction->getBasicBlocks().push_back(ElseBB);
        Builder.SetInsertPoint(ElseBB);
        if (node->children[3]->children.size()) {
            pushSymbolTable();
            codegen(node->children[3]->children[0]); // Else statements
            popSymbolTable();
        }
        Builder.CreateBr(MergeBB);

        // Emit merge block.
        // TheFunction->getBasicBlocks().push_back(MergeBB);
        Builder.SetInsertPoint(MergeBB);

        return Constant::getNullValue(Type::getInt32Ty(TheContext));
    }
    case NODE_LOOP_GENERAL_STATEMENT: {
        cerr << "NODE_LOOP_GENERAL_STATEMENT" << endl;
        // loop_statement: LOOP over id(0) expr(1) to expr(2) @ expr(3) '{' statements(4) '}'
        Function *TheFunction = Builder.GetInsertBlock()->getParent();

        BasicBlock *LoopBB = BasicBlock::Create(TheContext, "loop", TheFunction);
        BasicBlock *AfterLoopBB = BasicBlock::Create(TheContext, "afterloop", TheFunction);
        BasicBlock *CondBB = BasicBlock::Create(TheContext, "condition", TheFunction);

        Value *InitValue = codegen(node->children[1]);
        Value *FinValue = codegen(node->children[2]);
        Value *StepValue = codegen(node->children[3]);

        // cerr << node->children[0]->data_type->is_primitive << ' ' << node->children[0]->data_type->is_vector << ' '
        //      << (node->children[0]->data_type->basic_data_type == LONG) << endl;
        Type *CounterType = getLLVMType(node->children[0]->data_type);
        AllocaInst *LoopCounter = CreateEntryBlockAlloca(TheFunction, node->children[0]->text, CounterType);
        pushSymbolTable();
        NamedValues.back()[node->children[0]->text] = LoopCounter;
        InitValue = createCast(InitValue, CounterType);
        FinValue = createCast(FinValue, CounterType);
        StepValue = createCast(StepValue, CounterType);
        // Value *LoopCounter = Builder.CreateAlloca(InitValue->getType(), nullptr, "loopcounter");
        Builder.CreateStore(InitValue, LoopCounter);

        Builder.CreateBr(CondBB);
        Builder.SetInsertPoint(CondBB);

        Value *CurrentCounter = Builder.CreateLoad(InitValue->getType(), LoopCounter, "currentcounter");

        Value *CondV;
        if (CounterType->isDoubleTy())
            CondV = Builder.CreateFCmpULT(CurrentCounter, FinValue, "loopcond");
        else if (CounterType->isIntegerTy())
            CondV = Builder.CreateICmpSLT(CurrentCounter, FinValue, "loopcond");
        else {
            cerr << "Ayo you got the wrong type mate" << endl;
            cerr << CounterType->isIntegerTy() << endl;
        }

        Builder.CreateCondBr(CondV, LoopBB, AfterLoopBB);

        Builder.SetInsertPoint(LoopBB);

        codegen(node->children[4]); // Loop body statements

        Value *NewCounter;
        if (CounterType->isDoubleTy())
            NewCounter = Builder.CreateFAdd(CurrentCounter, StepValue, "increment");
        else
            NewCounter = Builder.CreateAdd(CurrentCounter, StepValue, "increment");
        Builder.CreateStore(NewCounter, LoopCounter);

        popSymbolTable();

        Builder.CreateBr(CondBB);

        Builder.SetInsertPoint(AfterLoopBB);

        return Constant::getNullValue(Type::getInt32Ty(TheContext));
    }
    case NODE_LOOP_REPEAT_STATEMENT: {
        cerr << "NODE_LOOP_REPEAT_STATEMENT" << endl;
        // loop_statement: LOOP expr '{' statements '}'
        Function *TheFunction = Builder.GetInsertBlock()->getParent();

        BasicBlock *LoopBB = BasicBlock::Create(TheContext, "loop", TheFunction);
        BasicBlock *AfterLoopBB = BasicBlock::Create(TheContext, "afterloop", TheFunction);
        BasicBlock *CondBB = BasicBlock::Create(TheContext, "condition", TheFunction);

        Value *NumIters = codegen(node->children[0]);

        Value *LoopCounter = Builder.CreateAlloca(NumIters->getType(), nullptr, "loopcounter");
        Builder.CreateStore(NumIters, LoopCounter);

        Builder.CreateBr(CondBB);
        Builder.SetInsertPoint(CondBB);

        Value *CurrentCounter = Builder.CreateLoad(NumIters->getType(), LoopCounter, "currentcounter");

        Value *CondV = Builder.CreateICmpSGT(CurrentCounter, ConstantInt::get(NumIters->getType(), 0), "loopcond");

        Builder.CreateCondBr(CondV, LoopBB, AfterLoopBB);

        Builder.SetInsertPoint(LoopBB);

        Value *DecrementedCounter = Builder.CreateSub(CurrentCounter, ConstantInt::get(NumIters->getType(), 1), "decrement");
        Builder.CreateStore(DecrementedCounter, LoopCounter);

        pushSymbolTable();
        codegen(node->children[1]); // Loop body statements
        popSymbolTable();

        Builder.CreateBr(CondBB);

        Builder.SetInsertPoint(AfterLoopBB);

        return Constant::getNullValue(Type::getInt32Ty(TheContext));
    }
    case NODE_LOOP_UNTIL_STATEMENT: {
        cerr << "NODE_LOOP_UNTIL_STATEMENT" << endl;
        // loop_statement: LOOP expr '{' statements '}'
        Function *TheFunction = Builder.GetInsertBlock()->getParent();

        BasicBlock *LoopBB = BasicBlock::Create(TheContext, "loop", TheFunction);
        BasicBlock *AfterLoopBB = BasicBlock::Create(TheContext, "afterloop", TheFunction);
        BasicBlock *CondBB = BasicBlock::Create(TheContext, "condition", TheFunction);

        Builder.CreateBr(CondBB);
        Builder.SetInsertPoint(CondBB);

        Value *CondV = codegen(node->children[0]);
        CondV = createCast(CondV, Type::getInt1Ty(TheContext));

        Builder.CreateCondBr(CondV, AfterLoopBB, LoopBB);

        Builder.SetInsertPoint(LoopBB);

        pushSymbolTable();
        codegen(node->children[1]); // Loop body statements
        popSymbolTable();

        Builder.CreateBr(CondBB);

        Builder.SetInsertPoint(AfterLoopBB);

        return Constant::getNullValue(Type::getInt32Ty(TheContext));
    }
    case NODE_BREAK_STATEMENT: {
        cerr << "NODE_BREAK_STATEMENT" << endl;
        // Implement break statement
        // For simplicity, we can skip implementing break and continue
        return Constant::getNullValue(Type::getInt32Ty(TheContext));
    }
    case NODE_CONTINUE_STATEMENT: {
        cerr << "NODE_CONTINUE_STATEMENT" << endl;
        // Implement continue statement
        return Constant::getNullValue(Type::getInt32Ty(TheContext));
    }
    case NODE_RETURN_STATEMENT: {
        cerr << "NODE_RETURN_STATEMENT" << endl;
        Function *TheFunction = Builder.GetInsertBlock()->getParent();
        Type *ReturnType = TheFunction->getReturnType();
        if (node->children.size() > 0) {
            Value *RetVal = codegen(node->children[0]);
            RetVal = createCast(RetVal, ReturnType);
            Builder.CreateRet(RetVal);
        } else {
            Builder.CreateRetVoid();
        }
        // Terminate the current block
        BasicBlock *UnreachableBB = BasicBlock::Create(TheContext, "unreachable");
        Builder.SetInsertPoint(UnreachableBB);
        return nullptr;
    }
    case NODE_PRINT_STATEMENT: {
        cerr << "NODE_PRINT_STATEMENT" << endl;
        declarePrintFunction();
        Value *ExprVal = codegen(node->children[0]);

        if (ExprVal->getType()->isIntegerTy()) {
            Value *FormatStr = Builder.CreateGlobalStringPtr("%ld\n");
            Builder.CreateCall(printFunction, {FormatStr, ExprVal});
        } else if (ExprVal->getType()->isDoubleTy()) {
            Value *FormatStr = Builder.CreateGlobalStringPtr("%f\n");
            Builder.CreateCall(printFunction, {FormatStr, ExprVal});
        } else if (ExprVal->getType()->isPointerTy()) {
            Value *FormatStr = Builder.CreateGlobalStringPtr("%s\n");
            Builder.CreateCall(printFunction, {FormatStr, ExprVal});
        }
        return nullptr;
    }
    case NODE_LOAD_STATEMENT: {
        cerr << "NODE_LOAD_STATEMENT" << endl;
        declareAudioFunctions();
        Value *FilenameVal = codegen(node->children[0]);
        Value *AudioData = Builder.CreateCall(loadAudioFunction, {FilenameVal}, "loadaudio");
        return AudioData;
    }
    case NODE_PLAY_STATEMENT: {
        cerr << "NODE_PLAY_STATEMENT" << endl;
        declareAudioFunctions();
        Value *AudioVal = codegen(node->children[0]);
        Builder.CreateCall(playAudioFunction, {AudioVal});
        return nullptr;
    }
    case NODE_SAVE_STATEMENT: {
        cerr << "NODE_SAVE_STATEMENT" << endl;
        declareAudioFunctions();
        Value *AudioVal = codegen(node->children[0]);
        Value *FilenameVal = codegen(node->children[1]);
        Builder.CreateCall(saveAudioFunction, {AudioVal, FilenameVal});
        return nullptr;
    }
    case NODE_AUDIO_FUNCTION: {
        cerr << "NODE_AUDIO_FUNCTION" << endl;
        declareAudioFunctions();
        Value *WavFunction = codegen(node->children[0]);
        Value *Freq = codegen(node->children[1]);
        Value *TimeSeconds = codegen(node->children[2]);
        TimeSeconds = createCast(TimeSeconds, Type::getDoubleTy(TheContext));
        Type *FreqType = Freq->getType();
        if (FreqType->isIntegerTy() || FreqType->isDoubleTy()) {
            Freq = createCast(Freq, Type::getDoubleTy(TheContext));
            Function *genAudioS = TheModule->getFunction("generate_audio_static");
            return Builder.CreateCall(genAudioS, {WavFunction, Freq, TimeSeconds}, "gen_audio");
            // return Builder.CreateCall(generateAudioFunctionStatic, {WavFunction, Freq, TimeSeconds});
        } else {
            Function *genAudioD = TheModule->getFunction("generate_audio_dynamic");
            return Builder.CreateCall(genAudioD, {WavFunction, Freq, TimeSeconds}, "gen_audio");
            // return Builder.CreateCall(generateAudioFunctionDynamic, {WavFunction, Freq, TimeSeconds});
        }
        return nullptr;
    }
    case NODE_NORMAL_FUNCTION: {
        cerr << "NODE_NORMAL_FUNCTION" << endl;
        // Implement function definition
        // string FuncName = node->children[0]->text;
        Stype *ParamListNode = node->children[0];
        DataType *ReturnTypeData = node->children[1]->data_type;
        Stype *BodyNode = node->children[2];

        vector<Type *> ParamTypes;
        vector<string> ParamNames;
        for (Stype *paramNode : ParamListNode->children) {
            DataType *ParamDataType = paramNode->data_type;
            Type *ParamType = getLLVMType(ParamDataType);
            ParamTypes.push_back(ParamType);
            ParamNames.push_back(paramNode->children[0]->text);
        }

        Type *ReturnType = getLLVMType(ReturnTypeData);
        FunctionType *FT = FunctionType::get(ReturnType, ParamTypes, false);
        Function *TheFunction = Function::Create(FT, Function::ExternalLinkage, "_func_" + to_string(num_funcs), TheModule.get());

        num_funcs++;

        unsigned idx = 0;
        for (auto &Arg : TheFunction->args()) {
            Arg.setName(ParamNames[idx++]);
        }

        function_nodes.push_back({node, TheFunction});
        PointerType *FuncPointerType = PointerType::get(FT, 0);
        return ConstantExpr::getBitCast(TheFunction, FuncPointerType);
    }
    case NODE_RETURNABLE_STATEMENTS:
    case NODE_STATEMENTS: {
        cerr << "NODE_STATEMENTS" << endl;
        Value *LastValue = nullptr;
        for (Stype *stmt : node->children) {
            LastValue = codegen(stmt);
        }
        return LastValue;
    }
    default:
        cerr << "forgot to handle a node bruh: " << node->node_type << endl;
        return nullptr;
    }
}

// Function to get LLVM type from DataType
Type *getLLVMType(DataType *dtype) {
    if (!dtype)
        return nullptr;
    if (dtype->is_primitive) {
        if (dtype->is_vector) {
            Type *elemType = getLLVMType(dtype->vector_data_type);
            return ArrayType::get(elemType, dtype->vector_size);
        } else {
            switch (dtype->basic_data_type) {
            case INT:
                return Type::getInt16Ty(TheContext);
            case LONG:
                return Type::getInt64Ty(TheContext);
            case FLOAT:
                return Type::getDoubleTy(TheContext);
            case BOOL:
                return Type::getInt1Ty(TheContext);
            case STRING:
                return Type::getInt8Ty(TheContext)->getPointerTo();
            case AUDIO:
                return AudioType;
            default:
                return nullptr;
            }
        }
    } else {
        vector<Type *> ParamTypes;
        for (DataType *paramType : dtype->parameters) {
            Type *llvmParamType = getLLVMType(paramType);
            ParamTypes.push_back(llvmParamType);
        }
        Type *ReturnType = getLLVMType(dtype->return_type);
        if (!ReturnType)
            ReturnType = Type::getVoidTy(TheContext);
        return FunctionType::get(ReturnType, ParamTypes, false);
    }
}

Value *convertToString(Value *Val) {
    Type *SrcType = Val->getType();

    Function *TheFunction = Builder.GetInsertBlock()->getParent();
    AllocaInst *Buffer = Builder.CreateAlloca(Type::getInt8Ty(TheContext), ConstantInt::get(Type::getInt32Ty(TheContext), 64), "strBuffer");

    Value *FormatStr;
    if (SrcType->isIntegerTy()) {
        FormatStr = Builder.CreateGlobalStringPtr("%ld", "formatStr");
    } else if (SrcType->isFloatingPointTy()) {
        FormatStr = Builder.CreateGlobalStringPtr("%f", "formatStr");
    } else {
        llvm_unreachable("Unsupported type for string conversion");
    }

    Function *SprintfFunc = TheFunction->getParent()->getFunction("sprintf");
    if (!SprintfFunc) {
        // int sprintf(char *buffer, const char *format, ...)
        FunctionType *SprintfType =
            FunctionType::get(Type::getInt32Ty(TheContext), {Type::getInt8Ty(TheContext)->getPointerTo(), Type::getInt8Ty(TheContext)->getPointerTo()}, true);
        SprintfFunc = Function::Create(SprintfType, Function::ExternalLinkage, "sprintf", TheFunction->getParent());
    }

    Builder.CreateCall(SprintfFunc, {Buffer, FormatStr, Val});

    return Buffer;
}

Value *createCast(Value *Val, Type *DestType) {
    Type *SrcType = Val->getType();

    if (SrcType == DestType)
        return Val;

    if (DestType->isFloatingPointTy()) {
        // assuming srctype is not invalid
        if (SrcType->isIntegerTy(1)) {
            // bool to float
            return Builder.CreateUIToFP(Val, DestType, "cast");
        }
        // int/long to float
        return Builder.CreateSIToFP(Val, DestType, "cast");
    } else if (DestType->isIntegerTy()) {
        if (SrcType->isFloatingPointTy()) {
            if (DestType->isIntegerTy(1))
                return Builder.CreateFCmpUNE(Val, ConstantFP::get(SrcType, 0.0), "to_bool");
            else
                return Builder.CreateFPToSI(Val, DestType, "cast");
        } else if (SrcType->isIntegerTy()) {
            if (DestType->isIntegerTy(1)) {
                // int to bool
                return Builder.CreateICmpNE(Val, ConstantInt::get(SrcType, 0), "cast");
            } else if (SrcType->getIntegerBitWidth() < DestType->getIntegerBitWidth()) {
                // int to long (sign-extend)
                return Builder.CreateSExt(Val, DestType, "cast");
            } else {
                // long to int (truncate)
                return Builder.CreateTrunc(Val, DestType, "cast");
            }
        }
    } else if (DestType->isPointerTy()) {
        return convertToString(Val);
    }

    llvm_unreachable("Unsupported cast type combination");
}

// Function to create an alloca instruction in the entry block
AllocaInst *CreateEntryBlockAlloca(Function *TheFunction, const string &VarName, Type *varType) {
    IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
    return TmpB.CreateAlloca(varType, 0, VarName.c_str());
}

// Function to get a function from the module
Function *getFunction(const string &name) {
    if (Function *F = TheModule->getFunction(name))
        return F;
    return nullptr;
}

// Main codegen entry point
int codegen_main(Stype *root) {
    freopen("out.ll", "w", stdout);
    // Initialize the module
    TheModule = make_unique<Module>("MyModule", TheContext);

    // Define the audio struct type
    AudioType = StructType::create(TheContext, "struct.Audio");
    AudioType->setBody({Type::getInt64Ty(TheContext), PointerType::get(Type::getInt32Ty(TheContext), 0)});
    declareAudioFunctions();

    // Generate code for the root node
    cerr << "------------------------------------------starting codegen---------------------------------------------" << endl;
    Function *main_function = Function::Create(FunctionType::get(Type::getInt32Ty(TheContext), {}, false), Function::ExternalLinkage, "main", TheModule.get());
    BasicBlock *BB = BasicBlock::Create(TheContext, "entry", main_function);
    Builder.SetInsertPoint(BB);
    pushSymbolTable();
    codegen(root);
    popSymbolTable();
    Builder.CreateRet(ConstantInt::get(Type::getInt32Ty(TheContext), 0));

    while (function_nodes.size() != 0) {
        auto node_func = function_nodes.back();
        function_nodes.pop_back();
        auto node = node_func.first;
        auto TheFunction = node_func.second;

        BasicBlock *BB = BasicBlock::Create(TheContext, "func_entry", TheFunction);
        Builder.SetInsertPoint(BB);

        pushSymbolTable();

        int idx = 0;
        for (auto &Arg : TheFunction->args()) {
            AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, Arg.getName().str(), Arg.getType());
            Builder.CreateStore(&Arg, Alloca);
            NamedValues.back()[Arg.getName().str()] = Alloca;
            idx++;
        }

        codegen(node->children[2]);

        auto RetType = getLLVMType(node->children[1]->data_type);

        if (RetType == Type::getVoidTy(TheContext)) {
            Builder.CreateRetVoid();
        }

        popSymbolTable();

        Builder.CreateRet(Constant::getNullValue(RetType));
        verifyFunction(*TheFunction);
    }

    cerr << "---------------------------------finished traversing tree for codegen----------------------------------" << endl;

    // Validate the generated code
    verifyModule(*TheModule, &errs());

    // Print the generated code
    TheModule->print(outs(), nullptr);

    return 0;
}
