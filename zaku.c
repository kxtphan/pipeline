/* This is the only file you will be editing and submit.
 * - Copyright of Starter Code: Prof. Yutao Zhong, George Mason University.  All Rights Reserved
 * - Copyright of Student Code: You!  
 * - Restrictions on Student Code: Do not post your code on any public site (eg. Github).
 * -- Feel free to post your code on a PRIVATE Github and give interviewers access to it.
 * -- You are liable for the protection of your code from others.
 * - Date: Oct 2024
 */

/* CS367 Project 3, Fall Semester, 2024
 * Fill in your Name, GNumber, and Section Number in the following comment fields
 * Name: Katherine Phan
 * GNumber: G01312303
 * Section Number: CS367-003            (Replace the _ with your section number)
 */

#include <sys/wait.h>
#include "zaku.h" // includes CONSTS MAXLINE (100), MAXARGS (25)
#include "parse.h" // includes instruction struct, parse(), is_whitespace(char), constructors/destructors, debug function
#include "util.h" // includes helper functions for strings
#include "logging.h" // includes logging functions and constants

/* Constants */
#define DEBUG 1 /* You can set this to 0 to turn off the debug parse information */
#define STOP_SHELL  0
#define RUN_SHELL   1


#define NUM_PATHS 2
#define NUM_INSTRUCTIONS 13

static const char *task_path[] = { "./", "/usr/bin/", NULL };
static const char *instructions[] = { "quit", "help", "list", "delete", "start_fg", "start_bg", "kill", "suspend", "resume_fg", "resume_bg", "fg", "pipe", "chain", NULL};
static int highestID = 1;

typedef struct task {
  int taskID; // task #
  int status;
  int exit_code;
  int PID;
  char cmd[MAXLINE];
  struct task* next;
} Task;

typedef struct task_list {
  int numTasks;
  Task* head;
  Task* tail;
} Task_List;

void initialize_TaskList(Task_List* list);
Task* findTaskNum(Task_List* list, int taskNum);
void deleteTask(Task_List* list, Task* task);
int isBuiltIn(char* cmd);
void addTask(Task_List* list, Task* task);
void startProcess(Task* task, char* inFile, char* outFile, int isFG);

