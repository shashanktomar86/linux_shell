/********************************************************************
 *
 * File                 : p2.c
 * Language             : c
 * Name                 : Shashank Tomar
 * Instructor Name      : John Carroll
 * Class                : CS570   
 * Due Date             : 14th Oct, 2013
 * 
 * Description          : This programm acts as simple command line
 *                        line interpreter for UNIX system.
 *
 ********************************************************************/

#include "p2.h"

/* declaration for extern variables */
extern char g_mchar;
extern char g_ignore;
extern char g_isPipe;
extern char *home;

/* global variables defination */
static int g_redirect_in  = STDIN_FILENO;           /* file descriptor for input redirection */
static int g_redirect_out = STDOUT_FILENO;          /* file descriptor for output redirection */
static int g_redirect_app = STDOUT_FILENO;          /* file descriptor for append */
static int g_pipedes[2];                            /* pipe descriptors */
static int g_argc2        = 0;                      /* starting argv count for second command 
                                                       in case of PIPE */
static char g_buff[TOTAL_WORDS * STORAGE] = {0};    /* to store argument vectors */



/********************************************************************
 * Function    : myhandler()
 * Input       : none.
 * Returns     : void.
 * 
 * Description : This is signal handler function for SIGTERM signal.
 ********************************************************************/
static void myhandler()
{
//    (void) printf ("p2 terminated.\n");
}



/********************************************************************
 * Function    : parse(int *, char **)
 * Input       : int *   - This pointer is pointed to the argc
                           count where argc need to be updated.
                 char ** - argv is an array of char pointer which
                           holds the argv data.
 * Returns     : int     - returns the word count or -1 for EOF.
 * 
 * Description : This function find the words from stdin and store
 *               it in a buff and assign it to argv.
 ********************************************************************/
static int parse(int *argc, char **argv)
{
    char temp[STORAGE] = {0};   // to store data from getword
    int count = 0;
    int i = 0, k = 0, l = 0;
    int buff_count = 0;
    char *ptr = g_buff;         // pointer to access the buffer

    (void) memset(g_buff,0,sizeof(g_buff));

    *argc = 0;

    /* initializing all the global file descriptors */    
    g_redirect_in  = STDIN_FILENO;
    g_redirect_out = STDOUT_FILENO;
    g_redirect_app = STDOUT_FILENO;
    g_pipedes[PIPE_IN]   = STDIN_FILENO;
    g_pipedes[PIPE_OUT]  = STDOUT_FILENO;

    for (k = 0; k < STORAGE; k++)
    {
        /* get a word from getword() and stored in local temp array */
        count = getword (temp);
        if ((P2ERR == count) || (0 == count))
        {
           break;
        }

        /* update the global var g_mchar if the word encounters with '&'.
           This is to give '\&' and '&' as same treatment */
        if (P2SUCCESS == (strcmp("&", temp)))
        {
            g_mchar |= BACKGROUND;
        }

        ++*argc;

        /* update the global buffer with the temp buffer values, each word 
           delimited by NULL */
        for (i = 0; temp[i] && (i < (STORAGE-1)); i++, buff_count++)
        {
            *(ptr+buff_count) = *(temp+i);
        }
        
        *(ptr+buff_count) = '\0';
        buff_count++;

        if (g_mchar)
        {
            (void) getchar();
            break;
        }
    
    }

    /* assign pointer to the start of the buffer */
    ptr = g_buff;

    /* each argv to be pointed at each word that is separated by NULL */
    for (l = 0; l < *argc; )
    {
        if ((*ptr == '$') && (!g_ignore) && (*(ptr + 1) != '\0'))
        {
            ++ptr;
            if (NULL == (argv[l] = getenv(ptr)))
            {
                (void) fprintf (stderr,"%s: Undefined variable\n",ptr);
                return P2INV_ARG;
            }
            ++l;
        }
        else if ((*ptr == '<') && (!g_ignore) && (*(ptr + 1) == '\0'))
        {
            ptr = ptr + 2;
            *argc -= 2;
            
            if (P2ERR == (g_redirect_in = open(ptr,O_RDONLY,S_IRUSR|S_IWUSR)))
            {
                perror ("open failed");
            }
        }
        else if ((*ptr == '>') && (!g_ignore) && (*(ptr + 1) == '\0'))
        {
            ptr = ptr + 2;
            *argc -= 2;
            
            if (P2ERR == (g_redirect_out = open(ptr,O_RDWR|O_CREAT|O_EXCL,S_IRUSR|S_IWUSR)))
            {
                perror ("open failed");
            }
        }
        else if ((*ptr == '>') && (!g_ignore) && (*(ptr + 1) == '>') && (*(ptr + 2) == '\0'))
        {
            ptr = ptr + 3;
            *argc -= 2;
            
            if (P2ERR == (g_redirect_app = open(ptr,O_RDWR|O_APPEND,S_IRUSR|S_IWUSR)))
            {
                perror ("open failed");
            }
        }
        else if ((*ptr == '|') && (!g_ignore) && (*(ptr + 1) == '\0'))
        {
            if (P2ERR == pipe(g_pipedes))
            {
                perror ("create pipe failed");
            }
            else
            {
                argv[l] = NULL;
                l++;
                g_argc2 = l;
            }
        }
        else
        {
            argv[l] = ptr;
            ++l;
        }
            
        while (*ptr)
        {
            ptr++;
        }

        ptr++;
    }
    
    g_ignore = 0;
    argv[l] = NULL;
    return count;
}


