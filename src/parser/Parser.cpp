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
    if (current_token_.type == TokenType::EXPLAIN) {
        return parseExplainStatement();
    }
    if (current_token_.type == TokenType::SELECT) {
        return parseSelectStatement();
    }
    return nullptr;
}

// NEW: Parses EXPLAIN followed by a SELECT
std::shared_ptr<Statement> Parser::parseExplainStatement() {
    nextToken(); // Move past 'EXPLAIN'

    // In Phase 3, we assume EXPLAIN is always followed by a SELECT query
    if (current_token_.type == TokenType::SELECT) {
        auto stmt = parseSelectStatement();
        return std::make_shared<ExplainStatement>(std::move(stmt));
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

// NEW: Parse either a regular column (user_id) or a function call (SUM(amount))
std::shared_ptr<Expression> Parser::parseColumnOrFunction() {
    std::string name = current_token_.literal;
    nextToken(); // Advance past the name

    // If the next token is a '(', this is a function call!
    if (current_token_.type == TokenType::LPAREN) {
        nextToken(); // Advance past '('
        
        std::vector<std::shared_ptr<Expression>> args;
        
        // Grab the argument inside the parentheses (e.g., 'amount')
        if (current_token_.type == TokenType::IDENTIFIER || current_token_.type == TokenType::STAR) {
            args.push_back(std::make_shared<Identifier>(current_token_.literal));
            nextToken();
        }

        if (current_token_.type == TokenType::RPAREN) {
            nextToken(); // Advance past ')'
        }
        
        return std::make_shared<FunctionCall>(name, args);
    }

    // If there was no '(', it's just a regular column
    return std::make_shared<Identifier>(name);
}

std::shared_ptr<SelectStatement> Parser::parseSelectStatement() {
    auto stmt = std::make_shared<SelectStatement>();

    nextToken(); // Move past 'SELECT'

    // Parse columns until we hit 'FROM' or EOF
    while (current_token_.type != TokenType::FROM && current_token_.type != TokenType::END_OF_FILE) {
        if (current_token_.type == TokenType::IDENTIFIER) {
            // NEW: Use our smart helper method
            stmt->columns.push_back(parseColumnOrFunction());
        } else {
            nextToken(); // Move past commas
        }
    }

    // Parse the main table name
    if (currentTokenIs(TokenType::FROM)) {
        if (expectPeek(TokenType::IDENTIFIER)) {
            stmt->tableName = current_token_.literal;
        }
    }
    nextToken(); // Advance past the table name

    // Check for JOIN clauses
    while (currentTokenIs(TokenType::JOIN)) {
        stmt->joins.push_back(parseJoinClause());
        nextToken(); 
    }

    // Check if there is a WHERE clause
    if (currentTokenIs(TokenType::WHERE)) {
        nextToken(); // Move past 'WHERE'
        stmt->whereClause = parseExpression();
    }
    
    // NEW: Check if there is a GROUP BY clause
    if (currentTokenIs(TokenType::GROUP)) {
        nextToken(); // Move past 'GROUP'
        if (currentTokenIs(TokenType::BY)) {
            nextToken(); // Move past 'BY'

            // Parse the grouping columns until the query ends
            while (current_token_.type != TokenType::SEMICOLON && current_token_.type != TokenType::END_OF_FILE) {
                if (current_token_.type == TokenType::IDENTIFIER) {
                    stmt->groupBy.push_back(std::make_shared<Identifier>(current_token_.literal));
                }
                nextToken();
            }
        }
    }

    if (peekTokenIs(TokenType::SEMICOLON) || currentTokenIs(TokenType::SEMICOLON)) {
        if (peekTokenIs(TokenType::SEMICOLON)) nextToken();
    }

    return stmt;
}

} // namespace quill