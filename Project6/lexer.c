#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include "lexer.h"
#include "utils.h"

Token tokens[MAX_TOKENS];
int nTokens;

int line = 1;		// the current line in the input file

// adds a token to the end of the tokens list and returns it
// sets its code and line
Token* addTk(int code) {
	if (nTokens == MAX_TOKENS)
		err("too many tokens");
	Token* tk = &tokens[nTokens];
	tk->code = code;
	tk->line = line;
	nTokens++;
	return tk;
}

// copy in the dst buffer the string between [begin,end)
char* copyn(char* dst, const char* begin, const char* end) {
	char* p = dst;
	if (end - begin > MAX_STR)
		err("string too long");
	while (begin != end)
		*p++ = *begin++;
	*p = '\0';
	return dst;
}

void tokenize(const char* pch) {
	const char* start;
	Token* tk;
	char buf[MAX_STR + 1];
	for (;;) {
		switch (*pch) {
		case ' ':
		case '\t':
			pch++;
			break;
		case '\r':		// handles different kinds of newlines (Windows: \r\n, Linux: \n, MacOS, OS X: \r or \n)
			if (pch[1] == '\n')pch++;
			// fallthrough to \n
		case '\n':
			line++;
			pch++;
			break;
		case '\0':
			addTk(FINISH);
			return;
		case ',':
			addTk(COMMA);
			pch++;
			break;
		case '=':
			if (pch[1] == '=') {
				addTk(EQUAL);
				pch += 2;
			}
			else {
				addTk(ASSIGN);
				pch++;
			}
			break;
		case '#':
			for (pch++; *pch != '\r' && *pch != '\n' && *pch != '\0'; pch++) {}
			break;
		case '"':
			for (start = ++pch; *pch != '"'; pch++) {}
			if (*pch == '\0')
				err("end of file while in string");
			tk = addTk(STR);
			copyn(tk->text, start, pch);
			pch++;
			break;
		case '(':
			tk = addTk(LPAR);
			pch++;
			break;
		case ')':
			tk = addTk(RPAR);
			pch++;
			break;
		case ':':
			tk = addTk(COLON);
			pch++;
			break;
		case '>':
			if (pch[1] == '=') {
				addTk(GREATEREQ);
				pch += 2;
			}
			else {
				addTk(GREATER);
				pch++;
			}
		case '<':
			tk = addTk(LESS);
			pch++;
			break;
		case ';':
			tk = addTk(SEMICOLON);
			pch++;
			break;
		case '+':
			tk = addTk(ADD);
			pch++;
			break;
		default:
			if (isalpha(*pch) || *pch == '_') {
				for (start = pch++; isalnum(*pch) || *pch == '_'; pch++) {}
				char* text = copyn(buf, start, pch);
				if (strcmp(text, "int") == 0)
					addTk(INT);
				else if (strcmp(text, "function") == 0)
					addTk(FUNCTION);
				else if (strcmp(text, "if") == 0)
					addTk(IF);
				else if (strcmp(text, "else") == 0)
					addTk(ELSE);
				else if (strcmp(text, "while") == 0)
					addTk(WHILE);
				else if (strcmp(text, "end") == 0)
					addTk(END);
				else if (strcmp(text, "real") == 0)
					addTk(REAL);
				else if (strcmp(text, "string") == 0)
					addTk(STR);
				else if (strcmp(text, "var") == 0)
					addTk(VAR);
				else if (strcmp(text, "return") == 0)
					addTk(RETURN);
				else {
					tk = addTk(ID);
					strcpy(tk->text, text);
				}
			}
			else if (isdigit(*pch)) {
				for (start = pch++; isdigit(*pch); pch++) {}
				if (*pch == '.') {
					for (pch++; isdigit(*pch); pch++) {}
					char* text = copyn(buf, start, pch);
					tk = addTk(REAL);
					tk->r = atof(text);
				}
				else {
					char* text = copyn(buf, start, pch);
					tk = addTk(INT);
					tk->i = atof(text);
				}
			}
			else err("invalid char: %c (%d)", *pch, *pch);
		}
	}
}

void showTokens()
{
	for (int i = 0; i < nTokens; i++) {
		Token* tk = &tokens[i];
		switch (tk->code)
		{
		case ID:
			printf("%d ID : %s\n", tk->line, tk->text);
			break;
		case FUNCTION:
			printf("%d FUNCTION\n", tk->line);
			break;
		case ELSE:
			printf("%d ELSE\n", tk->line);
			break;
		case IF:
			printf("%d IF\n", tk->line);
			break;
		case RETURN:
			printf("%d RETURN\n", tk->line);
			break;
		case WHILE:
			printf("%d WHILE\n", tk->line);
			break;
		case LPAR:
			printf("%d LPAR\n", tk->line);
			break;
		case COMMA:
			printf("%d COMMA\n", tk->line);
			break;
		case SEMICOLON:
			printf("%d SEMICOLON\n", tk->line);
			break;
		case RPAR:
			printf("%d RPAR\n", tk->line);
			break;
		case ADD:
			printf("%d ADD\n", tk->line);
			break;
		case VAR:
			printf("%d VAR\n", tk->line);
			break;
		case END:
			printf("%d END\n", tk->line);
			break;
		case AND:
			printf("%d AND\n", tk->line);
			break;
		case STR:
			printf("%d STR: %s\n", tk->line, tk->text);
			break;
		case NOT:
			printf("%d NOT\n", tk->line);
			break;
		case ASSIGN:
			printf("%d ASSIGN\n", tk->line);
			break;
		case EQUAL:
			printf("%d EQUAL\n", tk->line);
			break;
		case COLON:
			printf("%d COLON\n", tk->line);
			break;
		case LESS:
			printf("%d LESS\n", tk->line);
			break;
		case GREATER:
			printf("%d GREATER\n", tk->line);
			break;
		case INT:
			printf("%d INT: %d\n", tk->line, tk->i);
			break;
		case REAL:
			printf("%d REAL: %f\n", tk->line, tk->r);
			break;
		case FINISH:
			printf("%d FINISH\n", tk->line);
			break;
		}
	}
}