/*-------------------------------------------*/
/*-------------------------------------------*/
/* The entry of your task manager program */
int main() {
    char *cmd = NULL;
    int do_run_shell = RUN_SHELL;

    /* Intial Prompt and Welcome */
    log_intro();
    log_help();

    /* Creating the task list and initializing its values */
    Task_List *taskList = (Task_List*)malloc(sizeof(Task_List));
    initialize_TaskList(taskList);

    /* Shell looping here to accept user command and execute */
    while (do_run_shell == RUN_SHELL) {
        char *argv[MAXARGS+1] = {0};        /* Argument list */
        Instruction inst = {0};           /* Instruction structure: check parse.h */

        /* Print prompt */
        log_prompt();
        
        /* Get Input - Allocates memory for the cmd copy */
        cmd = get_input(); 
        /* If the input is whitespace/invalid, get new input from the user. */
        if(cmd == NULL) {
          continue;
        }
        
        /* Parse the Command and Populate the Instruction and Arguments */
        initialize_command(&inst, argv);    /* initialize arg lists and instruction */
        parse(cmd, &inst, argv);            /* call provided parse() */

        if (DEBUG) {  /* display parse result, redefine DEBUG to turn it off */
          debug_print_parse(cmd, &inst, argv, "main (after parse)");
	      }

        /* After parsing: your code to continue from here */
        /*================================================
         *| - The command has been parsed and you have cmd, inst, and argv filled with data
         *| - Very highly recommended to start calling your own functions after this point.
         *=+==============================================*/

         /* Adding a task entry if not a built-in function for ZAKU. */
        if (!isBuiltIn(inst.instruct)) {
          Task *newTask = (Task *)malloc(sizeof(Task));
          strncpy(newTask->cmd, cmd, MAXLINE - 1);
          newTask->cmd[MAXLINE - 1] = '\0';
          newTask->next = NULL;
          newTask->status = LOG_STATE_READY;
          newTask->PID = 0;
          newTask->exit_code = 0;
          newTask->next = NULL;
          newTask->taskID = highestID;
          addTask(taskList, newTask); // function already adds in incrasing order
          log_task_init(newTask->taskID, cmd);
          highestID++;
        }

        /*==BUILT_IN: quit===*/
        /* Added log_quit() to print out the predefined information. */
        if (strcmp(inst.instruct, "quit")==0){
          do_run_shell = STOP_SHELL;  /*set the main loop to exit when you finish processing it */
          log_quit();
        }
        
        /*BUILT_IN: help, logging the log_help() to print out predefined information. */
        if (strcmp(inst.instruct, "help")==0) {
          log_help();
        }

        /*BUILT_IN: list, lists the total number of existing tasks as well as their details. */
        if (strcmp(inst.instruct, "list")==0) {
          log_num_tasks(taskList->numTasks);
          Task* curr = taskList->head;
          /* Tasks should be listed in order of INCREASING task #. */
          for (int i = 0; i < taskList->numTasks; i++) {
            log_task_info(curr->taskID, curr->status, curr->exit_code, curr->PID, curr->cmd);
            curr = curr->next;
          }
        }

        /*BUILT_IN: delete, removes a task using its task number from list. */
        if (strcmp(inst.instruct, "delete")==0) {
          int taskID = inst.num;
          if (findTaskNum(taskList, taskID) != NULL) {
            Task* task = findTaskNum(taskList, taskID);
            // If the task is idle (READY 0, FINISHED 3, KILLED 4), it can be deleted.
            if (task->status == LOG_STATE_READY || task->status == LOG_STATE_FINISHED || task->status == LOG_STATE_KILLED) {
              deleteTask(taskList, task);
              log_delete(taskID);
            }
            else {
              log_status_error(task->taskID, task->status);
            }
          }
          else {
            log_task_num_error(taskID);
          }
        }
        
        /* BUILT_IN: start_fg, forks a task in the foreground, shell must wait for task to finish. */
        if (strcmp(inst.instruct, "start_fg")==0) {
          int taskID = inst.num;
          char *inFile = inst.infile;
          char *outFile = inst.outfile;

          // Log error if the task number does not exist.
          Task* task = findTaskNum(taskList, taskID);
          if (task == NULL) {
            log_task_num_error(taskID);
          }

          // Log error if the task is currently busy (RUNNING, SUSPENDED).
          if (task->status == LOG_STATE_RUNNING || task->status == LOG_STATE_SUSPENDED) {
            log_status_error(taskID, task->status);
          }

          startProcess(task, inFile, outFile, 1);
        }

        /* BUILT_IN: start_bg, runs a non-interactive program that system does not have to wait to finish for. */
        if (strcmp(inst.instruct, "start_bg")==0) {
          int taskID = inst.num;
          char *inFile = inst.infile;
          char *outFile = inst.outfile;

          // log error if the task number does not exist.
          Task* task = findTaskNum(taskList, taskID);
          if (task == NULL) {
            log_task_num_error(taskID);
          }

          // log error if the task is currently busy (RUNNING, SUSPENDED).
          if (task->status == LOG_STATE_RUNNING || task->status == LOG_STATE_SUSPENDED) {
            log_status_error(taskID, task->status);
          }

          startProcess(task, inFile, outFile, 0);
        }

        /*.===============================================.
         *| After your code: We cleanup before Looping to the next command.
         *| free_command WILL free the cmd, inst and argv data.
         *| Make sure you've copied any needed information to your Task first.
         *| Hint: You can use the util.c functions for copying this information.
         *+===============================================*/

        free_command(cmd, &inst, argv);
	      cmd = NULL;
    }  // end while

    return 0;
}  // end main()

/* Helper function to initialize the tasklist. */
void initialize_TaskList(Task_List* list) {
  list->head = NULL;
  list->tail = NULL;
  list->numTasks = 0;
}

/* Helper function to find if the task number exists in the task list, if found return task, if not return null. */
Task* findTaskNum(Task_List* list, int taskNum) {
  Task* curr = list->head;
  while (curr != NULL) {
    if (curr->taskID == taskNum) {
      return curr; // task num found
    }
    curr = curr->next;
  }
  return NULL; // task num not found
}

/* Helper function to delete a task within the task list assuming it exists. */
void deleteTask(Task_List* list, Task* task) {
  Task* curr = list->head;
  Task* prev = NULL;

  while (curr != NULL) {
    if (curr == task) {
      if (curr == list->head) {
        list->head = curr->next;
      }
      else {
        prev->next = curr->next;
      }

      if (curr == list->tail) {
        list->tail = prev;
      }
    
      free(curr);
      list->numTasks--;
      return;
    }
    prev = curr;
    curr = curr->next;
  }
}

/* Helper function to identify if user inputted command is a built-in or not. */
int isBuiltIn(char* instruct) {
  int length = (int) (sizeof(instructions) / sizeof(instructions[0]));
  for (int i = 0; i < length - 1; i++) {
    // If command is found in built-in, return true.
    if (strcmp(instruct, instructions[i])==0) {
      return 1;
    }
  }
  // Return false if not a built-in command.
  return 0;
}

