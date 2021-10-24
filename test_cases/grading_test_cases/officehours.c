/*
 * Name: Prithvidhar Pudu
 * Student ID: 1001570483 
 */
// Copyright (c) 2020 Trevor Bakker
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
  
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

/*** Constants that define parameters of the simulation ***/

#define MAX_SEATS 3        /* Number of seats in the professor's office */
#define professor_LIMIT 10 /* Number of students the professor can help before he needs a break */
#define MAX_STUDENTS 1000  /* Maximum number of students in the simulation */

#define CLASSA 0
#define CLASSB 1
#define CLASSC 2
#define CLASSD 3
#define CLASSE 4

/* TODO */
/* Add your synchronization variables here */
/*
 * Adding a semaphore to keep track of the number
 * seats available.
 * 
 * Adding a condition variable "leave" that prevents different classes from
 * being in the office at the same time.
 * 
 * Semaphore b is responsible for keeping students out when the 10th student
 * enters the officeto allow the professor to take a break
 * 
 * The Semaphores bs and as are responsile for ensuring that after 5 consecutive students
 * a student from the next class enters the office
 * 
 * The join condition is used to signal the professor to take breaks when
 * 10 students have been in his office
 * 
 * The pb condition is used to prevent students from entering the functions ntil the professor 
 * finishes his break
 */
 sem_t seats;
 sem_t b;
 sem_t as;
 sem_t bs;
 pthread_mutex_t classCheck;
 pthread_cond_t join;
 pthread_cond_t leave;
 pthread_mutex_t btime;
 pthread_cond_t breakTime;
 pthread_cond_t pb;
 
 
 
 static int aCount = 0;
 static int bCount = 0;
 static int switched = 0;
 static int athread = 0;
 static int bthread = 0;


/* Basic information about simulation.  They are printed/checked at the end 
 * and in assert statements during execution.
 *
 * You are responsible for maintaining the integrity of these variables in the 
 * code that you develop. 
 */

static int students_in_office;   /* Total numbers of students currently in the office */
static int classa_inoffice;      /* Total numbers of students from class A currently in the office */
static int classb_inoffice;      /* Total numbers of students from class B in the office */
static int students_since_break = 0;

//static int breakTime;



typedef struct 
{
  int arrival_time;  // time between the arrival of this student and the previous student
  int question_time; // time the student needs to spend with the professor
  int student_id;
  int class;
} student_info;

/* Called at beginning of simulation.  
 * TODO: Create/initialize all synchronization
 * variables and other global variables that you add.
 */
static int initialize(student_info *si, char *filename) 
{
  students_in_office = 0;
  classa_inoffice = 0;
  classb_inoffice = 0;
  students_since_break = 0;
 
 // breakTime = 0;

  /* Initialize your synchronization variables (and 
   * other variables you might use) here
   */
   /*
    * Initializing my seats semaphore to a value of 3 to
    * limit the amount of students in the office
    * 
    * Initializig the remaining semaphores based on how many students they allow to enter
    * 
    */
    sem_init(&seats, 0, 3);
    sem_init(&b, 0 ,10);
    sem_init(&as,0, 5);
    sem_init(&bs, 0, 5);
   
    pthread_mutex_init(&classCheck,NULL);
   
    
    


  /* Read in the data file and initialize the student array */
  FILE *fp;

  if((fp=fopen(filename, "r")) == NULL) 
  {
    printf("Cannot open input file %s for reading.\n", filename);
    exit(1);
  }

  int i = 0;
  while ( (fscanf(fp, "%d%d%d\n", &(si[i].class), &(si[i].arrival_time), &(si[i].question_time))!=EOF) && 
           i < MAX_STUDENTS ) 
  {
    i++;
  }

 fclose(fp);
 return i;
}

/* Code executed by professor to simulate taking a break 
 * You do not need to add anything here.  
 */
static void take_break() 
{
  printf("The professor is taking a break now.\n");
  sleep(5);
  printf("Students in office: %d",students_in_office);
  assert( students_in_office == 0 );
  students_since_break = 0;
}

/* Code for the professor thread. This is fully implemented except for synchronization
 * with the students.  See the comments within the function for details.
 * 
 * The professor waits on the "join" condition and when signalled checks
 * whether 10 studenst have entered. If so, the professor takes a break and 
 * sem posts the b semaphore to allow the students the function again
 */
