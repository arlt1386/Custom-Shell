/*
 * [YOUR NAME HERE]
 *
 * CS441/541: Project 3
 *
 */
#include "mysh.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char * argv[]) {
  int ret;

    /*
     * Parse Command line arguments to check if this is an interactive or batch
     * mode run.
     */
    if( 0 != (ret = parse_args_main(argc, argv))) {
        fprintf(stderr, "Error: Invalid command line!\n");
        return -1;
    }

    /*
     * If in batch mode then process all batch files
     */
    if( TRUE == is_batch) {
        if( TRUE == is_debug ) {
            printf("Batch Mode!\n");
        }

        if( 0 != (ret = batch_mode()) ) {
            fprintf(stderr, "Error: Batch mode returned a failure!\n");
        }
    }
    /*
     * Otherwise proceed in interactive mode
     */
    else if( FALSE == is_batch ) {
        if( TRUE == is_debug ) {
            printf("Interactive Mode!\n");
        }

        if( 0 != (ret = interactive_mode()) ) {
            fprintf(stderr, "Error: Interactive mode returned a failure!\n");
        }
    }
    /*
     * This should never happen, but otherwise unknown mode
     */
    else {
        fprintf(stderr, "Error: Unknown execution mode!\n");
        return -1;
    }


    /*
     * Display counts
     */
    printf("-------------------------------\n");
    printf("Total number of jobs               = %d\n", total_jobs);
    printf("Total number of jobs in history    = %d\n", total_history);
    printf("Total number of jobs in background = %d\n", total_jobs_bg);

    /*
     * Cleanup
     */


    return 0;
}

int parse_args_main(int argc, char **argv)
{

    //  If no command line arguments were passed then this is interactive mode

    if(argc == 1) {
        is_batch = FALSE;
    }

     // If command line arguments were supplied then this is batch mode.

    else {
        is_batch = TRUE;

        // Store batch file names from command line arguments
        for (int i = 1; i < argc; i++) {
            if (num_batch_files < MAX_JOBS) {
                batch_files[num_batch_files] = strdup(argv[i]);
                num_batch_files++;

                if (is_debug) {
                    printf("Debug: Added batch file: %s\n", argv[i]);
                }
            } else {
                fprintf(stderr, "Error: Too many batch files (max: %d)\n", MAX_JOBS);
                return -1;
            }
        }

        if (is_debug) {
            printf("Debug: Total batch files: %d\n", num_batch_files);
        }
    }

    return 0;
}




