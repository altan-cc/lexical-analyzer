/*
Assignment :
lex - Lexical Analyzer for PL /0
Author : Chen-An Chang, Luciano Paredes
Language : C ( only )
To Compile :
gcc - O2 - std = c11
-o lex lex . c
To Execute ( on Eustis ):
./ lex < input file >
where :
< input file > is the path to the PL /0 source program
Notes :
- Implement a lexical analyser for the PL /0 language .
- The program must detect errors such as
- numbers longer than five digits
- identifiers longer than eleven characters
- invalid characters .
- The output format must exactly match the specification .
- Tested on Eustis .
Class : COP 3402 - System Software - Fall 2025
Instructor : Dr . Jie Lin
Due Date : Friday , October 3 , 2025 at 11:59 PM ET
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_TOKENS 500
#define MAX_LEN    64

// tokenType enumeration
typedef enum {
    skipsym = 1,
    identsym,
    numbersym,
    plussym,
    minussym,
    multsym,
    slashsym,
    eqsym,
    neqsym,
    lessym,
    leqsym,
    gtrsym,
    geqsym,
    lparentsym,
    rparentsym,
    commasym,
    semicolonsym,
    periodsym,
    becomessym,
    beginsym,
    endsym,
    ifsym,
    fisym,
    thensym,
    whilesym,
    dosym,
    callsym,
    constsym,
    varsym,
    procsym,
    writesym,
    readsym,
    elsesym,
    evensym
} TokenType;

// reserved words
typedef struct {
    const char *word;
    int token;
} resWord;

static resWord reserved[] = {
    {"begin", beginsym},
    {"end", endsym},
    {"if", ifsym},
    {"fi", fisym},
    {"then", thensym},
    {"while", whilesym},
    {"do", dosym},
    {"call", callsym},
    {"const", constsym},
    {"var", varsym},
    {"procedure", procsym},
    {"write", writesym},
    {"read", readsym},
    {"else", elsesym},
    {"even", evensym},
    {NULL, 0}
};

// token storage
char lexeme[MAX_TOKENS][MAX_LEN];   // sequence of input characters that match pattern of token
char pattern[MAX_TOKENS][MAX_LEN];  // description of form that lexemes of token may take
char attr[MAX_TOKENS][MAX_LEN];     // pointer to symbol table
int tokenSym[MAX_TOKENS];           // internal representation of token (number, number, operator)
int count = 0;

// add one token
void addToken(const char* lex, const char* pat, int sym, const char* att) {
    strcpy(lexeme[count], lex);
    strcpy(pattern[count], pat);
    tokenSym[count] = sym;
    if (att) {
        strcpy(attr[count], att);
    }
    else {
        attr[count][0] = '\0';
    }
    count++;
}

// check reserved words
int checkReserved(const char* s) {
    int i = 0;
    while (reserved[i].word) {
        if (strcmp(s, reserved[i].word) == 0) {
            return reserved[i].token;
        }
        i++;
    }
    return 0;
}

// special symbols
int specialSym(const char* s, int* len) {
    if (s[0] == '+') {
        *len = 1;
        return plussym;
    }
    if (s[0] == '-') {
        *len = 1;
        return minussym;
    }
    if (s[0] == '*') {
        *len = 1;
        return multsym;
    }
    if (s[0] == '/') {
        *len = 1;
        return slashsym;
    }
    if (s[0] == '=') {
        *len = 1;
        return eqsym;
    }
    if (s[0] == '<') {
        if (s[1] == '=') {
            *len = 2;
            return leqsym;
        }
        if (s[1] == '>') {
            *len = 2;
            return neqsym;
        }
        *len = 1;
        return lessym;
    }
    if (s[0] == '>') {
        if (s[1] == '=') {
            *len = 2;
            return geqsym;
        }
        *len = 1;
        return gtrsym;
    }
    if (s[0] == '(') {
        *len = 1;
        return lparentsym;
    }
    if (s[0] == ')') {
        *len = 1;
        return rparentsym;
    }
    if (s[0] == ',') {
        *len = 1;
        return commasym;
    }
    if (s[0] == ';') {
        *len = 1;
        return semicolonsym;
    }
    if (s[0] == '.') {
        *len = 1;
        return periodsym;
    }
    if (s[0] == ':' && s[1] == '=') {
        *len = 2;
        return becomessym;
    }
    *len = 0; // no match
    return 0;
}

// scanner
void scan(const char* src) {
    const char* p = src;
    while (*p) {
        // // skip whitespace and special characters
        if (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') {
            p++;
            continue;
        }

        // skip comments
        if (p[0] == '/' && p[1] == '*') {
            p+=2;
            while (*p) {
                if (p[0]=='*' && p[1]=='/') {
                    p+=2;
                    break;
                }
                p++;
            }
            continue;
        }

        // skip identifiers
        if (isalpha((unsigned char)* p)) {
            char buff[MAX_LEN];
            int i = 0;

            // we continue until we get all of our alphanumeric characters
            while (isalnum((unsigned char)* p)) {
                buff[i] = *p++;
                i++;
            }

            // we finish the string with a '\0' and check if the identifier has more than 11 characters
            buff[i] = '\0';
            if (i > 11) {
                addToken(buff, "Identifier too long", skipsym, NULL);
            }
            else {
                // if the identifier is a reserved word we look for it's token
                int rw = checkReserved(buff);
                if (rw) {
                    char num[8];
                    sprintf(num, "%d", rw);
                    addToken(buff, num, rw, NULL);
                }
                else {
                    // otherwise we store it as an identifier token
                    addToken(buff, "2", identsym, buff);
                }
            }
            continue;
        }

        // skip numbers
        if (isdigit((unsigned char)*p)) {
            char buff[MAX_LEN];
            int i = 0;

            // we iterate until we find all following numbers
            while (isdigit((unsigned char)*p)) {
                buff[i]= *p++;
                i++;
            }

            // we finish the string and check if the number is too long and has more than 5 digits
            buff[i] = '\0';
            if (i>5) {
                addToken(buff, "Number too long", skipsym, NULL);
            }
            else {
                // we add the number as a number token
                addToken(buff, "3", numbersym, buff);
            }
            continue;
        }

        // skip special symbols
        int len = 0, tok = specialSym(p, &len);
        if (tok) {
            // special symbols have at most 2 characters, we allocate one more for '\0'
            char buff[3];
            strncpy(buff, p, len);
            buff[len] = '\0';

            // we convert the symbol to a character number represenation and add it as a token
            char num[8];
            sprintf(num, "%d", tok);
            addToken(buff, num, tok, NULL);
            p += len;
            continue;
        }

        //invalid symbol
        char buff[2] = {*p ,0};
        addToken(buff, "Invalid symbol", skipsym, NULL);
        p++;
    }
}

int main(int argc, char* argv[]) {
    // we close the program if it doesn't have exactly 2 args. 
    // the first argument is always the name of the program.
    if (argc != 2) {
        fprintf(stderr, "Usage: %s sourcefile\n", argv[0]);
        return 1;
    }

    // we open the file provided in the first argument in read mode
    FILE* f = fopen(argv[1],"r");
    if (!f) {
        perror("fopen");
        return 1;
    }

    // we allocate some memory to store our program's text so we can print it
    char* src = malloc(100000);
    size_t n = fread(src, 1, 100000-1, f);
    src[n]='\0';
    fclose(f);

    printf("Source Program:\n\n%s\n\n", src);

    // all the tokenizing happens here
    scan(src);

    // print all output formatted as stated in the assignment
    printf("Lexeme Table:\n\n");
    printf("lexeme\ttoken type\n");
    for (int i = 0; i < count; i++) {
        printf("%s\t%s\n", lexeme[i], pattern[i]);
    }

    printf("Token List:\n\n");
    for (int i = 0; i < count;i ++) {
        if (i) {
            printf(" ");
        }
        printf("%d", tokenSym[i]);
        if ((tokenSym[i] == identsym || tokenSym[i] == numbersym) && attr[i][0]) {
            printf(" %s", attr[i]);
        }
    }
    printf("\n");

    free(src);
    return 0;
}
