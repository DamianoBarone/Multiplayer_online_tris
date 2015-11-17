#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/select.h>
#include<sys/time.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<errno.h>
struct sockaddr_in srv_addr, my_addr, client_addr; 
int sk,sudp;  			// socket: uno per server e uno per clientp2p
char nomeutente[25];
char comandi[7][12]={"!help","!who","!connect","!disconnect","!quit","!show_map","!hit"};
struct timeval  tempo;
char simbolo;
char shell;
char turno;
unsigned int portaudpu;
fd_set 	master,	//i descittori
        read_fd; // fd di appoggio per la select
        int		maxfd;
struct datiavversario
{      
	char		    nomeavversario[25];
	unsigned long	IPavversario;
	unsigned short	porta_udp_avversario;	
	char 			simbolo;
};
struct datiavversario avversario;
char griglia[9];    //griglia tris
int conta;
void help()
{
		printf("Sono disponibili i seguenti comandi:\n * !help --> mostra l'elenco dei comandi disponibili\n * !who --> mostra l'elenco dei client connessi al server\n * !connect nome_client --> avvia una partita con l'utente nome_client\n * !disconnect --> disconnette il client dall'attuale partita intrapresa con un altro peer\n * !quit --> disconnette il client dal server\n * !show_map --> mostra la mappa di gioco\n * !hit num_cell --> marca la casella num_cell (valido solo quando e' il proprio turno)\n\n\n\n");
	
}



void quit()
{
	int ret;
	char mex='q';   //mando l'uscita	
	ret=send(sk,(void*)&mex,sizeof(char),0);
	if (ret ==-1)
			{
				printf ("errore nel mandare comando e' il mex d'uscita");
				exit(1);
			}
			if (shell=='#')//mando la fine della partita all'avversario
			{				
			ret = sendto(sudp, (void *)&mex, sizeof(char), 0, (struct sockaddr *)&client_addr, (socklen_t)sizeof(client_addr));
			if(ret==-1) 
			{
				printf("mexserver errore: errore in mandare quit all'avversario!\n");
				exit(1);
			}
			}
if (close(sk)==-1) //chiudo tcp
	{
		printf("errore chiusura");
		exit(1);
	}
if (close(sudp)!=0)  //chiudo udp
	{
	printf("errore chiusura");
	exit(1);
	}
printf("ti sei disconesso con successo\n");
exit(0);
}





void stampaclienti()
{
	int i,ret,conta,len;
	char nome[25];
	char mex='w';
	printf("Clienti connessi al server: \n");
	ret=send(sk,(void*)&mex,sizeof(char),0);
	if (ret ==-1)
		{
			printf ("errore nel mandare 'w' al server \n");
			exit(1);
		}	
	ret=recv(sk,(void*)&conta,sizeof(int),0);  //numero di utenti collegati
		if (ret ==-1)
			{
				printf ("errore nel ricevere quanti sono gli utenti \n");
				exit(1);
			}
		for (i=0;i<conta;i++)
		{	
		ret=recv(sk,(void*)&len,sizeof(int),0);	 //lunghezza nome
		if (ret ==-1)
			{printf ("errore nel ricevere lunghezza nome \n");
				exit(1);
			}
			ret=recv(sk,(void*)nome,len,0);     //nome
			if (ret ==-1)
			{
				printf ("errore nel ricevere nome \n");
				exit(1);
			}
			nome[len]='\0'; 
			printf("nome: ");
			printf("%s",nome);
			if (strcmp(nome,nomeutente)==0) //sono io
				printf("*");		
			ret=recv(sk,(void*)&mex,sizeof(char),0);
		if (ret ==-1)
			{
				printf ("errore nel ricevere stato \n");
				exit(1);
			}
			if (mex=='>')
				printf(" (libero)\n");
			else
				printf(" (occupato)\n");
		}			
}






