/*******************************************************************
* SERVIDOR no porto 9000, à escuta de novos clientes. Quando surjem
* novos clientes os dados por eles enviados são lidos e descarregados no ecran.
*******************************************************************/
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>

#define SERVER_PORT 9000
#define BUF_SIZE 1024

void erro(char *msg);
void process_client(int fd);
char * ler_servidor(int client_fd);
int verificar_login(FILE * users_passwords,char * username,int client_fd);
void adiciona_user(FILE * users_passwords,int client_fd);
void remove_user(FILE * users_passwords,char * username,int client_fd);
void apagar_linha(FILE *ficheiro_fonte, FILE *ficheiro_temporario, const int linha,char * nome_ficheiro_fonte);
void ver_caixa_entrada(char * username,int client_fd);
void replace(char * str,char find, char replace);
void ler_mensagens(char * username,int client_fd);
void enviar_mensagem_a_um_utilizador(FILE * users_passwords,char * username,int client_fd);
void enviar_mensagem_a_varios_utilizadores(FILE * users_passwords,char * username,int client_fd);
void apagar_mensagens_lidas(char *username,int client_fd);

int main() 
{
	int fd, client;
	struct sockaddr_in addr, client_addr;
	int client_addr_size;
	
	bzero((void *) &addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(SERVER_PORT);
	
	if ( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		erro("na funcao socket");
	if ( bind(fd,(struct sockaddr*)&addr,sizeof(addr)) < 0)
		erro("na funcao bind");
	if( listen(fd, 5) < 0)
		erro("na funcao listen");
	
	while (1) {
		client_addr_size = sizeof(client_addr);
		client = accept(fd,(struct sockaddr *)&client_addr, (socklen_t *)&client_addr_size);
		if (client > 0) {
			if (fork() == 0) 
			{
				close(fd);
				process_client(client);
				exit(0);
			}
			close(client);
		}
	}
	return 0;
}

void erro(char *msg)
{
	printf("Erro: %s\n", msg);
	exit(-1);
}

void process_client(int client_fd)
{
	//abrir o ficheiro que contem os utilizadores e passwords
	FILE * users_passwords;
	
	if ((users_passwords = fopen("users_passwords.txt", "a+")) == NULL)
	{
		char * erro=(char *)malloc(50*sizeof(char));
		erro="Erro ao abrir o ficheiro 'users_passwords.txt'";
		write(client_fd,erro,strlen(erro));
		exit(1);
	}

	
	char username[500];
	
	//verificar utilizador
	int tipo_user=verificar_login(users_passwords,username,client_fd);
	
	if(tipo_user==1)	//administrador
	{
		//menu
		char * escolha=(char *)malloc(2*sizeof(char));
		
		char aux_mensagem[200]="Bem vindo, admin!\n";
		char aux_mensagem2[200]="Escolha o que pretende fazer:\n";
		strcat(aux_mensagem,aux_mensagem2);
		char aux_mensagem3[200]="1 - Adicionar utilizador\n";
		strcat(aux_mensagem,aux_mensagem3);
		char aux_mensagem4[200]="2 - Remover utilizador\n";
		strcat(aux_mensagem,aux_mensagem4);
		char aux_mensagem5[200]="3 - Sair\n";
		strcat(aux_mensagem,aux_mensagem5);
		char aux_mensagem6[200]="A sua escolha: ";
		strcat(aux_mensagem,aux_mensagem6);
		
		//enviar menu para o admin
		write(client_fd,aux_mensagem,strlen(aux_mensagem));
		
		//ler escolha do admin
		escolha=ler_servidor(client_fd);
		
		//adicionar novo utiizador
		if(strcmp(escolha,"1")==0)
		{
			adiciona_user(users_passwords,client_fd);
		}
		//remover utilizador
		else if(strcmp(escolha,"2")==0)
		{
			remove_user(users_passwords,username,client_fd);
		}
		//sair do programa
		else if(strcmp(escolha,"3")==0)
		{
			//enviar mensagem de fim de programa
			char * aux_mensagem=(char *)malloc(20*sizeof(char));
			aux_mensagem="O programa terminou.\n";
			write(client_fd,aux_mensagem,strlen(aux_mensagem));
		}
		else
		{
			char * aux_mensagem=(char *)malloc(20*sizeof(char));
			aux_mensagem="Escolha inválida.\n";
			write(client_fd,aux_mensagem,strlen(aux_mensagem));
			
			while((strcmp(escolha,"1")!=0) && (strcmp(escolha,"2")!=0) && (strcmp(escolha,"3")!=0))
			{
				char aux_mensagem[200]="Escolha o que pretende fazer:\n";
				char aux_mensagem2[200]="1 - Adicionar utilizador\n";
				strcat(aux_mensagem,aux_mensagem2);
				char aux_mensagem3[200]="2 - Remover utilizador\n";
				strcat(aux_mensagem,aux_mensagem3);
				char aux_mensagem4[200]="3 - Sair\n";
				strcat(aux_mensagem,aux_mensagem4);
				char aux_mensagem5[200]="A sua escolha: ";
				strcat(aux_mensagem,aux_mensagem5);
				
				//enviar menu para o admin
				write(client_fd,aux_mensagem,strlen(aux_mensagem));
					
				//ler escolha do admin
				escolha=ler_servidor(client_fd);
				
				//adicionar novo utiizador
				if(strcmp(escolha,"1")==0)
				{
					adiciona_user(users_passwords,client_fd);
				}
				//remover utilizador
				else if(strcmp(escolha,"2")==0)
				{
					remove_user(users_passwords,username,client_fd);
				}
				//sair do programa
				else if(strcmp(escolha,"3")==0)
				{
					//enviar mensagem de fim de programa
					char * aux_mensagem=(char *)malloc(20*sizeof(char));
					aux_mensagem="O programa terminou.\n";
					write(client_fd,aux_mensagem,strlen(aux_mensagem));
				}
				else
				{
					char * aux_mensagem=(char *)malloc(20*sizeof(char));
					aux_mensagem="Escolha inválida.\n";
					write(client_fd,aux_mensagem,strlen(aux_mensagem));
				}
			}
		}
	}
	else if(tipo_user==0)	//utilizador
	{
		char * escolha=(char *)malloc(2*sizeof(char));;
		
		char aux_mensagem[500]="Bem vindo,";
		strcat(aux_mensagem,username);
		strcat(aux_mensagem,"!\n");
		char aux_mensagem2[500]="Escolha o que pretende fazer:\n";
		strcat(aux_mensagem,aux_mensagem2);
		char aux_mensagem3[500]="1 - Ver se existem mensagens enviadas por outros utilizadores\n";
		strcat(aux_mensagem,aux_mensagem3);
		char aux_mensagem4[500]="2 - Ler mensagens\n";
		strcat(aux_mensagem,aux_mensagem4);
		char aux_mensagem5[500]="3 - Enviar mensagem a outro utilizador\n";
		strcat(aux_mensagem,aux_mensagem5);
		char aux_mensagem6[500]="4 - Enviar a mesma mensagem a vários utilizadores\n";
		strcat(aux_mensagem,aux_mensagem6);
		char aux_mensagem7[500]="5 - Apagar mensagens lidas\n";
		strcat(aux_mensagem,aux_mensagem7);
		char aux_mensagem8[500]="A sua escolha:";
		strcat(aux_mensagem,aux_mensagem8);
		
		//enviar menu para o utilizador
		write(client_fd,aux_mensagem,strlen(aux_mensagem));
		
		//ler escolha do utilizador
		escolha=ler_servidor(client_fd);
		
		//ver se existem mensagem mensagens lidas
		if(strcmp(escolha,"1")==0)
		{
			ver_caixa_entrada(username,client_fd);
		}
		//ler mensagens da caixa de entrada
		else if(strcmp(escolha,"2")==0)
		{
			ler_mensagens(username,client_fd);
		}
		//enviar mensagem a um utilizador
		else if(strcmp(escolha,"3")==0)
		{
			enviar_mensagem_a_um_utilizador(users_passwords,username,client_fd);
		}
		//enviar a mesma mensagem a varios utilizadores
		else if(strcmp(escolha,"4")==0)
		{
			enviar_mensagem_a_varios_utilizadores(users_passwords,username,client_fd);
		}
		//apagar as mesnagens lidas
		else if(strcmp(escolha,"5")==0)
		{
			apagar_mensagens_lidas(username,client_fd);
		}
		else
		{
			char * aux_mensagem=(char *)malloc(20*sizeof(char));
			aux_mensagem="Escolha inválida.\n";
			write(client_fd,aux_mensagem,strlen(aux_mensagem));
			
			while((strcmp(escolha,"1")!=0) && (strcmp(escolha,"2")!=0) && (strcmp(escolha,"3")!=0) && (strcmp(escolha,"4")!=0) && (strcmp(escolha,"5")!=0))
			{
				char aux_mensagem[500]="Escolha o que pretende fazer:\n";
				char aux_mensagem2[500]="1 - Ver se existem mensagens enviadas por outros utilizadores\n";
				strcat(aux_mensagem,aux_mensagem2);
				char aux_mensagem3[500]="2 - Ler mensagens\n";
				strcat(aux_mensagem,aux_mensagem3);
				char aux_mensagem4[500]="3 - Enviar mensagem a outro utilizador\n";
				strcat(aux_mensagem,aux_mensagem4);
				char aux_mensagem5[500]="4 - Enviar a mesma mensagem a vários utilizadores\n";
				strcat(aux_mensagem,aux_mensagem5);
				char aux_mensagem6[500]="5 - Apagar mensagens lidas\n";
				strcat(aux_mensagem,aux_mensagem6);
				char aux_mensagem7[500]="A sua escolha:";
				strcat(aux_mensagem,aux_mensagem7);
				
				//enviar menu para o utilizador
				write(client_fd,aux_mensagem,strlen(aux_mensagem));
					
				//ler escolha do utilizador
				escolha=ler_servidor(client_fd);
				
				if(strcmp(escolha,"1")==0)
				{
					ver_caixa_entrada(username,client_fd);
				}
				else if(strcmp(escolha,"2")==0)
				{
					ler_mensagens(username,client_fd);
				}
				else if(strcmp(escolha,"3")==0)
				{
					enviar_mensagem_a_um_utilizador(users_passwords,username,client_fd);
				}
				else if(strcmp(escolha,"4")==0)
				{
					enviar_mensagem_a_varios_utilizadores(users_passwords,username,client_fd);
				}
				else if(strcmp(escolha,"5")==0)
				{
					apagar_mensagens_lidas(username,client_fd);
				}
				else
				{
					char * aux_mensagem=(char *)malloc(20*sizeof(char));
					aux_mensagem="Escolha inválida.\n";
					write(client_fd,aux_mensagem,strlen(aux_mensagem));
				}
			}
		}
	}
	else if(tipo_user==-1)
	{
		char * aux_mensagem=(char *)malloc(20*sizeof(char));
		aux_mensagem="O username que introduziu não existe.\n";
		write(client_fd,aux_mensagem,strlen(aux_mensagem));
	}
	
	fflush(stdout);
	close(client_fd);
}

//esta função lê o que o cliente introduzir e remove o '\n'
//retorna a string sem '\n'
char * ler_servidor(int client_fd)
{
	int nread = 0;
	char  * buffer=(char *)malloc(BUF_SIZE*sizeof(char));
	
	nread = read(client_fd, buffer, BUF_SIZE-1);
	buffer[nread] = '\0';
	
	replace(buffer,'\n','\0');
	
	return buffer;
}

//substitui na string str o caracter replace pelo caracter find
void replace(char * str,char find, char replace)
{
	int i = 0;
	while(str[i]!='\0')
	{
		if(str[i]==find)
			str[i]=replace;
		i++;
	}
}

//esta função pede ao cliente o nome de utilizador e password
//se o nome de utilizador não existir devolve -1
//se o nome de utilizador corresponder a um utilizador comum, devolve 0
//se o nome de utilizador for o admin devolve 1
int verificar_login(FILE * users_passwords,char * username,int client_fd)
{
	//pedir nome de utilizador	
	char user_introduzido_aux[BUF_SIZE]="Introduza o seu nome de utilizador:\n";
	write(client_fd,user_introduzido_aux,strlen(user_introduzido_aux));
	
	char * user_introduzido=ler_servidor(client_fd);
	
	printf("user_introduzido:'%s'\n",user_introduzido);
	
	char * user=(char *)malloc(50*sizeof(char));
	char * password=(char *)malloc(50*sizeof(char));
	
	//ler todas as linhas do ficheiro até ao fim do ficheiro
	while(fscanf(users_passwords,"%s",username)!=EOF)//pôe cada linha lida em username
	{	
		user = strtok(username,"|");//obtem utilizador atual para user a partir de username
		if(strcmp(user_introduzido,user)==0)//se o nome introduzido for igual à linha filtrada
		{
			printf("O utilizador introduzido pelo cliente foi encontrado.\n");
			password = strtok (NULL, "|");//obtem a password do utilizador
			break;
		}
	}
	
	char * password_introduzida=(char *)malloc(50*sizeof(char));
	
	//compara o nome de utilizador introduzido com o último lido
	if(strcmp(user_introduzido,user)!=0)//se o utilizador introduzido não corresponder ao último lido do ficheiro
	{
		printf("O username que o cliente introduziu não existe.\n");
		char * aux_mensagem=(char *)malloc(50*sizeof(char));
		aux_mensagem = "O username que introduziu não existe.\n";
		write(client_fd,aux_mensagem,strlen(aux_mensagem));
		free(aux_mensagem);	
		free(user_introduzido);
		free(password_introduzida);
		return -1;
	}
	else 	//se o utilizador existir
	{
		//pedir password ao cliente
		char * aux_mensagem=(char *)malloc(50*sizeof(char));
		aux_mensagem="Introduza a sua password:\n";
		write(client_fd,aux_mensagem,strlen(aux_mensagem));
		
		password_introduzida=ler_servidor(client_fd);
		
		//pedir password ao cliente até introduzir a correta
		while(strcmp(password,password_introduzida)!=0)
		{
			aux_mensagem="Password errada. Tente de novo:\n";
			write(client_fd,aux_mensagem,strlen(aux_mensagem));
			
			password_introduzida=ler_servidor(client_fd);
		}
		
		printf("A password introduzida pelo cliente está correta.\n");
	}
	
	//se for o admin
	if(strcmp(user,"admin")==0)
	{
		return 1;//devolve 1
	}
	else 	//se for outro utilizador
	{
		return 0;//devolve 0
	}	
}

//esta função é de acesso exclusivo do admin
//pede ao admin o nome do novo utilizador e a sua password
void adiciona_user(FILE * users_passwords,int client_fd)
{
	//pedir nome de utilizador
	char * aux_mensagem=(char *)malloc(100*sizeof(char));
	sprintf(aux_mensagem,"Introduza o nome do utilizador que pretende adicionar:\n");
	write(client_fd,aux_mensagem,strlen(aux_mensagem));
	
	char * utilizador_introduzido=(char *)malloc(50*sizeof(char));
	
	utilizador_introduzido=ler_servidor(client_fd);
	
	printf("Utilizador a adicionar: %s\n",utilizador_introduzido);
	
	//escreve nome de utilizador no ficheiro de utilizadores e passwords
	fprintf(users_passwords,"%s|",utilizador_introduzido);
	
	//pedir password
	sprintf(aux_mensagem,"Introduza a password do utilizador que pretende adicionar:\n");
	write(client_fd,aux_mensagem,strlen(aux_mensagem));
	
	char * password_utilizador=ler_servidor(client_fd);
	
	//escrever password no ficheiro de utilizadores e passwords
	fprintf(users_passwords,"%s\n",password_utilizador);
	
	//criar caixa de entrada do novo utilizador, caso não seja o admin
	if(strcmp(utilizador_introduzido,"admin")!=0)
	{
		char fich_caixa_entrada[500] = "";
		
		const char str1[30]="caixa_entrada_";
		
		strcat(fich_caixa_entrada,str1);
		
		strcat(fich_caixa_entrada,utilizador_introduzido);
		
		strcat(fich_caixa_entrada,".txt");
		
		printf("%s\n",fich_caixa_entrada);
		
		FILE * caixa_entrada_utilizador;
		
		//abrir caixa de entrada do novo utilizador e ver se há erro a abri-la
		if ((caixa_entrada_utilizador = fopen(fich_caixa_entrada, "a+")) == NULL)
		{
			perror("Erro ao abrir a caixa de entrada do novo utilizador");
			return;
		}
		
		//enviar mensagem de criação de caixa de entrada e user
		sprintf(aux_mensagem,"Utilizador criado com sucesso.\n");
		write(client_fd,aux_mensagem,strlen(aux_mensagem));
	}
	
	return;
}

//remove user do ficheiro de users e passwords
//e remove a sua caixa de entrada
void remove_user(FILE * users_passwords,char * username,int client_fd)
{
	//pede nome do utilizador a remover
	char * aux_mensagem=(char *)malloc(100*sizeof(char));
	sprintf(aux_mensagem,"Introduza o nome do utilizador que pretende remover:\n");
	write(client_fd,aux_mensagem,strlen(aux_mensagem));
	
	char * user_introduzido=ler_servidor(client_fd);
	
	//se o nome de utilizador introduzido for 'admin'
	if(strcmp(user_introduzido,"admin")==0)
	{
		sprintf(aux_mensagem,"Não pode eliminar o admin.\n");
		write(client_fd,aux_mensagem,strlen(aux_mensagem));
		return;
	}
	
	int linha=0;
	
	char * user;
	
	rewind(users_passwords);//põe o ponteiro do ficheiro no inicio do ficheiro
	
	while(fscanf(users_passwords,"%s",username)!=EOF)//pôe cada linha lida em username
	{	
		user = strtok(username,"|");//obtem utilizador atual para user a partir da linha atual
		
		linha ++;
		
		printf("%s\n",user);
		
		if(strcmp(user,user_introduzido)==0)//se o utilizador introduzido e lido forem iguais
			break;
	}
	
	//se o nome de utilizador introduzido não existir
	if(strcmp(user,user_introduzido)!=0)
	{
		sprintf(aux_mensagem,"Não existe nenhum utilizador com esse nome.\n");
		write(client_fd,aux_mensagem,strlen(aux_mensagem));
		return;
	}
	
	FILE * ficheiro_temporario;
	
	//abrir caixa de entrada e ver se houve algum erro a abri-la
	if ((ficheiro_temporario = fopen("apagar_linha.txt", "w")) == NULL)
	{
		perror("Erro ao abrir o ficheiro temporário.");
		return;
	}
	
	apagar_linha(users_passwords, ficheiro_temporario,linha,"users_passwords.txt");
	
	//apagar caixa de entrada do utilizador
	
	char fich_caixa_entrada[500] = "";
		
	const char str1[30]="caixa_entrada_";
	
	strcat(fich_caixa_entrada,str1);
	
	strcat(fich_caixa_entrada,username);
	
	strcat(fich_caixa_entrada,".txt");
	
	if(remove(fich_caixa_entrada))
	{
		perror("Erro ao apagar caixa de entrada");
		exit(1);
	}	
	
	sprintf(aux_mensagem,"Utilizador removido com sucesso.\n");
	write(client_fd,aux_mensagem,strlen(aux_mensagem));
}

//esta função copia para o ficheiro temporario toda a informação a menos a que se pretende remover
//e depois remove o ficheiro fonte e põe o nome do ficheiro fonte no ficheiro temporário
//tornando-o o novo ficheiro
void apagar_linha(FILE *ficheiro_fonte, FILE *ficheiro_temporario, const int linha,char * nome_ficheiro_fonte)
{
    char buffer[500];
    int contador = 1;//contador de linha
    
    rewind(ficheiro_fonte);

    while ((fgets(buffer, 500, ficheiro_fonte)) != NULL)//põe cada linha no buffer
    {
        if (linha != contador)//Se a linha atual não for a linha que se pretende remover
            fputs(buffer, ficheiro_temporario);//escreve a linha no ficheiro temporario

        contador++;//incrementa contador de linha
    }
    
    remove(nome_ficheiro_fonte);//remove o ficheiro original
    rename("apagar_linha.txt",nome_ficheiro_fonte);//e atribui e seu nome ao ficheiro que tem a informação que se pretende
}

//esta função verifica se a caixa de entrada do utilizador tem alguma mensagem
void ver_caixa_entrada(char * username,int client_fd)
{	
	//gerar nome do ficheiro que tem a caixa de entrada do utilizador
	char fich_caixa_entrada[500] = "";
	
	const char str1[30]="caixa_entrada_";
	
	strcat(fich_caixa_entrada,str1);
	
	strcat(fich_caixa_entrada,username);
	
	strcat(fich_caixa_entrada,".txt");
	
	FILE * caixa_entrada_utilizador;
	
	//abrir a caixa de entrada do utilizador
	if ((caixa_entrada_utilizador = fopen(fich_caixa_entrada, "a+")) == NULL)
	{
		perror("Erro ao abrir o ficheiro 'users_passwords.txt'");
		exit(1);
	}
	
	//por ponteiro da caixa de entrada no inicio do ficheiro
	rewind(caixa_entrada_utilizador);
	
	//gerar string vazia para comparação a seguir
	char comparacao[1];
	comparacao[0]='\0';
	
	//ver se a caixa de entrada está vazia
	if(fscanf(caixa_entrada_utilizador,"%s",comparacao)==EOF)
	{
		char * aux_mensagem=(char *)malloc(50*sizeof(char));
		sprintf(aux_mensagem,"Não tem mensagens enviadas por outros utilizadores.\n");
		write(client_fd,aux_mensagem,strlen(aux_mensagem));
	}
	else
	{
		char * aux_mensagem=(char *)malloc(50*sizeof(char));
		sprintf(aux_mensagem,"Tem mensagens enviadas por outros utilizadores.\n");
		write(client_fd,aux_mensagem,strlen(aux_mensagem));
	}
}

void ler_mensagens(char * username,int client_fd)
{
	//nome da caixa de entrada do utilizador
	char fich_caixa_entrada[500] = "";
	
	const char str1[30]="caixa_entrada_";
	
	strcat(fich_caixa_entrada,str1);
	
	strcat(fich_caixa_entrada,username);
	
	strcat(fich_caixa_entrada,".txt");
	
	printf("%s\n",fich_caixa_entrada);
	
	FILE * caixa_entrada_user;
	
	//abrir caixa de entrada do destinatário e ver se há erro a abri-la
	if ((caixa_entrada_user = fopen(fich_caixa_entrada, "r")) == NULL)
	{
		perror("Erro ao abrir a caixa de entrada do utilizador destino.\n");
		return;
	}
	
	//pôr ponteiro do inicio do ficheiro
	rewind(caixa_entrada_user);
	
	//abrir ficheiro temporario
	FILE * ficheiro_temporario;

	//ver se houve algum erro a abri-la
	if ((ficheiro_temporario = fopen("apagar_linha.txt", "w")) == NULL)
	{
		perror("Erro ao abrir o ficheiro temporário.");
		return;
	}
	
	//variável para linhas lidas da caixa de entrada
	char * linha_lida=(char *)malloc(200*sizeof(char));
	
	while(fgets(linha_lida,199,caixa_entrada_user)!=NULL)//ler cada linha para 'linha _lida'
	{
		replace(linha_lida,'\n','\0');
		
		int i=2,j=0,k=0;
		char * quem_enviou=(char *)malloc(200*sizeof(char));
		
		//ler quem enviou
		while(linha_lida[i]!='|')
		{
			quem_enviou[j]=linha_lida[i];
			j++;
			i++;
		}
		
		//fica pronto para ler a mensagem
		i++;
		
		//fazer write para o cliente com a informação de quem enviou
		char * aux_mensagem=(char *)malloc(100*sizeof(char));
		sprintf(aux_mensagem,"Mensagem enviada por: ");
		strcat (aux_mensagem,quem_enviou);
		strcat (aux_mensagem,"\n");
		write(client_fd,aux_mensagem,strlen(aux_mensagem));
			
		printf("Mensagem enviada por: %s\n",quem_enviou);
		
		char * mensagem=(char *)malloc(200*sizeof(char));
		
		//ler mensagem para 'mensagem'
		while(linha_lida[i]!='\0')
		{
			mensagem[k]=linha_lida[i];
			k++;
			i++;
		}
		
		//fazer write para o cliente com a informação da mensagem
		sprintf(aux_mensagem,"Mensagem: ");
		strcat (aux_mensagem,"'");
		strcat (aux_mensagem,mensagem);
		strcat (aux_mensagem,"'\n");
		write(client_fd,aux_mensagem,strlen(aux_mensagem));
		
		printf("Mensagem: '%s'\n",mensagem);
		
		linha_lida[0]='1';//marca a mensagem como lida
		
		strcat(linha_lida,"\n");
		fputs(linha_lida,ficheiro_temporario);//escreve a linha no ficheiro temporario
	}
	
	remove(fich_caixa_entrada);//remove o ficheiro original
	rename("apagar_linha.txt",fich_caixa_entrada);//e atribui e seu nome ao ficheiro que tem a informação que se pretende
}

void enviar_mensagem_a_um_utilizador(FILE * users_passwords,char * username,int client_fd)
{
	//pôr ponteiro do ficheiro de users e passwords no inicio do inicio do ficheiro
	rewind(users_passwords);
	
	//pedir ao utilizador o destinatário da mensagem
	char * aux_mensagem=(char *)malloc(500*sizeof(char));
	sprintf(aux_mensagem,"Introduza o nome do utilizador a quem pretende enviar a mensagem:\n");
	write(client_fd,aux_mensagem,strlen(aux_mensagem));
	
	//destinatario de 49 caracteres max
	char * utilizador_destino=(char *)malloc(50*sizeof(char));
	
	//ler destinatario introduzido pelo cliente
	utilizador_destino=ler_servidor(client_fd);
	
	//se o destinatario for o proprio utilizador
	if(strcmp(utilizador_destino,username)==0)
	{
		sprintf(aux_mensagem,"Não pode enviar uma mensagem a si próprio.\n");
		write(client_fd,aux_mensagem,strlen(aux_mensagem));
		return;
	}
	//se o destinatario for o admin
	else if(strcmp(utilizador_destino,"admin")==0)
	{
		sprintf(aux_mensagem,"Não pode enviar mensagens ao admin.\n");
		write(client_fd,aux_mensagem,strlen(aux_mensagem));
	}
	else
	{
		char * destinatario=(char *)malloc(50*sizeof(char));
		
		while(fscanf(users_passwords,"%s",destinatario)!=EOF)//põe cada linha lida(destinatario) em destinatario
		{	
			replace(destinatario,'|','\0');//obtem destinatario atual para user a partir da linha lida atual
						
			if(strcmp(destinatario,utilizador_destino)==0)//sai do ciclo while se encontrar o utilizador destinatario
			{
				printf("O utilizador para quem quer enviar mensagem foi encontrado.\n");
				break;
			}
		}
		
		//termina função com mensagem adequada caso não exista o utilizador introduzido
		if(strcmp(destinatario,utilizador_destino)!=0)
		{
			sprintf(aux_mensagem,"Não existe nenhum utilizador com o username que introduziu.\n");
			write(client_fd,aux_mensagem,strlen(aux_mensagem));
			return;
		}
		
		//pedir mensagem a enviar
		char * aux_mensagem=(char *)malloc(100*sizeof(char));
		sprintf(aux_mensagem,"Introduza a mensagem que pretende enviar:\n");
		write(client_fd,aux_mensagem,strlen(aux_mensagem));
		
		char * mensagem=(char *)malloc(100*sizeof(char));
		mensagem=ler_servidor(client_fd);
		printf("%s\n",mensagem);
		
		//string da caixa de entrada do destinatario
		char fich_caixa_entrada[500] = "";
		
		const char str1[30]="caixa_entrada_";
		
		strcat(fich_caixa_entrada,str1);
		
		strcat(fich_caixa_entrada,destinatario);
		
		strcat(fich_caixa_entrada,".txt");
		
		printf("%s\n",fich_caixa_entrada);
		
		FILE * caixa_entrada_destinatario;
		
		//abrir caixa de entrada do destinatário e ver se há erro a abri-la
		if ((caixa_entrada_destinatario = fopen(fich_caixa_entrada, "a+")) == NULL)
		{
			perror("Erro ao abrir a caixa de entrada do utilizador destino.\n");
			return;
		}
		
		//escrever mensagem de forma formatada
		//flag_lida|quem_enviou|mensagem
		
		//marcar a mensagem como não lida com uma flag
		fprintf(caixa_entrada_destinatario,"%s|","0");
		
		//escrever o nome do user que enviou a mensagem
		fprintf(caixa_entrada_destinatario,"%s|",username);
		
		//escrever mensagem
		fprintf(caixa_entrada_destinatario,"%s\n",mensagem);
		
		return;
	}
}

void enviar_mensagem_a_varios_utilizadores(FILE * users_passwords,char * username,int client_fd)
{
	//pedir mensagem a enviar
	char * aux_mensagem=(char *)malloc(100*sizeof(char));
	sprintf(aux_mensagem,"Introduza a mensagem que pretende enviar:\n");
	write(client_fd,aux_mensagem,strlen(aux_mensagem));
	
	char * mensagem=(char *)malloc(100*sizeof(char));
	mensagem=ler_servidor(client_fd);
	printf("%s\n",mensagem);
	
	//perguntar cada pessoa a quem pretende enviar
	
	char * escolha=(char *)malloc(2*sizeof(char));
	escolha="s";
	
	while(strcmp(escolha,"s")==0 || strcmp(escolha,"S")==0)
	{
		//pôr ponteiro do ficheiro de users e passwords no inicio do inicio do ficheiro
		rewind(users_passwords);
		
		//pedir ao utilizador o destinatário da mensagem
		char * aux_mensagem=(char *)malloc(500*sizeof(char));
		sprintf(aux_mensagem,"Introduza o nome do utilizador a quem pretende enviar a mensagem:\n");
		write(client_fd,aux_mensagem,strlen(aux_mensagem));
		
		//destinatario de 49 caracteres max
		char * utilizador_destino=(char *)malloc(50*sizeof(char));
		
		//ler destinatario introduzido pelo cliente
		utilizador_destino=ler_servidor(client_fd);
		
		//se o destinatario for o proprio utilizador
		if(strcmp(utilizador_destino,username)==0)
		{
			sprintf(aux_mensagem,"Não pode enviar uma mensagem a si próprio.\n");
			write(client_fd,aux_mensagem,strlen(aux_mensagem));
			return;
		}
		//se o destinatario for o admin
		else if(strcmp(utilizador_destino,"admin")==0)
		{
			sprintf(aux_mensagem,"Não pode enviar mensagens ao admin.\n");
			write(client_fd,aux_mensagem,strlen(aux_mensagem));
			return;
		}
		else
		{
			char * destinatario=(char *)malloc(50*sizeof(char));
			
			while(fscanf(users_passwords,"%s",destinatario)!=EOF)//põe cada linha lida(destinatario) em destinatario
			{	
				replace(destinatario,'|','\0');//obtem destinatario atual para user a partir da linha lida atual
							
				if(strcmp(destinatario,utilizador_destino)==0)//sai do ciclo while se encontrar o utilizador destinatario
				{
					printf("O utilizador para quem quer enviar mensagem foi encontrado.\n");
					break;
				}
			}
			
			//termina função com mensagem adequada caso não exista o utilizador introduzido
			if(strcmp(destinatario,utilizador_destino)!=0)
			{
				sprintf(aux_mensagem,"Não existe nenhum utilizador com o username que introduziu.\n");
				write(client_fd,aux_mensagem,strlen(aux_mensagem));
				return;
			}
		
			//string da caixa de entrada do destinatario
			char fich_caixa_entrada[500] = "";
			
			const char str1[30]="caixa_entrada_";
			
			strcat(fich_caixa_entrada,str1);
			
			strcat(fich_caixa_entrada,destinatario);
			
			strcat(fich_caixa_entrada,".txt");
			
			printf("%s\n",fich_caixa_entrada);
			
			FILE * caixa_entrada_destinatario;
			
			//abrir caixa de entrada do destinatário e ver se há erro a abri-la
			if ((caixa_entrada_destinatario = fopen(fich_caixa_entrada, "a+")) == NULL)
			{
				perror("Erro ao abrir a caixa de entrada do utilizador destino.\n");
				return;
			}
			
			//escrever mensagem de forma formatada
			//flag_lida|quem_enviou|mensagem
			
			//marcar a mensagem como não lida com uma flag
			fprintf(caixa_entrada_destinatario,"%s|","0");
			
			//escrever o nome do user que enviou a mensagem
			fprintf(caixa_entrada_destinatario,"%s|",username);
			
			//escrever mensagem
			fprintf(caixa_entrada_destinatario,"%s\n",mensagem);
			
			sprintf(aux_mensagem,"Pretende enviar esta mensagem a mais algum utilizador?(s/n):");
			write(client_fd,aux_mensagem,strlen(aux_mensagem));
			
			//ler mensagem do cliente
			escolha=ler_servidor(client_fd);
		}
	}
}
	

void apagar_mensagens_lidas(char *username,int client_fd)
{
	//nome da caixa de entrada
	char fich_caixa_entrada[500] = "";
	
	const char str1[30]="caixa_entrada_";
	
	strcat(fich_caixa_entrada,str1);
	
	strcat(fich_caixa_entrada,username);
	
	strcat(fich_caixa_entrada,".txt");
	
	printf("%s\n",fich_caixa_entrada);
	
	FILE * caixa_entrada_user;
	
	//abrir caixa de entrada do destinatário e ver se há erro a abri-la
	if ((caixa_entrada_user = fopen(fich_caixa_entrada, "r")) == NULL)
	{
		perror("Erro ao abrir a caixa de entrada do utilizador destino.\n");
		return;
	}
	
	FILE * ficheiro_temporario;
	
	//abrir caixa de entrada e ver se houve algum erro a abri-la
	if ((ficheiro_temporario = fopen("apagar_linha.txt", "w")) == NULL)
	{
		perror("Erro ao abrir o ficheiro temporário.");
		return;
	}
	
	//ler linhas do ficheiro uma a uma
	char * linha_lida=(char *)malloc(200*sizeof(char));
	int linha=1;
	
	while(fgets(linha_lida,199,caixa_entrada_user)!=NULL)//ler cada linha para 'linha _lida'
	{	
		if(linha_lida[0]=='0')//sai do ciclo while se encontrar o utilizador destinatario
		{
			fputs(linha_lida,ficheiro_temporario);//escreve a linha no ficheiro temporario
		}
		
		linha++;
	}
	
	remove(fich_caixa_entrada);//remove o ficheiro original
    rename("apagar_linha.txt",fich_caixa_entrada);//e atribui e seu nome ao ficheiro que tem a informação que se pretende
}
