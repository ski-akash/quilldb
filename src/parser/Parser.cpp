#include "parser/Parser.h"

namespace quill {

Parser::Parser(Lexer lexer) : lexer_(std::move(lexer)) {
    // Read two tokens immediately so both current_token_ and peek_token_ are populated
    nextToken();
    nextToken();
}

void Parser::nextToken() {
    current_token_ = peek_token_;
    peek_token_ = lexer_.nextToken();
}

bool Parser::currentTokenIs(TokenType type) const {
    return current_token_.type == type;
}

bool Parser::peekTokenIs(TokenType type) const {
    return peek_token_.type == type;
}

bool Parser::expectPeek(TokenType type) {
    if (peekTokenIs(type)) {
        nextToken();
        return true;
    } else {
        return false;
    }
}

std::vector<std::shared_ptr<Statement>> Parser::parse() {
    std::vector<std::shared_ptr<Statement>> statements;
    
    while (current_token_.type != TokenType::END_OF_FILE) {
        auto stmt = parseStatement();
        if (stmt != nullptr) {
            statements.push_back(stmt);
        }
        nextToken();
    }
    
    return statements;
}

std::shared_ptr<Statement> Parser::parseStatement() {
    if (current_token_.type == TokenType::SELECT) {
        return parseSelectStatement();
    }
    return nullptr;
}

// NEW: Parses basic binary expressions like "id = 42"
std::shared_ptr<Expression> Parser::parseExpression() {
    // 1. Grab the left side (e.g., 'users.id')
    auto left = std::make_shared<Identifier>(current_token_.literal);
    nextToken(); 

    // 2. Grab the operator (e.g., '=')
    std::string op = current_token_.literal;
    nextToken(); 

    // 3. Grab the right side. It could be a Number (42) or an Identifier (orders.user_id)
    std::shared_ptr<Expression> right;
    if (current_token_.type == TokenType::NUMBER) {
        right = std::make_shared<NumberLiteral>(current_token_.literal);
    } else {
        right = std::make_shared<Identifier>(current_token_.literal);
    }
    
    return std::make_shared<BinaryExpression>(std::move(left), std::move(op), std::move(right));
}

// NEW: Build the Join AST Node
std::shared_ptr<JoinClause> Parser::parseJoinClause() {
    nextToken(); // Move past 'JOIN'

    std::string joinTable;
    if (current_token_.type == TokenType::IDENTIFIER) {
        joinTable = current_token_.literal;
        nextToken(); // Move past table name
    }

    if (currentTokenIs(TokenType::ON)) {
        nextToken(); // Move past 'ON'
    }

    auto condition = parseExpression();

    return std::make_shared<JoinClause>(std::move(joinTable), std::move(condition));
}

std::shared_ptr<SelectStatement> Parser::parseSelectStatement() {
    auto stmt = std::make_shared<SelectStatement>();

    nextToken(); // Move past 'SELECT'

    // Parse columns until we hit 'FROM' or EOF
    while (current_token_.type != TokenType::FROM && current_token_.type != TokenType::END_OF_FILE) {
        if (current_token_.type == TokenType::IDENTIFIER) {
            stmt->columns.push_back(std::make_shared<Identifier>(current_token_.literal));
        }
        nextToken();
    }

    // Parse the main table name
    if (currentTokenIs(TokenType::FROM)) {
        if (expectPeek(TokenType::IDENTIFIER)) {
            stmt->tableName = current_token_.literal;
        }
    }
    nextToken(); // Advance past the table name

    // NEW: Check for JOIN clauses (loop in case there are multiple!)
    while (currentTokenIs(TokenType::JOIN)) {
        stmt->joins.push_back(parseJoinClause());
        // parseJoinClause leaves us sitting on the token *after* the ON condition
        // so we don't need to call nextToken() here.
        nextToken(); 
    }

    // Check if there is a WHERE clause
    if (currentTokenIs(TokenType::WHERE)) {
        nextToken(); // Move past 'WHERE' keyword
        stmt->whereClause = parseExpression();
    }
    
    if (peekTokenIs(TokenType::SEMICOLON) || currentTokenIs(TokenType::SEMICOLON)) {
        if (peekTokenIs(TokenType::SEMICOLON)) nextToken();
    }

    return stmt;
}

} // namespace quill