void connectserver(char* ip, char* porta)
{
	int ret;
	sk = socket(AF_INET, SOCK_STREAM, 0);		//afinet =ipv4  stream e' TCP, 0 protoccolo
	if (sk==-1) 
	{
		printf("errore nel creare socket\n");
		exit(1);
	}

	memset(&srv_addr, 0, sizeof(srv_addr));     //azzero la struttura
	srv_addr.sin_family=AF_INET;					// famiglia ipv4
	srv_addr.sin_port = htons(atoi(porta));			//porta
	ret = inet_pton(AF_INET, ip, &srv_addr.sin_addr); // per formato di rete
	
	if (ret==-1) 
	{
		printf("errore formato di rete\n");
		exit(1);
	}
	
	
	ret = connect(sk,(struct sockaddr *)&srv_addr, sizeof(srv_addr));  // mi collego con il server
		
	if (ret==-1)
		{
			printf("errore connessione\n");
			exit(1);
		}
	else
		printf("connessione riuscita\n");
return;
}

void riceviudpIP()
{ int ret;
	//ricevo dal server la porta dell'avversario
	ret = recv(sk, (void *)&avversario.porta_udp_avversario, sizeof(avversario.porta_udp_avversario), 0);
	if(ret==-1) {
		printf("errore nel ricevere la porta su cui e' in ascolto l'avversario!\n");
		exit(1);
	}
	//ricevo dal server l'IP dell'avversario
	ret = recv(sk, (void *)&avversario.IPavversario, sizeof(avversario.IPavversario), 0);
	if(ret==-1) 
	{
		printf("errore nel ricevere l'indirizzo dell'avversario!\n");
		exit(1);
	}
	
	//inizializzo parametri avversario
	memset(&client_addr, 0, sizeof(client_addr));
	client_addr.sin_family = AF_INET;
	client_addr.sin_port =htons( avversario.porta_udp_avversario);
	client_addr.sin_addr.s_addr = avversario.IPavversario;
	//aggiorno timer
	tempo.tv_sec = 60;
	tempo.tv_usec = 0;
	// inizializzo griglia
	}
	
	
	
	
void inizializzopartita()
{
	int i;
	for (i=0;i<9;i++)
		griglia[i]='-';
	conta=0;
	if (simbolo=='X')
	{
		turno='i';
		printf("il tuo simbolo e': X\ne' il tuo turno: \n");
	}
	else
	{
		turno='a';
		printf("il tuo simbolo e': 0 \ne' il turno dell'avversario.\n");
	}
	tempo.tv_sec=60;
	tempo.tv_usec=0;
}




void connetti()
{
	char s[30];
	char c='c';
	int ret,quanti;
	scanf("%s",s); //leggo il nome
	if (strcmp(s,nomeutente)==0)
		printf("come dovresti giocare con te stesso??\n prova con un altro nome!!\n");
	else
	{
		ret = send(sk, (void *)&c, sizeof(char), 0);  //mando il comando per il server
		if(ret==-1)
		{
			printf("connetti errore: errore in mandare comando al server!\n");
			exit(1);
		}
		quanti=strlen(s)+1;
		ret = send(sk, (void *)&quanti, sizeof(int), 0);  //mando quanti
		if(ret==-1) 
		{
			printf("connetti errore: errore in mandare quanti al server!\n");
			exit(1);
		}
		ret = send(sk, (void *)s, quanti, 0);  //gia incrementato quanti
		if(ret==-1) {
			printf("connetti errore: errore in mandare username al server!\n");
			exit(1);
			}
		ret = recv(sk, (void *)&c, sizeof(char), 0);  //leggo l'esito
		if(ret==-1) 
			{
			printf("connetti errore: errore in riceve esito dal server!\n");
			exit(1);
			}
		if(c=='Y')
		{		
			printf("ha accettato la tua richiesta!!!\n");		
			simbolo='X';
			avversario.simbolo='O';
			shell='#';
			riceviudpIP(); //SALVO UDP E IP DEL AVVERSARIO
			inizializzopartita();
		}
		else // RICHIESTA RIFIUTATA
		{
			ret = recv(sk, (void *)&c, sizeof(char), 0);  //leggo l'esito
			if(ret==-1) {
				printf("connetti errore: errore in riceve esito dal server!\n");
				exit(1);
				}
			if (c=='1')
				printf("Impossibile connettersi : giocatore occupato\n");
			else
				{
					if (c=='2')
						printf("Impossibile connettersi : giocatore non esiste");
					else
						printf("Impossibile connettersi : giocatore ha rifiutato la partita");
		}
				
	}
}		
}


