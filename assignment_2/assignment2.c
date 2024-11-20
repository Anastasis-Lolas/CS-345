#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <semaphore.h>





#define T 10 /*time for the bus to go to the next stop*/
#define N 15 /*max number of students no matter the department*/
#define S 10 /*the max stringlength for the department is 10*/
#define MAX_STUDENTS 200 /*Max students in the university are 200*/
#define true 1 /*set boolean values*/
#define false 0
#define BUS_CAPACITY 10 /*max capacity with each boarding the bus can*/

pthread_mutex_t student_mutex;/*we want a mutex to keep signle-access to shared data.. meaning between the threads*/
pthread_cond_t student_cond;/*we want condition variables to notify the threads for a certain event*/


int num,state=0,next_student=0,finish = false;

/*enumerator representing the possible location-stops of the student*/
enum Stops{
    home=0 ,
     StopA = 1,
     Bus = 2 , 
     University = 3
      ,StopB = 4
};

/*student struct with all the necessary information*/
typedef struct Student{
    int AM;
    int study_time;
    char * department;
    enum Stops current_stop;
    int brd;
   
} student;

/*we have 5 arrays of student Structs 1 represents the whole student force and the other 4 each stop*/
student UniStudents[MAX_STUDENTS],stopA[MAX_STUDENTS],bus[MAX_STUDENTS],stopB[MAX_STUDENTS],Uni[MAX_STUDENTS];

int students_in_stopA=0,students_in_Bus=0,students_in_StopB=0,g_counter = 0;/*counters for the arrays*/

int MathA=0,ChemA=0,PhA=0,CSA=0;/*counter for every department in stopA*/
int MathB=0,ChemB=0,PhB=0,CSB=0;/*counter for every department in stopB*/


/*here we select randomly 1 department but we make sure are all eqaully n/4*/
char * select_depart(){
    int t = rand() % 4 + 1 ; 
    char *c = malloc(sizeof(char) * S);

    if(t == 1  ){
        c= "Math";
    }else if(t == 2 ){
        c = "Chemistry";
    }else if(t == 3 ){
        c = "Physics";
    }else if(t ==4){
        c = "CSD";
    }

    return c;
}

/*This is a print function*/
void print(student US[],int curr){
    
    printf("\nStopA:");
    for(int i = 0 ; i <= curr; i++){
       if(US[i].current_stop == 1){
            printf("[%d,%s] ",US[i].AM,US[i].department);
       }
    }
    printf("\n");

    printf("Bus:");
    for(int i = 0 ; i <= curr; i++){
       if(US[i].current_stop == 2){
            printf("[%d,%s] ",US[i].AM,US[i].department);
       }
    }
    printf("\n");

    printf("University");
    for(int i = 0 ; i <= curr; i++){
       if(US[i].current_stop == 3){
            printf("[%d,%s] ",US[i].AM,US[i].department);
       }
    }
    printf("\n");

    printf("StopB:");
    for(int i = 0 ; i <= curr; i++){
       if(US[i].current_stop == 4){
            printf("[%d,%s] ",US[i].AM,US[i].department);
       }
    }
    printf("\n");
}


int can_board(int i, int flag) {
   
    /*Check if the department pointer is valid*/ 
    if (flag == 0 && stopA[i].AM != -1 && stopA[i].department != NULL) {
        if (students_in_Bus < BUS_CAPACITY) { /*if the bus can take more students*/
            if (strcmp(stopA[i].department, "Math") == 0 && MathA < (num / 4)) { /*and the N/4 boarding rule for each department is valid*/
                MathA++; /*increment the certain department counter for stopA*/
                return true; /*return a true  == 1 to notify that this student from this department can board*/
            } else if (strcmp(stopA[i].department, "Chemistry") == 0 && ChemA < (num / 4)) {
                ChemA++;
                return true;
            } else if (strcmp(stopA[i].department, "Physics") == 0 && PhA < (num / 4)) {
                PhA++;
                return true;
            } else if (strcmp(stopA[i].department, "CSD") == 0 && CSA < (num / 4)) {
                CSA++;
                return true;
            }
        }/*Same but for Stop B*/
    } else if (flag == 1 && stopB[i].AM != -1 && stopB[i].department != NULL) {
        if (students_in_Bus < BUS_CAPACITY) {
            if (strcmp(stopB[i].department, "Math") == 0 && MathB < (num / 4)) {
                MathB++;
                return true;
            } else if (strcmp(stopB[i].department, "Chemistry") == 0 && ChemB < (num / 4)) {
                ChemB++;
                return true;
            } else if (strcmp(stopB[i].department, "Physics") == 0 && PhB < (num / 4)) {
                PhB++;
                return true;
            } else if (strcmp(stopB[i].department, "CSD") == 0 && CSB < (num / 4)) {
                CSB++;
                return true;
            }
        }
    }

    /*if either the AM is invalid or the bus is full or the N/4 rule is full as well we return that this student cant board*/
    return false;
}

