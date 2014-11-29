#include <stdio.h>
#include "cdecl.h"

void test_file(char* filename, char* cdecl_str) {
  freopen(filename, "r", stdin);
  FILE* pFile;
  pFile = tmpfile();
  cdecler(stdin, pFile);
  rewind(pFile);
  fgets(cdecl_str, 256, pFile);
  fclose(pFile);
  return;
}

void test_cdecler_1(void) {
  char obtained_str[256];
  test_file("test_file_1.txt", obtained_str);
  char expected_str[] = "next is pointer to a function returning "
                        "pointer to read-only pointer to char static\n";
  if (strcmp(obtained_str, expected_str)) 
    printf("TEST 1 ERROR expected cdecl and obtained output string differ:\n%s\n%s\n", 
            expected_str, obtained_str);
  return;
}

void test_cdecler_2(void) {
  char obtained_str[256];
  test_file("test_file_2.txt", obtained_str);
  char expected_str[] = "c is an array[0..9] of pointer to a function "
                        "(getting in input pointer to pointer to int, and pointer to char,"
                        " and an array[0..9] of int, and an array of double long, and pointer to float) returning pointer to char\n";
  if (strcmp(obtained_str, expected_str)) 
    printf("TEST 2 ERROR expected cdecl and obtained output string differ:\n%s\n%s\n", 
            expected_str, obtained_str);
  return;
}

int main() {
  test_cdecler_1();
  test_cdecler_2();
  return 0;
}  