void disconnetti()
{
	int ret;
	char s='l';		//il server andraÌ a fine partita
	ret = sendto(sudp, (void *)&s, sizeof(char), 0, (struct sockaddr *)&client_addr, (socklen_t)sizeof(client_addr));
	if(ret==-1) 
			{
				printf("mexserver errore: errore in mandare prova dal server!\n");
				exit(1);
			}
	ret = send(sk, (void *)&s, sizeof(char), 0); //mando al server che siamo dinuovo disponibili 
	if(ret==-1)
		{
			printf("mexserver errore: errore in mandare risultato al server!\n");
			exit(1);
		}
	shell='>';
	printf("ti sei disconesso con successo: TI SEI ARRESO");
}




void showmap()
{
int i,j;
printf("ecco la situazione della partita\n");
int x=8;
	for (i=0;i<3;i++)
	{
		for (j=2;j>=0;j--)
		{
			printf("%c",griglia[x-j]);
			
		}
		x-=3;
		printf("\n");
	}
}




char controllovincitore()
{
	conta++;
	if(griglia[0]==griglia[3] && griglia[3]==griglia[6] && griglia[0]!='-')
		return griglia[0]; //prima colonna
	if(griglia[1]==griglia[4] && griglia[4]==griglia[7] && griglia[1]!='-') 
		return griglia[1]; //seconda colonna
	if(griglia[2]==griglia[5] && griglia[5]==griglia[8] && griglia[2]!='-') 
		return griglia[2]; //terza colonna
	if(griglia[6]==griglia[7] && griglia[7]==griglia[8] && griglia[6]!='-') 
		return griglia[6]; //prima riga
	if(griglia[3]==griglia[4] && griglia[4]==griglia[5] && griglia[3]!='-') 
		return griglia[3]; //seconda riga
	if(griglia[0]==griglia[1] && griglia[1]==griglia[2] && griglia[0]!='-')
		return griglia[0]; //terza riga
	if(griglia[0]==griglia[4] && griglia[4]==griglia[8] && griglia[0]!='-')
		return griglia[0]; //diagonale
	if(griglia[2]==griglia[4] && griglia[4]==griglia[6] && griglia[2]!='-') 
		return griglia[2]; // diagonale
	if (conta==9)//non ci sono piu' caselle 
	return 'P';
	return 'N';

}





void hit()
{ 
int ret;
int posizione;
char vincitore;
if (turno=='i')//turno mio
	{
		char s;
		char conferma='F';
		scanf("%d",&posizione);
		if (posizione>0 && posizione <=9)
			{	posizione--;
				if (griglia[posizione]=='-')
						{
							griglia[posizione]=simbolo;
							conferma='T';
						}
						else
							printf("posto gia' accupato\n");
						}		
		else 
						printf("devi mettere un numero da 1 a 9\n");
		if (conferma=='T')
			// devo mandare H e posizione udp
		{
			s='H';
			ret = sendto(sudp, (void *)&s, sizeof(char), 0, (struct sockaddr *)&client_addr, (socklen_t)sizeof(client_addr));
			if(ret==-1) 
			{
				printf("mexserver errore: errore in mandare prova dal server!\n");
				exit(1);
			}
			ret = sendto(sudp, (void *)&posizione, sizeof(int), 0, (struct sockaddr *)&client_addr, (socklen_t)sizeof(client_addr));
			if(ret==-1) 
			{
				printf("mexserver errore: errore in mandare prova dal server!\n");
				exit(1);
			}
			shell='#';
			turno='a'; 
			vincitore=controllovincitore();
			if (vincitore=='N')
				printf("turno dell'avversario\n");
			else   //finisce la partita
				{
					s='l';
					if(vincitore==simbolo)
						printf("complimenti hai vinto!!!!");
					else 
					{
						if(vincitore=='P')
							printf("hai pareggiato!!");
						else		
							printf("HAI PERSO!!!!!");
					}
					shell='>';
					ret = send(sk, (void *)&s, sizeof(char), 0); //mando al server che siamo dinuovo disponibili 
					if(ret==-1)
					{
						printf("mexserver errore: errore in mandare risultato al server!\n");
						exit(1);
					}
				}
			}
		}
	else //i!=turno
		printf("non e' il tuo turno!!\n");
}





