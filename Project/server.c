/* servTCPCSel.c - Exemplu de server TCP concurent

   Asteapta un "nume" de la clienti multipli si intoarce clientilor sirul
   "Hello nume" corespunzator; multiplexarea intrarilor se realizeaza cu select().

   Cod sursa preluat din [Retele de Calculatoare,S.Buraga & G.Ciobanu, 2003] si modificat de
   Lenuta Alboaie  <adria@infoiasi.ro> (c)2009

*/

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>

#include <time.h>

/* portul folosit */

#define PORT 2728

extern int errno;		/* eroarea returnata de unele apeluri */
char first_msg[100];
int all_msg_eq;

/* functie de convertire a adresei IP a clientului in sir de caractere */
char * conv_addr (struct sockaddr_in address)
{
  static char str[25];
  char port[7];

  /* adresa IP a clientului */
  strcpy (str, inet_ntoa (address.sin_addr));
  /* portul utilizat de client */
  bzero (port, 7);
  sprintf (port, ":%d", ntohs (address.sin_port));
  strcat (str, port);
  return (str);
}

/* realizeaza primirea si retrimiterea unui mesaj unui client */
void sayHello(int fd)
{
  char buffer[100];		/* mesajul */
  int bytes;			/* numarul de octeti cititi/scrisi */
  char msg[100];		//mesajul primit de la client
  char msgrasp[100]=" ";        //mesaj de raspuns pentru client

  bytes = read (fd, msg, sizeof (buffer));
  if (bytes < 0)
    {
      perror ("Eroare la read() de la client.\n");
      return;
    }
  printf ("[server]Mesajul a fost receptionat...%s\n", msg);

  if(strcmp(first_msg, "") == 0)
  {
    strcpy(first_msg, msg);
  }
  else
  {
    if (strcmp(first_msg, msg) !=0)
    {
      all_msg_eq=0;
    }
  }

  return;
}

void send_messages(int fd)
{
  char msgrasp[100]=" ";        //mesaj de raspuns pentru client
  /*pregatim mesajul de raspuns */
  bzero(msgrasp,100);
  if(all_msg_eq==1)
  {
    strcat(msgrasp,"continua");
  }
  else
  {
    strcat(msgrasp,"gata");
  }


  printf("[server]Trimitem mesajul inapoi...%s\n",msgrasp);

  if (write (fd, msgrasp, sizeof(msgrasp)) < 0)
    {
      perror ("[server] Eroare la write() catre client.\n");
      return;
    }

  return;
}


/* programul */
int
main ()
{
  struct sockaddr_in server;	/* structurile pentru server si clienti */
  struct sockaddr_in from;
  fd_set readfds;		/* multimea descriptorilor de citire */
  fd_set actfds;		/* multimea descriptorilor activi */
  struct timeval tv;		/* structura de timp pentru select() */
  int sd, client;		/* descriptori de socket */
  int optval=1; 			/* optiune folosita pentru setsockopt()*/
  int fd;			/* descriptor folosit pentru
				   parcurgerea listelor de descriptori */
  int nfds;			/* numarul maxim de descriptori */
  int len;			/* lungimea structurii sockaddr_in */

  time_t init_time;

  /* creare socket */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[server] Eroare la socket().\n");
      return errno;
    }

  /*setam pentru socket optiunea SO_REUSEADDR */
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR,&optval,sizeof(optval));

  /* pregatim structurile de date */
  bzero (&server, sizeof (server));

  /* umplem structura folosita de server */
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl (INADDR_ANY);
  server.sin_port = htons (PORT);

  /* atasam socketul */
  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[server] Eroare la bind().\n");
      return errno;
    }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen (sd, 5) == -1)
    {
      perror ("[server] Eroare la listen().\n");
      return errno;
    }

  /* completam multimea de descriptori de citire */
  FD_ZERO (&actfds);		/* initial, multimea este vida */
  FD_SET (sd, &actfds);		/* includem in multime socketul creat */

  tv.tv_sec = 1;		/* se va astepta un timp de 1 sec. */
  tv.tv_usec = 0;

  /* valoarea maxima a descriptorilor folositi */
  nfds = sd;

  printf ("[server] Asteptam la portul %d...\n", PORT);
  fflush (stdout);

  /* servim in mod concurent clientii... */
  init_time=time(0);
  while (1)
  {
    if(time(0) - init_time > 10 )
    {
      bzero(first_msg,100);
      all_msg_eq=1;


      printf ("[server] Au trecut cele S secunde. Se citesc noile mesaje\n");
      fflush (stdout);

      /* vedem daca e pregatit vreun socket client pentru a trimite raspunsul */
      for (fd = 0; fd <= nfds; fd++)	/* parcurgem multimea de descriptori */
      {
        /* este un socket de citire pregatit? */
        if (fd != sd && FD_ISSET (fd, &readfds))
        {
          sayHello(fd);
      }
    }			/* for */

    for (fd = 0; fd <= nfds; fd++)	/* parcurgem multimea de descriptori */
    {
      /* este un socket de citire pregatit? */
      if (fd != sd && FD_ISSET (fd, &actfds))
      {
        send_messages(fd);
        if (all_msg_eq==0)
        {
          printf ("[server] S-a deconectat clientul cu descriptorul %d.\n",fd);
          fflush (stdout);
          close (fd);		/* inchidem conexiunea cu clientul */
          FD_CLR (fd, &actfds);/* scoatem si din multime */
          FD_CLR (fd, &readfds);

        }
    }
  }			/* for */
  // Se reseteaza timerul
  init_time=time(0);
  }

      /* ajustam multimea descriptorilor activi (efectiv utilizati) */
      bcopy ((char *) &actfds, (char *) &readfds, sizeof (readfds));

      /* apelul select() */
      if (select (nfds+1, &readfds, NULL, NULL, &tv) < 0)
      {
        perror ("[server] Eroare la select().\n");
	       return errno;
	    }
      /* vedem daca e pregatit socketul pentru a-i accepta pe clienti */
      if (FD_ISSET (sd, &readfds))
	{
	  /* pregatirea structurii client */
	  len = sizeof (from);
	  bzero (&from, sizeof (from));

	  /* a venit un client, acceptam conexiunea */
	  client = accept (sd, (struct sockaddr *) &from, &len);

	  /* eroare la acceptarea conexiunii de la un client */
	  if (client < 0)
	    {
	      perror ("[server] Eroare la accept().\n");
	      continue;
	    }

          if (nfds < client) /* ajusteaza valoarea maximului */
            nfds = client;

	  /* includem in lista de descriptori activi si acest socket */
	  FD_SET (client, &actfds);

	  printf("[server] S-a conectat clientul cu descriptorul %d, de la adresa %s.\n",client, conv_addr (from));
	  fflush (stdout);
	}

    }				/* while */
}				/* main */
