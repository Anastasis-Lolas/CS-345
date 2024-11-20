#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include <string.h>
#include <sys/wait.h>
#include <ctype.h>
#include <fcntl.h>
#define MAX_ARGS 256
#define MAX_SIZE_LINE 1024  


char* read_command(char *buffer){ /*function to read commands from the stdin or the file*/
                                                                            
    if(fgets(buffer,MAX_SIZE_LINE,stdin)==NULL){
        perror("Error fgets!\n");
        exit(1);
    }
                                                                                      
                                                                              
    buffer[strcspn(buffer,"\n")] = '\0';

    return buffer;/*return the new string which is the command*/
}

int check_equal_symbols(char *command){
    /*we take the length of the string to iterate*/
    int i = 0,len=strlen(command),flag = 0;

    /*if we found the = operator that means we have to assign a global var*/
    for(i = 0 ; i < len; i++)
    {
        if(command[i] == '=' && flag == 0)
        {
           return 1; /*we have =*/
        }
    }

    return 0; /* we don't*/
}

char **split_equal_string(char *command) {
    int i = 0, len, counter_before = 0, counter_after = 0;
    char **splited_strings = malloc(2 * sizeof(char *)); 
    if (!splited_strings) {
        printf("Malloc failed in split_equal_string\n!");
        exit(1);
    }

    splited_strings[0] = malloc(MAX_SIZE_LINE);
    splited_strings[1] = malloc(MAX_SIZE_LINE);
    if (!splited_strings[0] || !splited_strings[1]) {
        printf("Malloc failed in split_equal_string [0][1]\n!");
        exit(1);
    }

    len = strlen(command);

   
    for (i = 0; i < len; i++) {
        /*We dont */
        if (command[i] == ' ') {
            continue;
        }

        /*check if we have global var or redirections*/
        if (command[i] == '=' || command[i] == '<' || command[i] == '>') {
            // Null terminate the first part of the string
            splited_strings[0][counter_before] = '\0';
            counter_before = 0; // Reset for second part
            
            i++;  
            
            /*Skip space before-after the = operator*/
            while (i < len && command[i] == ' ') {
                i++;
            }

            /*Second part of the string this is the after the = operator*/
            while (i < len) {
                    splited_strings[1][counter_after++] = command[i];
                i++;
            }

            /*finish the second string*/
            splited_strings[1][counter_after] = '\0';
            return splited_strings; 
            /*return the splited strigns to be used after for the enviromental call*/
        }

        /*add to the first split_string*/
        splited_strings[0][counter_before++] = command[i];
    }


    splited_strings[0][counter_before] = '\0'; 
    splited_strings[1][0] = '\0'; 

    return splited_strings;  
}


void env_calling(char *command){
    char **splits = split_equal_string(command);
    setenv(splits[0], splits[1], 1); /*we set the env for these 2 var for the global variables creation*/

    free(splits[0]); /*memory deallocation so as not to have leaks*/
    free(splits[1]);
    free(splits);
}

int check_redirect(char *command){
    int i,flag=0,len=strlen(command);

    for(i = 0 ; i < len ; i++){/*checking for redirections either <,>,>>*/
        if(command[i] == '<') 
        {
            flag = 1 ;
            break;

        }else if(command[i] == '>'){
            
            if(command[i + 1] == '>'){
                flag = 3;
                break;
            }

            flag = 2 ;
            break ; 
        }
    }

    return flag;

}

int number_of_var(char *command){
    int i,counter=0;

    for(i = 0 ; i < strlen(command); i++){

        if(command[i] == '$'){
            counter++;
        }
    }

    return counter;
}

int tokenize(char * command, char *args[MAX_ARGS],char *tokens){
   
    int i = 0;
    char *token = strtok(command, tokens); /*starting the tokenazation*/

    while (token != NULL) { /*we will put this tokens in array which we will have a "command" array*/
        args[i] = malloc(strlen(token) + 1);
        if(!args[i]){
            printf("Malloc failed at tokenize loop \n");
            exit(1);
        }
        strcpy(args[i], token);
        i++;
        token = strtok(NULL, tokens);
    }

    args[i] = NULL;

    return i;
}

