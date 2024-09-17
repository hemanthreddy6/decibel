#include "../lex.yy.c"
#include <stdio.h>

int main() {
  int res;
  while ((res = yylex())) {
    switch (res) {
    case IMPORT:
      printf("IMPORT\n");
      break;
    case CONST:
      printf("CONST\n");
      break;
    case LOAD:
      printf("LOAD\n");
      break;
    case SAVE:
      printf("SAVE\n");
      break;
    case PLAY:
      printf("PLAY\n");
      break;
    case FUNCTION:
      printf("FUNCTION\n");
      break;
    case IF:
      printf("IF\n");
      break;
    case OR:
      printf("OR\n");
      break;
    case OTHERWISE:
      printf("OTHERWISE\n");
      break;
    case LOOP:
      printf("LOOP\n");
      break;
    case OVER:
      printf("OVER\n");
      break;
    case LONG:
      printf("LONG\n");
      break;
    case INT:
      printf("INT\n");
      break;
    case FLOAT:
      printf("FLOAT\n");
      break;
    case STRING:
      printf("STRING\n");
      break;
    case AUDIO:
      printf("AUDIO\n");
      break;
    case BOOL:
      printf("BOOL\n");
      break;
    case TRUE:
      printf("TRUE\n");
      break;
    case FALSE:
      printf("FALSE\n");
      break;
    case CONTINUE:
      printf("CONTINUE\n");
      break;
    case BREAK:
      printf("BREAK\n");
      break;
    case RETURN:
      printf("RETURN\n");
      break;
    case HIGHPASS:
      printf("HIGHPASS\n");
      break;
    case LOWPASS:
      printf("LOWPASS\n");
      break;
    case EQ:
      printf("EQ\n");
      break;
    case SIN:
      printf("SIN\n");
      break;
    case COS:
      printf("COS\n");
      break;
    case EXP_DECAY:
      printf("EXP_DECAY\n");
      break;
    case LIN_DECAY:
      printf("LIN_DECAY\n");
      break;
    case SQUARE:
      printf("SQUARE\n");
      break;
    case SAW:
      printf("SAW\n");
      break;
    case TRIANGLE:
      printf("TRIANGLE\n");
      break;
    case PAN:
      printf("PAN\n");
      break;
    case TO:
      printf("TO\n");
      break;

    case SPEEDUP:
      printf(">>\n");
      break;
    case SPEEDDOWN:
      printf("<<\n");
      break;
    case LEQ:
      printf("<=\n");
      break;
    case GEQ:
      printf(">=\n");
      break;
    case EQUALS:
      printf("==\n");
      break;
    case NOT_EQUALS:
      printf("!=\n");
      break;
    case LOGICAL_AND:
      printf("&&\n");
      break;
    case LOGICAL_OR:
      printf("||\n");
      break;
    case POWER_EQUALS:
      printf("^=\n");
      break;
    case DISTORTION_EQUALS:
      printf("&=\n");
      break;
    case MULT_EQUALS:
      printf("*=\n");
      break;
    case DIVIDE_EQUALS:
      printf("/=\n");
      break;
    case MOD_EQUALS:
      printf("\%=\n");
      break;
    case PLUS_EQUALS:
      printf("+=\n");
      break;
    case MINUS_EQUALS:
      printf("-=\n");
      break;
    case OR_EQUALS:
      printf("|=\n");
      break;
    case RIGHT_ARROW:
      printf("->\n");
      break;
    case LEFT_ARROW:
      printf("<-\n");
      break;
    case IMPLIES:
      printf("=>\n");
      break;

    case IDENTIFIER:
      printf("IDENTIFIER\n");
      break;
    case INT_LITERAL:
      printf("INT_LITERAL\n");
      break;
    case FLOAT_LITERAL:
      printf("FLOAT_LITERAL\n");
      break;
    case STRING_LITERAL:
      printf("STRING_LITERAL\n");
      break;
    case INVALID_SYMBOL:
      printf("INVALID_SYMBOL\n");
      break;

    default:
      printf("%c\n", res);
      break;
    }
  }
}
