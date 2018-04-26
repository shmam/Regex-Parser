/**
 * @file pattern.c
 * @author sdcroche
 *
 * Pattern is responsible for creating freeing and locating the
 * regex matches in dynamically allocated patterns.
 */
#include "pattern.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/** Free the table inside a pattern, if there is one.

    @param this The pattern we're supposed to operate on.
*/
static void freeTable( Pattern *this )
{
  if ( this->table ) {
    for ( int r = 0; r <= this->len; r++ )
      free( this->table[ r ] );
    free( this->table );
  }
}

/** Make and initialize a new match table inside the given pattern,
    large enough to store matches for the given string.

    @param this The pattern we're supposed to operate on.
    @param str The string we're going to store mageches for */
static void initTable( Pattern *this, char const *str )
{
  // If we already had a table, free it.
  freeTable( this );

  // Make a table big enough for str.
  this->len = strlen( str );
  this->table = (bool **) malloc( ( this->len + 1 ) * sizeof( bool * ) );
  for ( int r = 0; r <= this->len; r++ )
    this->table[ r ] = (bool *) calloc( ( this->len + 1 ), sizeof( bool ) );
}

// Documented in the header.
bool matches( Pattern *pat, int begin, int end )
{
  return pat->table[ begin ][ end ];
}

/**
   A simple function that can be used to free the memory for any
   pattern that doesn't allocate any additional memory other than the
   struct used to represent it (e.g., if it doesn't contain any
   sub-patterns).  It's like a default implementation of the destroy
   method.

   @param pat The pattern to free memory for.
*/
static void destroySimplePattern( Pattern *pat )
{
  freeTable( pat );
  // If we don't need fields that are specific to the sub-type, we can just
  // free the block of memory where the object is stored.
  free( pat );
}

/**
   Type of pattern used to represent a single, ordinary symbol,
   like 'a' or '5'.
*/
typedef struct {
  // Fields from our superclass.
  int len;
  bool **table;
  void (*locate)( Pattern *pat, char const *str );
  void (*destroy)( Pattern *pat );

  /** Symbol this pattern is supposed to match. */
  char sym;
} SymbolPattern;


/**
 * Locates the correct spots in the matching table to indicate matching
 * patterns based on the parameters and the context of the pattern type
 *
 * @param pat pattern to locate matches for
 * @param string to parse through and mark matches
 */
static void locateSymbolPattern( Pattern *pat, char const *str )
{
  // Cast down to the struct type pat really points to.
  SymbolPattern *this = (SymbolPattern *) pat;

  // Make a fresh table for this input string.
  initTable( pat, str );

  // Find all occurreces of the symbol we are supposed to match
  for ( int begin = 0; str[ begin ]; begin++ ){
    if ( str[ begin ] == this->sym )
      this->table[ begin ][ begin + 1 ] = true;
  }
}

/**
 * Locates the correct spots in the matching table to indicate matching
 * patterns based on the parameters and the context of the pattern type
 *
 * @param pat pattern to locate matches for
 * @param string to parse through and mark matches
 */
static void locateSymbolPatternPeriod( Pattern *pat, char const *str )
{
  // Cast down to the struct type pat really points to.
  SymbolPattern *this = (SymbolPattern *) pat;

  // Make a fresh table for this input string.
  initTable( pat, str );

  // Find all occurreces of the symbol we are supposed to match
  for ( int begin = 0; str[ begin ]; begin++ ){
    if ( str[begin] >= ' ' && str[begin] <= 'z'){
      this->table[begin][begin + 1] = true;
    }
  }
}

/**
 * Locates the correct spots in the matching table to indicate matching
 * patterns based on the parameters and the context of the pattern type
 *
 * @param pat pattern to locate matches for
 * @param string to parse through and mark matches
 */
static void locateSymbolPatternCarrot( Pattern *pat, char const *str )
{
  // Cast down to the struct type pat really points to.
  SymbolPattern *this = (SymbolPattern *) pat;

  // Make a fresh table for this input string.
  initTable( pat, str );

  // Find all occurreces of the symbol we are supposed to match
  for ( int begin = 0; str[ begin ]; begin++ ){
    if (this->sym == '^' && begin == 0){
      this->table[begin][begin] = true;
    }
  }
}

