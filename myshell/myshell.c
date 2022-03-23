#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#define MAX_BUFFER 1024    // max line buffer
#define MAX_ARGS 64        // max # args
#define SEPARATORS " \t\n" // token sparators
#define README "readme"    // help file name

struct shellstatus_st
{
    int foreground;  // foreground execution flag
    char *infile;    // input redirection flag & file
    char *outfile;   // output redirection flag & file
    char *outmode;   // output redirection mode
    char *shellpath; // full pathname of shell
};
typedef struct shellstatus_st shellstatus;

extern char **environ;

void check4redirection(char **, shellstatus *);            // check command line for i/o redirection
void errmsg(char *, char *);                               // error message printout
void execute(char **, shellstatus);                        // execute command from arg array
char *getcwdstr(char *, int);                              // get current work directory string
FILE *redirected_op(shellstatus);                          // return required o/p stream
char *stripath(char *);                                    // strip path from filename
void syserrmsg(char *, char *);                            // system error message printout
static void delete_io_symbol(char **args, int loc, int k); // delete redirection symbol
/*******************************************************************/

int main(int argc, char **argv) // argc default is 1
{
    FILE *ostream = stdout;   // (redirected) o/p stream
    FILE *instream = stdin;   // batch/keyboard input
    char linebuf[MAX_BUFFER]; // line buffer
    char cwdbuf[MAX_BUFFER];  // cwd buffer
    char *args[MAX_ARGS];     // pointers to arg strings
    char **arg;               // working pointer thru args
    char *prompt = "==>";     // shell prompt
    char *readmepath;         // readme pathname
    shellstatus status;       // status structure
    int execargs;             // execute command in args
    // parse command line for batch input
    switch (argc)
    {
    case 1: // ./myshell
    {
        // keyboard input
        // TODO
        break;
    }

    case 2: // 批处理
    {
        // possible batch/script
        // TODO
        FILE *inp = fopen(argv[1], "r");
        if (!inp)
        {
            errmsg("no such file", "");
            fprintf(ostream, "\n");
            return 0;
        }
        instream = inp; // replace instream with file stream
        break;
    }
    default: // too many arguments
        fprintf(stderr, "%s command line error; max args exceeded\n"
                        "usage: %s [<scriptfile>]",
                stripath(argv[0]), stripath(argv[0]));
        exit(1);
    }

    // get starting cwd to add to readme pathname
    // TODO
    getcwdstr(cwdbuf, MAX_BUFFER); //
    readmepath = malloc(MAX_BUFFER);
    strcpy(readmepath, cwdbuf);
    strcat(readmepath, "/");
    strcat(readmepath, README);

    // get starting cwd to add to shell pathname
    // TODO
    status.shellpath = malloc(MAX_BUFFER);
    strcpy(status.shellpath, cwdbuf);
    strcat(status.shellpath, "/");
    strcat(status.shellpath, "myshell");

    // set SHELL= environment variable, malloc and store in environment
    //  TODO
    setenv("SHELL", status.shellpath, 1);

    signal(SIGINT, SIG_IGN);  // prevent ^C interrupt
    signal(SIGCHLD, SIG_IGN); // prevent Zombie children

    // keep reading input until "quit" command or eof of redirected input
    while (!feof(instream))
    {
        // (re)initialise status structure
        status.foreground = TRUE; // default mode is foreground

        // set up prompt
        // TODO
        getcwdstr(cwdbuf, MAX_BUFFER); // renew cwdbuffer
        printf("%s", cwdbuf);
        printf("%s", prompt);

        // get command line from input
        if (fgets(linebuf, MAX_BUFFER, instream))
        {
            // read a line
            // tokenize the input into args array
            arg = args;
            *arg++ = strtok(linebuf, SEPARATORS); // tokenize input
            while ((*arg++ = strtok(NULL, SEPARATORS)))
                ;

            // last entry will be NULL
            if (args[0])
            {
                // check for i/o redirection
                check4redirection(args, &status); // set status and delete some symbols: < > >> &
                ostream = redirected_op(status);  // set output redirection
                execargs = TRUE;                  // set default execute of args
                // check for internal/external commands
                // Internal commands
                // "cd" command
                if (!strcmp(args[0], "cd"))
                {
                    // TODO
                    char *output = malloc(MAX_BUFFER);
                    if (!args[1])
                    {
                        getcwd(output, MAX_BUFFER);
                        fprintf(ostream, "%s\n", output);
                    }
                    else if (!chdir(args[1]))
                    {
                        char *path = malloc(MAX_BUFFER);
                        strcat(path, "PWD=");
                        getcwd(output, MAX_BUFFER);
                        fprintf(ostream, "%s\n", output);
                        strcat(path, output);
                        putenv(path); // change PWD
                    }
                    else
                    {
                        errmsg("no such directory", "");
                    }
                    execargs = FALSE; // no need for further exec
                }
                // "clr" command
                else if (!strcmp(args[0], "clr"))
                {
                    // TODO
                    args[0] = "clear";
                    args[1] = NULL;
                    execute(args, status);
                    execargs = FALSE; // no need for further exec
                }
                // "dir" command
                else if (!strcmp(args[0], "dir"))
                {
                    // TODO
                    for (int i = MAX_ARGS - 1; i > 1; i--)
                    {
                        args[i] = args[i - 1];
                    }
                    args[0] = "ls";
                    args[1] = "-al";
                    if (!args[2]) // if arg[2] is empty
                    {
                        args[2] = ".";
                        args[3] = NULL;
                    }
                    execute(args, status);
                    execargs = FALSE; // no need for further exec
                }
                // "echo" command
                else if (!strcmp(args[0], "echo"))
                {
                    // TODO
                    int i = 1;
                    while (args[i])
                    {
                        fprintf(ostream, "%s ", args[i]);
                        i++;
                    }
                    fprintf(ostream, "\n");
                    execargs = FALSE; // no need for further exec
                }
                // "environ" command
                else if (!strcmp(args[0], "environ"))
                {
                    // TODO
                    int en_k = 0;
                    while (environ[en_k])
                    {
                        fprintf(ostream, "%s \n", environ[en_k]);
                        en_k++;
                    }
                    execargs = FALSE; // no need for further exec
                }
                // "help" command
                else if (!strcmp(args[0], "help"))
                {
                    args[0] = "more";
                    args[1] = readmepath;
                    args[2] = NULL;
                    execute(args, status);
                    execargs = FALSE; // no need for further exec
                }
                // "pause" command - note use of getpass - this is a made to measure way of turning off
                //  keyboard echo and returning when enter/return is pressed
                else if (!strcmp(args[0], "pause"))
                {
                    // TODO
                    getpass("Press Enter to continue ... \n");
                    execargs = FALSE;
                }
                // "quit" command
                else if (!strcmp(args[0], "quit"))
                {
                    execargs = FALSE; // no need for further exec
                    break;
                }
                // else pass command on to OS shell
                // external commands
                // TODO
                if (execargs)
                    execute(args, status);
            }
        }
    }
    return 0;
}

