#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MAX_TOKEN_LENGTH 100
#define MAX_TOKENS 1000

// Token types
typedef enum {
    TOKEN_INT, TOKEN_IF, TOKEN_IDENTIFIER, TOKEN_NUMBER,
    TOKEN_ASSIGN, TOKEN_PLUS, TOKEN_MINUS, TOKEN_CONDITION, 
    TOKEN_LPAREN, TOKEN_RPAREN, TOKEN_LBRACE, TOKEN_RBRACE,
    TOKEN_SEMICOLON, TOKEN_EOF, TOKEN_UNKNOWN
} TokenType;

// Token structure
typedef struct {
    TokenType type;
    char value[MAX_TOKEN_LENGTH];
} Token;

// AST Node Types
typedef enum {
    NODE_VAR_DECL, NODE_ASSIGN, NODE_BINARY_OP, NODE_IF, NODE_LITERAL, NODE_IDENTIFIER, NODE_CONDITION
} ASTNodeType;

// AST Node Structure
typedef struct ASTNode {
    ASTNodeType type;
    char value[100];
    struct ASTNode *left;   
    struct ASTNode *right;  
    struct ASTNode *body;   
    struct ASTNode *condition;  
} ASTNode;

typedef struct ASTMain{
    ASTNodeType type;
    char value[100];
    struct ASTMain *next;
    struct ASTNode *statement;

}ASTMain;

 typedef struct symbolTable{
    char sym[20];
    struct symbolTable* next;
 }symbolTable;


// Function prototypes
void tokenize(char *source);
void storeToken(Token token);
void printTokens();
Token getNextToken();

ASTMain* parseProgram();
ASTNode* parseStatement();
ASTNode* parseVarDeclaration();
ASTNode* parseAssignment();
ASTNode* parseExpression();
ASTNode* parseIfStatement();
ASTNode* createNode(ASTNodeType type, const char* value);
void AST(ASTNode* node, int indent);
void printAST(ASTMain* node);
void checkSymbol(const char* value );

void generateAssembly(ASTMain* node);
void Assembly(ASTNode* node);
void generateAssignment(ASTNode* node);
void generateIfStatement(ASTNode* node);
void generateExpression(ASTNode* node);
void generateLiteral(ASTNode* node);
void generateIdentifier(ASTNode* node);




// Global variables
char *src;
char currentChar;
int pos = 0;
Token tokens[MAX_TOKENS];
int tokenCount = 0;
int currentTokenIndex = 0;
FILE *asmFile;
symbolTable* rootTable;
symbolTable* curr;

void checkSymbol(const char* value ){
    symbolTable *temp=rootTable;
    while(temp){
        if(strcmp(temp->sym,value)==0) return;
        temp=temp->next;
    }
    printf("error: keyword- '%s' not defined", value);
    exit(1);
}
// Advance the current character pointer
void advance() {
    if (src[pos] != '\0') {
        currentChar = src[++pos];
    }
}

// Skip whitespace
void skipWhitespace() {
    while (isspace(currentChar)) {
        advance();
    }
}

// Check if a string is a keyword
TokenType checkKeyword(char *str) {
    if (strcmp(str, "int") == 0) return TOKEN_INT;
    if (strcmp(str, "if") == 0) return TOKEN_IF;
    return TOKEN_IDENTIFIER;
}

// Get an identifier or keyword
Token identifier() {
    Token token;
    int i = 0;
    while (isalnum(currentChar)) {
        token.value[i++] = currentChar;
        advance();
    }
    token.value[i] = '\0';
    token.type = checkKeyword(token.value);
    return token;
}

// Get a number token
Token number() {
    Token token;
    int i = 0;
    while (isdigit(currentChar)) {
        token.value[i++] = currentChar;
        advance();
    }
    token.value[i] = '\0';
    token.type = TOKEN_NUMBER;
    return token;
}