int batch_mode(void)
{
    FILE *file;
    char buf[MAX_COMMAND_LINE];
    char *line;
    int exit_requested = 0;

    if (is_debug) {
        printf("DEBUG: Starting batch mode with %d files\n", num_batch_files);
        for (int i = 0; i < num_batch_files; i++) {
            printf("DEBUG: Batch file %d: %s\n", i, batch_files[i]);
        }
    }

    for (int i = 0; i < num_batch_files && !exit_requested; i++) {
        file = fopen(batch_files[i], "r");
        if (file == NULL) {
            fprintf(stderr, "Error: Cannot open batch file '%s'\n", batch_files[i]);
            continue;
        }  if (is_debug) {
            printf("Debug: Processing batch file: %s\n", batch_files[i]);
        }

        // read one line at a time
        while (fgets(buf, sizeof(buf), file) != NULL && !exit_requested) {
            // Strip off newline character
            line = buf;
            size_t len = strlen(line);
            if (len > 0 && line[len-1] == '\n') {
                line[len-1] = '\0';
            }

            // Skip empty lines
            if (strlen(line) == 0) {
                continue;
            }

            if (is_debug) {
                printf("DEBUG: Read line: '%s'\n", line);
            }

            // Count how many commands are on this line by counting & and ; characters
            int command_count = 1;
            for (char *p = line; *p; p++) {
                if (*p == '&' || *p == ';') command_count++;
            }

            if (is_debug) {
                printf("DEBUG: Found %d commands on this line\n", command_count);
            } // Split the line into multiple commands using both & and ; (this was affecting counting num of jobs)
            char *command;
            char *saveptr;
            char *line_copy = strdup(line);

            int current_command = 0;
            command = strtok_r(line_copy, "&;", &saveptr);  // Split on both & and ;
            while (command != NULL && !exit_requested) {
                current_command++;

                // Trim whitespace from the command
                char *start = command;
                char *end = command + strlen(command) - 1;

                // Trim leading spaces
                while (*start == ' ' || *start == '\t') {
                    start++;
                }

                // Trim trailing spaces  
                while (end > start && (*end == ' ' || *end == '\t')) {
                    *end = '\0';
                    end--;
                }

                // Skip empty commands after trimming
                if (strlen(start) == 0) {
                    command = strtok_r(NULL, "&;", &saveptr);
                    continue;
                }

                if (is_debug) {
                    printf("DEBUG: Processing command %d/%d: '%s'\n", current_command, command_count, start);
                } // Parse the command line into arguments
                job_t job;
                memset(&job, 0, sizeof(job_t));

                job.full_command = strdup(start);

                // Determine if this is a background job:
                job.is_background = 0;

                if (current_command < command_count) {
                    char *cmd_end = strstr(line, start) + strlen(start);
                    // Skip whitespace after the command
                    while (*cmd_end == ' ' || *cmd_end == '\t') {
                        cmd_end++;
                    }
                    // Check if the next separator is &
                    if (*cmd_end == '&') {
                        job.is_background = 1;
                    }
                }

                // Parse command into arguments to check if it's a built-in
                char *token;
                char *args_saveptr;
                char *command_copy = strdup(start);
                job.argc = 0;
                job.argv = malloc(sizeof(char*) * MAX_COMMAND_LINE);

                token = strtok_r(command_copy, " \t", &args_saveptr);
                while (token != NULL && job.argc < MAX_COMMAND_LINE - 1) {
                    job.argv[job.argc] = strdup(token);
                    job.argc++;
                    token = strtok_r(NULL, " \t", &args_saveptr);
                }
                job.argv[job.argc] = NULL;
                if (job.argc > 0) {
                    job.binary = strdup(job.argv[0]);

                    // Built-in commands should not run in background
                    if (strcmp(job.argv[0], "jobs") == 0 ||
                        strcmp(job.argv[0], "history") == 0 ||
                        strcmp(job.argv[0], "wait") == 0 ||
                        strcmp(job.argv[0], "fg") == 0 ||
                        strcmp(job.argv[0], "exit") == 0) {
                        job.is_background = 0;
                        if (is_debug) {
                            printf("DEBUG: Built-in command '%s' forced to foreground\n", job.argv[0]);
                        }
                    }
                }

                if (history_count < MAX_HISTORY && job.argc > 0) {
                   command_history[history_count] = strdup(job.full_command);
                         history_count++;
                }

               // Handle built-in commands
               if (job.argc > 0) {
                total_history++;  // Count ALL commands in history(problem fixed)

                if (strcmp(job.argv[0], "jobs") == 0) {
                    builtin_jobs();
                } else if (strcmp(job.argv[0], "history") == 0) {
                    builtin_history();
                } else if (strcmp(job.argv[0], "wait") == 0) {
                    wait_for_all_background_jobs();
                } else if (strcmp(job.argv[0], "fg") == 0) {
                    if (job.argc == 1) {
                        builtin_fg();
                    } else {
                        char *endptr;
                        int job_num = strtol(job.argv[1], &endptr, 10);
                        if (*endptr == '\0') {
                            builtin_fg_num(job_num);
                        } else {
                            fprintf(stderr, "Error: Invalid job number\n");
                        }
                    }
                } else if (strcmp(job.argv[0], "exit") == 0) {
                    exit_requested = 1;
                    builtin_exit();
                    if (is_debug) {
                        printf("Debug: Exit command encountered, stopping batch processing\n");
                    }
                    // Cleanup before breaking
                    for (int j = 0; j < job.argc; j++) {
                        free(job.argv[j]);
                    }
                    free(job.argv);
                    free(job.full_command);
                    free(job.binary);
                    free(command_copy);
                    free(line_copy);
                    break;
                } else {
                    if (is_debug) {
                        printf("DEBUG: Launching job: '%s', background=%d\n", job.full_command, job.is_background);
                    }
                    launch_job(&job);
                }
            }

            // Cleanup
            for (int j = 0; j < job.argc; j++) {
                free(job.argv[j]);
            }
            free(job.argv);
            free(job.full_command);
            if (job.binary) {
                free(job.binary);
            }
            free(command_copy);

            check_bg_jobs();

            // Get next command from the same line
            command = strtok_r(NULL, "&;", &saveptr);
        }

        free(line_copy);
    }
    fclose(file);
    if (is_debug) {
        printf("Debug: Finished processing batch file: %s\n", batch_files[i]);
    }
}

// Wait for all background processes if needed
if (should_wait_for_background_jobs()) {
    int num_waiting = count_running_background_jobs();
    if (num_waiting > 0) {
        fprintf(stderr, "Waiting for %d background job(s) to finish\n", num_waiting);
    }
    wait_for_all_background_jobs();
}

// Display job statistics
display_job_statistics();

return exit_requested ? 1 : 0;
}