/* Helper function to add a task to the task list in increasing order. */
void addTask(Task_List* list, Task* task) {
  Task* curr = list->head;
  Task* prev = NULL;

  if (curr == NULL) {
    list->head = task;
    list->tail = task;
    task->next = NULL;
  }
  else {
    while (curr != NULL && curr->taskID < task->taskID) {
      prev = curr;
      curr = curr->next;
    }

    if (prev == NULL) {
      task->next = list->head;
      list->head = task;
    }
    else {
      task->next = curr;
      prev->next = task;
    }

    if (curr == NULL) {
      list->tail = task;
    }
  }

  list->numTasks++;
}

/* Helper function to start a process by forking. */
void startProcess(Task* task, char* inFile, char* outFile, int isFG) {
  pid_t pid = fork();
  // Adult process 1
  if (pid) {
    task->status = LOG_STATE_RUNNING;
    log_status_change(task->taskID, pid, isFG ? LOG_FG : LOG_BG, task->cmd, LOG_START);

    // Adult process will wait for child process to terminate only if foreground process.
    int status;
    if (isFG) {
      wait(&status);
    }
    
    // If child process terminated, change its state to finishe based on termination.
    if (WIFEXITED(status)) {
      task->exit_code = WEXITSTATUS(status);
      task->status = LOG_STATE_FINISHED;
      log_status_change(task->taskID, pid, isFG ? LOG_FG : LOG_BG, task->cmd, LOG_TERM);
    // If child process finishes, change its state to killed based on termination.
    } else if (WIFSIGNALED(status)) {
      task->exit_code = WIFSIGNALED(status);
      task->status = LOG_STATE_KILLED;
      log_status_change(task->taskID, pid, isFG ? LOG_FG : LOG_BG, task->cmd, LOG_TERM_SIG);
    // If child process was paused by a signal, change its state to suspended.
    } else if (WIFSTOPPED(status)) {
      task->exit_code = WIFSTOPPED(status);
      task->status = LOG_STATE_SUSPENDED;
      log_status_change(task->taskID, pid, isFG ? LOG_FG : LOG_BG, task->cmd, LOG_SUSPEND);
    // If child process continues by a signal, change its state to resume.
    } else if (WIFCONTINUED(status)) {
      task->exit_code = WIFCONTINUED(status);
      task->status = LOG_STATE_RUNNING;
      log_status_change(task->taskID, pid, isFG ? LOG_FG : LOG_BG, task->cmd, LOG_RESUME);
    }
    task->PID = pid;
  }
  // Child process 0
  else {
    // Placing the child process into a different process group.
    setpgid(0,0);

    if (inFile != NULL) {
      // Child process's input should be redirected from this file.
      int fd_in = open(inFile, O_RDONLY);
      // If redirect file cannot be opened, log error.
      if (fd_in == -1) {
        log_file_error(task->taskID, inFile);
        exit(EXIT_FAILURE);
      }
      // Changing default fd to new fd
      dup2(fd_in, STDIN_FILENO);
      close(fd_in);
      // Logging the file redirect
      log_redir(task->taskID, LOG_REDIR_IN, inFile);
    }

    if (outFile != NULL) {
      // Child process's output should be redirected to the file.
      int fd_out = open(outFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
      // If redirect file cannot be opened, log error.
      if (fd_out == -1) {
        log_file_error(task->taskID, outFile);
        exit(EXIT_FAILURE);
      }
      // Changing default fd to new fd
      dup2(fd_out, STDOUT_FILENO);
      close(fd_out);
      // Logging the file redirect
      log_redir(task->taskID, LOG_REDIR_OUT, outFile);
    }

    // Will run the exec()
    char path1[MAXLINE];
    char path2[MAXLINE];
    char* argv[MAXARGS + 1] = {NULL};
    char* curr = strtok(task->cmd, " ");
    int ind = 0;

    // Constructing the task's argument list into a char array
    while (curr != NULL && ind < MAXARGS) {
      argv[ind++] = curr;
      curr = strtok(NULL, " ");
    }
    argv[ind] = NULL;

    // Concatenating the paths to execute
    snprintf(path1, sizeof(path1), "%s%s", task_path[0], argv[0]);
    snprintf(path2, sizeof(path2), "%s%s", task_path[1], argv[0]);

    printf("%s\n", path1);
    printf("%s", path2);

    execv(path1, argv);
    execv(path2, argv);

    // If both execution paths do not work, log start error and exit with code 1.
    log_start_error(task->cmd);
    exit(EXIT_FAILURE);

    // Task terminated with no interruptions.
    exit(EXIT_SUCCESS);
  }
}