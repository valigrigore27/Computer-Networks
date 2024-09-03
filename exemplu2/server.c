#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <utmp.h>
#include <time.h>
#include <string.h>


int client_read, client_write;

int logged = 0;

struct incercare
{
    char nume[100];
    int estelogat;
};

////implementez toate comenzile din protocol
//  void   Login(char comanda[100])
//  {
//     pid_t pid = fork();
//     if (pid == 0)
//     {
//        char nume[100];
//      strcpy(nume, comanda + 8);
//     printf("Utilizatorul %s s-a autentificat.\n", nume);
//
//         int Users;
//         Users = open("users.txt", O_RDONLY);
//         int lg_allUsers;
//         char allUsers[1000];
//         allUsers[0] = '\0';
//         lg_allUsers = read(fdUsers, allUsers, 1000);
//         allUsers[lg_allUsers] = '\0';
//         char *incercare = strtok(allUsers, ",");
//         while (incercare)
//         {
//             if (strcmp(incercare, nume) == 0)
//             {
//                 exit(1);
//             }
//             incercare = strtok(NULL, ",");
//         }
//         exit(2);
//         close(fdUsers);
//     }
//     else
//     {
//         int semnal;
//          pid_t waitpid(pid, &semnal, 0);
//         if (WEXITSTATUS(semnal) != 2)
//         {
//             if (logged == 0)
//             {
//                 logged = 1;
//                 write(client_write, "Autentificarea s-a realizat cu succes!", strlen("Autentificarea s-a realizat cu succes!"));
//             }
//             else
//                 write(client_write, "Exista deja un utilizator pe server.", strlen("Exista deja un utilizator pe server."));
//         }
//         else if (WEXITSTATUS(semnal) == 2)
//             write(client_write, "Autentificarea a esuat!", strlen("Autentificarea a esuat!"));
//     }
// }
// void logout(char comanda[100])
// {
//     if (logged == 1)
//         write(client_write, "Te-ai delogat!", strlen("Te-ai delogat!"));
//     else
//         write(client_write, "NU te poti deloga, pentru ca nu te-ai logat", strlen("NU te poti deloga, pentru ca nu te-ai logat"));
//     logged = 0;
// }
void get_logged_users(char comanda[100])
{
int sock[3];
int copil, copil1;
socketpair(AF_UNIX,SOCK_STREAM,0,sock);
if(copil1== 0)
            {
                close(sock[1]);
                struct utmp *p;
                p = getutent();
                    write(sock[0],p->ut_user,strlen(p->ut_user));
                    write(sock[0],"\n",1);
                    write(sock[0],p->ut_host,strlen(p->ut_host));
                    write(sock[0],"\n",1);
                
                close(sock[0]);
            }
            else{
                close(sock[0]);
                char receive[100];
                receive[0]='\0';
//
                int lg_sock = read(sock[1],receive,100);
                write(client_write,receive,lg_sock);

                close(sock[1]);
            }
}
void get_proc_info(char comanda[100])
{//
}
void quit(char comanda[100])
{   
     if (logged == 1)
        logged = 0;
     else
     write(client_write, "Comanda nu poate fi executata deoarece nu te-ai logat", strlen("Comanda nu poate fi executata deoarece nu te-ai logat"));
}

//----------------------------

// fac o functie sa imi dau seama care comanda e

void alegere(char comanda[100])
{
    // if (strstr(comanda, "Login : ") != NULL)
    // {
    //     Login(comanda);
    // } 
    // else if (strcmp(comanda, "logout") == 0)
    // {
    //     //logout(comanda);
    // }
    
    if (strcmp(comanda, "get-logged-users") == 0)
    {
        get_logged_users(comanda);
    }
    else if (strstr(comanda, "get-proc-info : ") != NULL)
    {
        get_proc_info(comanda);
    }
   
    else if (strcmp(comanda, "quit") == 0)
    {
        quit(comanda);
    }
}
int main()
{
    if (access("cl-sv", F_OK) != 0) // daca client_to_server nu exista
    {
        if (mknod("cl-sv", S_IFIFO | 0666, 0) == -1) // il creez
            fprintf(stderr, "eroare la crearea FIFO client_to_server");
    }
    client_read = open("cl-sv", O_RDONLY); // serverul primeste mesaje de la client
    if (client_read == -1)
    {
        fprintf(stderr, "eroare la deschiderea fisier_to_server in server");
        exit(1);
    }

    if (access("sv-cl", F_OK) != 0) // daca server_to_client nu exista
    {
        if (mknod("sv-cl", S_IFIFO | 0666, 0) == -1) // il creez
            fprintf(stderr, "eroare la crearea FIFO server_to_client");
    }
    client_write = open("sv-cl", O_WRONLY); // serverul trimite mesaje la client

    if (client_write == -1)
    {
        fprintf(stderr, "eroare la crearea server_to_client in server");
        exit(3);
    }

//
    char comanda[100];
    int lg_comanda = 0;
    while (1)
    {
        lg_comanda = read(client_read, comanda, 100);
        comanda[lg_comanda] = '\0';
        alegere(comanda);
        //
            printf("%s\n", comanda);
        //
        if (write(client_write, comanda, sizeof(comanda)) == -1)
        {
            fprintf(stderr, "eroare la scrierea mesajului de la client");
            exit(4);
        }
    }
    return 0;
}