void mexcliente()
{
	int ret;
	char cmd;
	int addrlen = sizeof(client_addr);
	int posizione;
	char vincitore;
	ret = recvfrom(sudp, (void *)&cmd, sizeof(char),  0, (struct sockaddr *)&client_addr, (socklen_t *)&addrlen); // SI BLOCCA QUI!!!!!
	if(ret==-1)
	{
		printf("mexserver errore: errore in ricezione prova dal server!\n");
		exit(1);
	}
	//ricevo comando controllo 
	if (ret==0)//si e' scollegato
	{
		shell='>';
		printf("l'avversario si e' ritirato!!! HAI VINTO\n");//non faccio nient'altro perche il server aggiornera i miei campi
	}
	else
	{
		if(cmd=='H') //posizione
		{			
			ret = recvfrom(sudp, (void *)&posizione, sizeof(int),  0, (struct sockaddr *)&client_addr, (socklen_t *)&addrlen); // SI BLOCCA QUI!!!!!
			if(ret==-1) 
			{
				printf("mexserver errore: errore in ricezione prova dal server!\n");
				exit(1);
			}
			//altra recv contente posizione
			if (simbolo=='X')
				griglia[posizione]='O';
			else
				griglia[posizione]='X';
			vincitore=controllovincitore();
			if(vincitore=='N') //non e' finita
			{
				turno='i';
				printf("e' il tuo turno :\n");
			}
			else  //e' finita
			{
				if(vincitore==simbolo)
					printf("complimenti hai vinto!!!!");
				else 
				{
					if(vincitore=='P')
						printf("hai pareggiato!!");
					else		
						printf("HAI PERSO!!!!!");
				}
				shell='>';				
			}
		}
	else 
	{
		if(cmd=='l')
		{
			shell='>';
			printf("l'avversario si e' ritirato. Hai vinto!");
		}
	}
}
	
}









void mexserver() //mando richiesta
{
	char s;
	int ret,quanti;
	ret = recv(sk, (void *)&s, sizeof(char), 0);
	if(ret==-1) {
		printf("mexserver errore: errore in ricezione comando dal server!\n");
		exit(1);
	}
	if(ret==0) { //server si e' disconnesso
		printf("Il server ha chiuso la connessione!!\n");
		exit(3);
	}
	if (s=='k')
	{	
		ret = recv(sk, (void *)&quanti, sizeof(int), 0); //ricevo lunghezza nome
		if(ret==-1) {
			printf("mexserver errore: errore in ricezione lunghezza dal server!\n");
			exit(1);
			}
		ret = recv(sk, (void *)&avversario.nomeavversario, quanti, 0);  //gia e' incrementato quanti
		if(ret==-1)
		 {
			printf("mexserver errore: errore in ricezione quanri dal server!\n");
			exit(1);
		}	
		printf(" ti ha chiesto di giocare ");		
		printf("%s\n",avversario.nomeavversario);
		printf("accetti la richiesta?? Y/N\n");
		while(1)
		{
			scanf("%c",&s);
			if (s=='Y' || s=='N')
			{
				ret = send(sk, (void *)&s, sizeof(char), 0); 
				if(ret==-1) 
				{
					printf("mexserver errore: errore in mandare risultato al server!\n");
					exit(1);
				}
			break;
			}
		else
			printf ("devi digitare Y/N\n");
		}
		if(s=='Y')//cliente che ha ricevuto la richiesta
		{ 
			simbolo='O';
			avversario.simbolo='X';
		    riceviudpIP();
		    shell='#';
		    inizializzopartita();
	 	}					
  }
  else
   {
	   if(s=='q')// si e' ritirato
		{
			printf("complimenti hai vinto!!!!");
				shell='>';	
		}
	}

}
	
	
	



void tastiera()

