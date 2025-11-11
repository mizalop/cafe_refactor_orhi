/**
 * \file new_types.h
 * \brief Definiciones de tipos de datos.
 *
 * \version 1.0
 * \copyright Copyright (c) 2017 ULMA Embedded Solutions. All rights reserved.
 *
 *                       ULMA Embedded Solutions
 *                     ---------------------------
 *                        Embedded Design House
 *
 *                     http://www.ulmaembedded.com
 *                        info@ulmaembedded.com
 *
 *******************************************************************************
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 ******************************************************************************/
                                                                                
#ifndef SOURCE_NEW_TYPES_H
#define SOURCE_NEW_TYPES_H


/*******************************************************************************
 * Globals
 */
 
/*******************************************************************************
 * Types and constants
 */
#ifndef TRUE
#define TRUE  1 // Definition for TRUE.
#endif

#ifndef FALSE
#define FALSE 0 // Definition for FALSE.
#endif
 
#ifndef NULL
#define NULL        0
#endif //  NULL_DEFINED

#ifndef BOOL_T_DEFINED
#define BOOL_T_DEFINED
typedef char				 bool_t;
#endif // BOOL_T_DEFINED

#ifndef CHAR_T_DEFINED
#define CHAR_T_DEFINED
typedef char                char_t;
#endif // CHAR_T_DEFINED
/*******************************************************************************
 * Interface
 */

#endif  /* SOURCE_NEW_TYPES_H */

/*
 * End of file
 ******************************************************************************/