/***********************************************************************

void check4redirection(char ** args, shellstatus *sstatus);

check command line args for i/o redirection & background symbols
set flags etc in *sstatus as appropriate

***********************************************************************/

void check4redirection(char **args, shellstatus *sstatus) // only set sstatus, the io stream redirection is in other space
{
    sstatus->infile = NULL; // set defaults
    sstatus->outfile = NULL;
    sstatus->outmode = NULL;
    char **tmp = args;
    int k = 0;
    while (*args)
    {
        // input redirection
        if (!strcmp(*args, "<"))
        {
            // TODO

            sstatus->infile = *(args + 1); // input file name
            delete_io_symbol(tmp, k, 1);   // delete "<" from args
        }
        // output direction
        else if (!strcmp(*args, ">") || !strcmp(*args, ">>"))
        {
            // TODO
            if (!strcmp(*args, ">"))
            {
                sstatus->outmode = ">";
            }
            else
            {
                sstatus->outmode = ">>";
            }
            sstatus->outfile = *(args + 1);
            delete_io_symbol(tmp, k, 2);
            args--;
            k--; // avoid skipping args faultly
        }
        else if (!strcmp(*args, "&"))
        {
            // TODO
            sstatus->foreground = FALSE; // background mode
            delete_io_symbol(tmp, k, 1);
        }
        args++;
        k++;
    }
}

/***********************************************************************

  void execute(char ** args, shellstatus sstatus);

  fork and exec the program and command line arguments in args
  if foreground flag is TRUE, wait until pgm completes before
    returning

***********************************************************************/

