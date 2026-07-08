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
    // Right now we only support SELECT statements, but this switch-like
    // structure allows us to easily add INSERT, CREATE, etc. later.
    if (current_token_.type == TokenType::SELECT) {
        return parseSelectStatement();
    }
    return nullptr;
}

std::shared_ptr<SelectStatement> Parser::parseSelectStatement() {
    auto stmt = std::make_shared<SelectStatement>();

    // We are currently sitting on the 'SELECT' token. Move past it.
    nextToken();

    // Parse columns until we hit 'FROM' or EOF
    while (current_token_.type != TokenType::FROM && current_token_.type != TokenType::END_OF_FILE) {
        if (current_token_.type == TokenType::IDENTIFIER) {
            stmt->columns.push_back(std::make_shared<Identifier>(current_token_.literal));
        }
        nextToken();
    }

    // We should now be sitting on the 'FROM' token
    if (currentTokenIs(TokenType::FROM)) {
        // We expect the very next token to be the table name (an identifier)
        if (expectPeek(TokenType::IDENTIFIER)) {
            stmt->tableName = current_token_.literal;
        }
    }
    
    // If the query ends with a semicolon, consume it
    if (peekTokenIs(TokenType::SEMICOLON)) {
        nextToken();
    }

    return stmt;
}

} // namespace quill