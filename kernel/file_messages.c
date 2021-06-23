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
    if (fid < 0 || fid > NBQUEUE)
        return -1;                                                           // fid non valide
    mem_free(queue[fid].messages, queue[fid].size * sizeof(struct message)); // on libère une file de messages de capacité size
    queue[fid].messages = NULL;

    // fait passer dans l'état activable tous les processus qui se trouvaient bloqués sur la file (s'il en existe)
    for (int i = 0; i < NBPROC; i++)
    {
        if (file_procs[i] != NULL && file_procs[i]->etat == BLOQUE_FMSG)
            file_procs[i]->etat = ACTIVABLE;
    }

    return 0;
}

int psend(int fid, int message)
{
    if (fid < 0 || fid > NBQUEUE)
        return -1; // fid non valide

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
            if (file_procs[i] != NULL && file_procs[i]->etat == BLOQUE_FMSG)
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

        int i = 0;
        while (waiting_for_available_place_file[i].pid != -1)
        {
            if (i >= NBPROC)
                return -42; // err
            i++;
        }
        waiting_for_available_place_file[i].pid = getpid();
        waiting_for_available_place_file[i].fid = fid;
        waiting_for_available_place_file[i].msg = message;

        // TODO : Il est possible également, qu'après avoir été mis dans l'état bloqué sur file pleine, le processus soit remis dans l'état activable
        // par un autre processus ayant exécuté preset ou pdelete. Dans ce cas, la valeur de retour de psend est strictement négative.
    }

    if (_else)
    {
        // sinon, la file n'est pas pleine et aucun processus n'est bloqué en attente de message ; le message est alors déposé directement dans la file.
        queue[fid].messages[queue[fid].size - 1].content = message;
        queue[fid].messages[queue[fid].size - 1].active = true;
        tidy_up_queue(fid); // on range la file dès qu'un message est ajouté
    }

    return 0;
}

int preceive(int fid, int *message)
{
    if (fid < 0 || fid > NBQUEUE)
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
        if (message != 0)
            *message = queue[fid].messages[0].content;
        queue[fid].messages[0].active = false;
        tidy_up_queue(fid); // on range la file dès qu'un message est retiré
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
                // TODO : "L'obtention du premier message de la file peut nécessiter le passage dans l'état bloqué sur file vide jusqu'à ce qu'un autre processus exécute une primitive psend.
                // Il est possible également, qu'après avoir été mis dans l'état bloqué sur file vide, le processus soit remis dans l'état activable par un autre processus ayant exécuté preset ou pdelete. Dans ce cas, la valeur de retour de preceive est strictement négative.
                // Un processus bloqué sur file vide et dont la priorité est changée par chprio, est considéré comme le dernier processus (le plus jeune) de sa nouvelle priorité."
                break;
            }
        }
    }
    else if (empty)
    {
        // si aucun message n'est présent, le processus se bloque et sera relancé lors d'un dépôt ultérieur.
        file_procs[proc_actif]->etat = BLOQUE_FMSG;
    }

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

    // TODO : fait passer dans l'état activable ou actif (selon les priorités) tous les processus, s'il en existe, se trouvant dans l'état bloqué sur file pleine ou dans l'état bloqué sur file vide (ces processus auront une valeur strictement négative comme valeur de retour de psend ou preceive)

    return 0;
}

int pcount(int fid, int *count)
{
    if (fid < 0 || fid > NBQUEUE)
        return -1; // fid non valide

    // si count n'est pas nul, elle y place une valeur négative égale à l'opposé du nombre de processus bloqués sur file vide,
    // ou une valeur positive égale à la somme du nombre de messages dans la file et du nombre de processus bloqués sur file pleine.
    int vPos = 0;
    int vNeg = 0;
    if (*count != 0)
    {

        for (int i = 0; i < NBPROC; i++)
        {
            if (file_procs[i] != NULL && file_procs[i]->etat == BLOQUE_FMSG_PLEINE)
                vPos++;
            if (file_procs[i] != NULL && file_procs[i]->etat == BLOQUE_FMSG_VIDE)
                vNeg--;
            if (waiting_for_available_place_file[i].pid != -1)
                vPos++;
        }
    }

    if (vPos > 0)
        return vPos;
    else if (vNeg < 0)
        return vNeg;
    else
        return 0;
}

void init_waiting_for_available_place_file()
{
    for (int i = 0; i < NBPROC; i++)
    {
        waiting_for_available_place_file[i].pid = -1;
        waiting_for_available_place_file[i].fid = -1;
        waiting_for_available_place_file[i].msg = -1;
    }
}

void check_if_there_is_available_place()
{
    for (int i = 0; i < NBPROC; i++)
    {
        int pid = waiting_for_available_place_file[i].pid;
        int fid = waiting_for_available_place_file[i].fid;

        if (file_procs[getproc(pid)] != NULL && file_procs[getproc(pid)]->etat == BLOQUE_FMSG_PLEINE && !queue[fid].messages[queue[fid].size - 1].active)
        {
            // on ajoute le message à la file et on passe le processus à l'état activable
            queue[fid].messages[queue[fid].size - 1].content = file_procs[getproc(pid)]->lastmsg;
            queue[fid].messages[queue[fid].size - 1].active = true;
            tidy_up_queue(fid); // on range la file dès qu'un message est ajouté
            file_procs[getproc(pid)]->etat = ACTIVABLE;

            // on retire le message/processus traité de waiting_for_available_place_file
            waiting_for_available_place_file[i].pid = -1;
            waiting_for_available_place_file[i].fid = -1;
            waiting_for_available_place_file[i].msg = -1;

            // on range waiting_for_available_place_file (pour garder l'ordre d'anciennecité)
            int need_move = -1;
            for (int j = 0; j < NBPROC; j++)
            {
                if (!(waiting_for_available_place_file[j].pid == -1) && need_move == -1)
                    need_move = j;
                if (waiting_for_available_place_file[j].pid == -1 && need_move != -1)
                {
                    int tmp_1 = waiting_for_available_place_file[j].pid;
                    int tmp_2 = waiting_for_available_place_file[j].fid;
                    int tmp_3 = waiting_for_available_place_file[j].msg;

                    waiting_for_available_place_file[j].pid = -1;
                    waiting_for_available_place_file[j].fid = -1;
                    waiting_for_available_place_file[j].msg = -1;

                    waiting_for_available_place_file[j].pid = tmp_1;
                    waiting_for_available_place_file[j].fid = tmp_2;
                    waiting_for_available_place_file[j].msg = tmp_3;
                    need_move = -1;
                    j = 0;
                }
            }

            // file remplie, on s'arrête ici
            break;
        }
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
