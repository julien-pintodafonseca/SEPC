#ifndef FILE_MESSAGES_H_
#define FILE_MESSAGES_H_

#include "processus.h"

#define MAX_COUNT 1000000 // maximum accepté pour le count
#define NBQUEUE 20 // nombre maximum de files

struct message
{
    int content; // contenu du message
    bool active; // si inactif, on ne prend pas le message en compte et on peut le remplacer
};

struct msg_queue
{
    struct message *messages; // file de messages
    int size;                 // taille de la file
};

struct msg_queue queue[NBQUEUE];

struct waiting
{
    int pid; // pid du processus
    int fid; // identifiant de file
};

struct waiting waiting_for_new_message_file[NBPROC];     // liste des processus dans l'état bloqué sur file vide jusqu'à ce qu'un nouveau message soit disponible pour réception

int pcreate(int count);
int pdelete(int fid);
int psend(int fid, int message);
int preceive(int fid, int *message);
int preset(int fid);
int pcount(int fid, int *count);

void init_waiting_for_new_message_file();
void check_if_there_is_new_message();
void tidy_up_queue();
void tidy_up_waiting(struct waiting *waiting_for_something_file);

#endif /* FILE_MESSAGES_H_ */
