/* PURPOSE: This header defines standard bool data types for use
 *          in plain 'C' lexical analyser engines. This is done
 *          here, in wise prediction that some compiler distributions
 *          may not provide this standard header. For the standard
 *          reference, please review: "The Open Group Base Specifications 
 *          Issue 6, IEEE Std 1003.1, 2004 Edition".
 *
 * (C) 2008  Frank-Rene Schaefer */           
#ifndef __INCLUDE_GUARD_QUEX__CODE_BASE__COMPATIBILITY_PSEUDO_STDBOOL_H__
#define __INCLUDE_GUARD_QUEX__CODE_BASE__COMPATIBILITY_PSEUDO_STDBOOL_H__

#if defined(__QUEX_SETTING_PLAIN_C)

/* According to the C99 Standard 'bool' would have to be defined
 * as equal to '_Bool' which is also defined in some standard. The author
 * of this header, though, found it particularly hard to determine
 * whether the compiler obeys these standards or not. Even prominent
 * compilers (such as gcc) do not provide __STDC_VERSION__. Thus, 
 * the 'easy' solution to simply define it as 'int' and call this
 * file a 'pseudo-stdbool.h'. */
#ifndef bool
#   define bool int
#endif

#define true  ((int)(1))
#define false ((int)(0))

#define __bool_true_false_are_defined ((int)(1))

#endif /* __QUEX_SETTING_PLAIN_C */
#endif /* __INCLUDE_GUARD_QUEX__CODE_BASE__COMPATIBILITY_PSEUDO_STDBOOL_H__ */
