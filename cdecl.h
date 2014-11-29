#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#define MAXTOKENLEN 100
#define MAXTOKENSNR 256

typedef enum { IDENTIFIER, QUALIFIER, TYPE } type_tag;

typedef struct Token {
  char type;
  char string[MAXTOKENLEN];
} Token;
Token last_token;

static int top = -1;
static Token stack[MAXTOKENSNR];
static inline Token* pop(void) { return &stack[top--]; }
static inline void push(Token s) { stack[++top] = s; }

static int top_parameters = -1;
static Token parameters_stack[MAXTOKENSNR];
static inline Token* pop_parameter(void) { return &parameters_stack[top_parameters--]; }
static inline void push_parameter(Token s) { parameters_stack[++top_parameters] = s; }


void deal_with_param_declarator(FILE* istream, FILE* ostream);
void deal_with_declarator(FILE* istream, FILE* ostream);

type_tag classify_string(void) {
  char *p = last_token.string;

  if (!strcmp(p, "const")) {
    snprintf(last_token.string, sizeof(last_token.string), "read-only");
    return QUALIFIER;
  }

  if (!strcmp(p, "volatile") ||
      !strcmp(p, "static")) return QUALIFIER;

  if (!strcmp(p, "void") ||
      !strcmp(p, "char") ||
      !strcmp(p, "signed") ||
      !strcmp(p, "unsigned") ||
      !strcmp(p, "short") ||
      !strcmp(p, "int") ||
      !strcmp(p, "long") ||
      !strcmp(p, "float") ||
      !strcmp(p, "double") ||
      !strcmp(p, "struct") ||
      !strcmp(p, "union") ||
      !strcmp(p, "enum"))
    return TYPE;

  return IDENTIFIER;
}

void get_token(FILE* istream) {
  char* p = last_token.string;
  while ((*p = getc(istream)) == ' ');
  if (isalnum(*p)) {
    while (isalnum(*++p = getc(istream)));
    ungetc(*p, istream);
    *p = '\0';
    last_token.type = classify_string();
    return;
  }

  if (*p == '*') {
    snprintf(last_token.string, sizeof(last_token.string), " pointer to");
    last_token.type = '*';
    return;
  }

  last_token.string[1] = '\0';
  last_token.type = *p;
  return;
}

void deal_with_arrays(FILE* istream, FILE* ostream) {
  while (last_token.type == '[') {
    fprintf(ostream, " an array");
    get_token(istream);
    if (isdigit(last_token.string[0])) {
      fprintf(ostream, "[0..%d]", atoi(last_token.string)-1);
      get_token(istream);
    }
    get_token(istream);
    fprintf(ostream, " of");
  }
}

void deal_with_param_arrays(FILE* istream, FILE* ostream) {
  pop_parameter();
  fprintf(ostream, " an array");
  if (isdigit(parameters_stack[top_parameters].string[0])) {
    fprintf(ostream, "[0..%d]", atoi(parameters_stack[top_parameters].string)-1);
    pop_parameter();
  }
  pop_parameter();
  fprintf(ostream, " of");
}

void deal_with_function_args(FILE* istream, FILE* ostream) {
  fprintf(ostream, " a function");
  get_token(istream);
  if (last_token.type != ')') {
    fprintf(ostream, " (getting in input");
    for (;;) {
      if (last_token.type == ')')
        break;
      if (last_token.type == ',') {
        deal_with_param_declarator(istream, ostream);
        fprintf(ostream, ", and");
      } else {
        push_parameter(last_token);
      }
      get_token(istream);
    }
    if (top_parameters > 0) {
      deal_with_param_declarator(istream, ostream);
      fprintf(ostream, ")");
    }
  }
  fprintf(ostream, " returning");
  get_token(istream);
}

void deal_with_pointers(FILE* ostream) {
  while (stack[top].type == '*')
    fprintf(ostream, "%s", (*pop()).string);
}

void deal_with_param_pointers(FILE* ostream) {
  while (parameters_stack[top_parameters].type == '*')
    fprintf(ostream, "%s", (*pop_parameter()).string);
}

void deal_with_param_declarator(FILE* istream, FILE* ostream) {

  deal_with_param_pointers(ostream);

  while (top_parameters >= 0) {
    if (parameters_stack[top_parameters].type == '(') {
      pop_parameter();
      get_token(istream);
      deal_with_param_declarator(istream, ostream);
    } else if (parameters_stack[top_parameters].type == IDENTIFIER) {
      pop_parameter();
    } else if (parameters_stack[top_parameters].type == ']') {
      deal_with_param_arrays(istream, ostream);
    } else {
      if (parameters_stack[top_parameters].type == TYPE ||
          parameters_stack[top_parameters].type == QUALIFIER)
        fprintf(ostream, " ");
      fprintf(ostream, "%s", (*pop_parameter()).string);
    }
  }
};

void deal_with_declarator(FILE* istream, FILE* ostream) {
  if (last_token.type == '[')
    deal_with_arrays(istream, ostream);
  else if (last_token.type == '(')
    deal_with_function_args(istream, ostream);

  deal_with_pointers(ostream);

  while (top >= 0) {
    if (stack[top].type == '(') {
      pop();
      get_token(istream);
      deal_with_declarator(istream, ostream);
    } else {
      if (stack[top].type == TYPE || stack[top].type == QUALIFIER)
        fprintf(ostream, " ");
      fprintf(ostream, "%s", (*pop()).string);
    }
  }
};

void read_to_first_identifier(FILE* istream, FILE* ostream) {
  get_token(istream);
  while (last_token.type != IDENTIFIER) {
    push(last_token);
    get_token(istream);
  }
  fprintf(ostream, "%s is", last_token.string);
  get_token(istream);
}

void cdecler(FILE* istream, FILE* ostream) {
  read_to_first_identifier(istream, ostream);
  deal_with_declarator(istream, ostream);
  fprintf(ostream, "\n");
  fflush(ostream);
}
