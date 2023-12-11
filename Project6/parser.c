#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "lexer.h"

bool factor();
bool exprPrefix();
bool exprMul();
bool exprAdd();
bool exprComp();
bool exprAssign();
bool exprLogic();
bool expr();
bool instr();
bool funcParam();
bool funcParams();
bool block();
bool baseType();
bool defVar();
bool defFunc();
bool program();

int iTk;
Token* consumed;

void tkerr(const char* fmt, ...) {
    printf(stderr, "error in line %d: ", tokens[iTk].line);
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
    printf(stderr, "\n");
    exit(EXIT_FAILURE);
}

bool consume(int code) {
    if (tokens[iTk].code == code) {
        consumed = &tokens[iTk++];
        return true;
    }
    return false;
}

/*
factor ::= INT
        | REAL
        | STR
        | LPAR expr RPAR
        | ID ( LPAR ( expr ( COMMA expr )* )? RPAR )?
*/
bool factor() {
    if (consume(INT)) {
        return true;
    }
    if (consume(REAL)) {
        return true;
    }
    if (consume(STR)) {
        return true;
    }
    if (consume(LPAR)) {
        if (expr()) {
            if (consume(RPAR)) {
                return true;
            }
            else tkerr("missing ) or invalid argument after (");
        }
        else tkerr("Invalid/missing argument after (");
    }
    if (consume(ID)) {
        if (consume(LPAR)) {
            if (expr()) {
                while (consume(COMMA)) {
                    if (expr()) {
                    }
                    else tkerr("Invalid/missing call argument after ,");
                }
            }
            if (consume(RPAR)) {
            }
            else tkerr("missing ) or invalid call argument");
        }
        return true;
    }
    return false;
}

//exprPrefix ::= ( SUB | NOT )? factor
bool exprPrefix() {
    if (consume(SUB)) {
        if (factor()) {
            return true;
        }
        else tkerr("Invalid/missing expression after -");
    }
    if (consume(NOT)) {
        if (factor()) {
            return true;
        }
        else tkerr("Invalid/missing expression after ^");
    }
    return factor();
}

//exprMul ::= exprPrefix ( ( MUL | DIV ) exprPrefix )*
bool exprMul() {
    if (exprPrefix()) {
        for (;;) {
            const char* opName;
            if (consume(MUL)) {
                opName = "*";
            }
            else if (consume(DIV)) {
                opName = "/";
            }
            else break;
            if (exprPrefix()) {
            }
            else tkerr("Invalid/missing expression after %s", opName);
        }
        return true;
    }
    return false;
}

//exprAdd ::= exprMul ( ( ADD | SUB ) exprMul )*
bool exprAdd() {
    if (exprMul()) {
        for (;;) {
            const char* opName;
            if (consume(ADD)) {
                opName = "+";
            }
            else if (consume(SUB)) {
                opName = "-";
            }
            else break;
            if (exprMul()) {
            }
            else tkerr("Invalid/missing expression after %s", opName);
        }
        return true;
    }
    return false;
}

//exprComp ::= exprAdd ( ( LESS | EQUAL ) exprAdd )?
bool exprComp() {
    if (exprAdd()) {
        for (;;) {
            const char* opName;
            if (consume(LESS)) {
                opName = "<";
            }
            else if (consume(EQUAL)) {
                opName = "==";
            }
            else return true;
            if (exprAdd()) {
            }
            else tkerr("Invalid/missing expression after %s", opName);
        }
        return true;
    }
    return false;
}

//exprAssign ::= ( ID ASSIGN )? exprComp
bool exprAssign() {
    int iStart = iTk;
    if (consume(ID)) {
        if (consume(ASSIGN)) {
            if (exprComp()) {
                return true;
            }
            else tkerr("Invalid/missing expression after =");
        }
        iTk = iStart;
    }
    return exprComp();
}

//exprLogic ::= exprAssign ( ( AND | OR ) exprAssign )*
bool exprLogic() {
    if (exprAssign()) {
        for (;;) {
            const char* opName;
            if (consume(AND)) {
                opName = "&&";
            }
            else if (consume(OR)) {
                opName = "||";
            }
            else break;
            if (exprAssign()) {
            }
            else tkerr("Invalid/missing expression after %s", opName);
        }
        return true;
    }
    return false;
}

//expr ::= exprLogic
bool expr() {
    return exprLogic();
}

