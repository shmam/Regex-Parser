/**
 * @file: regular.c
 * @author: sdcroche
 *
 * Regular is the main component of the program, reading in user input
 * through either a file or standard input and reports the matches for
 * the input with a given regex
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pattern.h"
#include "parse.h"

// On the command line, which argument is the pattern.
#define PAT_ARG 1

// On the command line, which argument is the input file.
#define FILE_ARG 2

/** length of buffer string to read input */
#define BUFFLEN 200

/** valid line length */
#define LINELEN 100

#define ARGCFILE 3
#define ARGCNOFILE 2

/**
 * Report matches is responsible for providing the correct formatted output
 * for users to identify the matchex string given a regex
 *
 * @param pat pattern to use to match
 * @param pstr string of the regex pattern
 * @param string to detect and highlight matches for
 */
void reportMatches( Pattern *pat, char const *pstr, char const *str)
{

  // make the transition characters from red to white a character array
  char red[] = "\033[31m";

  char white[] = "\033[0m";

  int len = strlen( str );

  // runs through the match array ans see if there are any matches at all
  bool anyMatch = false;
  for ( int begin = 0; begin <= len; begin++ ){
    for ( int end = begin; end <= len; end++ ){
      if ( matches( pat, begin, end ) ) {
        anyMatch = true;
      }
    }
  }

  // if there is at least one match, we want to print output
  if ( anyMatch ){
    bool mflag = false;
    for ( int begin = 0; begin <= len; begin++ ){
      for ( int end = begin; end <= len; end++ ){

        // if there is a match at beginning,end
        if ( matches( pat, begin, end ) ) {
          //print the sequence to switch to red
          printf("%s", red);

          // loop from beginning to end and print all matching characters
          for ( int k = begin; k < end; k++ )
            putchar(str[ k ]);

          // print the sequence to switch back to white
          printf("%s", white);

          begin = end - 1;
          mflag = true;
        }
      }
      if (!mflag && begin != len){
        putchar(str[begin]);
      }
      mflag = false;

    }
    putchar('\n');
  }
}

/**
   Entry point for the program, parses command-line arguments, builds
   the pattern and then tests it against lines of input.

   @param argc Number of command-line arguments.
   @param argv List of command-line arguments.
   @return exit status for the program.
*/
int main( int argc, char *argv[] )
{
  FILE * in;

  if (argc == ARGCFILE){
    in = fopen(argv[FILE_ARG], "r");
  }
  else if (argc == ARGCNOFILE){
    in = stdin;
  }
  else{
    fprintf(stderr, "usage: regular <pattern> [input-file.txt]\n");
    exit(EXIT_FAILURE);
  }

  char *pstr = argv[PAT_ARG];
  Pattern *pat = parsePattern( pstr );

  if (in == NULL){
    fprintf(stderr, "Can't open input file: %s\n", argv[FILE_ARG]);
    exit(EXIT_FAILURE);
  }

  char *str = (char *)malloc(sizeof(char) * BUFFLEN);
  char c = '\0';
  // read the input line, checking to see if it is too long

  while (fscanf(in, "%[^\n]%c", str, &c) != EOF ){
    if (strlen(str) > LINELEN){
      fprintf(stderr, "Input line too long\n");
      exit(EXIT_FAILURE);
    }

    if (c == '\n'){
      // Find matches for this pattern.
      pat->locate( pat, str );

      reportMatches( pat, pstr, str);
    }
  }

  pat->destroy( pat );

  free(str);
  fclose(in);
  return EXIT_SUCCESS;
}