/**
 * Locates the correct spots in the matching table to indicate matching
 * patterns based on the parameters and the context of the pattern type
 *
 * @param pat pattern to locate matches for
 * @param string to parse through and mark matches
 */
static void locateSymbolPatternAnchor( Pattern *pat, char const *str )
{
  // Cast down to the struct type pat really points to.
  SymbolPattern *this = (SymbolPattern *) pat;

  // Make a fresh table for this input string.
  initTable( pat, str );

  // Find all occurreces of the symbol we're supposed to match, and
  // mark them in the match table as matching, 1-character substrings.
  for ( int begin = 0; str[ begin ]; begin++ ){
    if (this->sym == '$' && begin == (strlen(str) - 1)){
      this->table[begin+1][begin+1] = true;
    }
  }
}

// Documented in the header.
Pattern *makeSymbolPattern( char sym )
{
  // Make an instance of SymbolPattern, and fill in its state.
  SymbolPattern *this = (SymbolPattern *) malloc( sizeof( SymbolPattern ) );
  this->table = NULL;

  if (sym == '^'){
    this->locate = locateSymbolPatternCarrot;
  }
  else if (sym == '.'){
    this->locate = locateSymbolPatternPeriod;
  }
  else if (sym == '$'){
    this->locate = locateSymbolPatternAnchor;
  }
  else{
    this->locate = locateSymbolPattern;
  }
  this->destroy = destroySimplePattern;
  this->sym = sym;

  return (Pattern *) this;
}

/**
   Representation for a type of pattern that contains two sub-patterns
   (e.g., concatenation).  This representation could be used by more
   than one type of pattern, as long as it uses a pointer to a
   different locate() function.
*/
typedef struct {
  // Fields from our superclass.
  int len;
  bool **table;
  void (*locate)( Pattern *pat, char const *str );
  void (*destroy)( Pattern *pat );

  // Pointers to the two sub-patterns.
  Pattern *p1, *p2;
} BinaryPattern;

/**
 * Frees the dynamically allocated memory for the Pattern, and
 * all of it's contents
 *
 * @param pattern to free
 */
static void destroyBinaryPattern( Pattern *pat )
{
  // Cast down to the struct type pat really points to.
  BinaryPattern *this = (BinaryPattern *) pat;

  // Free our table.
  freeTable( pat );
  // Free our two sub-patterns.
  this->p1->destroy( this->p1 );
  this->p2->destroy( this->p2 );
  // Free the struct representing this object.
  free( this );
}

/**
 * Locates the correct spots in the matching table to indicate matching
 * patterns based on the parameters and the context of the pattern type
 *
 * @param pat pattern to locate matches for
 * @param string to parse through and mark matches
 */
static void locateConcatenationPattern( Pattern *pat, const char *str )
{
  // Cast down to the struct type pat really points to.
  BinaryPattern *this = (BinaryPattern *) pat;

  initTable( pat, str );

  //  Let our two sub-patterns figure out everywhere they match.
  this->p1->locate( this->p1, str );
  this->p2->locate( this->p2, str );

  // Then, based on their matches, look for all places where their
  // concatenaton matches.  Check all substrings of the input string.
  for ( int begin = 0; begin <= this->len; begin++ )
    for ( int end = begin; end <= this->len; end++ ) {

      // For the [ begin, end ) range, check all places where it could
      // be split into two substrings, the first matching p1 and the second
      // matching p2.
      for ( int k = begin; k <= end; k++ )
        if ( matches( this->p1, begin, k ) &&
             matches( this->p2, k, end ) )
          this->table[ begin ][ end ] = true;
    }
}

// Documented in header.
Pattern *makeConcatenationPattern( Pattern *p1, Pattern *p2 )
{
  // Make an instance of Binary pattern and fill in its fields.
  BinaryPattern *this = (BinaryPattern *) malloc( sizeof( BinaryPattern ) );
  this->table = NULL;
  this->p1 = p1;
  this->p2 = p2;

  this->locate = locateConcatenationPattern;
  this->destroy = destroyBinaryPattern;

  return (Pattern *) this;
}


/**
 * Locates the correct spots in the matching table to indicate matching
 * patterns based on the parameters and the context of the pattern type
 *
 * @param pat pattern to locate matches for
 * @param string to parse through and mark matches
 */
