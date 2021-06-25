#include "stdio.h"
#include "stdbool.h"
#include "mem.h"

#include "file_messages.h"
#include "processus.h"

int fcount = 0;

int pcreate(int count)
{
    if (count <= 0 || count >= MAX_COUNT || fcount >= MAX_COUNT)
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
        queue[fid].messages[mid].active = false; // par défaut, il n'y a aucun message (initialisation file vide)
    }
    fcount += count;

    return fid;
}

int pdelete(int fid)
{
    if (fid < 0 || fid > NBQUEUE || queue[fid].messages == NULL)
        return -1;                                                           // fid non valide
    fcount -= queue[fid].size;
    mem_free(queue[fid].messages, queue[fid].size * sizeof(struct message)); // on libère une file de messages de capacité size
    queue[fid].messages = NULL;

    // fait passer dans l'état activable tous les processus qui se trouvaient bloqués sur la file (s'il en existe)
    for (int i = 0; i < NBPROC; i++)
    {
        if (waiting_for_new_message_file[i].fid == fid)
        {
            procs[waiting_for_new_message_file[i].pid].del = true; // psend et preceive renvoient une erreur
            procs[waiting_for_new_message_file[i].pid].etat = ACTIVABLE;
            waiting_for_new_message_file[i].pid = -1;
            waiting_for_new_message_file[i].fid = -1;
        }
        if (waiting_for_new_place_file[i].fid == fid)
        {
            procs[waiting_for_new_place_file[i].pid].del = true; // psend et preceive renvoient une erreur
            procs[waiting_for_new_place_file[i].pid].etat = ACTIVABLE;
            waiting_for_new_place_file[i].pid = -1;
            waiting_for_new_place_file[i].fid = -1;
        }
    }
    ordonnance();
    return 0;
}

int psend(int fid, int message)
{
    if (fid < 0 || fid > NBQUEUE || queue[fid].messages == NULL)
        return -1; // fid non valide

    if (!queue[fid].messages[0].active) // la file est vide
    {
        for (int i = 0; i < NBPROC; i++)
        {
            if (waiting_for_new_message_file[i].fid == fid && waiting_for_new_message_file[i].pid != -1)
            { // la file est vide et un processus est bloqué en attente de message
                // on débloque le processus le plus ancien dans la file parmi les plus prioritaires
                procs[waiting_for_new_message_file[i].pid].etat = ACTIVABLE;
                // on donne le message directement à ce processus
                procs[waiting_for_new_message_file[i].pid].lastmsg = message;
                // on retire le processus de la file d'attente
                waiting_for_new_message_file[i].pid = -1;
                waiting_for_new_message_file[i].fid = -1;
                // on range la file dès qu'une entrée est retirée
                tidy_up_waiting(&waiting_for_new_message_file[0]);
                ordonnance();
                return 0;
            }
        }
        // la file est vide, mais aucun processus n'est bloqué en attente de message
        // on dépose donc le message dans la file
        queue[fid].messages[0].content = message;
        queue[fid].messages[0].active = true;
        ordonnance();
        return 0;
    }
    else if (queue[fid].messages[queue[fid].size - 1].active) // la file est pleine
    {
        // si la file est pleine, le processus appelant passe dans l'état bloqué
        // sur file pleine jusqu'à ce qu'une place soit disponible, dans la file,
        // pour y mettre le message.
        file_procs[proc_actif]->etat = BLOQUE_FMSG_PLEINE;
        file_procs[proc_actif]->lastmsg = message;
        int indice = -1;
        for (indice = 0; indice < NBPROC && waiting_for_new_place_file[indice].pid != -1 && (waiting_for_new_place_file[indice].fid != fid || procs[waiting_for_new_place_file[indice].pid].prio >= file_procs[proc_actif]->prio); indice++)
            ;
        for (int n = NBPROC - 1; n > indice; n--)
        {
            waiting_for_new_place_file[n].pid = waiting_for_new_place_file[n - 1].pid;
            waiting_for_new_place_file[n].fid = waiting_for_new_place_file[n - 1].fid;
        }
        waiting_for_new_place_file[indice].pid = getpid();
        waiting_for_new_place_file[indice].fid = fid;
        tidy_up_waiting(&waiting_for_new_place_file[0]); // on range la file dès qu'une entrée est ajoutée
        ordonnance();
        if (file_procs[proc_actif]->del)
        {
            file_procs[proc_actif]->del = false;
            return -2; // si pdelete ou preset
        }
        return 0;
    }
    // sinon, la file n'est pas pleine ni vide et aucun processus n'est bloqué
    // en attente de message ; le message est alors déposé directement dans la file.
    queue[fid].messages[queue[fid].size - 1].content = message;
    queue[fid].messages[queue[fid].size - 1].active = true;
    tidy_up_queue(fid); // on range la file dès qu'un message est ajouté
    ordonnance();
    return 0;
}

