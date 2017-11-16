
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "matrix2d.h"
#include "mplib3.h"
#include "thread.h"


void kill(char* reason) {
	fprintf(stderr, "%s\n", reason);
	exit(1);
}



void* theThread(void * a) {

	Thread_Arg arg = (Thread_Arg) a;
	DoubleMatrix2D* matrix = dm2dNew(getNLine(arg), getSizeLine(arg));
	DoubleMatrix2D* matrix_aux = dm2dNew(getNLine(arg), getSizeLine(arg));
	DoubleMatrix2D* tmp;


	/*buffer para receber mensagens*/
	double* buffer = (double*) malloc(getSizeLine(arg)*sizeof(double)); 

	if(matrix == NULL || matrix_aux == NULL || buffer == NULL) 
		kill("Erro ao alocar memoria numa thread");

	int trab = (getSizeLine(arg) - 2) / (getNLine(arg) - 2);    /*numero de threads*/
	int i; /*iterador*/

	/*recebe as fatias da matriz da main thread*/
	for (i = 0; i < getNLine(arg); i++) {
		if(receberMensagem(trab, getId(arg), buffer, getSizeLine(arg)*sizeof(double)) 
!= getSizeLine(arg)*sizeof(double)) 
			kill("Erro ao receber mensagem numa thread"); 
		dm2dSetLine(matrix, i, buffer);
	}

	dm2dCopy(matrix_aux, matrix);
	for (i = 0; i < getIter(arg); i++) {
		calc_values(matrix, matrix_aux, getNLine(arg), getSizeLine(arg));

		 /*esta condicao serve para resolver o problema que surge quando a 
capacidade de cada canal e 0, alternando a ordem com que se envia e recebe uma
mensagem entre cada thread consecutiva*/
		if(getId(arg) % 2 == 0) { 
			if (getId(arg) - 1 >= 0) {

				/*se existir uma thread anterior a atual, envia-se-lhe a nossa segunda 
linha*/
				if(enviarMensagem(getId(arg), getId(arg) - 1, dm2dGetLine(matrix_aux, 1),
getSizeLine(arg) * sizeof(double)) != getSizeLine(arg) * sizeof(double))
					kill("Erro ao enviar mensagem numa thread");

				/*de seguida recebe-se a nossa primeira linha atualizada dessa mesma
 thread*/
				if(receberMensagem(getId(arg) - 1, getId(arg), buffer, getSizeLine(arg) *
sizeof(double)) != getSizeLine(arg) *sizeof(double))
					kill("Erro ao receber mensagem numa thread"); 

				dm2dSetLine(matrix_aux, 0, buffer);
			}

			if (getId(arg) + 1 < trab) {

				/*se existir uma thread posterior a atual, envia-se-lhe a nossa 
penultima linha*/
				if(enviarMensagem(getId(arg), getId(arg) + 1, dm2dGetLine(matrix_aux, 
getNLine(arg) - 2), getSizeLine(arg) * sizeof(double)) != getSizeLine(arg) * sizeof(double))
					kill("Erro ao enviar mensagem numa thread");

				/*de seguida recebe-se a nossa ultima linha atualizada dessa mesma
thread*/
				if(receberMensagem(getId(arg) + 1, getId(arg), buffer, sizeof(double) *
getSizeLine(arg)) != sizeof(double) * getSizeLine(arg))
					kill("Erro ao receber mensagem numa thread"); 

				dm2dSetLine(matrix_aux, getNLine(arg) - 1, buffer);
			}
		}
		else {  /*processo alternado como foi explicado*/
			if (getId(arg) - 1 >= 0) {
				if(receberMensagem(getId(arg) - 1, getId(arg), buffer, getSizeLine(arg) * 
sizeof(double)) != getSizeLine(arg) * sizeof(double))
					kill("Erro ao receber mensagem numa thread");

				if(enviarMensagem(getId(arg), getId(arg) - 1, dm2dGetLine(matrix_aux, 1), 
getSizeLine(arg) * sizeof(double)) != getSizeLine(arg) * sizeof(double)) 
					kill("Erro ao enviar mensagem numa thread");

				dm2dSetLine(matrix_aux, 0, buffer);
			}
			if (getId(arg) + 1 < trab) {
				if(receberMensagem(getId(arg) + 1, getId(arg), buffer, sizeof(double) *
getSizeLine(arg)) != sizeof(double) * getSizeLine(arg))
		 			kill("Erro ao receber mensagem numa thread");

				if(enviarMensagem(getId(arg), getId(arg) + 1, dm2dGetLine(matrix_aux,
getNLine(arg) - 2), getSizeLine(arg) * sizeof(double)) != getSizeLine(arg) * sizeof(double))
					
					kill("Erro ao enviar mensagem numa thread");
				dm2dSetLine(matrix_aux, getNLine(arg) - 1, buffer);
			}
		}

		/*troca dos ponteiros matrix e matrix_aux*/
		tmp = matrix;
		matrix = matrix_aux;
		matrix_aux = tmp;
	}

	/*por ultimo, envia-se os resultados a main thread*/
	for (i = 1; i < getNLine(arg) - 1; i++) 
		if (enviarMensagem(getId(arg), trab, dm2dGetLine(matrix, i) , getSizeLine(arg) 
* sizeof(double)) != getSizeLine(arg) * sizeof(double))
			kill("Erro ao enviar mensagem numa thread");

	free(buffer);
	dm2dFree(matrix);
	dm2dFree(matrix_aux);


	return 0;
}




void calc_values(DoubleMatrix2D *matrix, DoubleMatrix2D *matrix_aux, int n_line, int size_line) {
	int i, j; /*iteradores*/
	for (i = 1; i < n_line - 1; i++)
		for(j = 1; j < size_line - 1; j++) 
			/*calculo dos valores segundo o algoritmo do enunciado*/
			dm2dSetEntry(matrix_aux, i, j, (dm2dGetEntry(matrix, i - 1, j) + 
dm2dGetEntry(matrix, i, j - 1) + dm2dGetEntry(matrix, i + 1, j) + 
dm2dGetEntry(matrix, i, j + 1)) / 4);
}




/*funcoes de abstracao*/

int getId(Thread_Arg arg) {return arg->id;}
int getSizeLine(Thread_Arg arg) {return arg->size_line;}
int getNLine(Thread_Arg arg) {return arg->n_line;}
int getIter(Thread_Arg arg) {return arg->iter;}
void setIter(Thread_Arg arg, int iter) {arg->iter = iter;}
void setId(Thread_Arg arg, int id) { arg->id = id;}
void setSizeLine(Thread_Arg arg, int size_line) {arg->size_line = size_line;}
void setNLine(Thread_Arg arg, int n_line) {arg->n_line = n_line;}