static void locateAlterationPattern( Pattern *pat, const char *str )
{
  // Cast down to the struct type pat really points to.
  BinaryPattern *this = (BinaryPattern *) pat;

  initTable( pat, str );

  //  Let our two sub-patterns figure out everywhere they match.
  this->p1->locate( this->p1, str );
  this->p2->locate( this->p2, str );

  for ( int begin = 0; begin <= this->len; begin++ )
    for ( int end = begin; end <= this->len; end++ ) {

      // if p1 is the first to match, we want our table to be all the values in p1
      if (matches(this->p1, begin, end)){
        for ( int begin = 0; begin <= this->len; begin++ ){
          for ( int end = begin; end <= this->len; end++ ) {
            if (matches(this->p1, begin, end)){
              this->table[begin][end] = true;
            }
          }
        }
        begin = end = (this->len + 1);break;
      }
      // if p2 is the first to match, we want our table to be all the values in p2
      else if (matches(this->p2, begin, end)){
        for ( int begin = 0; begin <= this->len; begin++ ){
          for ( int end = begin; end <= this->len; end++ ) {
            if (matches(this->p2, begin, end)){
              this->table[begin][end] = true;
            }
          }
        }
        begin = end = (this->len + 1);break;
      }
    }
}

// Documented in header.
Pattern *makeAlterationPattern( Pattern *p1, Pattern *p2 )
{
  // Make an instance of Binary pattern and fill in its fields.
  BinaryPattern *this = (BinaryPattern *) malloc( sizeof( BinaryPattern ) );
  this->table = NULL;
  this->p1 = p1;
  this->p2 = p2;

  this->locate = locateAlterationPattern;
  this->destroy = destroyBinaryPattern;

  return (Pattern *) this;
}


/**
   Type of pattern used for repititons
*/
typedef struct {
  // Fields from our superclass.
  int len;
  bool **table;
  void (*locate)( Pattern *pat, char const *str );
  void (*destroy)( Pattern *pat );

  int length;

  /** pattern this repetition is supposed to match  */
  Pattern *sym;

} RepetitionPattern;

// locate function for a optionalPattern
static void locateOptionalPattern( Pattern *pat, const char *str )
{
  // Cast down to the struct type pat really points to.
  RepetitionPattern *this = (RepetitionPattern *) pat;

  initTable( pat, str );

  //  Let our two sub-patterns figure out everywhere they match.
  this->sym->locate( this->sym, str );

  for ( int begin = 0; begin <= this->len; begin++ ){
    for ( int end = begin; end <= this->len; end++ ) {
      if (matches(this->sym, begin, end)){
        this->table[begin][end] = true;
        begin = end = (this->len + 1);break;
      }
      else{
        this->table[begin][end] = true;
      }
    }
  }
}

/**
 * Frees the dynamically allocated memory for the Pattern, and
 * all of it's contents
 *
 * @param pattern to free
 */
static void destroyRepetitionPattern( Pattern *pat )
{
  // Cast down to the struct type pat really points to.
  RepetitionPattern *this = (RepetitionPattern *) pat;

  // Free our table.
  freeTable( pat );
  // Free our sub-pattern.
  this->sym->destroy( this->sym );
  // Free the struct representing this object.
  free( this );
}

// Documented in header.
Pattern *makeOptionalPattern( Pattern *p1)
{
  // Make an instance of RepetitionPattern and fill in its fields.
  RepetitionPattern *this = (RepetitionPattern *) malloc( sizeof( RepetitionPattern ) );
  this->table = NULL;
  this->sym = p1;

  this->locate = locateOptionalPattern;
  this->destroy = destroyRepetitionPattern;

  return (Pattern *) this;
}

/**
 * Locates the correct spots in the matching table to indicate matching
 * patterns based on the parameters and the context of the pattern type
 *
 * @param pat pattern to locate matches for
 * @param string to parse through and mark matches
 */
