#include "stdio.h"
#include "stdbool.h"
#include "mem.h"

#include "file_messages.h"
#include "processus.h"

int pcreate(int count)
{
    if (count <= 0 || count >= MAX_COUNT)
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

    return fid;
}

int pdelete(int fid)
{
    if (fid < 0 || fid > NBQUEUE || queue[fid].messages == NULL)
        return -1;                                                           // fid non valide
    mem_free(queue[fid].messages, queue[fid].size * sizeof(struct message)); // on libère une file de messages de capacité size
    queue[fid].messages = NULL;

    // fait passer dans l'état activable tous les processus qui se trouvaient bloqués sur la file (s'il en existe)
    for (int i = 0; i < NBPROC; i++)
    {
        if (file_procs[i] != NULL && file_procs[i]->etat == BLOQUE_FMSG)
        {
            file_procs[i]->etat = ACTIVABLE;
            ordonnance();
        }
    }

    return 0;
}

int psend(int fid, int message)
{
    if (fid < 0 || fid > NBQUEUE || queue[fid].messages == NULL)
    {
        return -1; // fid non valide
    }

    bool empty = false;
    if (!queue[fid].messages[0].active)
        empty = true; // file vide

    bool full = false;
    if (queue[fid].messages[queue[fid].size - 1].active)
        full = true; // file pleine

    bool _else = true;

    if (empty)
    {
        // si la file est vide et que des processus sont bloqués en attente de message,
        // alors le processus le plus ancien dans la file parmi les plus prioritaires est débloqué et reçoit ce message.
        for (int i = 0; i < NBPROC; i++)
        {
            if (file_procs[i] != NULL && file_procs[i]->etat == BLOQUE_FMSG && file_procs[i]->etat == BLOQUE_FMSG_PLEINE && file_procs[i]->etat == BLOQUE_FMSG_VIDE)
            {
                file_procs[i]->etat = ACTIVABLE;
                file_procs[i]->lastmsg = message;
                ordonnance();
                _else = false;
                break;
            }
        }
    }
    else if (full)
    {
        // si la file est pleine, le processus appelant passe dans l'état bloqué sur file pleine jusqu'à ce qu'une place soit disponible, dans la file,
        // pour y mettre le message.
        file_procs[proc_actif]->etat = BLOQUE_FMSG_PLEINE;
        waiting_for_available_place_file[NBPROC - 1].pid = getpid();
        waiting_for_available_place_file[NBPROC - 1].fid = fid;
        tidy_up_waiting(&waiting_for_available_place_file[0]); // on range la file dès qu'une entrée est ajoutée
        ordonnance();
        for (int i = 0; i < NBPROC; i++)
        {
            int pid = waiting_for_available_place_file[i].pid;
            if (pid == getpid() && file_procs[getproc(pid)] != NULL)
            {
                // on retire l'entrée traitée sur waiting_for_available_place_file
                waiting_for_available_place_file[i].pid = -1;
                waiting_for_available_place_file[i].fid = -1;

                // on range la file dès qu'une entrée est retirée
                tidy_up_waiting(&waiting_for_available_place_file[0]);
            }
            break;
        }
    }

    if (_else)
    {
        // sinon, la file n'est pas pleine et aucun processus n'est bloqué en attente de message ; le message est alors déposé directement dans la file.
        queue[fid].messages[queue[fid].size - 1].content = message;
        queue[fid].messages[queue[fid].size - 1].active = true;
        tidy_up_queue(fid); // on range la file dès qu'un message est ajouté

        /* vérification processus bloqué en file d'attente vide */
        check_if_there_is_new_message();
    }

    return 0;
}

