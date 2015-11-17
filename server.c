#define MAX_CONNESSIONI 10
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
struct sockaddr_in my_addr, cl_addr;    // socket server e socket client
int sk, cn_sk; // sk per ascolto

struct clienti
{
	char* nomecliente;
	unsigned short portaudpu;
	unsigned long indirizzo; //ip
	struct clienti* next;  
	int sock;
	char primaistruzione; //serve per le receive consecutive nel creare l'account
	int quanti;
	char vogliogiocare;  //serve per le receive consecutive nel creare la partita
	char stato;				//libero o occupato
	int lunghezzaAvversario;
	struct clienti* richiedente; //struttura di quello che fa la domanda
};
struct clienti* listaclienti;
fd_set 	master,				//i descittori
        read_fd;			 // fd di appoggio per la select
        int		maxfd;



void quit(struct clienti* elemento )
{
	struct clienti* supporto=listaclienti;
	char conta='q';
	int ret;
	if (elemento->stato=='#')		//stava giocando e si e' disconesso
	{ 
		elemento->richiedente->stato='>';
		printf("%s si e' disconesso\n", elemento->nomecliente);
		printf("%s e' libero\n", elemento->richiedente->nomecliente);
		ret=send(elemento->richiedente->sock,(void*)&conta,sizeof(int),0); 
		if (ret ==-1)
			{
				printf ("errore  mandare q al cliente 2");
				exit(1);
			}
	}
		if (elemento==listaclienti)		//estraggo da lista
			listaclienti=elemento->next;
		else
			{
			while (supporto->next!=elemento)
				supporto=supporto->next;			
			supporto->next=elemento->next; //supporto e' il precedente di elemento
			}
		if (close(elemento->sock)==0)
				printf("chiusura riuscita\n");
		else
			{
				printf("errore chiusura");
				exit(1);
			}	
FD_CLR(elemento->sock, &master);
free(elemento->nomecliente); //lo tolgo da select
free(elemento);
}

void connessione(char* ip, char* porta)
{
int ret;
sk = socket(AF_INET, SOCK_STREAM, 0);   //tcp
if (sk==-1) 
	{
		printf("errore nel creare socket");
		exit(1);
	}

memset(&my_addr, 0, sizeof(my_addr));  	//azzero la struttura
my_addr.sin_family = AF_INET;  			// famiglia ipv4
my_addr.sin_addr.s_addr = htonl(INADDR_ANY); // formato di rete
my_addr.sin_port = htons(atoi(porta));				//porta
ret = bind(sk, (struct sockaddr*) &my_addr, sizeof(my_addr));	//Collega un indirizzo locale al socket creato con la socket
if(ret==-1)
	{
		printf("errore bind");
		exit(1);
	}
ret = listen(sk, MAX_CONNESSIONI);    			// in attesa di richieste
if(ret==-1)
	{
	printf("errore listen");
	exit(1);
	}
}


void finepartita(struct clienti* elemento)
{
	elemento->stato='>';
	elemento->richiedente->stato='>';
	printf("%s e' libero\n", elemento->nomecliente);
	printf("%s e' libero\n", elemento->richiedente->nomecliente);
	elemento->richiedente=NULL;
}


void stampaclienti(int gestionesocket)
{	
	struct clienti* supporto=listaclienti;
	int i,conta,ret,x;
	conta=0;	
	while (supporto!=0)
		{		
				conta++;
				supporto=supporto->next;
		}		
	ret=send(gestionesocket,(void*)&conta,sizeof(int),0);  //mando quandi sono gli utenti collegati
	if (ret ==-1)
		{
			printf ("errore send conta clienti");
			exit(1);
		}
	supporto=listaclienti;
	for (i=0;i<conta;i++)
	{
		x=strlen(supporto->nomecliente);
		ret=send(gestionesocket,(void*)&x,sizeof(int),0);  //mando quanto e'  lungo il nome dell'utente
		if (ret ==-1)
			{
			printf ("errore nel mandare la lunghezza del  nome cliente\n");
			exit(1);
			}
		printf(supporto->nomecliente);
		ret=send(gestionesocket,(void*)supporto->nomecliente,x,0); //mando il nome
		if (ret ==-1)
		{
			printf ("errore send nome cliente\n");
			exit(1);
		}
		ret=send(gestionesocket,(void*)&supporto->stato,sizeof(char),0);  //mando se l'utente e' libero o occupato
		if (ret ==-1)
			{
				printf ("errore send nome clienti\n");
				exit(1);
			}
		supporto=supporto->next;
	}	
}