// Tokenize the source code
void tokenize(char *source) {
    src = source;
    pos = 0;
    currentChar = src[pos];
    Token token;
    
    while (currentChar != '\0') {
        skipWhitespace();
        if (isalpha(currentChar)) {
            token = identifier();
        } else if (isdigit(currentChar)) {
            token = number();
        } else {
            int temp=0;
            switch (currentChar) {
                case '=':
                    temp=pos+1; 
                    while(1){
                        if(src[temp]!='=') break;
                        else {
                            temp++;
                        }
                        if(temp-pos>2) {
                            printf("error: expected '==' or '='\n");
                            exit(1);
                        }
                    }
                    if(temp-pos==1){
                        token.type = TOKEN_ASSIGN;
                        strcpy(token.value, "=");
                        advance();
                    }else{ 
                        strcpy(token.value, "==");
                        token.type = TOKEN_CONDITION;
                        advance();
                        advance();
                    } 
                    break;
                case '!': 
                    advance();
                    temp=pos+1; 
                    while(1){
                        if(src[temp]!='=') break;
                        else {
                            temp++;
                        }
                        if(temp-pos>1) {
                            printf("error: expected '==' , '=' or '!='\n");
                            exit(1);
                        }
                    }
                    strcpy(token.value, "!=");
                    token.type = TOKEN_CONDITION;
                    advance();
                    break;
                case '+': advance(); token.type = TOKEN_PLUS; strcpy(token.value, "+"); break;
                case '-': advance(); token.type = TOKEN_MINUS; strcpy(token.value, "-"); break;
                case ';': advance(); token.type = TOKEN_SEMICOLON; strcpy(token.value, ";"); break;
                case '{': advance(); token.type = TOKEN_LBRACE; strcpy(token.value, "{"); break;
                case '}': advance(); token.type = TOKEN_RBRACE; strcpy(token.value, "}"); break;
                case '(': advance(); token.type = TOKEN_LPAREN; strcpy(token.value, "("); break;
                case ')': advance(); token.type = TOKEN_RPAREN; strcpy(token.value, ")"); break;
                default: printf("error: undefined token: %c\n",currentChar); exit(1);
            }
        }
        storeToken(token);
    }
    token.type = TOKEN_EOF;
    strcpy(token.value, "EOF");
    storeToken(token);
}

// Store the token
void storeToken(Token token) {
    if (tokenCount < MAX_TOKENS) {
        tokens[tokenCount++] = token;
    }
}

// Get the next token
Token getNextToken() {
    if (currentTokenIndex < tokenCount) {
        return tokens[currentTokenIndex++];
    }
    Token eofToken = {TOKEN_EOF, "EOF"};
    return eofToken;
}

void printTokens() {
    for (int i = 0; i < tokenCount; i++) {
        printf("<%d, %s>\n", tokens[i].type, tokens[i].value);
    }
}


// Peek the current token without consuming it
Token peekToken() {
    if (currentTokenIndex < tokenCount) {
        return tokens[currentTokenIndex];
    }
    Token eofToken = {TOKEN_EOF, "EOF"};
    return eofToken;
}

// Parse the entire program
ASTMain* parseProgram() {
    ASTMain* root = (ASTMain*)malloc(sizeof(ASTMain));
    ASTMain* current = root;

    while (peekToken().type != TOKEN_EOF) {
        current->statement = parseStatement();
        if (!current->statement) {
            printf("Error: Invalid statement\n");
            exit(1);
        }
        ASTMain* temp = (ASTMain*)malloc(sizeof(ASTMain));
        current->next=temp;
        current=temp;
        current->next=NULL;
    }
    return root;
}

// Parse individual statements
ASTNode* parseStatement() {
    Token token = peekToken();
    //printf("parsing------ %s\n",token.value);
    if (token.type == TOKEN_INT) {
        return parseVarDeclaration();
    } else if (token.type == TOKEN_IDENTIFIER) {
        return parseAssignment();
    } else if (token.type == TOKEN_IF) {
        return parseIfStatement();
    }

    printf("Error: Unexpected token '%s'\n", token.value);
    exit(1);
}

// Parse variable declaration
ASTNode* parseVarDeclaration() {
    getNextToken();  // Consume 'int'
    Token identifier = getNextToken();

    if (identifier.type != TOKEN_IDENTIFIER) {
        printf("Error: Expected identifier after 'int'\n");
        exit(1);
    }

    Token semicolon = getNextToken();
    if (semicolon.type != TOKEN_SEMICOLON) {
        printf("Error: Expected ';' after variable declaration\n");
        exit(1);
    }
    if(rootTable == NULL){
        rootTable=(symbolTable*)malloc(sizeof(symbolTable));
        rootTable->next=NULL;
        strcpy(rootTable->sym,identifier.value);
        curr=rootTable;
    }else{
        symbolTable *temp=(symbolTable*)malloc(sizeof(symbolTable));
        strcpy(temp->sym,identifier.value);
        temp->next=NULL;
        curr->next=temp;
        curr=temp;
    }
    return createNode(NODE_VAR_DECL, identifier.value);
}