/**This function removes a student from a struct array */
void remove_student(int i,student board[]){
    board[i].AM = -1;
    board[i].current_stop = 0;
    board[i].study_time = - 1;
    board[i].department ='\0';
    
    /*Basically it is not removal we just assign pseudo-values*/
}

/*Boards the student*/
void board(int i,student board[]){
    printf("\nStudent %d (%s) boarded to the bus.\n",board[i].AM,board[i].department);
    UniStudents[i].current_stop = 2;
    /*change the location of the student*/
    print(UniStudents,num);
    
    /*Assign the student struct to the proper array-stop*/
    bus[i] = board[i];
    students_in_Bus++;

    remove_student(i,stopA);
    /*remove the student from the previous array to simulate board-deboard -leave- change stop*/
    
}


/*simulates a study for a student*/
void study(int i) {
    pthread_mutex_lock(&student_mutex);
    /*lock to make sure 1-1 student are studying*/
   
    sleep(UniStudents[i].study_time);  /*sleep = studying to simulate the threads*/

    printf("\nStudent %d (%s) studied for %d seconds! and is heading to stopB\n", UniStudents[i].AM, UniStudents[i].department, UniStudents[i].study_time);

  
    UniStudents[i].current_stop = 4; /*update the location*/
    UniStudents[i].brd = true; /*set that the student has studied and is ready to go home*/

    print(UniStudents, num);

    /*Move the student to stopB for the next boarding phase*/
    stopB[i] = Uni[i];
    remove_student(i, Uni);

    pthread_mutex_unlock(&student_mutex);
}

/*Routine function for the student threads that represents their studying and their creation*/
void *init_student(void *args){
    student *  Student = (student *) args;
    /*we pass the argument from the pthread_create function */

    pthread_mutex_lock(&student_mutex);
    /*student has been created*/
    printf("\nStudent : %d  (%s) created  .\n",Student->AM,Student->department);

    print(UniStudents,(Student->AM )- 1);
    pthread_mutex_unlock(&student_mutex); 

    /*while the bus hasnt reached  the University wait*/
    while(1){
        if(state == 1)
            break;
    }   

    /*and also while the student is not at the university*/
    while(1){
        if(Student->current_stop ==3)
            break;
    }

   
    /*if both of them are true that means that the student is ready to study*/
    study((Student->AM )- 1);

    return NULL;
}

/*A student deboards from the bus in stopB*/
void deboard(int i){
    if(UniStudents[i].current_stop==2){
        printf("\nStudent %d (%s) got of the bus!\n",bus[i].AM,bus[i].department);
        UniStudents[i].current_stop = 4;
        /*update location*/

        /*assign the student struct correctly*/
        stopB[i] = bus[i];
        stopB[i].AM = bus[i].AM;
        
        remove_student(i,bus);
        /*remove the student from the previous array*/
        

        print(UniStudents,num);
    }
}
  