int preceive(int fid, int *message)
{
    if (fid < 0 || fid > NBQUEUE || queue[fid].messages == NULL)
        return -1; // fid non valide

    bool empty = false;
    if (!queue[fid].messages[0].active)
        empty = true; // file vide

    bool full = false;
    if (queue[fid].messages[queue[fid].size - 1].active)
        full = true; // file pleine

    // si un message au moins est disponible dans la file, le premier message est transmis au processus :
    // - lit et enlève le premier (plus ancien) message de la file fid.
    // - le message lu est placé dans *message si message n'est pas nul, sinon il est oublié.
    if (queue[fid].messages[0].active)
    {
        if (message != NULL)
            *message = queue[fid].messages[0].content;
        queue[fid].messages[0].active = false;
        tidy_up_queue(fid); // on range la file dès qu'un message est retiré

        /* vérification processus bloqué en file d'attente pleine */
        check_if_there_is_available_place();
    }

    if (full)
    {
        // si la file était pleine, il faut alors immédiatement compléter la file avec le message du premier processus bloqué sur file pleine ;
        // ce processus devient activable ou actif selon sa priorité
        for (int i = 0; i < NBPROC; i++)
        {
            if (file_procs[i] != NULL && file_procs[i]->etat == BLOQUE_FMSG_PLEINE)
            {
                queue[fid].messages[queue[fid].size - 1].content = file_procs[i]->lastmsg;
                queue[fid].messages[queue[fid].size - 1].active = true;
                tidy_up_queue(fid); // on range la file dès qu'un message est ajouté
                file_procs[i]->etat = ACTIVABLE;
                ordonnance();
                break;
            }
        }
    }
    else if (empty)
    {
        // si aucun message n'est présent, le processus se bloque et sera relancé lors d'un dépôt ultérieur.
        file_procs[proc_actif]->etat = BLOQUE_FMSG_VIDE;
        waiting_for_new_message_file[NBPROC - 1].pid = getpid();
        waiting_for_new_message_file[NBPROC - 1].fid = fid;
        tidy_up_waiting(&waiting_for_new_message_file[0]); // on range la file dès qu'une entrée est ajoutée
        ordonnance();
        for (int i = 0; i < NBPROC; i++)
        {
            int pid = waiting_for_new_message_file[i].pid;
            if (pid == getpid() && file_procs[getproc(pid)] != NULL)
            {
                // on retire l'entrée traitée sur waiting_for_new_message_file
                waiting_for_new_message_file[i].pid = -1;
                waiting_for_new_message_file[i].fid = -1;

                // on range la file dès qu'une entrée est retirée
                tidy_up_waiting(&waiting_for_new_message_file[0]);
            }
            break;
        }
        preceive(fid, message);
    }

    return 0;
}

int preset(int fid)
{
    if (fid < 0 || fid > NBQUEUE || queue[fid].messages == NULL)
        return -1; // fid non valide

    // on réinitialise la file
    int count = queue[fid].size;
    pdelete(fid);
    pcreate(count);

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

void init_waiting_for_available_place_file()
{
    for (int i = 0; i < NBPROC; i++)
    {
        waiting_for_available_place_file[i].pid = -1;
        waiting_for_available_place_file[i].fid = -1;
    }
}

void init_waiting_for_new_message_file()
{
    for (int i = 0; i < NBPROC; i++)
    {
        waiting_for_new_message_file[i].pid = -1;
        waiting_for_new_message_file[i].fid = -1;
    }
}

void check_if_there_is_new_message()
{
    for (int i = 0; i < NBPROC; i++)
    {
        int pid = waiting_for_new_message_file[i].pid;
        int fid = waiting_for_new_message_file[i].fid;

        // chaque processus bloqué passe dans l'état activable lorsqu'il y a une place disponible pour envoyer un message
        if (file_procs[getproc(pid)] != NULL && file_procs[getproc(pid)]->etat == BLOQUE_FMSG_PLEINE && !queue[fid].messages[queue[fid].size - 1].active)
            file_procs[getproc(pid)]->etat = ACTIVABLE;
    }

    ordonnance();
}

void check_if_there_is_new_message()
{
        printf("\n---\n");
        for (int x = 0; x < NBPROC; x++)
            printf(" %d |", waiting_for_new_message_file[x].pid);
        printf("\n-F--\n");
        sleep(1);
    for (int i = 0; i < NBPROC; i++)
    {
        int pid = waiting_for_new_message_file[i].pid;
        int fid = waiting_for_new_message_file[i].fid;

        // chaque processus bloqué passe dans l'état activable lorsqu'il y a un nouveau message
        if (file_procs[getproc(pid)] != NULL && file_procs[getproc(pid)]->etat == BLOQUE_FMSG_VIDE && queue[fid].messages[0].active)
            file_procs[getproc(pid)]->etat = ACTIVABLE;
    }

    ordonnance();
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