void *professorthread(void *junk) 
{
  printf("The professor arrived and is starting his office hours\n");

  /* Loop while waiting for students to arrive. */
  while (1) 
  {
 
    pthread_mutex_lock(&classCheck);

    pthread_cond_wait(&join, &classCheck);
    
   
    if(students_since_break == 10)
    {
      
     // sem_wait(&breakTime);
     
     
      while(students_in_office >0)
      {
        pthread_cond_wait(&leave, &classCheck);
      }
      
  
      
      take_break();
      int count=0;
      while(count<10)
      {
        sem_post(&b);
        count++;
      }
      
      pthread_cond_signal(&pb);
    }

    pthread_mutex_unlock(&classCheck);
    
    

  
    
  }
  pthread_exit(NULL);
}


/* Code executed by a class A student to enter the office.
 * You have to implement this.  Do not delete the assert() statements,
 * but feel free to add your own.
 * 
 * The athread is incremented then decremented to check if they are any
 * student A coming too the office
 * aCount is used to keep track of consecutive students
 */
void classa_enter() 
{
  athread++;
  if(students_since_break>=10)
 {
   printf("PPOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\n");
 }
  
  
  /* TODO */
  /* Request permission to enter the office.  You might also want to add  */
  /* synchronization for the simulations variables below                  */
  /*  YOUR CODE HERE. 
   * Adding a sem wait to signify that this student in a seat
   * 
   * Class mixing is prevented by using a while loop and a condition variable that is
   * signalled when a student leaves
   * 
   * The aThread varible is used to check whteher they are any remaining A students to process.
   */   
  // sem_get
   sem_wait(&as);
 
   
   sem_wait(&seats);
    
   
   sem_wait(&b);
  
   pthread_mutex_lock(&classCheck);
   
   while((classb_inoffice > 0 ))
    {
      
      pthread_cond_wait(&leave,&classCheck);
      
      
    }
    athread--;
    while(students_since_break ==10)
    {
      
      pthread_cond_wait(&pb,&classCheck);
     
      
    }
   
  
   
                                                  
  
  aCount++;

 pthread_cond_signal(&join);
  students_in_office += 1;
  students_since_break += 1;
  classa_inoffice += 1;
  
  
    
  
   
   
   
   
 
  pthread_mutex_unlock(&classCheck);
  
 
  
  
  

}

/* Code executed by a class B student to enter the office.
 * You have to implement this.  Do not delete the assert() statements,
 * but feel free to add your own.
 */
void classb_enter() 
{
 
 
  /* TODO */
  /* Request permission to enter the office.  You might also want to add  */
  /* synchronization for the simulations variables below                  */
  /*  YOUR CODE HERE.  
   * Adding a sem wait to signify that this student in a seat  
   * bthread is incremented to keep track of b students coming to office
   * bCount keeps track of consecutive students
   *                                                  */ 
   bthread++;
   sem_wait(&bs);
 
  sem_wait(&seats);

   
    sem_wait(&b);
  
  pthread_mutex_lock(&classCheck);
  while((classa_inoffice > 0))
    {
      
      pthread_cond_wait(&leave,&classCheck);
    }
    bthread--;
    while(students_since_break ==10)
    {
      
      pthread_cond_wait(&pb,&classCheck);
       
      
    }
   
  
 
      
                                              
  
 
  bCount++;
 pthread_cond_signal(&join);
  students_in_office += 1;
  students_since_break += 1;
  classb_inoffice += 1;
  
  
  
    
  
   
  
   
  
  pthread_mutex_unlock(&classCheck);
  
  
  

}

/* Code executed by a student to simulate the time he spends in the office asking questions
 * You do not need to add anything here.  
 */
static void ask_questions(int t) 
{
  sleep(t);
}


/* Code executed by a class A student when leaving the office.
 * You need to implement this.  Do not delete the assert() statements,
 * but feel free to add as many of your own as you like.
 * 
 * The bthread if condition allows students to pass through sem as if
 * there are no Student A incoming
 * 
 * The while loop is responsible for sem posting the bs semaphore to allow
 * the b students to enter
 */
static void classa_leave() 
{
  /* 
   *  TODO
   *  YOUR CODE HERE. 
   * Adding a sem post to signify that a student has left a seat
   * and is now availble for another student.
   */
  pthread_mutex_lock(&classCheck);
  students_in_office -= 1;
  classa_inoffice -= 1;
  

  
 
  if(bthread==0)
  {
    sem_post(&as);
  }
     int count = 0;
  while(count <bCount)
  {
    sem_post(&bs);
    count++;
  }
  bCount  = 0;
   
   
  pthread_cond_signal(&leave);
  pthread_mutex_unlock(&classCheck);
  sem_post(&seats);
  
  
  
  
  
}