static void locatePlusPattern( Pattern *pat, char const *str )
{
  // Cast down to the struct type pat really points to.
  RepetitionPattern *this = (RepetitionPattern *) pat;

  // Make a fresh table for this input string.
  initTable( pat, str );

  //locate the subpattern in the symbol pattern
  this->sym->locate( this->sym, str );

  for ( int begin = 0; begin <= this->len; begin++ )
    for ( int end = begin; end <= this->len; end++ ) {
      if (matches(this->sym, begin, end)){
        int steps = 0;
        int i = 0;
        while ((begin + i ) <= this->len && (end + i ) <= this->len){
          if (matches(this->sym, begin + i, end + i)){
            steps = i;
          }
          i++;
        }
        this->table[begin][end + steps] = true;
      }
    }
}

/**
 * Locates the correct spots in the matching table to indicate matching
 * patterns based on the parameters and the context of the pattern type
 *
 * @param pat pattern to locate matches for
 * @param string to parse through and mark matches
 */
static void locateAsteriskPattern(Pattern *pat, char const *str )
{
  // Cast down to the struct type pat really points to.
  RepetitionPattern *this = (RepetitionPattern *) pat;

  // Make a fresh table for this input string.
  initTable( pat, str );

  //locate the subpattern in the asterisk pattern
  this->sym->locate( this->sym, str );


  for (int begin = 0; begin <= this->len; begin++){
    this->table[begin][begin] = true;
    for (int end = begin; end <= this->len; end++){
      for (int k = begin; k <= end; k++){
        if ( matches(this->sym, begin, k) || matches(this->sym, k, end)){
          this->table[begin][end] = true;
        }
      }
    }
  }
}

// Documented in the header.
Pattern *makeAsteriskPattern( Pattern *pat )
{
  // Make an instance of RepetitionPattern, and fill in its state.
  RepetitionPattern *this = (RepetitionPattern *) malloc( sizeof( RepetitionPattern ) );
  this->table = NULL;

  this->locate = locateAsteriskPattern;
  this->destroy = destroyRepetitionPattern;
  this->sym = pat;

  return (Pattern *) this;
}


// Documented in the header.
Pattern *makePlusPattern( Pattern *pat )
{
  // Make an instance of RepetitionPattern, and fill in its state.
  RepetitionPattern *this = (RepetitionPattern *) malloc( sizeof( RepetitionPattern ) );
  this->table = NULL;

  this->locate = locatePlusPattern;
  this->destroy = destroyRepetitionPattern;
  this->sym = pat;

  return (Pattern *) this;
}

/**
   Type of pattern used to match a character class for multiple characters
*/
typedef struct {
  // Fields from our superclass.
  int len;
  bool **table;
  void (*locate)( Pattern *pat, char const *str );
  void (*destroy)( Pattern *pat );

  /** character class to match to */
  char *cclass;
} CharacterClassPattern;


/**
 * Frees the dynamically allocated memory for the Pattern, and
 * all of it's contents
 *
 * @param pattern to free
 */
static void destroyCharacterClassPattern( Pattern *pat )
{
  // Cast down to the struct type pat really points to.
  CharacterClassPattern *this = (CharacterClassPattern *) pat;

  // Free our table.
  freeTable( pat );

  // Free the string
  if (this->cclass){
    free(this->cclass);
  }
  // Free the struct representing this object.
  free( this );
}

/**
 * Locates the correct spots in the matching table to indicate matching
 * patterns based on the parameters and the context of the pattern type
 *
 * @param pat pattern to locate matches for
 * @param string to parse through and mark matches
 */
static void locateCharacterClassPattern( Pattern *pat, char const *str )
{
  // Cast down to the struct type pat really points to.
  CharacterClassPattern *this = (CharacterClassPattern *) pat;

  // Make a fresh table for this input string.
  initTable( pat, str );

  // Find all occurreces of the symbol we're supposed to match
  for ( int begin = 0; str[ begin ]; begin++ ){
    for ( int i = 0; i < strlen(this->cclass); i++){
      if (str[begin] == this->cclass[i]){
        this->table[begin][begin + 1] = true;
      }
    }
  }
}

// Documented in the header.
Pattern *makeCharacterClassPattern( char * sym )
{
  // Make an instance of CharacterClassPattern, and fill in its state.
  CharacterClassPattern *this = (CharacterClassPattern *) malloc( sizeof( CharacterClassPattern ) );
  this->table = NULL;
  this->locate = locateCharacterClassPattern;

  this->destroy = destroyCharacterClassPattern;
  this->cclass = sym;

  return (Pattern *) this;
}
