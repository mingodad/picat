/*
  Regular expressions.

  C example from 
  http://stackoverflow.com/questions/1085083/regular-expressions-in-c-examples

  Syntax:  
  bp.regex(re,string)

  Note the bp. prefix!

  Examples:
  bp.regex("^a","abcd")


  Hakan Kjellerstrand, hakank@gmail.com
  

*/
    
#include "picat.h"
#include "regex.h"

/*
  regex/2:  regex(re,string)
  true if the regular expression re matches the string string
*/
regex(){
    TERM str, re;
    regex_t regexp;
    int reti;
    char msgbuf[MAX_STR_LEN];
    char str_s[MAX_STR_LEN];
    char re_s[MAX_STR_LEN];
  
    /* Prepare Picat terms */
    re = picat_get_call_arg(1,2); /* first argument */
    str = picat_get_call_arg(2,2); /* second argument */

    picat_str_to_c_str(re, re_s, MAX_STR_LEN);
    picat_str_to_c_str(str, str_s, MAX_STR_LEN);

    // fprintf(stderr, "re_s: %s str_s: %s\n", re_s, str_s);

    /* Compile regular expression */
    reti = regcomp(&regexp, re_s, 0);
    if (reti) {
        fprintf(stderr,"Could not compile regex\n");
        return PICAT_FALSE;
    } 

    /* Execute regular expression */
    reti = regexec(&regexp, str_s, 0, NULL, 0);
    if (!reti) {
        regfree(&regexp);
        return PICAT_TRUE;
    } else if (reti == REG_NOMATCH) {
        regfree(&regexp);
        return PICAT_FALSE;
    } else {
        regerror(reti, &regexp, msgbuf, sizeof(msgbuf));
        regfree(&regexp);
        return PICAT_FALSE;
    }


}
