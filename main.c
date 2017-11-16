
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "matrix2d.h"
#include "mplib3.h"
#include "thread.h"



/*--------------------------------------------------------------------
| Function: parse_integer_or_exit
---------------------------------------------------------------------*/

int parse_integer_or_exit(char const *str, char const *name)
{
	int value;
 
	if(sscanf(str, "%d", &value) != 1) {
		fprintf(stderr, "\nErro no argumento \"%s\".\n\n", name);
		 exit(1);
  	}
  	return value;
}

/*--------------------------------------------------------------------
| Function: parse_double_or_exit
---------------------------------------------------------------------*/

double parse_double_or_exit(char const *str, char const *name)
{
	double value;

	if(sscanf(str, "%lf", &value) != 1) {
		fprintf(stderr, "\nErro no argumento \"%s\".\n\n", name);
    		exit(1);
  	}
  	return value;
}





int main(int argc, char ** argv) {
	
	if(argc != 9) {
    		fprintf(stderr, "\nNumero invalido de argumentos.\n");
 		fprintf(stderr, "Uso: heatSim N tEsq tSup tDir tInf iter trab csz\n\n");
 		return 1;
	}


	int N = parse_integer_or_exit(argv[1], "N");
	double tEsq = parse_double_or_exit(argv[2], "tEsq");
	double tSup = parse_double_or_exit(argv[3], "tSup");
	double tDir = parse_double_or_exit(argv[4], "tDir");
	double tInf = parse_double_or_exit(argv[5], "tInf");
	int iter = parse_integer_or_exit(argv[6], "iter");
	int trab = parse_integer_or_exit(argv[7], "trab");
	int csz = parse_integer_or_exit(argv[8], "csz");


	if (N < 1 || tEsq < 0 || tSup < 0 || tDir < 0 || tInf < 0 || iter < 1 || trab < 1 ||
N % trab != 0 || csz < 0 ) {
		printf("Argumentos invalidos\n"); return 1;
	}

		
	fprintf(stderr, "\nArgumentos:\nN=%d tEsq=%.1f tSup=%.1f tDir=%.1f" 
" tInf=%.1f iteracoes=%d\n",	N, tEsq, tSup, tDir, tInf, iter);

	DoubleMatrix2D *matrix;

	matrix = dm2dNew(N+2, N+2);

	/*valores iniciais da matrix*/
	dm2dSetLineTo (matrix, 0, tSup);
	dm2dSetLineTo (matrix, N+1, tInf);
	dm2dSetColumnTo (matrix, 0, tEsq);
	dm2dSetColumnTo (matrix, N+1, tDir);

	
	/*alocacao dos threads, seus argumentos e um buffer para a main thread*/
	pthread_t *threads = (pthread_t*) malloc(trab *  sizeof(pthread_t));
	Thread_Arg arguments = (Thread_Arg) malloc(trab * sizeof(struct thread_arg));
	double *buffer = (double*) malloc((N + 2) * sizeof(double));

	/*inicializacao dos canais de comunicacao entre threads*/
	if (inicializarMPlib(csz, trab + 1) != 0) {
   		printf("Erro ao inicializar MPLib.\n"); return 1;
	}

	/*atribuicao dos argumentos e inicializacao das threads*/
	int i, j; /*iteradores*/
	for (i = 0; i < trab; i++) {
		setId(&arguments[i], i);
		setSizeLine(&arguments[i], N + 2);
		setNLine(&arguments[i], N / trab + 2);
		setIter(&arguments[i], iter);
		if (pthread_create(&threads[i], NULL, theThread, &arguments[i]) != 0) {
      			fprintf(stderr, "\nErro ao criar uma thread.\n"); return 1;
   		}  			

		/*envio das fatias da matrix as respectivas threads*/
		for (j = 0; j < N / trab + 2; j++) 
			if(enviarMensagem(trab, i, dm2dGetLine (matrix, i * N / trab + j ), (N + 2)
* sizeof(double)) != (N + 2) * sizeof(double))
				kill("Erro ao enviar mensagem no main thread");
	}


	/*recebe os resultados das threads*/
	for (i = 0; i < trab; i++) 
		for (j = 0; j < N/trab; j++) {
			if(receberMensagem(i, trab, buffer, sizeof(double) * (N + 2)) 
!= sizeof(double) * (N + 2))
				kill("Erro ao receber mensagem no main thread");

			dm2dSetLine (matrix, i * N / trab + j + 1, buffer);
		}

	/*imprime a matrix resultante*/
	dm2dPrint(matrix);

	for (i = 0; i < trab; i++) {
    		if (pthread_join(threads[i], NULL) != 0) 
     			kill("Erro ao esperar por uma thread");
    
  	}


	free(threads);
	libertarMPlib();
	dm2dFree(matrix);
	free(arguments);
	free(buffer);


	return 0;
	
}
