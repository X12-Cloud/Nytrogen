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
struct EnumStatementNode;

struct EnumInfo {
    std::string name;
    std::shared_ptr<EnumStatementNode> node; // The AST node for the enum
};

// Represents information about a single symbol (variable, function, struct member)
struct Symbol {
    enum class SymbolType {
        VARIABLE,
        FUNCTION,
        STRUCT_DEFINITION,
        STRUCT_MEMBER, // For members within a struct definition
        CONSTANT,
        ENUM_TYPE,
        ENUM_MEMBER
    };

    SymbolType type;
    std::string name;
    std::unique_ptr<TypeNode> dataType; // The type of the symbol (e.g., int, string, Point)
    std::shared_ptr<StructDefinitionNode> structDef; // For struct definitions
    int offset; // For variables: offset from base pointer; for struct members: offset within struct
    int size;   // Size in bytes (for variables or struct members)
    std::unique_ptr<ASTNode> value; // For constants
    std::shared_ptr<EnumInfo> enumInfo; // For enum types
    StructMember::Visibility visibility; // For struct members

    // Constructor for variables/members
    Symbol(SymbolType type, std::string name, std::unique_ptr<TypeNode> dataType, int offset = 0, int size = 0, StructMember::Visibility visibility = StructMember::Visibility::PUBLIC)
        : type(type), name(std::move(name)), dataType(std::move(dataType)), structDef(nullptr), offset(offset), size(size), value(nullptr), enumInfo(nullptr), visibility(visibility) {}

    // Constructor for functions
    Symbol(SymbolType type, std::string name, std::unique_ptr<TypeNode> dataType, std::vector<std::unique_ptr<TypeNode>> paramTypes)
        : type(type), name(std::move(name)), dataType(std::move(dataType)), structDef(nullptr), parameterTypes(std::move(paramTypes)), offset(0), size(0), value(nullptr), enumInfo(nullptr), visibility(StructMember::Visibility::PUBLIC) {}

    // Constructor for struct definitions
    Symbol(SymbolType type, std::string name, std::shared_ptr<StructDefinitionNode> structDef)
        : type(type), name(std::move(name)), dataType(nullptr), structDef(std::move(structDef)), offset(0), size(structDef->size), value(nullptr), enumInfo(nullptr), visibility(StructMember::Visibility::PUBLIC) {}

    // Constructor for constants
    Symbol(SymbolType type, std::string name, std::unique_ptr<TypeNode> dataType, std::unique_ptr<ASTNode> value)
        : type(type), name(std::move(name)), dataType(std::move(dataType)), structDef(nullptr), offset(0), size(0), value(std::move(value)), enumInfo(nullptr), visibility(StructMember::Visibility::PUBLIC) {}

    // Constructor for enum types
    Symbol(SymbolType type, std::string name, std::shared_ptr<EnumInfo> enumInfo)
        : type(type), name(std::move(name)), dataType(nullptr), structDef(nullptr), offset(0), size(0), value(nullptr), enumInfo(std::move(enumInfo)), visibility(StructMember::Visibility::PUBLIC) {}


    std::vector<std::unique_ptr<TypeNode>> parameterTypes; // For functions: types of parameters
};

// Represents a single scope in the symbol table (e.g., global, function body)
class Scope {
public:
    std::map<std::string, Symbol> symbols;
    int currentOffset; // For local variables, tracks the current stack offset
    Scope* parent;

    Scope(Scope* p = nullptr) : currentOffset(0), parent(p) {}

    void addSymbol(Symbol&& symbol) {
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
    // This vector is now an ARCHIVE. We only push_back, NEVER pop_back.
    std::vector<std::unique_ptr<Scope>> all_scopes; 
    
    // This is our "Read/Write Head"
    Scope* current_scope;

    std::map<std::string, StructDefinitionNode*> struct_definitions;

    SymbolTable() : current_scope(nullptr) {
        enterScope(); // Creates the Global Scope
    }

    void enterScope() {
        // Create new scope linked to the current one
        auto new_scope = std::make_unique<Scope>(current_scope);
        current_scope = new_scope.get(); // Move the head to the new scope
        
        all_scopes.push_back(std::move(new_scope)); // Save to the archive
        
        std::cerr << "Debug: Entered new scope. Total scopes in archive: " << all_scopes.size() << std::endl;
    }

    void exitScope() {
        if (current_scope && current_scope->parent) {
            current_scope = current_scope->parent; // Just climb up! No deletion!
            std::cerr << "Debug: Exited scope. Head moved to parent." << std::endl;
        }
    }

    Symbol* addSymbol(Symbol&& symbol) {
        if (current_scope) {
            std::cerr << "Debug: Adding symbol '" << symbol.name << "' to current scope." << std::endl;
            // Use current_scope instead of scopes.back()
            auto result = current_scope->symbols.emplace(symbol.name, std::move(symbol));
            return &(result.first->second);
        }
        return nullptr;
    }

    Symbol* lookup(const std::string& name) {
        // Search starting from current_scope and follow parent pointers
        Scope* search_head = current_scope;
        while (search_head != nullptr) {
            if (Symbol* symbol = search_head->lookup(name)) {
                return symbol;
            }
            search_head = search_head->parent; // Move to outer scope
        }
        return nullptr;
    }

    bool isStructDefined(const std::string& name) {
        return struct_definitions.count(name) > 0;
    }

    void addStructDefinition(const std::string& name, StructDefinitionNode* node) {
        struct_definitions[name] = node;
    }

    std::map<std::string, StructDefinitionNode*> getStructDefinitions() {
        return struct_definitions;
    }
};

#endif // SYMBOL_TABLE_HPP