//check if to wait for background jobs
int should_wait_for_background_jobs(void)
{
    return count_running_background_jobs() > 0;
}
//count how many background jobs are still running
int count_running_background_jobs(void)
{
    int count = 0;
    for (int i = 0; i < num_jobs; i++) {
        if (!bg_jobs[i].is_done) {
            count++;
        }
    }
    return count;
}

 //wait for all background jobs to complete
void wait_for_all_background_jobs(void)
{
    int still_running;
    
    if (is_debug) {
        printf("debug: Waiting for background jobs to complete...\n");
    }
    
    do {
        still_running = 0;
        check_bg_jobs(); 
        
        // Check if any jobs are still running
        for (int i = 0; i < num_jobs; i++) {
            if (!bg_jobs[i].is_done) {
                still_running = 1;
                break;
            }
        }
        
        if (still_running) {
            sleep(1); // Wait a bit before checking again
        }
    } while (still_running);
    
    if (is_debug) {
        printf("DEBUG: All background jobs completed\n");
    }
}

//display job statistics 
void display_job_statistics(void)
{
    printf("---\n");
    printf("Total number of jobs = %d\n", total_jobs);
    printf("Total number of jobs in history = %d\n", total_history);
    printf("Total number of jobs in background = %d\n", total_jobs_bg);
}

int interactive_mode(void)
{
  char * d = "exit ";
  char whole_command[MAX_COMMAND_LINE];
    
  do{
    /*
     * Print the prompt
     */
    printf(PROMPT);
    
    /*
     * Read stdin, break out of loop if Ctrl-D
     */
    if(fgets(whole_command, sizeof(whole_command), stdin) == NULL){
      builtin_exit();  
    }
    
    //remove trailing newline
    if(strlen(whole_command) > 0 && whole_command[strlen(whole_command) - 1] == '\n'){
      whole_command[strlen(whole_command)-1] = ' ';
      whole_command[strlen(whole_command)] = '\0';
    }

    //if "exit" is entered, exit program
    if((strcmp(whole_command, d)) == 0){
      builtin_exit();
    }

    if (strlen(whole_command) == 0) {
      printf("Empty command\n");
    }else{
      //call to loop through 
      if(loop_through(whole_command, 0) == 2){
	//if a file, exit right away after file
	builtin_exit();
      }
    }

    
  }while(1);

    return 0;
}
	