void aggiungicliente1(struct clienti* nuovocliente)   //primo stadio per aggiungere un cliente
{
	int ret;	
	ret=recv(nuovocliente->sock,(void*)&nuovocliente->quanti,sizeof(int),0);  // ricevo lunghezza username

	if(ret ==-1)
	{
		printf("errore ricevuti quanti");
		exit(1);
	}
	
	nuovocliente->primaistruzione='1';
		if (ret ==0)
			quit(nuovocliente);
		
}	
	

void aggiungicliente2(struct clienti* nuovocliente) //ricevo username
{
	
int ret;
struct clienti* supporto=listaclienti;
char conferma;
nuovocliente-> nomecliente= malloc(nuovocliente->quanti+1);   
ret=recv(nuovocliente->sock,(void*)nuovocliente-> nomecliente,nuovocliente->quanti,0);  // ricevo username gia' quanti al +1
if(ret==-1 || ret < nuovocliente->quanti)
	{
		printf("errore ricevuti username");
		exit(1);
	}
conferma='Y';  
while (supporto!=0 )
	{
			if(supporto->primaistruzione=='3') //mi serve per non controllare quelli che non sono ancora completamente registrati
			{
				if(strcmp(nuovocliente-> nomecliente,supporto->nomecliente)==0 )
					{
						if(strcmp(nuovocliente-> nomecliente,supporto->nomecliente)==0 && nuovocliente->sock!=supporto->sock)
						{
							conferma='N';
							printf("nome gia' esistente conferma\n");
							nuovocliente->primaistruzione='0';  	//deve ricominciare'
							free(nuovocliente-> nomecliente);
							ret=send(cn_sk,(void*)&conferma, sizeof(char),0);
							if(ret==-1)
							{
								printf("errore nel mandare dati\n");
								exit(1);
							}
							break;
						}
					}
			}
			supporto=supporto->next;
	}
if(conferma=='Y' || supporto==0)    //e' un nuovo nome
{	
	conferma='Y';
	ret=send(nuovocliente->sock,(void*)&conferma, sizeof(char),0);
	nuovocliente->primaistruzione='2';			 
}
		
}


void aggiungicliente3(struct clienti* nuovocliente) //ricevo udp
{	int ret;
	
	ret=recv(nuovocliente->sock,(void*)&nuovocliente->portaudpu, sizeof(int),0);  // ricevo  porta udp
	
	nuovocliente->primaistruzione='3';	
	if(ret==-1)
	{
		printf("errore ricevuti udp\n");
		exit(1);
	}
	printf("si e' connesso : %s\n",nuovocliente->nomecliente);
	printf("%s %s\n",nuovocliente->nomecliente, " e' libero");
	if (ret ==0)
		quit(nuovocliente);
			
		
}

