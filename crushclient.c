#define SA struct sockaddr;
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
struct sockaddr_in srv_addr;    
int sk;  			//struttura del socket
void connectserver(char* ip, char* porta)
{
	int ret;
	sk = socket(AF_INET, SOCK_STREAM, 0);		//afinet =ipv4  stream e' TCP, 0 protoccolo
	if (sk==-1) 
	{
		printf("errore nel creare socket");
		return;
	}
printf("1");
	memset(&srv_addr, 0, sizeof(srv_addr));     //azzero la struttura
	srv_addr.sin_family=AF_INET;					// famiglia ipv4
	srv_addr.sin_port = htons(atoi(porta));			//porta
	ret = inet_pton(AF_INET, ip, &srv_addr.sin_addr); // per formato di rete
	printf("%d",ret);
	if (ret==-1) 
	{
		printf("errore formato di rete");
		return;
	}
	
	
	ret = connect(sk,(struct sockaddr *)&srv_addr, sizeof(srv_addr));  // mi collego con il server
		
	if (ret==-1)
		{
			printf("errore connessione");
			return;
		}
	else
	printf("connessione ok");
}
int main(int quantiparametri,char* arg[])
{
	printf("lollo");
		
	int ret;
	char msg[5]="ciao";
	msg[4]=0;
	if (quantiparametri != 3)
		{
		 printf ("devi passare 2 parametri : ip e porta");
		exit(1);
		}
	printf("p1");
			
	connectserver(arg[1],arg[2]);
		printf("ciao numero dati inviati : ");
	ret=send(sk,(void*)msg,5,0);
		printf("%d",ret);
	if (ret==-1 || ret < strlen(msg))
			printf ("errore send");
	else
			printf("mandati:");

	return 0;
}