int loop_through(char * line, int is_file) {
  char * temp = strtok(line, " ");
  int i = 0;
  char * args[MAX_COMMAND_LINE];	

  job_t * job = malloc(sizeof(job_t));
  if(job == NULL){
    perror("malloc failed");
    return -1;
  }
  //
  job->last = 1;
  job->is_file = is_file;
  job->input_file = NULL;
  job->output_file = NULL;
  job->argv = malloc(MAX_COMMAND_LINE);

  
  while(temp != NULL){
    args[i] = strdup(temp);
    i++;
    //handle redirection
    if(strcmp(temp, "<") == 0){
      temp = strtok(NULL, " ");
      if(temp != NULL){
	job->input_file = strdup(temp);
	job->binary = strdup(args[i-2]);
	job->argv[0] = malloc(sizeof(args));
	job->argv[0] = strdup(args[i-2]);
	job->argv[1] = strdup("\0");
      }else{
	printf("need a file after\n");
      }
      temp = strtok(NULL, " ");
      launch_job(job); //treat redirections different than other commands
    }else if(strcmp(temp, ">") == 0){
      temp = strtok(NULL, " ");
      if(temp != NULL){
        job->output_file = strdup(temp);
	job->binary = strdup(args[i-2]);
        job->argv[0] = malloc(sizeof(args));
        job->argv[0] = strdup(args[i-2]);
        job->argv[1] = strdup("\0");
      }else{
        printf("need a file after\n");
      }
      temp = strtok(NULL, " ");
      launch_job(job); //treat redirections different than other commands
    }
    
    //if the end of a job
    if(strcmp(temp, ";") == 0 || strcmp(temp, "&") == 0 || 
       strcmp(temp, "\n") == 0){
      if(strcmp(temp, "&") == 0) {
	job->is_background = 1;
	args[i-1] = NULL;
      }else {// if ends in \n
	job->is_background = 0;	
      }
      args[i] = NULL;
      i --;
      if(i == 0){
	return 0;
      }

      parse_command(args, i, job);
      int k ;
      for(k = 0; k < i; k++){
	job->argv[k] = NULL;
      }

      //reset all vars
      i = 0;
      job->full_command = NULL;
      job->argc = 0;
      job->is_background = 0;
      job->binary = NULL;
      job->is_file = 0;
      job->last = 1;
      job->input_file = NULL;
      job->output_file = NULL;
    }
   temp = strtok(NULL, " ");

   //give it one more chance to complete last command
   if(temp == NULL && job->last == 1){
      temp = strdup("\n");
      job->last = 0;
    }
  }

  //end right away after if file is inputted
  if(job->is_file){
    return 2;
  }

  //free varibles
  free(job->argv);
  free(job->binary);
  free(job);
  
  return 0; 
}


int parse_command(char ** args, int argc, job_t * job)
{
  char command[MAX_COMMAND_LINE];
  int k = 0;
  //error checking
  if(args[0] == NULL){
    return 0;
  }
  
  //fill out job argv
  for(k = 0; k < argc; k++) {
    job->argv[k] = malloc(sizeof(args)* 20);
    job->argv[k] = strdup(args[k]);
  }
  
  //make sure last index is null for execvp
  job->argv[k+1] = NULL;
  job->argc = argc;
  if(args[0] != NULL){
    job->binary = strdup(args[0]);
  }
  //start parsing command
  command[0] = '\0';
  for(k = 0; k < argc; k++) {
    strcat(command, args[k]);
    if(args[k] != NULL) {
      strcat(command, " ");
    }
  }
  job->full_command = strdup(command);

  //background check
  if((strstr(job->binary, "wait") != NULL || strstr(job->binary, "jobs") != NULL ||
      strstr(job->binary, "history") != NULL || strstr(job->binary, "fg") != NULL) &&
     job->is_background == 1){
    printf("%s cannot be run in the background\n", job->binary);
    return -1;
  }

  //if this job is a file
  //if this command was in a file, it will never also be a file
  if(args[0][0] == '.' && args[0][1] == '/'){
    job->is_file = 1;
    FILE * f = fopen(job->binary, "r");
    if(f != NULL){
      char string[MAX_COMMAND_LINE];
      while(fgets(string, sizeof(string), f)){//call recursively to read file
	loop_through(string, 1);
      }
    }else{
      perror("failed to open file\n");
    }
    
  }else {
    //set binary for file
    if(job->is_file == 1){
       //index of the first occr
      int idx = strcspn(job->binary, "\n");
      job->binary[idx] = '\0';
      job->argv[0][idx] = '\0';
    }
    
    launch_job(job);
  }
  //reset varibles
  for (k = 0; k < job->argc; k++) {
    free(job->argv[k]);
  }
  
  return 0;
}
/*
 * You will want one or more helper functions for parsing the commands 
 * and then call the following functions to execute them
 */