/********************************************************************
 * Function    : change_dir(int, char **)
 * Input       : char **  - argv is an array of char pointer which
                            holds the argv data.
 * Returns     : p2_ecode - returns P2ERR or P2SUCCESS.
 * 
 * Description : This function handle the different scenarios to
 *               change directory using chdir() system call
 ********************************************************************/
static p2_ecode change_dir(int argc, char **argv)
{
    char pwd[MAX_DIR_SZ];
    p2_ecode ecode = P2ERR;

    do
    {
        if (argc > CD_ARGC)
        {
            (void) fprintf(stderr, "Usage: %s directory\n", argv[0]);
            break;
        }

        /* if no argument is given with "cd", then switch to home directory */
        if (argc == 1)
        {
            argv[1] = home;
            argv[2] = NULL;
        }

        if (P2ERR == chdir(argv[1]))
        {
            perror("cd failed");
            break;
        }

        /* after changing the dir, print the current directory */
        (void) printf ("%s\n", getcwd(pwd, MAX_DIR_SZ));
        ecode = P2SUCCESS;
    
    } while (0);

    return ecode;
}


/********************************************************************
 * Function    : handle_pipe(char **)
 * Input       : char **  - argv is an array of char pointer which
                            holds the argv data.
 * Returns     : none
 * 
 * Description : This function handle the different scenarios of
 *               pipe command.
 ********************************************************************/
static void handle_pipe(char **argv)
{
    pid_t pid1, pid2;
    int devNull = open("/dev/null",0);

    /* first process to handle the command before the pipe */
    if (0 == (pid1 = fork()))
    {
        MYDUP2(g_redirect_in, STDIN_FILENO, devNull);
        CHK(dup2(g_pipedes[PIPE_OUT], STDOUT_FILENO));
        CHK(close(g_pipedes[PIPE_IN]));
        CHK(close(g_pipedes[PIPE_OUT]));

        if (P2ERR == execvp(argv[0], argv))
        {
            perror("before pipe execvp failed:");
        }
    }

    /* second process to handle the command after the pipe */
    if (0 == (pid2 = fork()))
    {
        MYDUP2(g_redirect_out, STDOUT_FILENO, devNull);
        MYDUP2(g_redirect_app, STDOUT_FILENO, devNull);

        (void) fflush(stdout);
        
        
        if (BACKGROUND == (BACKGROUND & g_mchar))
        {
            if (P2ERR == dup2(devNull, STDIN_FILENO))
                perror("dup2 failed:");
        }

        CHK(dup2(g_pipedes[PIPE_IN], STDIN_FILENO));
        CHK(close(g_pipedes[PIPE_IN]));
        CHK(close(g_pipedes[PIPE_OUT]));

        if (P2ERR == execvp(argv[0 + g_argc2], (argv + g_argc2)))
        {
            perror("after pipe execvp failed:");
        }
    }

    /* close the file descriptor, else the parent keeps its desc
       pointing at write end of pipe and thus read end always looking
       for the data and does not flushes its output onto STDOUT */
    CHK(close(g_pipedes[PIPE_IN]));
    CHK(close(g_pipedes[PIPE_OUT]));

    /* in case of background process, print the process name and 
       pid of first process */
    if (BACKGROUND == (BACKGROUND & g_mchar))
    {
        (void) printf ("%s [%d]\n", argv[0], pid1);
        (void) fflush(stdout); 
    }
    /* wait for fore ground child process only */ 
    else
    {
        for (;;)
        {
            pid_t pid;
            CHK(pid = wait(NULL));
       
            if (pid == pid2)
            {
                break;
            }
        }
    }

}


