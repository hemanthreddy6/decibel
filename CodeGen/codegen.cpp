// Codegen/codegen.cpp

// #include "codegen.h"

// Include LLVM headers
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"
#include <iostream>

#define CODEGEN 1

#include "../Semantic/semantic_analyser.cpp"

using namespace llvm;
using namespace std;

// LLVM Global Variables
static LLVMContext TheContext;
static IRBuilder<> Builder(TheContext);
static unique_ptr<Module> TheModule;

// Symbol Table for code generation
static vector<unordered_map<string, AllocaInst *>> NamedValues;

// Function Declarations
Value *codegen(Stype *node);
Type *getLLVMType(DataType *dtype);
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
Function *printFunction = nullptr;

// Push a new symbol table onto the stack
void pushSymbolTable() { NamedValues.emplace_back(); }

// Pop the current symbol table off the stack
void popSymbolTable() { NamedValues.pop_back(); }

// Look up a variable in the symbol table
AllocaInst *findVariable(const string &name) {
    for (auto it = NamedValues.rbegin(); it != NamedValues.rend(); ++it) {
        auto varIter = it->find(name);
        if (varIter != it->end())
            return varIter->second;
    }
    return nullptr;
}

// Declare external audio functions
void declareAudioFunctions() {
    if (!loadAudioFunction) {
        // Define the AudioData* type (we'll use i8* for simplicity)
        Type *AudioDataPtrType = Type::getInt8Ty(TheContext)->getPointerTo();

        // Declare load_audio function
        FunctionType *LoadAudioType = FunctionType::get(AudioDataPtrType, {Type::getInt8Ty(TheContext)->getPointerTo()}, false);
        loadAudioFunction = Function::Create(LoadAudioType, Function::ExternalLinkage, "load_audio", TheModule.get());

        // Declare play_audio function
        FunctionType *PlayAudioType = FunctionType::get(Type::getVoidTy(TheContext), {AudioDataPtrType}, false);
        playAudioFunction = Function::Create(PlayAudioType, Function::ExternalLinkage, "play_audio", TheModule.get());

        // Declare save_audio function
        FunctionType *SaveAudioType = FunctionType::get(Type::getVoidTy(TheContext), {AudioDataPtrType, Type::getInt8Ty(TheContext)->getPointerTo()}, false);
        saveAudioFunction = Function::Create(SaveAudioType, Function::ExternalLinkage, "save_audio", TheModule.get());

        // Declare free_audio function
        FunctionType *FreeAudioType = FunctionType::get(Type::getVoidTy(TheContext), {AudioDataPtrType}, false);
        freeAudioFunction = Function::Create(FreeAudioType, Function::ExternalLinkage, "free_audio", TheModule.get());
    }
}

// Declare the printf function
void declarePrintFunction() {
    if (!printFunction) {
        std::vector<Type *> PrintArgs;
        PrintArgs.push_back(Type::getInt8Ty(TheContext)->getPointerTo()); // char*
        FunctionType *PrintType = FunctionType::get(Type::getInt32Ty(TheContext), PrintArgs, true);
        printFunction = Function::Create(PrintType, Function::ExternalLinkage, "printf", TheModule.get());
    }
}

