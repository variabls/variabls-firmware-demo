#include <Arduino.h>

uint64_t SparksMillis(){
  noInterrupts();
  uint64_t t = millis();
  interrupts();
  return t;
}

typedef void (*task_execute)(void);
typedef void (*task_execute_p)(void *);

class SparksTask {
  public:
    // maximum number of functions
    const static int MAX_FUNCTIONS = 16;

    void init(){
      unsigned long current_millis = SparksMillis();

      for (int i = 0; i < MAX_FUNCTIONS; i++){
        memset(&task[i], 0, sizeof(task_t));
        task[i].prev_millis = current_millis;

        numTask = 0;
      }
    }

    int setTask(String name, unsigned long d, task_execute f, unsigned n){
      return setupTask(name, d, (void *)f, NULL, false, n);
    }

    void deleteTask(unsigned taskId){
      if (taskId >= MAX_FUNCTIONS){
        return;
      }

      if (numTask == 0){
        return;
      }

      if (task[taskId].function != NULL){
        memset(&task[taskId], 0, sizeof (task_t));
        task[taskId].prev_millis = SparksMillis();

        numTask--;
      }
    }

    void run(){
      // Get Millis
      unsigned long current_millis = SparksMillis();

      //Check all task listed to be execute
      for (int i = 0; i < MAX_FUNCTIONS; i++){
        if (task[i].function != NULL){
          if ((current_millis - task[i].prev_millis) >= task[i].delay){
            //Check interval from current_millis to previous match to delay
            unsigned long skipTimes = (current_millis - task[i].prev_millis) / task[i].delay;
            task[i].prev_millis += task[i].delay * skipTimes;
            
            //Check that task is enabled to be execute
            if (task[i].enable){
              //check if number run is "0" it's mean run forever in loop with interval delay
              if (task[i].taskNumRun == 0){
                //Execute the task function
                if (task[i].hasParam){
                  (*(task_execute_p)task[i].function)(task[i].param); //function with param
                }
                else {
                  (*(task_execute)task[i].function)(); //without param
                }
              }

              //Run functions in desired number with interval delay
              else if (task[i].taskRun < task[i].taskNumRun){
                if (task[i].hasParam){
                  (*(task_execute_p)task[i].function)(task[i].param); //function with param
                }
                else {
                  (*(task_execute)task[i].function)(); //without param
                }
                Serial.println("[Sparks Task - " + String(i) + "]: " + task[i].taskRun);

                task[i].taskRun++; //update number execution after each runs

                //after last number of runs delete the task
                if (task[i].taskRun >= task[i].taskNumRun){
                  deleteTask(i);
                }
              }
            }
          }
        }
      }
    }

    bool filter_by_name(String name, int size, int &taskId){
      // bool found = false;
      for(int i = 0; i < size; i++){
        if (name == task[i].name){
          taskId = i;
          return true;
        }
      }
      return false;
    }


  private:
    int findFreeSlot() {
      //no slot available
      if (numTask >= MAX_FUNCTIONS){
        return -1;
      }

      for (int i = 0; i < MAX_FUNCTIONS; i++){
        if (task[i].name == NULL){
          return i;
        }
      }

      return -1;
    }

    int setupTask(String name, unsigned long d, void* f, void* p, bool h, unsigned n){
      int freeTask;

      if (numTask < 0){
        init();
      }

      freeTask = findFreeSlot();
      if (freeTask < 0){
        return -1;
      }

      if (f == NULL){
        return -1;
      }

      task[freeTask].name = name;
      task[freeTask].delay = d;
      task[freeTask].function = f;
      task[freeTask].hasParam = h;
      task[freeTask].param = p;
      task[freeTask].taskNumRun = n;
      task[freeTask].prev_millis = SparksMillis();
      task[freeTask].enable = true;

      numTask++;
      return freeTask;
    }

    typedef struct
    {
      unsigned long prev_millis;
      String name;
      void* function;
      void* param;
      bool hasParam;
      unsigned long delay;
      bool enable;
      unsigned taskNumRun;
      unsigned taskRun;
      
    } task_t;

    task_t task[MAX_FUNCTIONS];

    int numTask;

    // task_t* filter_by_name(String* name, task_t* tasks, int task_size, ){

    // }  
    
};