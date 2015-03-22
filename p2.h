/********************************************************************
 *
 * File                 : p2.h
 * Language             : h
 * Name                 : Shashank Tomar
 * Instructor Name      : John Carroll
 * Class                : CS570   
 * Due Date             : 14th Oct, 2013
 * 
 * Description          : This header file contains all the declarations
 *                        related to p2.c
 *
 ********************************************************************/

/* including system headers */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

/* including user headers */
#include "getword.h"

/* p2 specific macro definations */
#define CHMOD_ARGC   3      // to check proper arguments for chmod() system call
#define CD_ARGC      2      // to check proper arguments for chdir() system call
#define NUM_ARG      20     // max num of argument vectors
#define TOTAL_WORDS  100    // total words an array can hold of 255 size each
#define MAX_DIR_SZ   40     // size of maximum absolute path
#define BASE_OCTAL   8      // to represent octal base
#define PIPE_OUT     1      // to represent pipe output fd
#define PIPE_IN      0      // to represent pipe input fd

#define CHK(x)  \
  do {if((x) == P2ERR)\
        {\
            (void) fprintf(stderr,"In file %s, on line %d:\n",__FILE__,__LINE__);\
            perror("Exiting because");\
            exit(EXIT_FAILURE);\
        }\
     } while(0)

#define MYDUP2(OLD_FD,NEW_FD,DEVNULL)\
        if ((OLD_FD != P2ERR) && (OLD_FD != NEW_FD))\
        {\
            if (P2ERR == dup2(OLD_FD, NEW_FD))\
            {\
                perror("dup2 failed");\
                (void) dup2(DEVNULL, STDOUT_FILENO);\
            }\
            CHK(close(OLD_FD));\
        }\

/* enum declaration for meta-characters.
   This type of declaration helps in bitwise
   manipulation */
typedef enum 
{
    NONE                = 0x00,
    SEMICOLON           = 0x01,
    BACKGROUND          = 0x02,
    NEWLINE             = 0x04,
    EOFILE              = 0x08

} e_metachar;


/* enum declaration for error codes */
typedef enum e_ecode
{
    P2INV_ARG           = -2,
    P2ERR               = -1,
    P2SUCCESS           = 0

} p2_ecode;


/* function prototypes */
static void myhandler();

static int parse(int *argc, char **argv);

static p2_ecode change_dir(int argc, char **argv);

static void handle_pipe(char **argv);

static void execute(char **argv);

static p2_ecode addmode(char **argv, int *mode);

static p2_ecode change_mode(int argc, char **argv);


