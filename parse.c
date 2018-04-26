/**
 * @file: parse.c
 * @author: sdcroche
 *
 * Parse is responsible for reading through a regex pattern
 * and creating the appropriate pattern tree structure to
 * match correct patterns
 */
#include "parse.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/**
   Return true if  the given character is ordinary, if it should just
   match occurrences of itself.  This returns false for metacharacters
   like '*' that control how patterns are matched.

   @param c Character that should be evaluated as ordinary or special.
   @return True if c is not special.
*/
static bool ordinary( char c )
{
  // See if c is on our list of special characters.
  if ( strchr( ".^$*?+|()[{", c ) )
    return false;
  return true;
}

/**
  Print the appropriate error message for an invalid pattern and exit
  unsuccessfully.
*/
static void invalidPattern()
{
  fprintf( stderr, "Invalid pattern\n" );
  exit( EXIT_FAILURE );
}

/**
   Parse regular expression syntax with the highest precedence,
   individual, ordinary symbols, start and end anchors, character
   classes and patterns surrounded by parentheses.

   @param str The string being parsed.
   @param pos A pass-by-reference value for the location in str being parsed,
              increased as characters from str are parsed.
   @return a dynamically allocated representation of the pattern for the next
           portion of str.
*/
static Pattern *parseAtomicPattern( char const *str, int *pos )
{
  if ( ordinary( str[ *pos ] ) )
    return makeSymbolPattern( str[ (*pos)++ ] );

  else{
    while (str[ *pos ] && strchr( ".^$([", str[ *pos ])){
      if ( str[ *pos] == '('){
        int i = 0;
        char subs[strlen(str)];
        (*pos)++;
        while (str[ *pos] != ')'){
          subs[i] = str[*pos];
          i++;
          (*pos)++;
        }
        (*pos)++;
        subs[i] = '\0';
        return parsePattern(subs);
      }

      else if ( str[ *pos] == '['){
        int i = 0;
        char subs[strlen(str)];
        (*pos)++;
        while (str[ *pos] != ']'){
          if (!str[*pos]){
            invalidPattern();
          }
          subs[i] = str[*pos];
          i++;
          (*pos)++;
        }
        (*pos)++;
        subs[i] = '\0';
        char *p = (char *)malloc(sizeof(char) * (strlen(subs) + 1));
        strcpy(p, subs);
        return makeCharacterClassPattern(p);
      }

      else{
        return makeSymbolPattern( str[ (*pos)++ ] );
      }
    }

  }

  invalidPattern();
  return NULL; // Just to make the compiler happy :)
}

/**
   Parse regular expression syntax with the second-highest precedence,
   a pattern, p, optionally followed by one or more repetition syntax like '*' or '+'.
   If there's no repetition syntax, it just returns the pattern object for p.

   @param str The string being parsed.
   @param pos A pass-by-reference value for the location in str being parsed,
              increased as characters from str are parsed.
   @return a dynamically allocated representation of the pattern for the next
           portion of str.
*/
static Pattern *parseRepetition( char const *str, int *pos )
{
  Pattern *p = parseAtomicPattern( str, pos );

  while (str[ *pos ] && (str[ *pos ] == '*' || str[ *pos ] == '+' || str[ *pos ] == '?')){
    if (str[ *pos ] == '*'){
      (*pos)++;
      p = makeAsteriskPattern(p);
    }
    else if ( str[ *pos ] == '+' ){
      (*pos)++;
      p = makePlusPattern(p);
    }
    else if (str[ *pos ] == '?'){
      (*pos)++;
      p = makeOptionalPattern(p);
    }
  }

  return p;
}

/**
   Parse regular expression syntax with the third-highest precedence,
   one pattern, p, (optionally) followed by additional patterns
   (concatenation).  If there are no additional patterns, it just
   returns the pattern object for p.

   @param str The string being parsed.
   @param pos A pass-by-reference value for the location in str being parsed,
              increased as characters from str are parsed.
   @return a dynamically allocated representation of the pattern for the next
           portion of str.
*/
static Pattern *parseConcatenation( char const *str, int *pos )
{
  // Parse the first pattern
  Pattern *p1 = parseRepetition( str, pos );
  // While there are additional patterns, parse them
  while ( str[ *pos ] && str[ *pos ] != '|' && str[ *pos ] != ')' ) {
    Pattern *p2 = parseRepetition( str, pos );
    // And build a concatenation pattern to match the sequence.
    p1 = makeConcatenationPattern( p1, p2 );
  }

  return p1;
}

/**
   Parse regular expression syntax with the lowest precedence, one
   pattern, p, (optionally) followed by additional patterns separated
   by | (alternation).  If there are no additional patterns, it just
   returns the pattern object for p.

   @param str The string being parsed.
   @param pos A pass-by-reference value for the location in str being
              parsed, increased as characters from str are parsed.
   @return a dynamically allocated representation of the pattern for
   the next portion of str.
*/
static Pattern *parseAlternation( char const *str, int *pos )
{
  Pattern *p1 = parseConcatenation( str, pos );

  while (str[*pos] && str[*pos] == '|'){
    (*pos)++;
    Pattern *p2 = parseConcatenation( str, pos );
    p1 = makeAlterationPattern(p1, p2);
  }

  return p1;
}

// Documented in the header
Pattern *parsePattern( char const *str )
{
  // Parse the argument into a tree of pattern objects.
  int pos = 0;
  Pattern *pat = parseAlternation( str, &pos );

  // Complain if this didn't consume the whole pattern.
  if ( str[ pos ] )
    invalidPattern();

  return pat;
}
