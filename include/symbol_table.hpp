#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <iostream> // For std::cerr and std::endl
#include <ostream>  // For std::endl
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
    std::unique_ptr<StructDefinitionNode> structDef; // For struct definitions
    int offset; // For variables: offset from base pointer; for struct members: offset within struct
    int size;   // Size in bytes (for variables or struct members)

    // Constructor for variables/members
    Symbol(SymbolType type, std::string name, std::unique_ptr<TypeNode> dataType, int offset = 0, int size = 0)
        : type(type), name(std::move(name)), dataType(std::move(dataType)), structDef(nullptr), offset(offset), size(size) {}

    // Constructor for functions
    Symbol(SymbolType type, std::string name, std::unique_ptr<TypeNode> dataType, std::vector<std::unique_ptr<TypeNode>> paramTypes)
        : type(type), name(std::move(name)), dataType(std::move(dataType)), structDef(nullptr), parameterTypes(std::move(paramTypes)), offset(0), size(0) {}

    // Constructor for struct definitions
    Symbol(SymbolType type, std::string name, std::unique_ptr<StructDefinitionNode> structDef)
        : type(type), name(std::move(name)), dataType(nullptr), structDef(std::move(structDef)), offset(0), size(0) {}

    std::vector<std::unique_ptr<TypeNode>> parameterTypes; // For functions: types of parameters
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
        std::cerr << "Debug: Entered new scope. Total scopes: " << scopes.size() << std::endl;
    }

    void exitScope() {
        if (!scopes.empty()) {
            scopes.pop_back();
            std::cerr << "Debug: Exited scope. Total scopes: " << scopes.size() << std::endl;
        }
    }

    Symbol* addSymbol(Symbol symbol) {
        if (!scopes.empty()) {
            std::cerr << "Debug: Adding symbol '" << symbol.name << "' to scope " << scopes.size() << ". Offset: " << symbol.offset << std::endl;
            auto result = scopes.back()->symbols.emplace(symbol.name, std::move(symbol));
            return &(result.first->second);
        } else {
            // Handle error: no active scope
            std::cerr << "Error: Attempted to add symbol '" << symbol.name << "' with no active scope." << std::endl;
            return nullptr;
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

    bool isStructDefined(const std::string& name) {
        Symbol* symbol = lookup(name);
        return symbol && symbol->type == Symbol::SymbolType::STRUCT_DEFINITION;
    }
};

#endif // SYMBOL_TABLE_HPP
