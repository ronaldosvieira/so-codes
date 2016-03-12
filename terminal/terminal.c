#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define KGRN "\x1B[32m"
#define KCYN "\x1B[36m"
#define RESET "\033[0m"

int main() {
	char consulta[256];
	char* p;
	char* argv[100];
	int pid, i;

	while(1) {
		printf(KCYN "ronaldo's terminal %s$" RESET, KGRN);

		// le o comando
		fgets(consulta, sizeof(consulta), stdin);

		// retira manualmente o \n no fim do comando
		for (i = 0; consulta[i] != '\n'; i++);
		consulta[i] = '\0';

		// caso o comando seja 'exit', termina o programa
		if (strcmp(consulta, "exit") == 0) break;

		// separa a string por espacos e guarda no vetor argv
		i = 0;
		p = strtok(consulta," ");
		while (p != NULL) {
			argv[i++] = p;
			p = strtok(NULL, " ");
		}

		argv[i++] = NULL;

		// caso o comando seja 'cd', move-se para o diretorio recebido
		if (strcmp(argv[0], "cd") == 0) {
			chdir(argv[1]);
			continue;
		}

		// debug: mostra conteudo de argv
		/*for (j = 0; j < i; j++) {
			printf("pos %d: %s\n", j, argv[j]);
		}*/

		// cria um novo processo e executa o comando
		pid = fork();

		if (!pid) {
			execvp(argv[0], argv);
			break;
		}

		wait(NULL);
	}
}