// Main code generation function
Value *codegen(Stype *node) {
    switch (node->node_type) {
    case NODE_ROOT: {
        cout << "Root node" << endl;
        auto val = codegen(node->children[0]);
        if (node->children[1]->children.size()) {
            codegen(node->children[1]->children[1]);
            // main block processed last
            val = codegen(node->children[1]);
        }
        return val;
    }
    case NODE_MAIN_BLOCK: {
        cout << "Main block node" << endl;
        // before traversing the statements node, do stuff to make the scope be
        // main block and stuff
        // symbol_table[current_scope_table].push_back(unordered_map<string, StEntry>());
        // current_scope = 1;
        return codegen(node->children[0]);
        // current_scope = 0;
    }
    // Literals and Identifiers
    case NODE_INT_LITERAL: {
        cout << "NODE_INT_LITERAL" << endl;
        int64_t val = std::stoll(node->text);
        return ConstantInt::get(TheContext, APInt(64, val));
    }
    case NODE_FLOAT_LITERAL: {
        cout << "NODE_FLOAT_LITERAL" << endl;
        double val = std::stod(node->text);
        return ConstantFP::get(TheContext, APFloat(val));
    }
    case NODE_BOOL_LITERAL: {
        cout << "NODE_BOOL_LITERAL" << endl;
        bool val = (node->text == "true");
        return ConstantInt::get(TheContext, APInt(1, val));
    }
    case NODE_STRING_LITERAL: {
        cout << "NODE_STRING_LITERAL" << endl;
        return Builder.CreateGlobalStringPtr(node->text);
    }
    case NODE_IDENTIFIER: {
        cout << "NODE_IDENTIFIER" << endl;
        AllocaInst *var = findVariable(node->text);
        return Builder.CreateLoad(var->getAllocatedType(), var, node->text.c_str());
    }
    // Expressions
    case NODE_PLUS_EXPR: {
        cout << "NODE_PLUS_EXPR" << endl;
        Value *LHS = codegen(node->children[0]);
        Value *RHS = codegen(node->children[1]);
        if (LHS->getType()->isFloatingPointTy() || RHS->getType()->isFloatingPointTy()) {
            return Builder.CreateFAdd(LHS, RHS, "addtmp");
        } else {
            return Builder.CreateAdd(LHS, RHS, "addtmp");
        }
    }
    case NODE_MINUS_EXPR: {
        cout << "NODE_MINUS_EXPR" << endl;
        Value *LHS = codegen(node->children[0]);
        Value *RHS = codegen(node->children[1]);
        if (LHS->getType()->isFloatingPointTy() || RHS->getType()->isFloatingPointTy()) {
            return Builder.CreateFSub(LHS, RHS, "subtmp");
        } else {
            return Builder.CreateSub(LHS, RHS, "subtmp");
        }
    }
    case NODE_MULT_EXPR: {
        cout << "NODE_MULT_EXPR" << endl;
        Value *LHS = codegen(node->children[0]);
        Value *RHS = codegen(node->children[1]);
        if (LHS->getType()->isFloatingPointTy() || RHS->getType()->isFloatingPointTy()) {
            return Builder.CreateFMul(LHS, RHS, "multmp");
        } else {
            return Builder.CreateMul(LHS, RHS, "multmp");
        }
    }
    case NODE_DIVIDE_EXPR: {
        cout << "NODE_DIVIDE_EXPR" << endl;
        Value *LHS = codegen(node->children[0]);
        Value *RHS = codegen(node->children[1]);
        if (LHS->getType()->isFloatingPointTy() || RHS->getType()->isFloatingPointTy()) {
            return Builder.CreateFDiv(LHS, RHS, "divtmp");
        } else {
            return Builder.CreateSDiv(LHS, RHS, "divtmp");
        }
    }
    case NODE_POWER_EXPR: {
        cout << "NODE_POWER_EXPR" << endl;
        // Implement power operation using llvm.pow function
        Value *LHS = codegen(node->children[0]);
        Value *RHS = codegen(node->children[1]);
        Function *PowFunc = Intrinsic::getDeclaration(TheModule.get(), Intrinsic::pow, {Type::getDoubleTy(TheContext)});
        LHS = Builder.CreateSIToFP(LHS, Type::getDoubleTy(TheContext));
        RHS = Builder.CreateSIToFP(RHS, Type::getDoubleTy(TheContext));
        return Builder.CreateCall(PowFunc, {LHS, RHS}, "powtmp");
    }
    case NODE_EQUALS_EXPR: {
        cout << "NODE_EQUALS_EXPR" << endl;
        Value *LHS = codegen(node->children[0]);
        Value *RHS = codegen(node->children[1]);
        return Builder.CreateICmpEQ(LHS, RHS, "eqtmp");
    }
    case NODE_NOT_EQUALS_EXPR: {
        cout << "NODE_NOT_EQUALS_EXPR" << endl;
        Value *LHS = codegen(node->children[0]);
        Value *RHS = codegen(node->children[1]);
        return Builder.CreateICmpNE(LHS, RHS, "netmp");
    }
    case NODE_LEQ_EXPR: {
        cout << "NODE_LEQ_EXPR" << endl;
        Value *LHS = codegen(node->children[0]);
        Value *RHS = codegen(node->children[1]);
        return Builder.CreateICmpSLE(LHS, RHS, "leqtmp");
    }
    case NODE_GEQ_EXPR: {
        cout << "NODE_GEQ_EXPR" << endl;
        Value *LHS = codegen(node->children[0]);
        Value *RHS = codegen(node->children[1]);
        return Builder.CreateICmpSGE(LHS, RHS, "geqtmp");
    }
    case NODE_LE_EXPR: {
        cout << "NODE_LE_EXPR" << endl;
        Value *LHS = codegen(node->children[0]);
        Value *RHS = codegen(node->children[1]);
        return Builder.CreateICmpSLT(LHS, RHS, "letmp");
    }
    case NODE_GE_EXPR: {
        cout << "NODE_GE_EXPR" << endl;
        Value *LHS = codegen(node->children[0]);
        Value *RHS = codegen(node->children[1]);
        return Builder.CreateICmpSGT(LHS, RHS, "getmp");
    }
    case NODE_LOGICAL_AND_EXPR: {
        cout << "NODE_LOGICAL_AND_EXPR" << endl;
        Value *LHS = codegen(node->children[0]);
        Value *RHS = codegen(node->children[1]);
        return Builder.CreateAnd(LHS, RHS, "andtmp");
    }
    case NODE_LOGICAL_OR_EXPR: {
        cout << "NODE_LOGICAL_OR_EXPR" << endl;
        Value *LHS = codegen(node->children[0]);
        Value *RHS = codegen(node->children[1]);
        return Builder.CreateOr(LHS, RHS, "ortmp");
    }
    case NODE_UNARY_LOGICAL_NOT_EXPR: {
        cout << "NODE_UNARY_LOGICAL_NOT_EXPR" << endl;
        Value *Operand = codegen(node->children[0]);
        return Builder.CreateNot(Operand, "nottmp");
    }
    // case NODE_NEGATE_EXPR: {cout << "NODE_NEGATE_EXPR" << endl;
    //     Value* Operand = codegen(node->children[0]);
    //     if (Operand->getType()->isFloatingPointTy()) {
    //         return Builder.CreateFNeg(Operand, "negtmp");
    //     } else {
    //         return Builder.CreateNeg(Operand, "negtmp");
    //     }
    // }
    case NODE_FUNCTION_CALL: {
        cout << "NODE_FUNCTION_CALL" << endl;
        string FuncName = node->children[0]->text;
        Function *CalleeF = getFunction(FuncName);

        vector<Value *> ArgsV;
        Stype *FuncArgsNode = node->children[1];
        for (Stype *arg : FuncArgsNode->children) {
            Value *ArgVal = codegen(arg);
            ArgsV.push_back(ArgVal);
        }

        return Builder.CreateCall(CalleeF, ArgsV, "calltmp");
    }

    // case NODE_CAST_EXPR: {cout << "NODE_CAST_EXPR" << endl;
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
    // case NODE_VECTOR_ACCESS: {cout << "NODE_VECTOR_ACCESS" << endl;
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
        cout << "NODE_MOD_EQUALS_ASSIGNMENT_STATEMENT" << endl;
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
        cout << "NODE_DECLARATION_STATEMENT_WITH_TYPE" << endl;
        // Declaration: IDENTIFIER '<-' expr
        string VarName = node->children[0]->text;
        cout << VarName << endl;
        Stype *ExprNode = node->children[1];

        Value *InitVal = codegen(ExprNode);

        // Get the function we are currently in.
        Function *TheFunction = Builder.GetInsertBlock()->getParent();
        cout << TheFunction << endl;
        cout << "Done with decl stmt" << endl;

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
        cout << "NODE_IF_STATEMENT" << endl;
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
        BasicBlock *ElseBB = BasicBlock::Create(TheContext, "else");
        BasicBlock *MergeBB = BasicBlock::Create(TheContext, "ifcont");

        Builder.CreateCondBr(CondV, ThenBB, ElseBB);

        // Emit then block.
        Builder.SetInsertPoint(ThenBB);
        pushSymbolTable();
        codegen(node->children[1]); // Then statements
        popSymbolTable();
        Builder.CreateBr(MergeBB);

        // Emit else block.
        // TheFunction->getBasicBlocks().push_back(ElseBB);
        Builder.SetInsertPoint(ElseBB);
        if (node->children.size() > 2) {
            pushSymbolTable();
            codegen(node->children[2]); // Else statements
            popSymbolTable();
        }
        Builder.CreateBr(MergeBB);

        // Emit merge block.
        // TheFunction->getBasicBlocks().push_back(MergeBB);
        Builder.SetInsertPoint(MergeBB);

        return Constant::getNullValue(Type::getInt32Ty(TheContext));
    }
    case NODE_LOOP_REPEAT_STATEMENT: {
        cout << "NODE_LOOP_REPEAT_STATEMENT" << endl;
        // loop_statement: LOOP expr '{' statements '}'
        Function *TheFunction = Builder.GetInsertBlock()->getParent();

        BasicBlock *LoopBB = BasicBlock::Create(TheContext, "loop", TheFunction);
        BasicBlock *AfterLoopBB = BasicBlock::Create(TheContext, "afterloop");

        // Initial jump to loop block
        Builder.CreateBr(LoopBB);

        // Emit loop block
        Builder.SetInsertPoint(LoopBB);
        pushSymbolTable();
        codegen(node->children[1]); // Loop body statements
        popSymbolTable();

        // Condition check
        Value *CondV = codegen(node->children[0]);
        if (CondV->getType()->isIntegerTy())
            CondV = Builder.CreateICmpNE(CondV, ConstantInt::get(CondV->getType(), 0), "loopcond");
        else if (CondV->getType()->isFloatingPointTy())
            CondV = Builder.CreateFCmpONE(CondV, ConstantFP::get(TheContext, APFloat(0.0)), "loopcond");

        Builder.CreateCondBr(CondV, LoopBB, AfterLoopBB);

        // Emit after loop block
        // TheFunction->getBasicBlocks().push_back(AfterLoopBB);
        Builder.SetInsertPoint(AfterLoopBB);

        return Constant::getNullValue(Type::getInt32Ty(TheContext));
    }
    case NODE_BREAK_STATEMENT: {
        cout << "NODE_BREAK_STATEMENT" << endl;
        // Implement break statement
        // For simplicity, we can skip implementing break and continue
        return Constant::getNullValue(Type::getInt32Ty(TheContext));
    }
    case NODE_CONTINUE_STATEMENT: {
        cout << "NODE_CONTINUE_STATEMENT" << endl;
        // Implement continue statement
        return Constant::getNullValue(Type::getInt32Ty(TheContext));
    }
    case NODE_RETURN_STATEMENT: {
        cout << "NODE_RETURN_STATEMENT" << endl;
        if (node->children.size() > 0) {
            Value *RetVal = codegen(node->children[0]);
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
        cout << "NODE_PRINT_STATEMENT" << endl;
        declarePrintFunction();
        Value *ExprVal = codegen(node->children[0]);

        if (ExprVal->getType()->isIntegerTy(64)) {
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
        cout << "NODE_LOAD_STATEMENT" << endl;
        declareAudioFunctions();
        Value *FilenameVal = codegen(node->children[0]);
        Value *AudioData = Builder.CreateCall(loadAudioFunction, {FilenameVal}, "loadaudio");
        return AudioData;
    }
    case NODE_PLAY_STATEMENT: {
        cout << "NODE_PLAY_STATEMENT" << endl;
        declareAudioFunctions();
        Value *AudioVal = codegen(node->children[0]);
        Builder.CreateCall(playAudioFunction, {AudioVal});
        return nullptr;
    }
    case NODE_SAVE_STATEMENT: {
        cout << "NODE_SAVE_STATEMENT" << endl;
        declareAudioFunctions();
        Value *AudioVal = codegen(node->children[0]);
        Value *FilenameVal = codegen(node->children[1]);
        Builder.CreateCall(saveAudioFunction, {AudioVal, FilenameVal});
        return nullptr;
    }
    case NODE_NORMAL_FUNCTION: {
        cout << "NODE_NORMAL_FUNCTION" << endl;
        // Implement function definition
        string FuncName = node->children[0]->text;
        Stype *ParamListNode = node->children[1];
        DataType *ReturnTypeData = node->data_type;
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
        Function *TheFunction = Function::Create(FT, Function::ExternalLinkage, FuncName, TheModule.get());

        unsigned idx = 0;
        for (auto &Arg : TheFunction->args()) {
            Arg.setName(ParamNames[idx++]);
        }

        BasicBlock *BB = BasicBlock::Create(TheContext, "entry", TheFunction);
        Builder.SetInsertPoint(BB);

        pushSymbolTable();

        idx = 0;
        for (auto &Arg : TheFunction->args()) {
            AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, Arg.getName().str(), Arg.getType());
            Builder.CreateStore(&Arg, Alloca);
            NamedValues.back()[Arg.getName().str()] = Alloca;
            idx++;
        }

        codegen(BodyNode);

        if (ReturnType == Type::getVoidTy(TheContext)) {
            Builder.CreateRetVoid();
        }

        popSymbolTable();

        verifyFunction(*TheFunction);
        return TheFunction;
    }
    case NODE_STATEMENTS: {
        cout << "NODE_STATEMENTS" << endl;
        Value *LastValue = nullptr;
        for (Stype *stmt : node->children) {
            LastValue = codegen(stmt);
        }
        return LastValue;
    }
    default:
        cout << "forgot to handle a node bruh" << endl;
        return nullptr;
    }
}