/********************************************************************
 * Function    : execute(char **)
 * Input       : char **  - argv is an array of char pointer which
                            holds the argv data.
 * Returns     : void
 * 
 * Description : This function handle the execution of shell commands
 *               by using execvp() system call in a different process
 ********************************************************************/
static void execute(char **argv)
{
    pid_t pid = 0;
    int devNull = open("/dev/null",0);
    
    if (g_isPipe)
    {
        g_isPipe == 1 ? handle_pipe(argv) : (void) fprintf(stderr, "single pipe supported\n");
        g_isPipe = 0;
    }
    else
    {
        /* create a process to execute the commands.
           Command to be executed in child process.
           If encountered with background process, then
           duplicate the stdin to /dev/null so that
           it does not grab the keyboard */
        if (0 == (pid = fork()))
        {
    
            if (BACKGROUND == (BACKGROUND & g_mchar))
            {
                if (P2ERR == dup2(devNull, STDIN_FILENO))
                    perror("dup2 failed:");
            }

            /* redirect the input and output for '<', '>' or '>>' */
            MYDUP2(g_redirect_in, STDIN_FILENO, devNull);
            MYDUP2(g_redirect_out, STDOUT_FILENO, devNull);
            MYDUP2(g_redirect_app, STDOUT_FILENO, devNull);

            (void) fflush(stdout);
        
            g_redirect_in = STDIN_FILENO;
            g_redirect_out = STDOUT_FILENO;
            g_redirect_app = STDOUT_FILENO;
            
            if (P2ERR == execvp(argv[0], argv))
            {
                perror("execvp failed:");
                exit(EXIT_FAILURE);
            }
        }
        /* --- Parent Process started --- */
        else if (0 < pid)
        {
            if (BACKGROUND == (BACKGROUND & g_mchar))
            {
                (void) printf ("%s [%d]\n", argv[0], pid);   
                (void) fflush(stdout); 
            }
            /* wait for fore ground child process only */ 
            else
            {
                for (;;)
                {
                    pid_t id;

                    if (P2ERR == (id = wait(NULL)))
                    {
                        perror ("wait");
                    }

                    if (id == pid)
                    {
                        break;
                    }
                }
            }
        }
        else
        {
            perror ("fork failed:");
        }
    }

}


/********************************************************************
 * Function    : addmode(char **, int *)
 * Input       : char **  - argv is an array of char pointer which
                            holds the argv data.
                 int *    - to update the mode of file
 * Returns     : p2_ecode - returns P2ERR or P2SUCCESS.
 * 
 * Description : This function adds the permission with the existing 
 *               permissions of input file.
 ********************************************************************/
static p2_ecode addmode(char **argv, int *mode)
{
    struct stat sb;
    p2_ecode ecode = P2SUCCESS;

    if (NULL != argv[2])
    {
        /* get the current status of the file */
        if (stat(argv[2], &sb) == P2ERR)
        {
            perror("stat");
            ecode = P2ERR;
        }
        /* add permissions with the current permissions of file */
        else
        {
            *mode = strtol (argv[1], (char **)NULL, BASE_OCTAL);
            *mode |= sb.st_mode;
        }
    }

    return ecode;
}