void connetti(struct clienti* elemento)
{
	
	int ret;
	char c;
	char richiesta;
	char* nome;
	char risposta='0';//risposta 0 ok 1 occupato 2 non esiste 3 attesa risposta
	if (elemento->vogliogiocare=='0') //prima volta
		{
			if (elemento->stato=='#') //gia sta giocando
			{				risposta='1';
							ret=send(elemento->sock,(void*)&risposta,sizeof(char),0);  // mando esisto negativo
								if(ret ==-1)
								{
									printf("errore mandare esito negativo  connetti\n");
									exit(1);
								}	
				
			}
			else
			elemento->vogliogiocare='1';
		}
	else
		{
			if (elemento->vogliogiocare=='1') //ricevo quanti
				{
					ret=recv(elemento->sock,(void*)&elemento->lunghezzaAvversario,sizeof(int),0);  //posso usare lo stesso quanti de quello del nuovo cliente
					if(ret ==-1)
					{
						printf("errore ricevuti quanti\\");
						exit(1);
					}
					elemento->vogliogiocare='2';
				}
			else //sono nel caso del username ...... voglio giocare=2
				{
					if (elemento->vogliogiocare=='2')
					{
						nome= malloc(elemento->lunghezzaAvversario+1);   
						struct clienti* supporto=listaclienti;
						ret=recv(elemento->sock,(void*)nome,elemento->lunghezzaAvversario,0);  // ricevo username
						if(ret ==-1)
						{
							printf("errore ricevuti username della connetti\n");
							exit(1);
						}
						while (supporto!=0) //cerco l'utente
						{
							if(strcmp(supporto->nomecliente,nome)==0) //trovato
							{
								if(supporto->stato=='#') //sta giocando
									risposta='1';//occupato
								else
									risposta='3';//attesa risposta
								break;
							}
							supporto=supporto->next;
						}
						if (supporto==0)// non c'e' nessuno con quel nome
							risposta='2';			
						if(risposta=='1' || risposta=='2') //occupato o non esiste;
							{	
								c='k';
								elemento->vogliogiocare='0';
								ret=send(elemento->sock,(void*)&c,sizeof(char),0);  // mando esisto negativo
								if(ret ==-1)
								{
									printf("errore mandare esito negativo  connetti\n");
									exit(1);
								}
								ret=send(elemento->sock,(void*)&risposta,sizeof(char),0);  // mando esisto negativo
								if(ret ==-1)
								{
									printf("errore mandare esito negativo  connetti\n");
									exit(1);
								}
							}
						else //mando richiesta al cliente 2
						{	
							richiesta='k';						//comando per far capire al client della richiesta
							supporto->richiedente=elemento;		//supporto e' cliente 2
							elemento->richiedente=supporto;
							ret=send(supporto->sock,(void*)&richiesta,sizeof(char),0);  // mandi richiesta al cliente 2
							if(ret ==-1)
								{
									printf("errore mandare richiesta connetti\n");
									exit(1);
								}
							ret=send(supporto->sock,(void*)&supporto->richiedente->quanti,sizeof(int),0); //mando la lunghezze del nome del cliente1 al cliente 2 
							if(ret ==-1)
								{
									printf("errore mandare quantita della connetti\n");
									exit(1);
								}
							ret=send(supporto->sock,(void*)supporto->richiedente->nomecliente,supporto->richiedente->quanti,0); //mando il nome del cliente1 al cliente 2 
							if(ret ==-1)
								{
									printf("errore mandare username della connetti\n");
									exit(1);
								}
							elemento->vogliogiocare='a';//stato di attesa del cliente 2
							supporto->vogliogiocare='3'; //cosi il cliente 2 quando risponde finisce nel caso 3
																
						}
						}
						else //caso 3 cioe' mi ha scritto il cliente 2
						{   
							
							elemento->vogliogiocare='0';  //cliente 2  resetto
							elemento->richiedente->vogliogiocare='0'; //cliente 1  resetto
							ret=recv(elemento->sock,(void*)&richiesta,sizeof(char),0);  // ricevo Y o N dal cliente 2
							if(ret ==-1)
								{
								printf("errore ricevuti risposta cliente2 connetti\n");
								exit(1);
								}
							if (richiesta=='Y') //DEVO COMINCIARE LA PARTITA
							{ 
								elemento->richiedente->richiedente=elemento; //metto cliente 2 nella struttura di cliente 1;
								elemento->richiedente->stato='#'; //cliente1
								elemento->stato='#'	;			//cliente2
								ret=send(elemento->richiedente->sock,(void*)&richiesta,sizeof(char),0);   // mandi udp del cliente 2 al cliente 1
								if(ret ==-1)
								{
									printf("errore mandare send udp della connetti cliente richiedente \n");
									exit(1);
								}		
								//mando a entrambi i client la porta udp e ip
								ret=send(elemento->richiedente->sock,(void*)&elemento->portaudpu,sizeof(elemento->portaudpu),0);   // mandi udp del cliente 2 al cliente 1
								if(ret ==-1)
								{
									printf("errore mandare send udp dela connetti cliente richiedente \n");
									exit(1);
								}		
								ret=send(elemento->richiedente->sock,(void*)&elemento->indirizzo,sizeof(elemento->indirizzo),0);   // mandi indirizzo del cliente 2 al cliente 1
								if(ret ==-1)
								{
									printf("errore mandare send udp dela connetti cliente richiedente \n");
									exit(1);
								}							
								ret=send(elemento->sock,(void*)&elemento->richiedente->portaudpu,sizeof(elemento->richiedente->portaudpu),0);     // mandi udp del cliente 1 al cliente 2
							
								if(ret ==-1)
								{
									printf("errore mandare send udp dela connetti cliente invitato\n");
									exit(1);
								}
								ret=send(elemento->sock,(void*)&elemento->richiedente->indirizzo,sizeof(elemento->richiedente->indirizzo),0);     // mandi indirizzo del cliente 1 al cliente 2
								if(ret ==-1)
								{
									printf("errore mandare send indirizzo dela connetti cliente invitato\n");
									exit(1);
								}
								printf("%s si e' connesso a %s\n",  elemento->richiedente->nomecliente,elemento->nomecliente);
								
							}
							else   //ha detto no
							{							
								ret=send(elemento->richiedente->sock,(void*)&richiesta,sizeof(char),0);  // mando N al cliente 1
								if(ret ==-1)
								{
									printf("errore mandare richiesta  della connetti\n");
									exit(1);
								}
								richiesta='3'; //cosi riconosco la causa, cioe' ha detto no
								ret=send(elemento->richiedente->sock,(void*)&richiesta,sizeof(char),0);  // mando giocatore ha detto no al cliente 1
								if(ret ==-1)
								{
									printf("errore mandare richiesta  della connetti\n");
									exit(1);
								}
								
							}
							
						}
				}
			}
}