void print(char *vars[MAX_ARGS],int i){
    int k;

    for(k = 0 ; k < i; k++){
        printf("Vars[%d]= %s\n",k,vars[k]);
    }

}


int replace_symbol(char *command,char *dollar_args[MAX_ARGS],int tak){
    int i,dollar_v,len=strlen(command);
    const char * env_command= malloc(sizeof(char) * MAX_SIZE_LINE);

    env_command = command;

    dollar_v = number_of_var(command);

    for(i = 0 ; i < len ; i++){
        if(env_command[i] == '$'){

        }
    }
    
    return tak;
}



int exec_redirects(char *command) {
    char **splits;
    int redirect_type = check_redirect(command);

    if (redirect_type == 1) {  
        splits = split_equal_string(command);
        int file = open(splits[1], O_RDONLY);  /*read-only mode*/

        if (file == -1) {
            perror("Error opening file for reading\n");
            return 1;  
        }

        int saved_in = dup(STDIN_FILENO);  /*save the stdin to put it in the redirection after*/
        dup2(file, STDIN_FILENO);  /*redirect it*/

        int pid = fork();  
        if (pid == 0) {
            // Child process
            if (system(splits[0]) != 0) { /*splits is the first part of  var < something  this command*/
                perror("System call failed");
                exit(1);
            }
            exit(0);
        } else if (pid > 0) {
            // Parent process
            waitpid(-1, NULL, 0);  
        }

        dup2(saved_in, STDIN_FILENO);  /*duplicate it in order to restore */
        close(saved_in);
        close(file);  

        free(splits[0]);
        free(splits[1]);
        free(splits);

        return 1;

    } else if (redirect_type == 2) {  
        splits = split_equal_string(command);
        int file = open(splits[1], O_WRONLY | O_CREAT | O_TRUNC, 0777);

        if (file == -1) {
            perror("Error opening file for writing\n");
            return 1;
        }

        int saved_out = dup(STDOUT_FILENO);  
        dup2(file, STDOUT_FILENO);  

        int pid = fork();  
        if (pid == 0) {
            // Child process
            if (system(splits[0]) != 0) {
                perror("System call failed");
                exit(1);
            }
            exit(0);
        } else if (pid > 0) {
            // Parent process
            waitpid(-1, NULL, 0);  
        }

        dup2(saved_out, STDOUT_FILENO);  
        close(saved_out);
        close(file);  

        free(splits[0]);
        free(splits[1]);
        free(splits);

        return 1;

    } else if (redirect_type == 3) {  
        splits = split_equal_string(command);
        int file = open(splits[1], O_WRONLY | O_CREAT | O_APPEND, 0777); /*we want this file to be write-only to append
        and to be created*/

        if (file == -1) {
            perror("Error opening file for appending\n");
            return 1;
        }

        int saved_out = dup(STDOUT_FILENO);  
        dup2(file, STDOUT_FILENO);  

        int pid = fork();  
        if (pid == 0) {
            system(splits[0]);
            exit(0);
        } else if (pid > 0) {
            
            waitpid(-1, NULL, 0);  
        }

        dup2(saved_out, STDOUT_FILENO); 
        close(saved_out);
        close(file);  

        free(splits[0]);
        free(splits[1]);
        free(splits);

        return 1;
    }

    return 0;
}


int check_piping_symbols(char * command){
    int i,counter=0,len=strlen(command),flag = 0;

    for(i = 0 ; i < len ; i++){
        if(command[i] == '"') 
            flag = 1;
        if(command[i] == '|' && flag == 0){    
            counter++;
        }
    }
  
    return counter; /*the number of pipes we have to do f.e ls | uniq|sort|wc pipes = 4 */

}