void execute(char **args, shellstatus sstatus)
{
    int status;
    pid_t child_pid;
    char tempbuf[MAX_BUFFER];

    switch (child_pid = fork())
    {
    case -1:
        syserrmsg("fork", NULL);
        break;
    case 0:
        // execution in child process
        // reset ctrl-C and child process signal trap
        signal(SIGINT, SIG_DFL);
        signal(SIGCHLD, SIG_DFL);

        // i/o redirection */
        // TODO
        freopen(sstatus.infile, "r", stdin);
        if (sstatus.outmode == ">")
        {
            freopen(sstatus.outfile, "w", stdout);
        }
        else
        {
            freopen(sstatus.outfile, "a", stdout);
        }

        // set PARENT = environment variable, malloc and put in nenvironment
        // TODO
        setenv("PARENT", sstatus.shellpath, 1);
        // final exec of program

        execvp(args[0], args);
        syserrmsg("exec failed -", args[0]);
        exit(127);
    }

    // continued execution in parent process
    // TODO

    if (sstatus.foreground)
    {
        int stat_val;
        waitpid(child_pid, &stat_val, WUNTRACED); // CONTINUE UNTIL CHILD STOP

        // if (WIFSIGNALED(stat_val))
        //     printf("Child terminated abnormally, signal %d\n", WTERMSIG(stat_val));
        return;
    }
    else
    {
        int stat_val;
        waitpid(child_pid, &stat_val, WNOHANG); // CONTINUE STRICTLY

        // if (WIFSIGNALED(stat_val))
        //     printf("Child terminated abnormally, signal %d\n", WTERMSIG(stat_val));
        return;
    }
}

/***********************************************************************

 char * getcwdstr(char * buffer, int size);

return start of buffer containing current working directory pathname

***********************************************************************/

char *getcwdstr(char *buffer, int size)
{
    // TODO
    getcwd(buffer, size);
    return buffer;
}

/***********************************************************************

FILE * redirected_op(shellstatus ststus)

  return o/p stream (redirected if necessary)

***********************************************************************/

FILE *redirected_op(shellstatus status)
{
    FILE *ostream = stdout;

    // TODO
    if (status.outfile)
    {
        FILE *outf;
        if (access(status.outfile, 0))
        {
            if (!strcmp(status.outmode, ">"))
            {
                outf = fopen(status.outfile, "w");
            }
            else
            {
                outf = fopen(status.outfile, "a");
            }
        }
        else
        {
            if (!strcmp(status.outmode, ">"))
            {
                outf = fopen(status.outfile, "w+");
            }
            else
            {
                outf = fopen(status.outfile, "a+");
            }
        }
        ostream = outf;
    }

    return ostream;
}

/*******************************************************************

  char * stripath(char * pathname);

  strip path from file name

  pathname - file name, with or without leading path

  returns pointer to file name part of pathname
            if NULL or pathname is a directory ending in a '/'
                returns NULL
*******************************************************************/

char *stripath(char *pathname)
{
    char *filename = pathname;

    if (filename && *filename)
    {                                      // non-zero length string
        filename = strrchr(filename, '/'); // look for last '/'
        if (filename)                      // found it
            if (*(++filename))             //  AND file name exists
                return filename;
            else
                return NULL;
        else
            return pathname; // no '/' but non-zero length string
    }                        // original must be file name only
    return NULL;
}

/********************************************************************

void errmsg(char * msg1, char * msg2);

print an error message (or two) on stderr

if msg2 is NULL only msg1 is printed
if msg1 is NULL only "ERROR: " is printed
*******************************************************************/

void errmsg(char *msg1, char *msg2)
{
    fprintf(stderr, "ERROR: ");
    if (msg1)
        fprintf(stderr, "%s; ", msg1);
    if (msg2)
        fprintf(stderr, "%s; ", msg2);
    return;
    fprintf(stderr, "\n");
}

/********************************************************************

  void syserrmsg(char * msg1, char * msg2);

  print an error message (or two) on stderr followed by system error
  message.

  if msg2 is NULL only msg1 and system message is printed
  if msg1 is NULL only the system message is printed
 *******************************************************************/

void syserrmsg(char *msg1, char *msg2)
{
    errmsg(msg1, msg2);
    perror(NULL);
    return;
}

// delete_io_symbol
// loc: args location   k: bits to delete
static void delete_io_symbol(char **args, int loc, int k)
{
    int j;
    for (j = loc; args[j + k]; ++j)
        args[j] = args[j + k];
    for (int i = 0; i <= k; i++)
    {
        args[j + i] = NULL;
    }
}