#ifndef PATTERN_H
#define PATTERN_H

#include <stdbool.h>

//////////////////////////////////////////////////////////////////////
// Superclass for Patterns

/** A short name to use for the Pattern interface. */
typedef struct PatternStruct Pattern;

/**
  Structure used as a superclass/interface for a regular expression
  pattern.  It includes a representation for a resizable 2D table
  where we can record regions (substrings) of an input string that are
  matched by the pattern.  There's a function pointer for an
  overridable method, locate(), that uses the table to mark all the
  places where the pattern matches the input string, and another
  overridable method for freeing resources for the pattern.
*/
struct PatternStruct {
  /** Length of the current input string, as recorded by the latest call
      to locate(). */
  int len;

  /** The match table, a (len + 1) X (len + 1) 2D array represented as
      an array of pointers to arrays.  The job of the locate() functon
      is to fill in this table so table[ begin ][ end ] is true if
      this pattern can match the [ begin, end ) substring of the
      current input string.  */
  bool **table;

  /** Relallocate the match table.  Find all the [ begin, end )
      substrings of input string, str, that match the pattern.  For
      any match, set table[ begin ][ end ] to true in the match
      table.

      @param pat pointer to the pattern being matched (essentially, a this
                 pointer.
      @param str input string in which we're finding matches.
  */
  void (*locate)( Pattern *pat, char const *str );

  /** Free memory for this pattern, including any subpatterns it contains.
      @param pat pattern to free.
  */
  void (*destroy)( Pattern *pat );
};

/** Report elements of the match table for the given pattern.  This
    can be called after the pattern's locate() function, to see where
    it found a match.  It's like a non-overridable instance method for
    the Pattern object.

    @param pat pointer to the pattern for which we want to check
               reported matches.
    @param begin index of the first character in the substring
    @param end index one-past-the-end of the string
    @return true if this pattern matches the [ begin, end ) substring
            of the most recent input string
 */
bool matches( Pattern *pat, int begin, int end );

/**
  Make a pattern for a single, non-special character, like `a` or `5`.

  @param sym The symbol this pattern is supposed to match.
  @return A dynamically allocated representation for this new pattern.
*/
Pattern *makeSymbolPattern( char sym );

/**
 * Make a pattern for a singe . regex symbol, where
 * any character can match with an individual . in a pattern
 *
 * @param the character symbol '.'
 * @return A synamically allocated pattern for the . pattern
 */
Pattern *makeSymbolPatternPeriod( char sym );

/**
  Make a pattern for the concatenation of patterns p1 and p2.  It
  should match anything that can be broken into two substrings, s1 and
  s2, where the p1 matches the first part (s1) and p2 matches the
  second part (s2).

  @param p1 Subpattern for matching the first part of the string.
  @param p2 Subpattern for matching the second part of the string.
  @return A dynamically allocated representation for this new pattern.
*/
Pattern *makeConcatenationPattern( Pattern *p1, Pattern *p2 );

/**
 * Makes an alteration pattern based off the binary structure, where
 * a string can match with either p1 or p2 first represented as p1|p2
 *
 * @param p1 pattern
 * @param p2 pattern
 * @return the dyamically allocated alteration pattern representation
 */
Pattern *makeAlterationPattern( Pattern *p1, Pattern *p2 );

/**
 * Makes an optinal symbol pattern based off the repetion pattern struct
 * where a symbol can optionally match in a string
 *
 * @param p1 pattern that is optional
 * @return the dynamically allocated optional pattern representation
 */
Pattern *makeOptionalPattern( Pattern *p1 );

/**
 * Makes a pattern symbol for the asterisk regex symbol, which marks a pattern
 * as something that can be matched either zero or many times in a string represented
 * for a pattern p as: p*
 *
 * @param pat pattern to apply to the asterisk
 * @return the dynamically allocated asterisk pattern
 */
Pattern *makeAsteriskPattern( Pattern *pat );

/**
 * Makes a pattern symbol using the repetition struct for the plus regex symbol,
 * which marks a pattern as something that can be matched one or more times in
 * a stirng represented for the pattern p as: p+
 *
 * @param pat pattern to apply to the plus
 * @return the dynamically allocated plus pattern
 */
Pattern *makePlusPattern( Pattern *pat );

/**
 * Makes a character class pattern, which takes in a string of characters between
 * brackets, and matches any character in a string that appears in the brackets.
 * This is made using it's own custom structure, build to handle the string pointer
 *
 * @param sym string
 * @return the dynamically allocated character class pattern
 */
Pattern *makeCharacterClassPattern( char *sym );

#endif
