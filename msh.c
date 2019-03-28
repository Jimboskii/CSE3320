/*
Jimmy Pham
1001193667
*/
#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 10     // Mav shell only supports five arguments

//Global variables
int status = 0;
pid_t child_pid, pid;
pid_t listpids[15];
int k = 0;
char *historylist[50];
int j = 0;
int historyinput;

static void SZ(int num)
{
}
static void SC(int num)
{
}

int main()
{


  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
  static int HistoryIndex = 0;
  while( 1 )
  {
    // Print out the msh prompt
    printf ("msh> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

    if(HistoryIndex < 50)
    {
      historylist[HistoryIndex] = strndup(cmd_str, MAX_COMMAND_SIZE);
      HistoryIndex++;
    }

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int   token_count = 0;

    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;

    char *working_str  = strdup( cmd_str );

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input stringswith whitespace used as the delimiter
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) &&
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }

    // Now print the tokenized input as a debug check
    // \TODO Remove this code and replace with your shell functionality

/////////////// Checking if the input is null
    if(token[0] == NULL)

      continue;

      //double checking inputs (getting so weird errors)
      // printf("%s\n", token[0]);

      //////////// Quit function
      if((strcmp("exit", token[0]) == 0)||(strcmp("quit", token[0]) == 0))
      {
          exit(0);
      }

      ///// Changing directories
      else if(strcmp("cd", token[0]) == 0)
      {

        chdir(token[1]);
      }




/////////// Listpids functionality
      else if(strcmp("listpids", token[0]) == 0)
      {
          if (listpids[j] != 0)
          {
            printf("%d: %d\n", j, listpids[j]);
          }
      }



      else if(strcmp("bg", token[0]) == 0)
      {
        kill(child_pid, SIGCONT);
      }


      else if(strcmp("history", token[0]) == 0)
      {
        int h;
        if((HistoryIndex - 50) > 0)
        {
          for(h = 0; h < HistoryIndex; h++)
          {
            printf("%d: %s", h, historylist[h]);
          }
        }

        else
        {
          for(h = 0; h < HistoryIndex; h++)
          {
           printf("%d: %s", h, historylist[h]);
          }
        }
      }

      else if((token[0][0]) == '!')
      {
        //Taking the part after ! to turn into an integer
        historyinput = atoi(&token[0][1]);
        printf("I have no idea how to do this.\n");
        printf("(=.=')\n");
      }
/////////// ls Function


          //If all else fails, fork
          else
          {
          child_pid = fork();
          listpids[k++] = child_pid;
          }
          //storing the pid value to be printed later using listpids
          //pid = child_pid;

//*********** Entering the child process
          if(child_pid == 0)
          {

            char * totalString = malloc(MAX_COMMAND_SIZE );

            //Concatenating a string to use in exec. If fails then will continue
            //All others below will do the same
            //Since you dont care about memory im just gonna malloc everything
            //instead of clearing the memory in the array

            strcat(totalString, "./");
            strcat(totalString, token[0]);
            execv(totalString,token);
            totalString = malloc(MAX_COMMAND_SIZE );


            strcat(totalString,"/usr/local/bin/");
            strcat(totalString, token[0]);
            execv(totalString,token);
            totalString = malloc(MAX_COMMAND_SIZE );


            strcat(totalString,"/usr/bin/");
            strcat(totalString, token[0]);
            execv(totalString,token);
            totalString = malloc(MAX_COMMAND_SIZE );


            strcat(totalString,"/bin/");
            strcat(totalString, token[0]);
            execv(totalString,token);
            totalString = malloc(MAX_COMMAND_SIZE );

            //If all else fails then command not found
            printf("%s: Command Not Found\n", token[0]);

            //Exiting out of the child process so that you dont have to type
            //"exit" or "quit" twice
            exit(0);
          }

          else
          {
            // Signal handling ctrl + z
            signal(SIGTSTP,SZ);
            //Signal handling ctrl + c
            signal(SIGINT, SC);
            // WUNTRACED
            // WNOHANG

            //Waiting for the child to finish
            waitpid(child_pid, &status, 0);
            fflush(NULL);

            //Getting the parent pid
          }

///////////////// History function



    free( working_root );
}

      return 0;
    }
