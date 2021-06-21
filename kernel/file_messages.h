#ifndef FILE_MESSAGES_H_
#define FILE_MESSAGES_H_

#define NBQUEUE 20 // nombre maximum de files

typedef enum TYPE
{
    REQUETE,
    REPONSE
} TYPE;

struct message
{
    TYPE type;   // type du message
    int content; // contenu du message
};

struct msg_queue
{
    //int fid;                  // identifiant de file
    struct message *messages; // file de messages
    int size;                 // taille de la file
};

struct msg_queue queue[NBQUEUE];

int pcreate(int count);
int pdelete(int fid);
int psend(int fid, int message);
int preceive(int fid, int *message);
int preset(int fid);
int pcount(int fid, int *count);

#endif /* FILE_MESSAGES_H_ */