// Parse assignment statements
ASTNode* parseAssignment() {
    
    Token identifier = getNextToken();
    checkSymbol(identifier.value);
    if (identifier.type != TOKEN_IDENTIFIER) {
        printf("Error: Expected identifier for assignment\n");
        exit(1);
    }

    Token assign = getNextToken();
    if (assign.type != TOKEN_ASSIGN) {
        printf("Error: Expected '=' in assignment\n");
        exit(1);
    }

    ASTNode* expr = parseExpression();

    Token semicolon = getNextToken();
    if (semicolon.type != TOKEN_SEMICOLON) {
        printf("Error: Expected ';' after assignment\n");
        exit(1);
    }

    ASTNode* node = createNode(NODE_ASSIGN, "=");
    node->left = createNode(NODE_IDENTIFIER, identifier.value);
    node->right = expr;
    return node;
}

// Parse expressions (supports only addition for now)
ASTNode* parseExpression() {
    Token token = getNextToken();

    if (token.type == TOKEN_NUMBER) {
        return createNode(NODE_LITERAL, token.value);
    } else if (token.type == TOKEN_IDENTIFIER) {
        checkSymbol(token.value);
        ASTNode* node = createNode(NODE_IDENTIFIER, token.value);

        // Check for binary operators (+, -)
        Token nextToken = peekToken();
        if (nextToken.type == TOKEN_PLUS || nextToken.type == TOKEN_MINUS) {
            getNextToken();  // Consume the operator
            ASTNode* binaryNode = createNode(
                nextToken.type == TOKEN_PLUS ? NODE_BINARY_OP : NODE_BINARY_OP,
                nextToken.type == TOKEN_PLUS ? "+" : "-"
            );
            binaryNode->left = node;
            binaryNode->right = parseExpression();
            return binaryNode;
        }else if(nextToken.type == TOKEN_CONDITION){
            getNextToken();
            ASTNode* binaryNode = createNode(NODE_CONDITION,nextToken.value);
            binaryNode->left = node;
            binaryNode->right = parseExpression();
            return binaryNode;
        }
        return node;
    }

    printf("Error: Invalid expression\n");
    exit(1);
}

// Parse if statements
ASTNode* parseIfStatement() {
    getNextToken();  // Consume 'if'

    Token lparen = getNextToken();
    if (lparen.type != TOKEN_LPAREN) {
        printf("Error: Expected '(' after 'if'\n");
        exit(1);
    }

    ASTNode* condition = parseExpression();

    Token rparen = getNextToken();
    if (rparen.type != TOKEN_RPAREN) {
        printf("Error: Expected ')' after condition\n");
        exit(1);
    }

    Token lbrace = getNextToken();
    if (lbrace.type != TOKEN_LBRACE) {
        printf("Error: Expected '{' after condition\n");
        exit(1);
    }

    ASTNode* body = parseStatement();

    Token rbrace = getNextToken();
    if (rbrace.type != TOKEN_RBRACE) {
        printf("Error: Expected '}' after if body\n");
        exit(1);
    }

    ASTNode* node = createNode(NODE_IF, "if");
    node->condition = condition;
    node->body = body;
    return node;
}

// Create an AST node
ASTNode* createNode(ASTNodeType type, const char* value) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = type;
    strcpy(node->value, value);
    node->left = node->right = node->body = node->condition = NULL;
    return node;
}

// Print the AST
void AST(ASTNode* node, int indent) {
    if (!node) return;

    for (int i = 0; i < indent; i++) printf("  ");
    printf("%s\n", node->value);

    AST(node->left, indent + 1);
    AST(node->right, indent + 1);
    AST(node->condition, indent + 1);
    AST(node->body, indent + 1);

}
void printAST(ASTMain* node){
    while(node){
        AST(node->statement,0);
        node=node->next;
    }
}
void generateAssembly(ASTMain* node){
    fprintf(asmFile, ".text\n\n");
    while(node){
        Assembly(node->statement);
        node=node->next;
    }
    fprintf(asmFile, "out 0\n");
    fprintf(asmFile, "hlt\n");
    fprintf(asmFile, "\n.data\n\n");
    symbolTable* temp= rootTable;
    while(temp){
        fprintf(asmFile, "%s = 0\n",temp->sym);
        temp=temp->next;
    }
}
void Assembly(ASTNode* node){
    if(!node) return;

    switch(node->type){
        case NODE_VAR_DECL: break;
        case NODE_ASSIGN:generateAssignment(node); break;
        case NODE_IF:generateIfStatement(node); break;
        default: printf("Error: Unsupported AST node type\n"); break;

    }
}

