/*
 * [YOUR NAME HERE]
 *
 * CS441/541: Project 3
 *
 */
#ifndef MYSHELL_H
#define MYSHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//For open, close, and dup2
#include <unistd.h>
#include <fcntl.h>

/* For fork, exec, sleep */
#include <sys/types.h>
#include <unistd.h>
/* For waitpid */
#include <sys/wait.h>

/******************************
 * Defines
 ******************************/
#define TRUE  1
#define FALSE 0

#define MAX_COMMAND_LINE 1024
#define MAX_ARGS 100

#define MAX_JOBS 100
#define MAX_HISTORY 100
#define PROMPT ("mysh$ ")


/******************************
 * Structures
 ******************************/
/*
 * A job struct.  Feel free to change as needed.
 */
struct job_t {
    char * full_command;
    int argc;
    char **argv;
    int is_background;
    char * binary;
  int is_file;
  int last;
  char * input_file;
  char * output_file;
};
typedef struct job_t job_t;

typedef struct {
        pid_t pid;
        char full_command[1024];
        int is_done;
} bg_job_t;

bg_job_t bg_jobs[MAX_JOBS];
int num_jobs = 0;
//int cur_bg_jobs = 0;
/******************************
 * Global Variables
 ******************************/
char *batch_files[MAX_JOBS];
int num_batch_files = 0;
/*
 * Interactive or batch mode
 */
int is_batch = FALSE;

/*
 * Counts
 */
int total_jobs_display_ctr = 0;
int total_jobs    = 0;
int total_jobs_bg = 0;
int total_history = 0;
job_t * jobs;

//History Tracking
char *command_history[MAX_HISTORY];
int history_count = 0;

/*
 * Debugging mode
 */
int is_debug = TRUE;


/******************************
 * Function declarations
 ******************************/
/*
 * Parse command line arguments passed to myshell upon startup.
 *
 * Parameters:
 *  argc : Number of command line arguments
 *  argv : Array of pointers to strings
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int parse_args_main(int argc, char **argv);

/*
 * Main routine for batch mode
 *
 * Parameters:
 *  None
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int batch_mode(void);

/*
 * Main routine for interactive mode
 *
 * Parameters:
 *  None
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int interactive_mode(void);

/*
 * Launch a job
 *
 * Parameters:
 *   loc_job : job to execute
 *
 * Returns:
 *   0 on success
 *   Negative value on error 
 */
int launch_job(job_t * loc_job);

/*
 * Built-in 'exit' command
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int builtin_exit(void);

/*
 * Built-in 'jobs' command
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int builtin_jobs(void);

/*
 * Built-in 'history' command
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int builtin_history(void);

/*
 * Built-in 'wait' command
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int builtin_wait(void);

/*
 * Built-in 'fg' command
 *
 * Parameters:
 *   None (use default behavior)
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int builtin_fg(void);

/*
 * Built-in 'fg' command
 *
 * Parameters:
 *   Job id
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int builtin_fg_num(int job_num);
void check_bg_jobs();
void display_job_statistics(void);
void wait_for_all_background_jobs(void);
int count_running_background_jobs(void);
int should_wait_for_background_jobs(void);
int loop_through(char * line, int is_file);
int parse_command(char ** args, int argc, job_t * job);
#endif /* MYSHELL_H */
