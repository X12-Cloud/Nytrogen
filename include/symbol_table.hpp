#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "ast.hpp" // For TypeNode and other AST types

// Forward declaration for StructDefinitionNode if needed, though ast.hpp should include it

// Represents information about a single symbol (variable, function, struct member)
struct Symbol {
    enum class SymbolType {
        VARIABLE,
        FUNCTION,
        STRUCT_DEFINITION,
        STRUCT_MEMBER // For members within a struct definition
    };

    SymbolType type;
    std::string name;
    std::unique_ptr<TypeNode> dataType; // The type of the symbol (e.g., int, string, Point)
    int offset; // For variables: offset from base pointer; for struct members: offset within struct
    int size;   // Size in bytes (for variables or struct members)

    // Constructor for variables/members
    Symbol(SymbolType type, std::string name, std::unique_ptr<TypeNode> dataType, int offset = 0, int size = 0)
        : type(type), name(std::move(name)), dataType(std::move(dataType)), offset(offset), size(size) {}

    // Constructor for functions (dataType would be return type)
    // For functions, offset/size might not be directly applicable here, or could represent stack frame size etc.
    // We can refine this as needed.
    Symbol(SymbolType type, std::string name, std::unique_ptr<TypeNode> dataType)
        : type(type), name(std::move(name)), dataType(std::move(dataType)), offset(0), size(0) {}
};

// Represents a single scope in the symbol table (e.g., global, function body)
class Scope {
public:
    std::map<std::string, Symbol> symbols;
    int currentOffset; // For local variables, tracks the current stack offset

    Scope() : currentOffset(0) {}

    void addSymbol(Symbol symbol) {
        symbols.emplace(symbol.name, std::move(symbol));
    }

    Symbol* lookup(const std::string& name) {
        auto it = symbols.find(name);
        if (it != symbols.end()) {
            return &it->second;
        }
        return nullptr;
    }
};

// Main Symbol Table class
class SymbolTable {
public:
    std::vector<std::unique_ptr<Scope>> scopes; // Stack of scopes

    SymbolTable() {
        enterScope(); // Start with a global scope
    }

    void enterScope() {
        scopes.push_back(std::make_unique<Scope>());
    }

    void exitScope() {
        if (!scopes.empty()) {
            scopes.pop_back();
        }
    }

    void addSymbol(Symbol symbol) {
        if (!scopes.empty()) {
            scopes.back()->addSymbol(std::move(symbol));
        } else {
            // Handle error: no active scope
        }
    }

    Symbol* lookup(const std::string& name) {
        // Search from innermost to outermost scope
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            if (Symbol* symbol = (*it)->lookup(name)) {
                return symbol;
            }
        }
        return nullptr; // Not found
    }

    // Specific storage for struct definitions, accessible globally
    std::map<std::string, std::unique_ptr<StructDefinitionNode>> definedStructs;

    // Helper to add a struct definition to the global storage
    void addStructDefinition(std::unique_ptr<StructDefinitionNode> structDef) {
        definedStructs[structDef->name] = std::move(structDef);
    }

    // Helper to lookup a struct definition
    StructDefinitionNode* lookupStructDefinition(const std::string& name) {
        auto it = definedStructs.find(name);
        if (it != definedStructs.end()) {
            return it->second.get();
        }
        return nullptr;
    }
};

#endif // SYMBOL_TABLE_HPP
