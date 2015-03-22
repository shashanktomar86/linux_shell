/********************************************************************
 *
 * File                 : getword.c
 * Language             : C
 * Name                 : Shashank Tomar
 * Instructor Name      : John Carroll
 * Class                : CS570   
 * Due Date             : 18th Sept, 2013
 * 
 * Description          : This programm gets the word from stdin as the per
 *                        the guidelines given in getword.h
 *
 ********************************************************************/


#include <stdlib.h>
#include <ctype.h>
#include "getword.h"


/* macro definations for meta characters */
#define BKSLASH         92
#define NONE            0x00
#define SEMICOLON       0x01
#define BACKGROUND      0x02
#define NEWLINE         0x04
#define EOFILE          0x08

/* defination for global variables */
char g_mchar      = NONE;
char g_ignore     = 0;
char g_isPipe     = 0;
char *home        = NULL;


/********************************************************************
 * Function    : getword(char *)
 * Input       : char * - This pointer is pointed to the output
                          array where result need to be put.
 * Returns     : int    - returns the word count or -1 for EOF.
 * 
 * Description : This function find the words from stdin and handle
 *               special meta-characters (\, <, >, >>, ~, ;, &) 
 ********************************************************************/
int getword(char *buff)
{
    char *temp = buff;
    char *home_final;
    int  ch = 0;
    int  i  = 0;
    int count = 0;

    /* Declaration for flags used for meta-characters */
    static char flag_amp        = 0; // flag for '&'
    static char flag_redirect   = 0; // flag for '>'
    static char flag_less       = 0; // flag for '<'
    static char flag_pipe       = 0; // flag for '|'
    static char flag_bkslash    = 0; // flag for '\'

    g_mchar = NONE;
    home = getenv ("HOME");

    ch = getchar();

    /* while loop to remove trailing white spaces */
    while ((' ' == ch) || ('\t' == ch))
    {
        ch = getchar();
    }

    /* for EOF, return -1 */
    if (EOF == ch)
    {
        *temp = '\0';
        g_mchar |= EOFILE;
        return -1;
    }
    
    /* handle ';' if it comes in starting */
    if (';' == ch)
    {
        ch = getchar();

        if(!((' ' == ch) || ('\t' == ch) || ('\n' == ch)))
        {
            (void) ungetc (ch, stdin);
            *temp = '\0';
            g_mchar |= SEMICOLON;
            return 0;
        }
    }

    /* if new line is present, return from here */
    if ('\n' == ch)
    {
        *temp = '\0';
        g_mchar |= NEWLINE;
        return 0;
    }

    (void) ungetc (ch, stdin);
    
    /* main loop to process each word and put it to the output buffer */
    for (i = 0; (STORAGE -1) > i; i++)
    {
        ch = getchar();

        /* get the alphabets and numbers from the input */
        if (isalnum(ch))
        {
            if (flag_bkslash)
            {
                flag_bkslash = 0;
            }
            ++count;
            *temp++ = ch;
        }
        else
        {
            /* switch case to handle special case and meta-characters */
            switch (ch)
            {

                case '~':
                    if (!flag_bkslash)
                    {
                        /* copy the home directory if the buffer is present, 
                           else ignore the overflowing characters */
                        home_final = strncpy (temp, home, (STORAGE - i-1));
                        temp += strlen (home_final);
                        count += strlen (home_final);
                    }
                    else
                    {
                        flag_bkslash = 0;
                        *temp++ = ch;
                        ++count;
                    }
                    continue;

                case '<':
                    /* if '\' is not present before it, then 
                       treat this as meta-character, otherwise treat
                       it as normal characters and append it to
                       the word. 
                    */
                    if (!flag_bkslash)
                    {
                        if (flag_less)
                        {
                            *temp++ = ch;
                            ++count;
                            flag_less = 0;
                        }
                        else
                        {
                            flag_less = 1;
                            (void) ungetc (ch, stdin);
                            ch = 0;
                        }

                            break;
                    }
                    else
                    {
                        flag_bkslash = 0;
                        *temp++ = ch;
                        ++count;
                        continue;
                    }

                case '>':
                    /* if '\' is not present before it, then 
                       treat this as meta-character, otherwise treat
                       it as normal characters and append it to
                       the word. 
                    */
                    if (!flag_bkslash)
                    {
                        if (flag_redirect)
                        {
                            flag_redirect = 0;
                            *temp++ = ch;
                            ++count;

                            /* if '>>' detects, then copy the whole as a word,
                               else put the word back to the shell */
                            if ('>' == (ch = getchar()))
                            {
                                *temp++ = ch;
                                ++count;
                            }
                            else
                            {
                                (void) ungetc(ch, stdin);
                                ch = 0;
                            }
                        }
                        else
                        {
                            flag_redirect = 1;
                            (void) ungetc (ch, stdin);
                            ch = 0;
                        }

                        break;
    
                    }
                    else
                    {
                        flag_bkslash = 0;
                        *temp++ = ch;
                        ++count;
                        continue;
                    }

                case '|':
                    /* if '\' is not present before it, then 
                       treat this as meta-character, otherwise treat
                       it as normal characters and append it to
                       the word. 
                    */
                    if (!flag_bkslash)
                    {
                        if (flag_pipe)
                        {
                            *temp++ = ch;
                            ++count;
                            flag_pipe = 0;
                            g_isPipe ++;
                        }
                        else
                        {
                            flag_pipe = 1;
                            (void) ungetc (ch, stdin);
                            ch = 0;
                        }
    
                            break;
                    }
                    else
                    {
                        flag_bkslash = 0;
                        *temp++ = ch;
                        ++count;
                        continue;
                    }

                case ';':
                    /* if '\' is not present before it, then 
                       treat this as meta-character, otherwise treat
                       it as normal characters and append it to
                       the word. 
                    */
                    if (!flag_bkslash)
                    {
                        (void) ungetc (ch, stdin);
                        break;
                    }
                    else
                    {
                        flag_bkslash = 0;
                        *temp++ = ch;
                        ++count;
                        continue;
                    }

                case '&':
                    /* if '\' is not present before it, then 
                       treat this as meta-character, otherwise treat
                       it as normal characters and append it to
                       the word. 
                    */
                    if (!flag_bkslash)
                    {
                        if (flag_amp)
                        {
    //                        *temp++ = ch;// commented to work for background process, no need to put this in argv.
     //                       ++count;
                            flag_amp = 0;
                            g_mchar |= BACKGROUND;
                        }
                        else
                        {
                            flag_amp = 1;
                            (void) ungetc (ch, stdin);
                            ch = 0;
                        }

                            break;
                    }
                    else
                    {
                        flag_bkslash = 0;
                        *temp++ = ch;
                        ++count;
                        continue;
                    }

                case BKSLASH:
                    flag_bkslash = 1;
                    g_ignore = 1;

                    /* if again '\' detects, then copy it to the word,
                       else put the word back to the shell */
                    if (BKSLASH == (ch = getchar()))
                    {
                        *temp++ = ch;
                        ++count;
                    }
                    else
                    {
                        (void) ungetc(ch, stdin);
                        ch = 0;
                    }

                    continue;

                case ' ':
                    /* if '\' is not present before it, then 
                       break the word, otherwise treat
                       it as normal characters and append it to
                       the word. 
                    */
                    if (!flag_bkslash)
                    {

                        break;
                    }
                    else
                    {
                        flag_bkslash = 0;
                        *temp++ = ch;
                        ++count;
                        continue;
                    }

                /* if new line or EOF occurs, then return from here */
                case '\n':
                case EOF:
                    (void) ungetc(ch, stdin);
                    *temp = '\0';
                    g_mchar |= NEWLINE;
                    return count;


                /* all rest cahracters not to acknowledge as meta-characters */
                default:
                    if (flag_bkslash)
                    {
                        flag_bkslash = 0;
                    }
           
                    *temp++ = ch;
                    ++count;
                    continue;
            }

            
            if (0 == count)
            {
                continue;
            }
            else
            {
                break; // to break from the loop so as to return to main()
            }
        }

    }



    *temp = '\0';
    return count; 

}