/* Code executed by a class B student when leaving the office.
 * You need to implement this.  Do not delete the assert() statements,
 * but feel free to add as many of your own as you like.
 * 
 * The athread if condition allows b students to pass if no Student As are incoming
 * 
 * The while loop allows the A students to pass through the sem as.
 */
static void classb_leave() 
{
  /* 
   * TODO
   * YOUR CODE HERE. 
   * Adding a sem post to signify that a student has left a seat
   * and is now availble for another student.
   */
  pthread_mutex_lock(&classCheck);
  students_in_office -= 1;
  classb_inoffice -= 1;
 
 

  if(athread==0)
  {
    sem_post(&bs);
  }
  int count = 0;
   while(count <aCount)
  {
    sem_post(&as);
    count++;
  }
  aCount= 0;
 
   
  pthread_cond_signal(&leave);
  pthread_mutex_unlock(&classCheck);
  sem_post(&seats);
  

  
  
}


/* Main code for class A student threads.  
 * You do not need to change anything here, but you can add
 * debug statements to help you during development/debugging.
 */
void* classa_student(void *si) 
{
  student_info *s_info = (student_info*)si;

  /* enter office */
  classa_enter();
  
   
   
  printf("Student %d from class A enters the office\n", s_info->student_id);

  

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
 
  
  assert(classb_inoffice == 0 );
  
  /* ask questions  --- do not make changes to the 3 lines below*/
  printf("Student %d from class A starts asking questions for %d minutes\n", s_info->student_id, s_info->question_time);
  ask_questions(s_info->question_time);
  printf("Student %d from class A finishes asking questions and prepares to leave\n", s_info->student_id);

  /* leave office */
  classa_leave();  

  printf("Student %d from class A leaves the office\n", s_info->student_id);
 

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);

  pthread_exit(NULL);
}

/* Main code for class B student threads.
 * You do not need to change anything here, but you can add
 * debug statements to help you during development/debugging.
 */
void* classb_student(void *si) 
{
  student_info *s_info = (student_info*)si;

  /* enter office */
  
  classb_enter();
 
  
  printf("Student %d from class B enters the office\n", s_info->student_id);
  

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);
  //DEBUG
  printf("Debug (classA in office): %d\n", classa_inoffice);
  assert(classa_inoffice == 0 );

  printf("Student %d from class B starts asking questions for %d minutes\n", s_info->student_id, s_info->question_time);
  ask_questions(s_info->question_time);
  printf("Student %d from class B finishes asking questions and prepares to leave\n", s_info->student_id);

  /* leave office */
  classb_leave();        

  printf("Student %d from class B leaves the office\n", s_info->student_id);
 

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);

  pthread_exit(NULL);
}

/* Main function sets up simulation and prints report
 * at the end.
 * GUID: 355F4066-DA3E-4F74-9656-EF8097FBC985
 */
int main(int nargs, char **args) 
{
  int i;
  int result;
  int student_type;
  int num_students;
  void *status;
  pthread_t professor_tid;
  pthread_t student_tid[MAX_STUDENTS];
  student_info s_info[MAX_STUDENTS];

  if (nargs != 2) 
  {
    printf("Usage: officehour <name of inputfile>\n");
    return EINVAL;
  }

  num_students = initialize(s_info, args[1]);
  if (num_students > MAX_STUDENTS || num_students <= 0) 
  {
    printf("Error:  Bad number of student threads. "
           "Maybe there was a problem with your input file?\n");
    return 1;
  }

  printf("Starting officehour simulation with %d students ...\n",
    num_students);

  result = pthread_create(&professor_tid, NULL, professorthread, NULL);

  if (result) 
  {
    printf("officehour:  pthread_create failed for professor: %s\n", strerror(result));
    exit(1);
  }

  for (i=0; i < num_students; i++) 
  {

    s_info[i].student_id = i;
    sleep(s_info[i].arrival_time);
                
    student_type = random() % 2;

    if (s_info[i].class == CLASSA)
    {
      result = pthread_create(&student_tid[i], NULL, classa_student, (void *)&s_info[i]);
    }
    else // student_type == CLASSB
    {
      result = pthread_create(&student_tid[i], NULL, classb_student, (void *)&s_info[i]);
    }

    if (result) 
    {
      printf("officehour: thread_fork failed for student %d: %s\n", 
            i, strerror(result));
      exit(1);
    }
  }

  /* wait for all student threads to finish */
  for (i = 0; i < num_students; i++) 
  {
    pthread_join(student_tid[i], &status);
  }

  /* tell the professor to finish. */
  pthread_cancel(professor_tid);

  printf("Office hour simulation done.\n");

  return 0;
}