// Function to get LLVM type from DataType
Type *getLLVMType(DataType *dtype) {
    if (dtype->is_primitive) {
        if (dtype->is_vector) {
            Type *elemType = getLLVMType(dtype->vector_data_type);
            return ArrayType::get(elemType, dtype->vector_size);
        } else {
            switch (dtype->basic_data_type) {
            case INT:
                return Type::getInt32Ty(TheContext);
            case LONG:
                return Type::getInt64Ty(TheContext);
            case FLOAT:
                return Type::getDoubleTy(TheContext);
            case BOOL:
                return Type::getInt1Ty(TheContext);
            case STRING:
                return Type::getInt8Ty(TheContext)->getPointerTo();
            case AUDIO:
                return Type::getInt8Ty(TheContext)->getPointerTo(); // Represent audio data as i8*
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
    // Initialize the module
    TheModule = make_unique<Module>("MyModule", TheContext);

    // Generate code for the root node
    cout << "-------------------------------starting codegen--------------------------------------" << endl;
    Function* main_function = Function::Create(FunctionType::get(Type::getVoidTy(TheContext), {}, false), Function::ExternalLinkage, "main", TheModule.get());
    BasicBlock *BB = BasicBlock::Create(TheContext, "entry", main_function);
    Builder.SetInsertPoint(BB);
    pushSymbolTable();
    codegen(root);
    popSymbolTable();
    cout << "--------------------------traversed tree for codegen---------------------------------" << endl;

    // Validate the generated code
    verifyModule(*TheModule, &errs());

    // Print the generated code
    TheModule->print(outs(), nullptr);

    return 0;
}
