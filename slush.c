//David Philiphose
//9-22-17
//Creating shell that executes basic commands line functins, programs, and piping

#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>

//function to fork() and exec() for individual processes
int execProcess(pid_t process, char* file, char* const* args) {
   int x;
   process = fork();
   if (process < 0) { //failed fork
      perror("Error fork()'ing");
      return(-1);
   }
   else if (process > 0) { //parent fork
      waitpid(process, NULL, 0);
   }
   else { //child fork
      x = execvp(file, args);
      if (x == -1) {
         perror("Error during execution");
         return (-1);
      }
   }
}

//function for piping multiple processes together
int pipeProcess(pid_t process, int count, char* file, char* const* args) {
   int x; int i;
   int reader;
   int pipefd[2];
   int execPipe;

   execPipe = pipe(pipefd); //initialize the pipe
   if (execPipe == (-1)){
      perror("Error creating pipe");
      return (-1);
   }

   process = fork(); 
   if (process < 0) { //failed fork
      perror("Error fork()'ing");
      return (-1);
   }
   else if (process > 0) { //parent fork
      close(pipefd[1]);
      if (reader !=0)
         close(reader);
      reader = pipefd[0];
      waitpid(process, NULL, 0);
   }
   else { //child fork
      if (i == (count-1)) { //first command
         dup2(pipefd[1], STDOUT_FILENO);
         close(pipefd[0]);
         x = execvp(file, args);
         if (x == (-1)) {
            perror("Error in execution");
            return(-1);
         }
      }
      else if (i == 0) { //last command
         dup2(reader, STDIN_FILENO);
         x = execvp(file, args);
         if (x == (-1)) {
            perror ("Error in execution");
            return (-1);
         }
      }
      else { //intermediate commands
         dup2(reader, STDIN_FILENO);
         dup2(reader, STDOUT_FILENO);
         close(pipefd[0]);
         x = execvp(file, args);
         if (x == (-1)) {
            perror("Error in execution");
            return (-1);
         }
      }
   }
}

int main(int argc, char* argv[]) {

   //entering slush program
   printf("Entering slush\n");
   sleep(1);
   printf("You are now using the slush terminal\n");
   printf("use the following format: prog_1[args] ( prog_2[args] ( ...\n");

   int loop = 1;
   char cwd[256];
   //loop for slush to stay active
   while (loop == 1) {

      //displays current directory
      if (getcwd(cwd, sizeof(cwd)) != NULL) {
         printf("[slush]"); 
         printf("%s", cwd);
         printf("$ ");
      }
      else {
         perror("cwd error");
         return(-1);
      }

      //input, strips trailing '\n', checks for syntax errors
      char input[256];
      char* strip;
      char* cmd;

      fgets(input, sizeof(input), stdin);
      strip = strtok(input, "\n");
      if (strip[strlen(strip)-1] == '(') {
         printf("invalid input\n");
         continue;;
      }
      if (strip[0] == '(') {
         printf("invalid input\n");
         continue;;
      }

      //handle 'cd'
      int change;
      if (strncmp(strip, "cd", 2) == 0) {
         change = chdir((strip+3));
         if (change != 0) {
            perror("Error changing directory");
            return (-1);
         }
         continue;
      }

      //input into character array for exec()
      char* cmds[15];
      char** next = cmds;
      int cmdCount = 0;

      cmd = strtok(strip, "(");
      while (cmd != NULL) {
         cmds[cmdCount] = cmd;
         cmd = strtok(NULL, "(");
         cmdCount++;
      }

      //parse inputs for arguments
      char* arg;
      char* args[15];
      int i;
      int argCount = 0;
      pid_t caller;
      
      for (i = (cmdCount - 1); i > (-1); i--) {
         arg = strtok(cmds[i], " ");
         args[0] = arg;
         while (arg != NULL) {
            args[argCount] = arg;
            arg = strtok(NULL, " ");
            argCount++;
         }
         args[argCount] = '\0';

         //if one command, call execProcess
         //if more than one, call pipeProcess
         pid_t caller;
         if (cmdCount == 1) 
            execProcess(caller, cmds[0], args);
         else if (cmdCount > 1)
            pipeProcess(caller, cmdCount, cmds[0], args);
      }
   }
   return 0;
}