/********************************************************************
 * Function    : change_mode(int , char **)
 * Input       : int      - argument count
 *               char **  - argv is an array of char pointer which
 *                          holds the argv data.
 * Returns     : p2_ecode - returns P2ERR or P2SUCCESS.
 * 
 * Description : This function handle the addmod(), setmod() or chmod()
 *               It makes a call to chmod() system call and change the
 *               permissions of input file.
 ********************************************************************/
static p2_ecode change_mode(int argc, char **argv)
{
    int mode = 0;
    p2_ecode ecode = P2SUCCESS;

    do
    {
        if (CHMOD_ARGC != argc)
        {
            (void) fprintf(stderr, "Usage: %s mode filename\n", argv[0]);
            ecode = P2ERR;
            break;
        }

        /* addmod will simply changes the mode of the file by 
           overwritting the previous permission modes */
        if (P2SUCCESS == strcmp("addmod", argv[0]))
        {
            ecode = addmode(argv, &mode);
            
            if (P2ERR == (chmod (argv[2], mode)))
            {
                perror("chmod");
                ecode = P2ERR;
            }
        }

    } while (0);

    return ecode;
}



/********************************************************************
 * Function    : main()
 * Input       : none
 * Returns     : int - returns P2ERR or P2SUCCESS.
 * 
 * Description : This is main function which internally calls other
 *               methods to make it functioning as UNIX shell.
 ********************************************************************/
int main()
{
    char *argv[NUM_ARG] = {NULL};
    int argc = 0;
    char *env_name = NULL;
    p2_ecode ecode = P2SUCCESS;
   
    /* signal handler for SIGTERM */
    (void) signal (SIGTERM, myhandler);
     
    for (;;)
    {
        (void) printf ("p2: ");
        (void) fflush(stdout);

        /* call the parse function to get the values in *argv[] and argc */
        ecode = parse (&argc, argv);
        if (P2ERR == ecode)
        {
            break;
        }

        if ((NULL != argv[0]) && (P2INV_ARG != ecode))
        {
            /* no need to create a process for "cd" command.
               It need to be handled by system call chdir() */
            if ((P2SUCCESS == strcmp("cd", argv[0])))
            {
                (void) change_dir(argc, argv);
            }
            /* no need to create a process for wrong printenv args. */
            else if ((P2SUCCESS == strcmp("printenv", argv[0])) &&
                     (((NULL == argv[1]) || (NULL != argv[2]))  &&
                      (!g_isPipe)))
            {
                (void) fprintf (stderr,"printenv: wrong input\n");
            }
            /* no need to create a process for changing the environment variables.
               It need to be handled by system call putenv() */
            else if (P2SUCCESS == strcmp("setenv", argv[0]))
            {
                if ((NULL == argv[1]) || (NULL == argv[2]) || (NULL != argv[3]))
                {
                    (void) fprintf (stderr,"setenv: wrong input\n");
                }
                else
                {
                    /* doing malloc for "strlen + 2" to acomodate null character and '=' */
                    env_name = (char*) malloc (sizeof(char) * (strlen(argv[1]) + strlen(argv[2] + 2)));

                    (void*) strncpy (env_name, argv[1], strlen(argv[1])+1);
                    (void*) strcat (env_name, "=");
                    (void*) strcat (env_name, argv[2]);

                    (void) putenv (env_name);
                }

            }
            /* no need to create a process for changing the file modes.
               It need to be handled by system call chmod() */
            else if ((P2SUCCESS == strcmp("chmod", argv[0]))  ||
                     (P2SUCCESS == strcmp("setmod", argv[0])) ||
                     (P2SUCCESS == strcmp("addmod", argv[0])))
            {
                if (P2SUCCESS == strcmp("addmod", argv[0]))
                {
                    (void) change_mode (argc, argv);
                }
                else if (P2ERR == chmod (argv[2], strtol(argv[1], (char **) NULL, BASE_OCTAL)))
                {
                    perror ("chmod");
                }
            }
            /* process the rest of shell commands by doing execvp() in a child process */
            else
            {

                execute(argv);
            }
        }
    }

    /* killpg() sends signal to a process group and call its signal handler */
    (void) killpg(getpid(), SIGTERM);
    
    if (env_name)
    {
        free (env_name);
        env_name = NULL;
    }

    (void) printf ("p2 terminated.\n");
    return 0;
}