int exec_piping(char *command) {
    int counter = check_piping_symbols(command);  /*Number of pipes*/
    char *pipe_args[MAX_SIZE_LINE];
    
    if (counter > 0) {
        int n = tokenize(command, pipe_args, "|");  /*tokenize the commands by this | operator*/

        int fd[counter][2];  /*we have counter pipes so we need an array of file descriptors for each one so it is a 2D*/
        int pids[n]; /*child processes table for fork*/
        

        for (int j = 0; j < counter; j++) {
            if (pipe(fd[j]) == -1) {
                perror("Error creating pipe\n");
                exit(1);
            }
        }

        /*for each tokenized command between the pipes*/
        for (int j = 0; j < n; j++) {
            pids[j] = fork();
           
            if (pids[j] == 0) { 
                
                /*if the first child and if we have redirection*/
                if (j == 0 && check_redirect(pipe_args[j]) == 1) {
                    exec_redirects(pipe_args[j]); /*we call it to pass it to the file*/
                }

               /*first command*/
                if (j == 0) {
                    dup2(fd[j][1], STDOUT_FILENO);  /*we only have to redirect to next pipe and not read 
                    from anything previous*/
                }
                /*Last command: the exact opposite of the first one ( j== 0)*/
                else if (j == n - 1) { /*for every other command*/
                    dup2(fd[j - 1][0], STDIN_FILENO);  /*we have to take the input firstly from the previous pipe*/
                    if (check_redirect(pipe_args[j]) == 2 || check_redirect(pipe_args[j]) == 3) {
                        exec_redirects(pipe_args[j]);  /*and we check if we have any >*/
                    }
                }
                /*every other command that has no > < >> operators we redirect the previous stdin to the current one*/
                /*and than we continue by redirecting the stdout to the next pipe*/
                else {
                    dup2(fd[j - 1][0], STDIN_FILENO);  
                    dup2(fd[j][1], STDOUT_FILENO);  
                }

                /*close all file descriptors*/
                for (int k = 0; k < counter; k++) {
                    close(fd[k][0]);
                    close(fd[k][1]);
                }

               /*Executing command in pipes with exec is more preferable than executing with system()*/
                char *temp[MAX_ARGS + 1];
                tokenize(pipe_args[j], temp, " ");
                execvp(temp[0], temp);

                perror("Error in execvp\n");
                exit(1);
            }
        }

       /*Close parents*/
        for (int j = 0; j < counter; j++) {
           close(fd[j][0]);
           close(fd[j][1]);
        }

        
        for (int j = 0; j < n; j++) {
            wait(NULL);
        }

        return 1;
    }

    return 0;
}


void forking(char * args){
     int pid = fork();
        if (pid == 0) {  
            if (system(args) != 0) {
                exit(1);
                perror("System command failed!\n");
            }
        } else if (pid > 0) {
            waitpid(-1, NULL, 0);
        } else {
            perror("Fork failed\n");
        }
}


int execute_command(char* command) {
    char *args[MAX_ARGS],*args2[MAX_ARGS];
     
    int i ,j,redi_flag,pipe_flag,vr;
    
    i = tokenize(command,args,";");

    for(j = 0 ; j < i ; j++){
     
        if(check_equal_symbols(args[j]) == 1 ){
            env_calling(args[j]);      
        }

        char *dollar_args[MAX_ARGS] ;
        int p = replace_symbol(args[j],dollar_args,p);

        redi_flag=exec_redirects(args[j]);
        if(redi_flag == 1) continue;

        pipe_flag=exec_piping(args[j]);
        if(pipe_flag == 1) continue;
    
      
       
       
        forking(args[j]);
         
       
    }

}




int main(int* argc,char* argv[]){   
    char *name="csd5197";                                                                            
    char buffer[1024];
    char username[1024];
    char cwd[1024];

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s-hy345 %s\n",name, cwd);
    }else {
        perror("getcwd() error");
    }

    while (1) {
        
        read_command(buffer); 
        execute_command(buffer);
    }

    return 0;                                                                             
}                                                                              
