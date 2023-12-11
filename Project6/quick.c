#include <stdio.h>
#include <stdlib.h>

#include "parserer.h"
#include "lexer.h"
#include "utils.h"

int main() {
	char* inbuf = loadFile("1.q");
	puts(inbuf);
	tokenize(inbuf);
	showTokens();
	free(inbuf);
	parse();
	return 0;
}