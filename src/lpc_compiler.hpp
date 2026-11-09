// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#pragma once
#include <cstddef>
#include <fstream>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include "code_generator.hpp"
#include "const_pool.hpp"
#include "fabric.hpp"
#include "location.hh"
#include "lpc_types.hpp"

struct ASTNode;
class ConstTable;
class SymbolTable;
class ObjectClass;

class LpcCompiler {
public:
    using FabricType = ManagedRuntime;

    LpcCompiler(FILE* source_file = nullptr);
    ~LpcCompiler();

    // context data used by the preprocessor, lexer and parser
    struct Context {
        std::shared_ptr<FabricType> fabric; // fabric providing filesystem access and other resources
        std::vector<PathType> include_directories;
        std::vector<PathType> active_includes;
        PathType source_path; // physical path, unaffected by #line
        // Bison locations borrow filenames; keep them stable through lookahead
        // and share storage with nested preprocessor scanners.
        std::shared_ptr<std::set<std::string>> source_names =
            std::make_shared<std::set<std::string>>();

        void* ppscanner{nullptr}; // pointer to the preprocessor scanner (reentrant lexer handle)
        std::vector<char> ppbuffer;
        bool pp_eof{false}; // EOF reached by macro argument lookahead
        yy::location pp_location; // position in original preprocessor input
        std::shared_ptr<SymbolTable> defines; // preprocessor defines (PREPROCESSOR scope)

        std::shared_ptr<ASTNode> root;
        std::shared_ptr<SymbolTable> current_scope;
        std::shared_ptr<ConstTable> consts;

        ConstPool::IndexType current_file; // index of the current source filename in the constant pool
        ConstPool::IndexType current_func; // index of the current function name in the constant pool
        uint32_t next_func_idx; // index for the next function
        uint32_t next_global;
        uint32_t next_local;
        SymbolSignature variable_signature{};

        std::string current_string;
        std::string raw_string_delimiter;
        yy::location location;
        yy::position string_begin;
    
        Context(): current_file(ConstPool::INVALID_INDEX), current_func(ConstPool::INVALID_INDEX), next_func_idx(0), next_global(0), next_local(0) {}
        ~Context() = default;
    };

    // compiler messages (errors, warnings, etc.)
    struct Message {
        int lineno;
        int column;
        std::string text;
    };

    void use_fabric(std::shared_ptr<FabricType> sb);
    // Replace the ordered include search directories. Quoted includes first
    // search the including file's directory, when its filename is known.
    void use_include_directories(std::vector<PathType> directories);
    void use_predefines(std::shared_ptr<SymbolTable> predefines);
    void set_filename(const std::string_view fn) { filename_ = fn; }

    std::string preprocess() noexcept;
    std::string preprocess(const std::string& source) noexcept;

    bool parse() noexcept;
    bool parse (const std::string& source) noexcept;
    std::shared_ptr<ObjectClass> generate_code(bool string_accumulator = true) noexcept;

    const std::vector<Message>& messages() const { return messages_; }

    const SymbolTable* get_globals() const { return globals_.get(); }
    const ConstTable* get_consts() const { return consts_.get(); }
    const ASTNode* get_ast() const { return ast_.get(); }

private:
    struct internal_s;
    internal_s* impl_;

    FILE* source_file_;
    std::string filename_;  // for yylloc
    std::vector<Message> messages_;

    std::shared_ptr<FabricType> fabric_;
    std::vector<PathType> include_directories_;
    std::shared_ptr<SymbolTable> predefines_;

    std::shared_ptr<ConstTable> consts_;
    std::shared_ptr<SymbolTable> globals_;
    std::shared_ptr<ASTNode> ast_;

    bool init();

public:
    // ===================================================================
    // code generation helpers

    struct FunctionBuilder {
        LpcCompiler& compiler_;
        lpcvm::Bytecode& codegen_;
        VirtualTable& vtable_;

        FunctionBuilder(LpcCompiler& compiler, lpcvm::Bytecode& codegen, VirtualTable& vtable)
            : compiler_(compiler), codegen_(codegen), vtable_(vtable) {}

        void build(const AST::FunctionDef& func_def);
    };
};

#undef YY_EXTRA_TYPE
#define YY_EXTRA_TYPE LpcCompiler::Context*