/*
instr ::= expr? SEMICOLON
        | IF LPAR expr RPAR block ( ELSE block )? END
        | RETURN expr SEMICOLON
        | WHILE LPAR expr RPAR block END
*/
bool instr() {

    if (expr()) {
        if (consume(SEMICOLON)) {
            return true;
        }
        else tkerr("missing ; after expression");
    }
    else if (consume(SEMICOLON)) {
        return true;
    }
    else if (consume(IF)) {
        if (consume(LPAR)) {
            if (expr()) {
                if (consume(RPAR)) {
                    if (block()) {
                        if (consume(ELSE)) {
                            if (block()) {
                            }
                            else tkerr("Invalid/missing else block");
                        }
                        if (consume(END)) {
                            return true;
                        }
                        else tkerr("missing end after if body");
                    }
                    else tkerr("invalid/missing if body");
                }
                else tkerr("missing ) after if condition");
            }
            else tkerr("invalid/missing id condition");
        }
        else tkerr("missing ( after if");
    }
    else if (consume(RETURN)) {
        if (expr()) {
            if (consume(SEMICOLON)) {
                return true;
            }
            else tkerr("missing ; after the return expression");
        }
        else tkerr("invalid/missing expression after return");
    }
    else if (consume(WHILE)) {
        if (consume(LPAR)) {
            if (expr()) {
                if (consume(RPAR)) {
                    if (block()) {
                        if (consume(END)) {
                            return true;
                        }
                        else tkerr("missing end after while loop");
                    }
                    else tkerr("invalid/missing while block");
                }
                else tkerr("missing ) after while condition");
            }
            else tkerr("invalid/missing while condition");
        }
        else tkerr("missing ( after while");
    }
    return false;
}

//funcParam ::= ID COLON baseType
bool funcParam() {
    if (consume(ID)) {
        if (consume(COLON)) {
            if (baseType()) {
                return true;
            }
            else tkerr("Invalid/missing parameter type");
        }
        else tkerr("missing : after parameter name");
    }
    return false;
}

//funcParams ::= funcParam ( COMMA funcParam )*
bool funcParams() {
    if (funcParam()) {
        while (consume(COMMA)) {
            if (funcParam()) {
            }
            else tkerr("Invalid/missing function parameter after ,");
        }
    }
    return false;
}

//block :: = instr +
bool block() {
    if (instr()) {
        while (instr()) {}
        return true;
    }
    return false;
}

//baseType ::= TYPE_INT | TYPE_REAL | TYPE_STR
bool baseType() {
    if (consume(INT)) {
        return true;
    }
    if (consume(REAL)) {
        return true;
    }
    if (consume(STR)) {
        return true;
    }
    return false;
}

//defVar ::= VAR ID COLON baseType SEMICOLON
bool defVar() {
    if (consume(VAR)) {
        if (consume(ID)) {
            if (consume(COLON)) {
                if (baseType()) {
                    if (consume(SEMICOLON)) {
                        return true;
                    }
                    else tkerr("missing ; after var type");
                }
                else tkerr("invalid/missing var type");
            }
            else tkerr("missing : after var name");
        }
        else tkerr("missing var name");
    }
    return false;
}

//defFunc ::= FUNCTION ID LPAR funcParams? RPAR COLON baseType defVar* block END
bool defFunc() {
    if (consume(FUNCTION)) {
        if (consume(ID)) {
            if (consume(LPAR)) {
                funcParams();
                if (consume(RPAR)) {
                    if (consume(COLON)) {
                        if (baseType()) {
                            while (defVar()) {
                            }
                            if (block()) {
                                if (consume(END)) {
                                    return true;
                                }
                                else tkerr("missing end after function body");
                            }
                            else tkerr("invalid/missing function body");
                        }
                        else tkerr("invalid/missing function return type after :");
                    }
                    else tkerr("missing : before function return type");
                }
                else tkerr("invalid/missing function parameters or missing )");
            }
            else tkerr("missing ( after function name");
        }
        else tkerr("missing function name");
    }
    return false;
}

//program ::= ( defVar | defFunc | block )* FINISH
bool program() {
    for (;;) {
        if (defVar()) {}
        else if (defFunc()) {}
        else if (block()) {}
        else break;
    }
    if (consume(FINISH)) {
        printf("\n ---------- SYNTAX OK ---------- \n");
        return true;
    }
    else tkerr("\n ---------- SYNTAX ERROR ---------- \n");
    return false;
}

void parse() {
    iTk = 0;
    program();
}