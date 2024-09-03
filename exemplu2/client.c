#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>

int server_write, server_read;

int main()
{
    if (access("cl-sv", F_OK) != 0) // daca cl-sv nu exista
    {
        if (mknod("cl-sv", S_IFIFO | 0666, 0) == -1) // il creez
            fprintf(stderr, "eroare la crearea FIFO client_to_server");
    }
    server_write = open("cl-sv", O_WRONLY); // clientu trimite msg la server
    if (server_write == -1)
    {
        fprintf(stderr, "eroare la deschiderea fisier_to_server in client");
        exit(1);
    }

    if (access("sv-cl", F_OK) != 0) // daca sv-cl nu exista
    {
        if (mknod("sv-cl", S_IFIFO | 0666, 0) == -1) // il creez
            fprintf(stderr, "eroare la crearea FIFO server_to_client");
    }
    server_read = open("sv-cl", O_RDONLY); // clientul primeste mesaje de la server
    if (server_read == -1)
    {
        fprintf(stderr, "eroare la crearea server_to_client in client");
        exit(3);
    }
    char comanda[100];
    comanda[0]='\0';
    int lg_comanda = 0;
    while (1)
    {
    printf("Introduceti comanda: ");
    fgets(comanda, sizeof(comanda), stdin);

    if ( write (server_write, comanda, sizeof(comanda)) == -1 ) 
    {
        fprintf(stderr, "eroare la scrierea mesajului catre server");
        exit (2);
    }

    if ( read (server_read, comanda, sizeof(comanda)) == -1 )
    {
        fprintf ( stderr, "eroare la citirea mesajului primit de la server");
        exit (4);
    }
    
        

    }
    return 0;
}
