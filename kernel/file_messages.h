#ifndef FILE_MESSAGES_H_
#define FILE_MESSAGES_H_

#include "processus.h"

#define NBQUEUE 20 // nombre maximum de files

typedef enum TYPE
{
    REQUETE,
    REPONSE
} TYPE;

struct message
{
    //TYPE type;   // type du message
    int content; // contenu du message
    bool active; // si inactif, on ne prend pas le message en compte et on peut le remplacer
};

struct msg_queue
{
    //int fid;                  // identifiant de file
    struct message *messages; // file de messages
    int size;                 // taille de la file
};

struct msg_queue queue[NBQUEUE];

struct waiting
{
    int pid; // pid du processus
    int fid; // identifiant de file
    int msg; // message
};

struct waiting waiting_for_available_place_file[NBPROC]; // liste des processus dans l'état bloqué sur file pleine jusqu'à ce qu'une place soit disponible pour y mettre le message
struct waiting waiting_for_new_message_file[NBPROC];     // liste des processus dans l'état bloqué sur file vide jusqu'à ce qu'un nouveau message soit disponible pour réception

int pcreate(int count);
int pdelete(int fid);
int psend(int fid, int message);
int preceive(int fid, int *message);
int preset(int fid);
int pcount(int fid, int *count);

void init_waiting_for_available_place_file();
void init_waiting_for_new_message_file();
void check_if_there_is_available_place();
void check_if_there_is_new_message();
void tidy_up_queue();

#endif /* FILE_MESSAGES_H_ */