void generateAssignment(ASTNode* node) {
    // Expect left child (variable) and right child (expression)
    if (!node->left || !node->right) {
        printf("Error: Invalid assignment node\n");
        exit(1);
    }

    generateExpression(node->right); // Generate code for the right-hand side
    fprintf(asmFile, "sta %%%s\n", node->left->value); // Store the result in the variable
                   // Restore the saved value
}

void generateExpression(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case NODE_LITERAL:
            generateLiteral(node);
            break;
        case NODE_IDENTIFIER:
            generateIdentifier(node);
            break;
        case NODE_BINARY_OP:
            if (node->left && node->right) {
                //fprintf(asmFile, "PUSH\n");              // Save the current value
                generateExpression(node->left); // Left operand
                fprintf(asmFile, "ldi B %s\n",node->right->value);
                if (strcmp(node->value, "+") == 0) {
                    fprintf(asmFile, "add\n");       // Add the two values
                } else if (strcmp(node->value, "-") == 0) {
                    fprintf(asmFile, "sub\n");       // Subtract the two values
                } else {
                    printf("Error: Unsupported binary operator '%s'\n", node->value);
                    exit(1);
                }
                //fprintf(asmFile, "POP\n");               // Restore the saved value
            }
            break;
        case NODE_CONDITION:
            if (node->left && node->right) {
                generateExpression(node->left);
                fprintf(asmFile, "ldi B %s\n",node->right->value);
            }
            break;
        default:
            printf("Error: Unsupported expression type- %s\n",node->value);
            exit(1);
    }
}

void generateLiteral(ASTNode* node) {
    fprintf(asmFile, "ldi A %s\n", node->value); // Load literal into accumulator
}

void generateIdentifier(ASTNode* node) {
    fprintf(asmFile, "lda %%%s\n", node->value); // Load variable value into accumulator
}

void generateIfStatement(ASTNode* node) {
    if (!node->condition || !node->body) {
        printf("Error: Invalid if statement\n");
        exit(1);
    }

    // Generate condition evaluation
    generateExpression(node->condition);

    // Output comparison instruction
    static int labelCount = 0;
    int labelNum = labelCount++;
    fprintf(asmFile, "cmp\n");                  // Compare accumulator with the condition's value
    if(strcmp(node->condition->value,"==")==0){
        fprintf(asmFile, "je %%IF_TRUE_%d\n", labelNum); // Jump to IF_TRUE if condition is true
    }else fprintf(asmFile, "jne %%IF_TRUE_%d\n", labelNum);
    fprintf(asmFile, "jmp %%END_IF_%d\n", labelNum);  // Jump to the end if the condition is false

    // Generate the 'if' body
    fprintf(asmFile, "IF_TRUE_%d:\n", labelNum);
    Assembly(node->body);

    // End label
    fprintf(asmFile, "END_IF_%d:\n", labelNum);
}

// Main function
int main(int argc, char *argv[]) {
    int flag=0;
    if(argc<2){
         printf("Usage %s <filename>\n",argv[0]);
        return 1;
    }
    if(argc==3 && strcmp(argv[2],"d")==0) flag=1;
    printf("%s\n", argv[1]);
    FILE *file = fopen(argv[1],"rb");
    if(file == NULL){
        printf("error opening file\n");
        return 1;
    }
    fseek(file,0,SEEK_END);
    long fsize=ftell(file);
    fseek(file,0,SEEK_SET);

    char *str=malloc(fsize+1);
    fread(str,fsize,1,file);
    fclose(file);

    printf("Tokenizing SimpleLang code...\n");
    tokenize(str);
    if(flag) printTokens();

    currentTokenIndex=0;

    ASTMain* ast = parseProgram();
    if(flag) printAST(ast);

    asmFile=fopen("a.asm","w");
    generateAssembly(ast);
    fclose(asmFile);
    system("python asm\\asm.py a.asm > memory.list");
    system("mingw32-make clean");
    system("mingw32-make run");
    return 0;
}
