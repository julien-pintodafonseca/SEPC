#include "stdio.h"
#include "stdbool.h"
#include "mem.h"

#include "file_messages.h"
#include "processus.h"

// TODO ? "L'utilisation conjointe d'un tampon de stockage de messages et d'une file de processus bloqués en émission permet de simuler un tampon de taille non bornée."

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

    queue[fid].messages = mem_alloc(count * sizeof(struct message)); // on alloue une file de messages de capacité count
    queue[fid].size = count;
    for (int mid = 0; mid < count; mid++)
    {
        queue[fid].messages[mid].active = false; // par défaut, il n'y a aucun message (file vide)
    }

    return fid;
}

int pdelete(int fid)
{
    if (fid < 0 || fid > NBQUEUE)
        return -1; // fid non valide
    mem_free(queue[fid].messages, queue[fid].size * sizeof(struct message));
    queue[fid].messages = NULL;

    // fait passer dans l'état activable tous les processus, s'il en existe, qui se trouvaient bloqués sur la file.
    for (int i = 0; i < NBPROC; i++)
    {
        if (file_procs[i] != NULL && file_procs[i]->etat == BLOQUE_ES)
        {
            file_procs[i]->etat = ACTIVABLE;
        }
    }

    return 0;
}

int psend(int fid, int message)
{
    if (fid < 0 || fid > NBQUEUE)
        return -1; // fid non valide

    // si la file est vide et que des processus sont bloqués en attente de message,
    // alors le processus le plus ancien dans la file parmi les plus prioritaires est débloqué et reçoit ce message.
    bool empty = true;
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

    // si la file est pleine, le processus appelant passe dans l'état bloqué sur file pleine jusqu'à ce qu'une place soit disponible dans la file pour y mettre le message.
    bool full = true;
    for (int i = 0; i < fid; i++)
    {
        if (queue[i].messages == NULL)
        {
            full = false;
            break;
        }
    }
    if (full)
    {
        file_procs[proc_actif]->etat = BLOQUE_ES;
        // TODO : "jusqu'à ce qu'une place se libère"
    }

    // sinon, la file n'est pas pleine et aucun processus n'est bloqué en attente de message. Le message est alors déposé directement dans la file.
    if (!queue[fid].messages[queue[fid].size - 1].active)
    {
        queue[fid].messages[queue[fid].size - 1].content = message;
        queue[fid].messages[queue[fid].size - 1].active = true;
        sort_queue(fid); // on range la file dès qu'un message est ajouté
    }
    else
    {
        return -42; // si on est là, c'est pas normal
        sort_queue(fid);
        psend(fid, message);
    }

    // TODO : Il est possible également, qu'après avoir été mis dans l'état bloqué sur file pleine, le processus soit remis dans l'état activable par un autre processus ayant exécuté preset ou pdelete. Dans ce cas, la valeur de retour de psend est strictement négative.
    return 0;
}

int preceive(int fid, int *message)
{
    if (fid < 0 || fid > NBQUEUE)
        return -1; // fid non valide

    bool full = false;
    if (queue[fid].messages[queue[fid].size - 1].active)
        full = true;

    bool empty = false;
    if (!full && !queue[fid].messages[0].active)
        empty = true;

    // lit et enlève le premier (plus ancien) message de la file fid. Le message lu est placé dans *message si message n'est pas nul, sinon il est oublié.
    if (queue[fid].messages[0].active)
    {
        *message = queue[fid].messages[0].content;
        queue[fid].messages[0].active = false;
    }
    sort_queue(fid); // on range la file dès qu'un message est retiré

    // Si la file était pleine, il faut alors immédiatement compléter la file avec le message du premier processus bloqué sur file pleine ; ce processus devient activable ou actif selon sa priorité ;
    for (int i = 0; i < NBPROC; i++)
    {
        if (file_procs[i] != NULL && file_procs[i]->etat == BLOQUE_ES)
        {
            file_procs[i]->etat = ACTIVABLE;
            // TODO : activeable ou actif selon sa priorité + on complète la file avec le message du processus
            // TODO : "L'obtention du premier message de la file peut nécessiter le passage dans l'état bloqué sur file vide jusqu'à ce qu'un autre processus exécute une primitive psend." ??
            // "Il est possible également, qu'après avoir été mis dans l'état bloqué sur file vide, le processus soit remis dans l'état activable par un autre processus ayant exécuté preset ou pdelete. Dans ce cas, la valeur de retour de preceive est strictement négative.""
            // "Un processus bloqué sur file vide et dont la priorité est changée par chprio, est considéré comme le dernier processus (le plus jeune) de sa nouvelle priorité.""
            break;
        }
    }

    // si aucun message n'est présent, le processus se bloque et sera relancé lors d'un dépôt ultérieur.
    if (empty)
        file_procs[proc_actif]->etat = BLOQUE_ES;

    return 0;
}

int preset(int fid)
{
    if (fid < 0 || fid > NBQUEUE)
        return -1; // fid non valide

    // on réinitialise la file
    int count = queue[fid].size;
    pdelete(fid);
    pcreate(count);

    // TODO ? fait passer dans l'état activable ou actif (selon les priorités) tous les processus, s'il en existe, se trouvant dans l'état bloqué sur file pleine ou dans l'état bloqué sur file vide (ces processus auront une valeur strictement négative comme valeur de retour de psend ou preceive)

    return 0;
}

int pcount(int fid, int *count)
{
    // TODO
    printf("%d%d", fid, *count);
    return 0;
}

// supprime les "espaces" vides entre plusieurs messages s'il y en a (permet de ranger les messages du plus ancien au moins ancien, sans espace dans le tableau)
void sort_queue(int fid)
{
    /*printf("--D RANGEMENT--\n");
    for (int x = 0; x < queue[fid].size; x++)
    {
        //printf("\n%d\n", x);
        printf("%d", queue[fid].messages[x].active);
    }
    printf("\n");*/

    int need_move = -1;
    for (int mid = 0; mid < queue[fid].size; mid++)
    {
        if (!queue[fid].messages[mid].active && need_move == -1)
            need_move = mid;
        if (queue[fid].messages[mid].active && need_move != -1)
        {
            int tmp = queue[fid].messages[mid].content;
            queue[fid].messages[mid].active = false;
            queue[fid].messages[need_move].content = tmp;
            queue[fid].messages[need_move].active = true;
            need_move = -1;
            mid = 0;
        }
    }

    /*for (int x = 0; x < queue[fid].size; x++)
    {
        printf("%d", queue[fid].messages[x].active);
    }
    printf("\n--F RANGEMENT--\n");*/
}