/*A thread routine for the bus thread*/
void *buss(){

    while(1){
        sleep(3);

        /*We have the Boarding phase for A*/
        /*say that we are not in university now*/
        state = 0;

        printf("\nBus is arriving at stopA\n");

        /*check first if we have someone to drop off from the Uni*/
        for(int i = 0; i < num ; i++){
            pthread_mutex_lock(&student_mutex);
            sleep(0.4); /*sleep for a bit just for design*/
            if(UniStudents[i].brd == true && UniStudents[i].AM != -1 && UniStudents[i].current_stop == 2){ /*if this student isnt removed and was ready to be dropped home*/
                
                 printf("\nStudent %d (%s) went home!\n",UniStudents[i].AM,UniStudents[i].department);
                 UniStudents[i].current_stop = 0; /*assign the location to home*/
                 g_counter--; /*reduce the counter so we can keep track how many students are left for the programm to finish*/
                
                remove_student(i,bus); /*remove from the bus*/
                remove_student(i,UniStudents);/*since he went home we remove him permanently*/

               
            }
            pthread_mutex_unlock(&student_mutex);
        }

        if(g_counter == 0) break;/*if every student went home break the loop so the program can exit*/

        for(int i = 0 ; i < num ;i++){ /*for each student we check*/
            pthread_mutex_lock(&student_mutex); /*make sure each thread goes 1-1*/
            sleep(0.4);
            if(stopA[i].AM != -1){ /*if the student is not removed from stopA*/
                if(can_board(i,0)){ /*check if he can board to the bus*/
                    board(i,stopA); /*if he can board him else print a message*/
                }else{
                    printf("\nStudent %d (%s) cannot enter the bus!\n",stopA[i].AM,stopA[i].department);
                }
            }
            pthread_mutex_unlock(&student_mutex); /*keeping the fifo line*/
        }

        sleep(2);
        /*reset all the counters for the stops*/
        MathA=0,MathB=0; 
        ChemA=0,ChemB=0;
        PhA=0,PhB=0;
        CSA=0,CSB=0;

        printf("\nBus is heading to University!\n");
        sleep(T);
        printf("\nBus arrived at the University!\n");

        /*Arrived at StopB*/
        /*Deboard the students*/
        for(int i = 0 ; i < num; i++){ /*for each student in the bus deboard him to stopB*/
            pthread_mutex_lock(&student_mutex);
            sleep(0.4);
            deboard(i);
            pthread_mutex_unlock(&student_mutex);
        }
        
        students_in_Bus = 0;/*reset the counter of the bus meaning that the bus is empty again*/
        for(int i = 0 ; i< num ; i++){
            pthread_mutex_lock(&student_mutex);
            sleep(0.4);
            if(stopB[i].AM != -1 && stopB[i].AM != 0 && UniStudents[i].brd == false){ /*if the student has not studied yet and it is a not removed student*/
                
                Uni[i] = stopB[i];
                printf("\nStudent %d (%s) went to University!\n",stopB[i].AM,stopB[i].department); /*make him go to university*/
                UniStudents[i].current_stop = 3; /*update the location*/

                remove_student(i,stopB);/*remove him from stopB*/

                print(UniStudents,num);
            }

           
            pthread_mutex_unlock(&student_mutex);
        }
        

        for(int i = 0 ; i < num; i++){
            pthread_mutex_lock(&student_mutex);
            /*this person studied for random time and now he wants to leave*/
            if(UniStudents[i].brd == true && UniStudents[i].AM != -1){ /*if the student is valid and is ready to leave*/
                if(stopB[i].AM != -1){ /*make sure is valid again*/
                    if(can_board(i,1)){ /*check if he can board or not*/
                        board(i,stopB);
                    }else{
                        printf("\nStudent %d (%s) cannot enter the bus!\n",stopB[i].AM,stopB[i].department);
                    }
                    
                }

            }
            pthread_mutex_unlock(&student_mutex);
        }

        state = 1;/*change the stae meaning that we arrived at the uni so now the threads each one will start studying*/

        sleep(5);
        printf("\nBus is going heading to stopA\n");


    }


    finish = true; /*notify the loop*/
    
    return NULL;
}


int main(){

    srand(time(NULL)); /*for the randomness*/

    /*Selecting the number of students and checking if it doesnt surpass the N */
    printf("Enter the number of students:\n");
    scanf("%d",&num);

    if(num > N){
        printf("num cannot surpass the max number of students !\n");
        exit(0);
    }

    g_counter = num; /*assign the counter equal to the number of students*/

    /*Creating the student,bus threads */
    pthread_t students[num]; /*initialize the mutex and the threads*/
    pthread_t bus;
    pthread_cond_init(&student_cond,NULL);
    pthread_mutex_init(&student_mutex,NULL);
    

    
    /*initialize each student struct*/

    for(int i = 0 ; i < num ; i++){
        student  new_st;
        
        int new_time =  rand() % 11 +5;
    
        new_st.study_time = new_time;
        new_st.current_stop = 1 ;
        new_st.AM= i + 1; 
        new_st.department = select_depart();
        new_st.brd = false;
        
        UniStudents[i] = new_st;
        stopA[students_in_stopA] = new_st;
        students_in_stopA++;
    }

    int *a;
    for(int i = 0; i < num ; i++){
        a = malloc(sizeof(int));
        /*create the thread*/
        if(pthread_create(&students[i],NULL,init_student,&UniStudents[i]) != 0){
            printf("Error creating Thread-Student!\n");
            exit(0);
        }
        sleep(1);
    }

    if(pthread_create(&bus,NULL,&buss,NULL) != 0){
        printf("Eroor creating  Bus-Thread!\n");
        exit(0);
    }


    for(int i = 0; i < num ; i++){
        if(pthread_join(students[i], NULL) != 0){
            printf("Error in joining Thread-Student!\n");
            exit(0);
        }
    }

    if(pthread_join(bus,NULL) != 0 ){
        printf("Error joining bus thread!\n");
        exit(0);
    }


    

    pthread_mutex_destroy(&student_mutex);
     pthread_cond_destroy(&student_cond);

    
    /*the end*/
    if(finish){
        printf("\n**********************************************\n");
        printf("\n****      EVERY STUDENT WENT HOME        *****\n");
        printf("\n**********************************************\n");
    }


    free(a);


}