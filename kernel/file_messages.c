#include "stdio.h"
#include "stdbool.h"
#include "mem.h"

#include "file_messages.h"
#include "processus.h"

int pcreate(int count)
{
    if (count <= 0)
        return -1; // count non valide
    int fid = 0;
    while (queue[fid].messages != NULL)
    {
        fid++;
        if (fid > NBQUEUE)
            return -1; // plus de files disponibles
    }
    queue[fid].messages = mem_alloc(count); // on alloue une file de messages de capacité count
    queue[fid].size = count;
    return fid;
}

int pdelete(int fid)
{
    if (fid < 0 || fid > NBQUEUE)
        return -1; // fid non valide
    mem_free(queue[fid].messages, queue[fid].size);
    queue[fid].messages = NULL;
    // TODO : fait passer dans l'état activable tous les processus, s'il en existe, qui se trouvaient bloqués sur la file
    // Les processus libérés auront une valeur strictement négative comme retour de psend ou preceive.
    // Les messages se trouvant dans la file sont abandonnés.

    return 0;
}

int psend(int fid, int message)
{
    // Si la file est vide et que des processus sont bloqués en attente de message,
    // alors le processus le plus ancien dans la file parmi les plus prioritaires est débloqué et reçoit ce message.
    bool empty = true;
    ;
    for (int i = 0; i < fid; i++)
    {
        if (queue[i].messages != NULL)
        {
            empty = false;
            break;
        }
    }

    if (empty)
    {
        for (int i = 0; i < NBPROC; i++)
        {
            if (file_procs[i] != NULL && file_procs[i]->etat == BLOQUE_ES)
            {
                file_procs[i]->etat = ACTIVABLE;
                // TODO : on donne le message au processus
                break;
            }
        }
    }

    // TODO : suite fonction

    printf("%d", message);
    return 0;
}