int launch_job(job_t * loc_job)
{
       	pid_t pid;
//	total_history++; // (this was interfering with batch mode count) 
    /*
     * Display the job
     */
	pid = fork(); // fork process

       	if(pid < 0) {
	       	perror("fork failed");
	       	return -1;
       	}

	if(pid == 0) {
	  //handle file redirection
	  if(loc_job->input_file != NULL){
	    int file_in = open(loc_job->input_file, O_RDONLY); //opens file to be read
	    if(file_in < 0){
	      perror("couldn't open file\n");
	      return -1;
	    }
	    dup2(file_in, STDIN_FILENO); //makes the file and input stream ready for execvp()!
	    close(file_in);
	  }
	  if(loc_job->output_file != NULL){
	    int file_out = open(loc_job->output_file, O_WRONLY); // opens file to be written
	    if(file_out < 0){
	      perror("couldn't open file\n");
	      return -1;
	    }
	    dup2(file_out, STDOUT_FILENO); //makes the file and input stream ready for execvp()!
	    close(file_out);
	  }
	  execvp(loc_job->binary, loc_job->argv); // replace process with binary  
	  perror("execvp failed"); //if execvp fails
	  //printf("binary [%s]\n", loc_job->binary);
	  exit(1);
       	} else if (loc_job->is_background) {
	  
	  total_jobs++;
               // background job
	       printf("Started background job, PID: %d\n", pid);
	       total_jobs_bg++;
	       if(num_jobs < MAX_JOBS) {
		       bg_jobs[num_jobs].pid = pid;
		       strncpy(bg_jobs[num_jobs].full_command, loc_job->full_command, 1023);
		       bg_jobs[num_jobs].full_command[1023] = '\0';
                       bg_jobs[num_jobs].is_done = 0;
                        num_jobs++;
	       } else {
		       fprintf(stderr, "Max background jobs reached.\n");
	       }
          }
       else { // foreground job
	 printf("Started foreground job, PID: %d\n", pid);
	 total_jobs++;
	 waitpid(pid, NULL, 0);
       }
	
	return 0;
  }
  
  void check_bg_jobs() {
      for (int i = 0; i < num_jobs; i++) {
              if (!bg_jobs[i].is_done) {
                      pid_t result = waitpid(bg_jobs[i].pid, NULL, WNOHANG);
                      if (result == bg_jobs[i].pid) {
                              bg_jobs[i].is_done = 1;
                       }
              }
      }
  }
  
int builtin_exit(void)
{
  if(should_wait_for_background_jobs() == 0){
    display_job_statistics();
    exit(0);
    
  }else{
    printf("waiting for %d jobs\n", count_running_background_jobs());
     wait_for_all_background_jobs();
     display_job_statistics();
     exit(0);
     
  }
}


int builtin_jobs(void) {
    for (int i = 0; i < num_jobs; i++) {
        if (!bg_jobs[i].is_done) {
            printf("[%d] Running %s\n", i+1, bg_jobs[i].full_command);
        } else {
            printf("[%d] Done %s\n", i+1, bg_jobs[i].full_command);
        }
    }
    return 0;
}    
   
int builtin_history(void) {
    printf("Command history:\n");
    for (int i = 0; i < history_count; i++) {
        printf("%d: %s\n", i + 1, command_history[i]);
    }
    return 0;
}



int builtin_fg(void)
{
    // Find the most recent background job that's still running
    int found_job = -1;
    
    for (int i = num_jobs - 1; i >= 0; i--) {
        if (!bg_jobs[i].is_done) {
            found_job = i;
            break;
        }
    }
    
    if (found_job == -1) {
        fprintf(stderr, "fg: no current job\n");
        return -1;
    }
    
    if (is_debug) {
        printf("DEBUG: Bringing job %d to foreground: %s\n", 
               found_job + 1, bg_jobs[found_job].full_command);
    }
    
    // Wait for the job to complete
    printf("Bringing job to foreground: %s\n", bg_jobs[found_job].full_command);
    waitpid(bg_jobs[found_job].pid, NULL, 0);
    
    // Mark the job as done
    bg_jobs[found_job].is_done = 1;
    
    return 0;
}

int builtin_fg_num(int job_num)
{
    // Validate job number
    if (job_num < 1 || job_num > num_jobs) {
        fprintf(stderr, "fg: %d: no such job\n", job_num);
        return -1;
    }
    
    int job_index = job_num - 1;  // Converting to 0-based index
    
    // Check if job exists and is not done
    if (bg_jobs[job_index].is_done) {
        fprintf(stderr, "fg: job %d has already completed\n", job_num);
        return -1;
    }
    
    if (is_debug) {
        printf("DEBUG: Bringing job %d to foreground: %s\n", 
               job_num, bg_jobs[job_index].full_command);
    }
    
    // Wait for the job to complete
    printf("Bringing job to foreground: %s\n", bg_jobs[job_index].full_command);
    waitpid(bg_jobs[job_index].pid, NULL, 0);
    
    // Mark job as done
    bg_jobs[job_index].is_done = 1;
    
    return 0;
}

