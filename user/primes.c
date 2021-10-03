#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void cull(int p) {
  int n;
  while (read(0, &n, sizeof(n))) {
    // n is not prime
    if (n % p != 0) {      //I'm checking to see if it's something multiple of the prime number I have now (p) 
      write(1, &n, sizeof(n));  //everything what is not multiple, I will write further in the parent pipe
    }
  }
}

void redirect(int k, int pd[]) { //on K place, get pipe FD
  close(k);			 //0R -> read from pipe
  dup(pd[k]);			 //1W -> write to pipe
  close(pd[0]);
  close(pd[1]);
}

void right() {  //parent comes here
  int pd[2], p;

  // read p
  if (read(0, &p, sizeof(p))) {  //read first prime number send from left(child)
    printf("prime %d\n", p);   //print prime number

    if (pipe(pd) < 0){   //make pipe
  	fprintf(2, "pipe() error\n");
  	exit(1);
    }
    
    int ret = fork();
    if (ret) {   // parent
      redirect(0, pd); //redirect R W -> read from new pipe
      right();
    } else if (ret == 0){  //child
      redirect(1, pd);   //redirect R W -> write to new pipe, read from old pipe
      cull(p);
    }
    else{	//error
    fprintf(2, "fork() error\n");
    exit(1);
    }
  }
}

int main(int argc, char *argv[]) {
  int pd[2];
  
  if (pipe(pd) < 0){   //make pipe
  	fprintf(2, "pipe() error\n");
  	exit(1);
  }
  
  int ret = fork();
  if (ret) {  //parent
    redirect(0, pd);  //redirect R W -> read from pipe 
    right();  //go right
  } 
  else if (ret == 0) {  //child
    redirect(1, pd);    //redirect R W -> write to pipe
    for (int i = 2; i < 36; i++) {   
      write(1, &i, sizeof(i));  //write to pipe, to parent, to right
    }
  }
  else{
    //chyba
    fprintf(2, "fork() error\n");
    exit(1);
  }

  exit(0);
}