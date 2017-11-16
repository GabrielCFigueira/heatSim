#ifndef _THREAD_H_
#define _THREAD_H_



typedef struct thread_arg{
	int 		id;
  int     size_line;
  int     n_line;
	int 		iter;
}*Thread_Arg;


void kill(char* reason);

void* theThread(void * a);
void calc_values(DoubleMatrix2D *matrix, DoubleMatrix2D *matrix_aux, int n_line, int size_line);
int getId(Thread_Arg arg);
int getSizeLine(Thread_Arg arg);
int getNLine(Thread_Arg arg);
int getIter(Thread_Arg arg);
void setIter(Thread_Arg arg, int iter);
void setId(Thread_Arg arg, int id);
void setSizeLine(Thread_Arg arg, int size_line);
void setNLine(Thread_Arg arg, int n_line);



#endif