{
int i;
char s[30];
scanf("%s",s);
for (i=0;i<7;i++)
{
	if (strcmp(s,comandi[i])==0)
		{
			break;
		}
	
}
switch(i){
	
	case 0: 
			help();
			break;
	case 1:	
			stampaclienti();
			break;
	case 2:
			if (shell=='>')
				connetti();
			else
				printf("stai gia' giocando non puoi fare altre richiesta\n");
			break;
	case 3: 
			disconnetti();
			break;
	case 4: 
			quit();
			break;
	case 5: 
			if (shell=='>')
				printf ("non stai giocando");
			else
				showmap();
			break;
	case 6: 
			hit();
			break;
	default:
			printf("comando non riconosciuto/n");
	}		
} 
int main(int quantiparametri,char* arg[])
{
	int i;
	char conferma;
	struct timeval* t;
	int scaduto;  //per il timer
	char udp [7];	
	int ret;
	int quanti;
	shell='>';
	if (quantiparametri != 3)
		{
		 printf ("devi passare 2 parametri : ip e porta \n");
		exit(1);
		}		
	connectserver(arg[1],arg[2]);
	help();
	for(;;)
	{
			printf("dimmi il tuo username \n");
			printf(">");
			scanf("%s",nomeutente);
			quanti=strlen(nomeutente)+1;
			ret=send(sk,(void*)&quanti,sizeof(int),0);  //gli mando prima quanto e' grande l'utente
			if (ret ==-1)
				{
					printf ("errore send quanti");
					exit(1);
				}
			ret=send(sk,(void*)nomeutente,quanti,0); //mando pure \0 perche quanti++
			if (ret==-1 || ret < strlen(nomeutente))
			{	printf ("errore send username");
				exit(1);
			}	
			ret=recv(sk,(void*)&conferma,sizeof(char),0);
			if (ret ==-1)
				{
				printf ("errore ricevere conferma");
				exit(1);
				}
			if (conferma=='Y')
					break;
			else 
				printf("nome utente gia usato!\n");
				
	}
	for(;;)
	{
		printf("dimmi la tua porta udp \n>");
		scanf("%s",udp);				  
		portaudpu=atoi(udp);
			if (portaudpu>65535 || portaudpu<1024)
				printf("errore porta \n");
			else
				break;			
	}
	printf(">");
	ret=send(sk,(void*)&portaudpu,sizeof (int),0);	
	if (ret==-1 || ret < sizeof (int))
			{
				printf ("errore send porta udp");
				exit(1);
			}
	//UDP
	if (( sudp= socket(AF_INET, SOCK_DGRAM, 0)) == -1) 
		{
			printf("errore socket udp");
			exit(1);
		}
	memset(&my_addr, 0, sizeof(my_addr)); //azzera la struttura
	my_addr.sin_family = AF_INET;  //ipv4
	my_addr.sin_port = htons(portaudpu); //formato di rete
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY); //server si mette in ascolto su una qualsiasi delle sue interfacce di rete
	if (ret==-1) 
	{
		printf("errore formato di rete\n");
		exit(1);
	}
	ret = bind(sudp, (struct sockaddr*) &my_addr, sizeof(my_addr));	//Collega un indirizzo locale al socket creato con la socket
	if (ret==-1) 
	{
		printf("errore bind\n");
		exit(1);
	}
	FD_ZERO(&master);	//azzero master
	FD_ZERO(&read_fd);
	FD_SET(sk,&master);   //setto server
	FD_SET(sudp,&master);  //setto udp
	FD_SET(0,&master);		//setto 0 per tastiera
	if (sudp>sk)
		maxfd=sudp;
	else
		maxfd=sk;
	tempo.tv_sec=60; //secondi
	tempo.tv_usec=0; //microsecondi
	
	for (;;)

	{
		
		t=&tempo;
		if(shell=='#') //non faccio partire il timer
			t=NULL;
		read_fd=master;  //perche la select mi 'danegerebbe' master
		fflush(stdout);
		scaduto=select(maxfd+1, &read_fd, NULL, NULL, t);
		if (scaduto==-1)
		{
		printf("errrore select");
		exit(1);
		}
	if (scaduto==0)  //tempo scaduto
		{	
			printf("scaduto");
			quit();
			exit(1);
		}
	for (i=0; i<=maxfd;i++)
	{	
		if(FD_ISSET(i, &read_fd))
		{
			if (i==0) //tastiera
			{
				tastiera();	
				tempo.tv_sec=60;
				tempo.tv_usec=0;	
			}
			if (i==sk) //mex dal server		
				mexserver();
			if (i==sudp) //mex cliente
			{
				mexcliente();
				tempo.tv_sec=60;
				tempo.tv_usec=0;
			}
			printf("\n%c",shell);
		}
	}
}
return 0;
}