void gestioneclienti(struct clienti* elemento)
{
	int ret;
	char msg;		
	ret=recv(elemento->sock,(void*)&msg, sizeof(char),0);
	if(ret==-1)
		{
			printf("errore nel ricevere dati\n");
			exit(1);
		}
	if (ret==0)
			quit(elemento);
	else
	{
		switch(msg){
			case 'q':	quit(elemento);
						break;
			case 'c':   
						connetti(elemento);
						break;
			case 'w':
						stampaclienti(elemento->sock);
						break;
			case 'l':	finepartita(elemento);
						break;
			default: 	
						
						printf("comando non riconosciuto");
						exit(1);
				}	
	}
}



int main(int quantiparametri,char* arg[])
{
int len;
int i;
unsigned int udp;
FD_ZERO(&master);	//azzero fd
FD_ZERO(&read_fd); 
if (quantiparametri != 3)
		{
			printf ("devi passare 2 parametri : ip e porta\n");
			exit(1);
		}
udp=atoi(arg[2]);
if (udp>65535 || udp<1024)
	{
		printf("errore porta\n");
		exit(1);
	}		
connessione(arg[1],arg[2]);
FD_SET(sk,&master); // setto il socket che voglio controllare
maxfd=sk; // per ora e' il massimo
for(;;)
{	
	read_fd=master;  //perche la select mi 'danegerebbe' master
	if(select(maxfd+1, &read_fd, NULL, NULL, NULL) == -1)
	{ 
		printf("errore nella select!");
		exit(1);
	}
		for ( i=0; i<=maxfd;i++)
		{	
			if(FD_ISSET(i, &read_fd))
			{
				if (i==sk)   // un client si vuole connettere, quindi lo devo accettare
				{	
					len = sizeof(cl_addr);	
					cn_sk = accept(sk, (struct sockaddr*) &cl_addr,(socklen_t *) &len); // richiesta accettata presa dal listening del socket, cn_sk ha la conessione mi servira per send e receive
					if(cn_sk==-1)
					{
						printf("errore di accettazione\n");
						exit(1);
					}
					FD_SET(cn_sk,&master);
					if(cn_sk> maxfd)
						maxfd=cn_sk;
					printf("nuova connessione\n");	
					struct clienti* nuovocliente; //creo il nuovo utente
					nuovocliente= malloc(sizeof (struct clienti));
					int length = sizeof(cl_addr);
					memset(&cl_addr, 0, length);
					getpeername(cn_sk, (struct sockaddr *)&cl_addr, (socklen_t *)&length); //trovo l'indirizzo del client che si e' connesso
					nuovocliente->primaistruzione='1'; //vari salti per la creazione, perche non posso fare due receive di fila  
					nuovocliente->sock=cn_sk;
					nuovocliente->next=0;
					nuovocliente->stato='>'; //non sta giocando ancora
					nuovocliente->vogliogiocare='0';
					nuovocliente->indirizzo = cl_addr.sin_addr.s_addr; //indirizzo 
					nuovocliente->primaistruzione='0';
					struct clienti* appoggio=listaclienti; //metto nella lista il nuovo utente
					if (listaclienti==0) // nessun cliente
						listaclienti=nuovocliente;
					else
					{
						appoggio=listaclienti;
						while(appoggio->next!=0)
							appoggio=appoggio->next; //scorro la lista, inserimento in fondo
						appoggio->next=nuovocliente;
					}					
				}
				else
				{
					struct clienti* elemento=listaclienti; 
					while(elemento!=0)
					{
					if (elemento->sock==i)    //elemento trovato
						break;
					elemento=elemento->next;
					}			
					switch (elemento->primaistruzione){
					case '0':
								aggiungicliente1(elemento);      //quantita
								break;
					case '1':
								aggiungicliente2(elemento);		//username
								break;
					case '2':
								aggiungicliente3(elemento);		//udp e finalmente creato l'utente
								break;
					default:
								if(elemento->vogliogiocare!='0') //gestione connetti()	 varie receive di fila
									connetti(elemento);
								else
									gestioneclienti(elemento);
				}
				
			}
		}	
	}
}
return 0;
}

