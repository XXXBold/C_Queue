#ifndef _BOOLTYPE_H_INCLUDED
  #define _BOOLTYPE_H_INCLUDED

#if defined (__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) /* Check if C99 or higher Standard */
  #include <stdbool.h>
#else /* Old Standard, defining own bool... */
    typedef enum
    {
      false=0,
      true=!false
    }bool;
#endif /* __STDC_VERSION__ */

#endif /* !_BOOLTYPE_H_INCLUDED */