int preceive(int fid, int *message)
{
    if (fid < 0 || fid > NBQUEUE || queue[fid].messages == NULL)
        return -1; // fid non valide

    if (queue[fid].messages[0].active)
    { // la file n'est pas vide
        // on transmet le message si le pointeur message n'est pas NULL, sinon il est oublié
        if (message != NULL)
            *message = queue[fid].messages[0].content;
        queue[fid].messages[0].active = false;
        tidy_up_queue(fid); // on range la file dès qu'un message est retiré

        if (queue[fid].messages[queue[fid].size - 2].active)
        { // la file était pleine
            // On complete la file avec le message du plus ancien processus bloqué en émission parmi les plus prioritaires
            // Ce processus devient ACTIVABLE
            for (int i = 0; i < NBPROC; i++)
            {
                if (waiting_for_new_place_file[i].fid == fid && waiting_for_new_place_file[i].pid != -1)
                { // la file est vide et un processus est bloqué en attente de message
                    // on débloque le processus le plus ancien dans la file parmi les plus prioritaires
                    procs[waiting_for_new_place_file[i].pid].etat = ACTIVABLE;
                    // on dépose le message en fin de file
                    queue[fid].messages[queue[fid].size - 1].content = procs[waiting_for_new_place_file[i].pid].lastmsg;
                    queue[fid].messages[queue[fid].size - 1].active = true;
                    // on retire le processus de la file d'attente
                    waiting_for_new_place_file[i].pid = -1;
                    waiting_for_new_place_file[i].fid = -1;
                    // on range la file dès qu'une entrée est retirée
                    tidy_up_waiting(&waiting_for_new_place_file[0]);
                    ordonnance();
                    return 0;
                }
            }
        }
    }
    else
    { // la file est vide
        // si aucun message n'est présent, le processus se bloque et sera relancé lors d'un dépôt ultérieur.
        file_procs[proc_actif]->etat = BLOQUE_FMSG_VIDE;
        int indice;
        for (indice = 0; indice < NBPROC && waiting_for_new_message_file[indice].pid != -1 && (waiting_for_new_message_file[indice].fid != fid || procs[waiting_for_new_message_file[indice].pid].prio >= file_procs[proc_actif]->prio); indice++)
            ;
        for (int n = NBPROC - 1; n > indice; n--)
        {
            waiting_for_new_message_file[n].pid = waiting_for_new_message_file[n - 1].pid;
            waiting_for_new_message_file[n].fid = waiting_for_new_message_file[n - 1].fid;
        }
        waiting_for_new_message_file[indice].pid = getpid();
        waiting_for_new_message_file[indice].fid = fid;
        ordonnance();
        if (file_procs[proc_actif]->del)
        {
            file_procs[proc_actif]->del = false;
            return -2; // si pdelete ou preset
        }
        // else
        if (message != NULL)
            *message = procs[getpid()].lastmsg;
    }
    ordonnance();
    return 0;
}

int preset(int fid)
{
    if (fid < 0 || fid > NBQUEUE || queue[fid].messages == NULL)
        return -1; // fid non valide
    // on réinitialise la file
    // fait passer dans l'état activable tous les processus qui se trouvaient bloqués sur la file (s'il en existe)
    for (int i = 0; i < NBPROC; i++)
    {
        if (waiting_for_new_message_file[i].fid == fid)
        {
            procs[waiting_for_new_message_file[i].pid].del = true; // psend et preceive renvoient une erreur
            procs[waiting_for_new_message_file[i].pid].etat = ACTIVABLE;
            waiting_for_new_message_file[i].pid = -1;
            waiting_for_new_message_file[i].fid = -1;
        }
        if (waiting_for_new_place_file[i].fid == fid)
        {
            procs[waiting_for_new_place_file[i].pid].del = true; // psend et preceive renvoient une erreur
            procs[waiting_for_new_place_file[i].pid].etat = ACTIVABLE;
            waiting_for_new_place_file[i].pid = -1;
            waiting_for_new_place_file[i].fid = -1;
        }
    }
    for (int mid = 0; mid < queue[fid].size; mid++)
    {
        queue[fid].messages[mid].active = false; // par défaut, il n'y a aucun message (initialisation file vide)
    }

    return 0;
}

int pcount(int fid, int *count)
{
    if (fid < 0 || fid > NBQUEUE || queue[fid].messages == NULL)
        return -1; // fid non valide

    // si count n'est pas nul, elle y place une valeur négative égale à l'opposé du nombre de processus bloqués sur file vide,
    // ou une valeur positive égale à la somme du nombre de messages dans la file et du nombre de processus bloqués sur file pleine.
    int vPos = 0;
    int vNeg = 0;
    if (count != NULL)
    {
        for (int i = 0; i < queue[fid].size; i++)
        {
            if (queue[fid].messages[i].active)
                vPos++;
        }
        for (int i = 0; i < NBPROC; i++)
        {
            if (file_procs[i] != NULL && file_procs[i]->etat == BLOQUE_FMSG_PLEINE)
                vPos++;
            if (file_procs[i] != NULL && file_procs[i]->etat == BLOQUE_FMSG_VIDE)
                vNeg--;
        }
    }

    if (vPos > 0)
        *count = vPos;
    else if (vNeg < 0)
        *count = vNeg;
    else
        *count = 0;

    return 0;
}

void init_waiting_files()
{
    for (int i = 0; i < NBPROC; i++)
    {
        waiting_for_new_message_file[i].pid = -1;
        waiting_for_new_message_file[i].fid = -1;
        waiting_for_new_place_file[i].pid = -1;
        waiting_for_new_place_file[i].fid = -1;
    }
}

// permet de ranger les messages du plus ancien au moins ancien, sans espace dans le tableau
void tidy_up_queue(int fid)
{
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
}

// permet de ranger les messages du plus ancien au moins ancien, sans espace dans le tableau
void tidy_up_waiting(struct waiting *waiting_for_something_file)
{
    int need_move = -1;
    for (int j = 0; j < NBPROC; j++)
    {
        if (waiting_for_something_file[j].pid == -1 && need_move == -1)
            need_move = j;
        if (waiting_for_something_file[j].pid != -1 && need_move != -1)
        {
            int tmp_1 = waiting_for_something_file[j].pid;
            int tmp_2 = waiting_for_something_file[j].fid;

            waiting_for_something_file[j].pid = -1;
            waiting_for_something_file[j].fid = -1;

            waiting_for_something_file[need_move].pid = tmp_1;
            waiting_for_something_file[need_move].fid = tmp_2;
            need_move = -1;
            j = 0;
        }
    }
}
