/* Phase 2 : Traitement de l'horloge et des processus */
#include "stdio.h"
#include "stdbool.h"
#include "cpu.h"
#include "mem.h"

#include "processus.h"
#include "horloge.h"
#include "file_messages.h"

/**
 * Fonction d'initialisation du processus principal (idle) qui ne doit jamais mourir
 * pt_func : pointeur vers la fonction à exécuter au lancement du processus, à caster de la façon suivante (int (*)(void *))(func)
 * name    : nom du processus
 */
void initialize(int (*pt_func)(void *), const char *name)
{
    for (int i = 0; i < NBPROC; i++)
    {
        procs[i].pid = -1;
    }
    procs[0].pid = 0;
    sprintf(procs[0].nom, "%p", name);
    procs[0].etat = ACTIF;
    procs[0].prio = 0;

    //init pile
    procs[0].taille_pile = 4000 + 256 * sizeof(int);
    procs[0].pile = mem_alloc(procs[0].taille_pile);
    int index_int = procs[0].taille_pile / 4;
    procs[0].pile[index_int - 3] = (int)(pt_func);
    procs[0].zone_sauv[1] = (int)(&procs[0].pile[index_int - 3]);
    procs[0].parent = -1;
    for (int n = 0; n < NBPROC; n++)
    {
        procs[0].fils[n] = -1;
    }
    file_procs[0] = &procs[0];
}

/**
 * Crée un nouveau processus et l'ajoute à la file
 * pt_func : pointeur vers la fonction à exécuter au lancement du processus, à caster de la façon suivante (int (*)(void *))(func)
 * ssize   : taille de la pile
 * prio    : priorité du processus
 * name    : nom du processus
 * arg     : arguments de la fonction pt_func
 * return : le PID du processus créé
 *      ou -1 en cas d'erreur dans les arguments où impossibilité de créer le processus
 */
int start(int (*pt_func)(void *), unsigned long ssize, int prio, const char *name, void *arg)
{
    if (ssize > MAX_INT)
        return -1; // erreur de dépassement de la pile
    int i;
    struct processus *tmp;
    for (i = 0; i < NBPROC && file_procs[i] != NULL; i++)
        ;
    if (i >= NBPROC)
        return -1;
    // On place le processus à la fin de la file
    int pid = pidlibre();
    procs[pid].pid = pid;
    sprintf(procs[pid].nom, "%s", name);
    procs[pid].etat = ACTIVABLE;
    procs[pid].prio = prio;
    // on libère la pile si elle est init
    if (procs[pid].pile != NULL)
    {
        mem_free(procs[pid].pile, procs[pid].taille_pile);
        procs[pid].pile = NULL;
    }
    // on init la pile
    procs[pid].taille_pile = ssize + 256 * sizeof(int);
    procs[pid].pile = mem_alloc(procs[pid].taille_pile);
    int index_int = procs[pid].taille_pile / 4;
    procs[pid].pile[index_int - 3] = (int)(pt_func);
    procs[pid].pile[index_int - 2] = (int)(exit_proc_actif);
    procs[pid].pile[index_int - 1] = (int)(arg);
    procs[pid].zone_sauv[1] = (int)(&procs[pid].pile[index_int - 3]);
    procs[pid].parent = getpid();
    for (int n = 0; n < NBPROC; n++)
    {
        procs[pid].fils[n] = -1;
    }
    int f;
    for (f = 0; procs[getpid()].fils[f] != -1; f++)
        ;
    file_procs[proc_actif]->fils[f] = pid;
    file_procs[i] = &procs[pid];
    // On remonte le processus tant que sa priorité
    // est supérieure au processus précédent dans la file
    for (; i >= 1; i--)
    {
        if (file_procs[i - 1] != NULL && file_procs[i] != NULL && file_procs[i - 1]->pid != -1 && file_procs[i - 1]->prio < file_procs[i]->prio)
        {
            if (proc_actif == i)
                proc_actif--;
            else if (proc_actif == i - 1)
                proc_actif++;
            tmp = file_procs[i - 1];
            file_procs[i - 1] = file_procs[i];
            file_procs[i] = tmp;
        }
    }
    for (; i < NBPROC - 1; i++)
    {
        if (file_procs[i + 1] != NULL && file_procs[i] != NULL && file_procs[i + 1]->pid != -1 && file_procs[i + 1]->prio >= file_procs[i]->prio)
        {
            if (proc_actif == i)
                proc_actif++;
            else if (proc_actif == i + 1)
                proc_actif--;
            tmp = file_procs[i + 1];
            file_procs[i + 1] = file_procs[i];
            file_procs[i] = tmp;
        }
    }
    if (prio > file_procs[proc_actif]->prio)
        ordonnance();
    return pid;
}

/**
 * Retourne le premier PID disponible dans le tableau procs
 */
int pidlibre(void)
{
    int pid;
    for (pid = 0; pid < NBPROC && procs[pid].pid != -1; pid++)
        ;
    if (pid >= NBPROC)
        return -1;
    // else
    return pid;
}

/**
 * Retourne le PID du processus courant proc_actif
 */
int getpid(void)
{
    return file_procs[proc_actif]->pid;
}

/**
 * Retourne l'indice i correspondant au processeur de PID donné
 * pid : PID du processus cherché
 */
int getproc(int pid)
{
    if (pid < 0 || pid >= NBPROC || procs[pid].pid < 0) // PID invalide
        return -1;
    int i;
    for (i = 0; i < NBPROC && (file_procs[i] == NULL || file_procs[i]->pid != pid); i++)
        ;
    if (i == NBPROC) // PID invalide
        return -1;
    return i;
}

/**
 * Retourne la priorité du processus de PID donné
 * pid : PID du processus cherché
 */
int getprio(int pid)
{
    int p = getproc(pid);
    if (p == -1)
    {
        return -1; // PID invalide
    }
    return file_procs[p]->prio;
}

/**
 * Donne la main à un nouveau processus
 */
void ordonnance(void)
{
    int old, i;
    if (file_procs[proc_actif] == NULL)
    { // le processus actif s'est suicidé
        old = pid_sauv;
    }
    else
    {
        old = getpid();
    }
    // vérifier qu'il n'y a pas de nouveau processus avec une priorité supérieure
    // ou qu'il n'y a plus de processus activable ou actif de la priorité actuelle
    // (on vérifie avec le premier processus activable ou actif de la liste)
    for (i = 0; i < NBPROC && (file_procs[i] == NULL || (file_procs[i]->etat != ACTIVABLE && file_procs[i]->etat != ACTIF)); i++)
        ;
    if (file_procs[proc_actif] == NULL || file_procs[i]->prio != file_procs[proc_actif]->prio)
    {
        proc_actif = i;
    }
    else
    {
        do
        {
            if (proc_actif >= NBPROC - 1 || (file_procs[proc_actif + 1] != NULL && file_procs[proc_actif + 1]->prio < procs[old].prio))
            {
                proc_actif = 0;
            }
            else
            {
                proc_actif++;
            }
        } while (file_procs[proc_actif] == NULL || file_procs[proc_actif]->prio != procs[old].prio || (file_procs[proc_actif]->etat != ACTIVABLE && file_procs[proc_actif]->etat != ACTIF));
    }
    context_switch(old, getpid());
}

/**
 * Change de contexte de l'ancien processus actif au nouveau
 * old : pid de l'ancien processus actif
 * new : pid du nouveau processus actif
 */
void context_switch(int old, int new)
{
    if (getproc(new) == -1)
        return; // erreur, on switch sur un proocessus mort ou invalide
    if (procs[old].etat == ACTIF)
        procs[old].etat = ACTIVABLE;
    if (procs[new].etat == ACTIVABLE)
        procs[new].etat = ACTIF;
    else
        return; // erreur, on switch sur un processus non activable
    ctx_sw(procs[old].zone_sauv, procs[new].zone_sauv);
}

/**
 * Exécuté quand un processus se termine
 * Récupère la retval du processus et appelle exit(retval)
 */
void exit_proc_actif(void)
{
    int retval = 0;
    __asm__ __volatile__("\t movl %%eax, %0"
                         : "=r"(retval));
    exit(retval);
}

/**
 * Exécuté quand un processus se termine
 * retval : valeur de retour du processus terminé
 */
void exit(int retval)
{
    file_procs[proc_actif]->retval = retval;
    exit_procs(proc_actif);
    /* vérification processus endormi */
    check_if_need_wake_up();
    /* vérification processus attendant un fils */
    check_if_child_is_end();
    /* changement de processus actif */
    ordonnance();
    while (1)
        ;
}

/**
 * Termine un processus
 * processus : indice dans la file du processus à terminer
 */
void exit_procs(int processus)
{
    int pid_fils;
    for (int i = 0; i < NBPROC; i++)
    {
        pid_fils = file_procs[processus]->fils[i];
        if (pid_fils != -1) // c'est un fils
        {
            procs[pid_fils].parent = -1;
            if (procs[pid_fils].etat == ZOMBIE)
            { // le fils est un zombie => il faut le détruire
                exit_procs(getproc(pid_fils));
            }
        }
    }
    if (file_procs[processus]->parent != -1) // le père existe
    {
        file_procs[processus]->etat = ZOMBIE;
    }
    else // il n'y a plus de père
    {
        file_procs[processus]->pid = -1;
        file_procs[processus] = NULL;
    }
}

/**
 * Tue un processus
 * pid : PID du processus tué
 * return : 0 si le processus est tué correctement
 *      ou -1 en cas d'erreur
 */
int kill(int pid)
{
    int proc = getproc(pid);
    bool suicide = pid == getpid();
    if (suicide) // on sauvegarde le pid pour aller chercher la zone de sauvegarde lors du contexte switch à venir
        pid_sauv = pid;
    if (pid <= 0 || proc == -1 || file_procs[proc]->etat == ZOMBIE) // PID invalide
        return -1;
    if (file_procs[proc]->etat == ENDORMI)
    {
        // supprimer de la file d'attente
        for (int i = 0; i < NBPROC; i++)
        {
            if (sleeping_file_procs[i].pid_wait == pid)
            {
                sleeping_file_procs[i].pid_wait = -1;
                sleeping_file_procs[i].clk_wait = -1;
                break;
            }
        }
    }
    if (file_procs[proc]->etat == BLOQUE_FILS)
    {
        // supprimer de la file des pères attendant leurs fils
        for (int i = 0; i < NBPROC; i++)
        {
            if (bloque_fils_file_procs[i].pid_pere == pid)
            {
                bloque_fils_file_procs[i].pid_pere = -1;
                bloque_fils_file_procs[i].pid_fils = -1;
                break;
            }
        }
    }
    file_procs[proc]->retval = 0;
    exit_procs(proc);
    if (suicide)
    {
        ordonnance();
    }
    return 0;
}

/**
 * Un processus père attend un processus fils
 * pid     : PID du processus fils attendu. Si -1, on attend n'importe quel fils
 * retvalp : pointeur dans lequel sera mise la valeur de retour du fils. Peut être NULL
 * return : le PID du fils qui s'est terminé
 *      ou -1 en cas d'erreur
 */
int waitpid(int pid, int *retvalp)
{
    if (pid < 0) // on attend n'importe quel fils
    {
        int i;
        int pid_fils = -1;
        for (i = 0; i < NBPROC && file_procs[proc_actif]->fils[i] < 0; i++)
        {
            pid_fils = file_procs[proc_actif]->fils[i];
            if (pid_fils != -1 && file_procs[getproc(pid_fils)] != NULL && procs[pid_fils].etat == ZOMBIE)
            {
                // il existe un fils qui est déjà fini
                if (retvalp != NULL)
                {
                    *retvalp = procs[pid_fils].retval;
                }
                int proc = getproc(pid_fils);
                file_procs[proc]->pid = -1;
                file_procs[proc] = NULL;
                file_procs[proc_actif]->fils[i] = -1;
                return pid_fils;
            }
        }
        if (i >= NBPROC) // pas de fils existant
            return -1;
        // else : il  existe au moins un fils mais pas de fils terminé
        // on met le père en attente d'un fils
        int j;
        // on cherche la première case vide dans bloque_fils_file_procs
        for (j = 0; bloque_fils_file_procs[j].pid_pere != -1; j++)
            ;
        bloque_fils_file_procs[j].pid_pere = getpid();
        bloque_fils_file_procs[j].pid_fils = pid;
        file_procs[proc_actif]->etat = BLOQUE_FILS;
        ordonnance();
        //  on se réveille et on va chercher le fils terminé
        for (i = 0; i < NBPROC && (pid_fils == -1 || (file_procs[getproc(pid_fils)] != NULL && procs[pid_fils].etat != ZOMBIE)); i++)
        {
            pid_fils = file_procs[proc_actif]->fils[i];
        }
        i--;
        if (retvalp != NULL)
        {
            *retvalp = procs[pid_fils].retval;
        }
        int proc = getproc(pid_fils);
        file_procs[proc]->pid = -1;
        file_procs[proc] = NULL;
        file_procs[proc_actif]->fils[i] = -1;
        return pid_fils;
    }
    else // on attend le fils pid
    {
        int i;
        for (i = 0; i < NBPROC && file_procs[proc_actif]->fils[i] != pid; i++)
            ;
        if (i >= NBPROC) // pas un fils du proc_actif
        {
            return -1;
        }
        int proc = getproc(pid);
        if (proc == -1) // pid invalide
        {
            return -1;
        }
        if (file_procs[proc]->etat != ZOMBIE)
        { // on met le père en attente de son fils
            int j;
            for (j = 0; bloque_fils_file_procs[j].pid_pere != -1; j++)
                ;
            bloque_fils_file_procs[j].pid_pere = getpid();
            bloque_fils_file_procs[j].pid_fils = pid;
            file_procs[proc_actif]->etat = BLOQUE_FILS;
            ordonnance();
        }
        if (retvalp != NULL)
        {
            *retvalp = file_procs[proc]->retval;
        }
        file_procs[proc]->pid = -1;
        file_procs[proc] = NULL;
        file_procs[proc_actif]->fils[i] = -1;
        return pid;
    }
}

/**
 * Change la priorité d'un processus
 * pid     : PID du processus dont on change la priorité
 * newprio : nouvelle priorité donnée au processus
 * return : l'ancienne priorité du processus
 *      ou -1 si PID invalide
 *      ou -2 si newprio invalide
 */
int chprio(int pid, int newprio)
{
    int p = getproc(pid);
    if (p == -1 || file_procs[p]->etat == ZOMBIE)
    {
        return -1; // PID invalide
    }
    if (newprio <= 0 || MAXPRIO < newprio)
    {
        return -2; // newprio invalide
    }
    int oldprio = getprio(pid);
    file_procs[p]->prio = newprio;
    struct processus *tmp;
    // On réajuste la place du processus dans la file d'attente
    for (; p >= 1; p--)
    {
        if (file_procs[p - 1] != NULL && file_procs[p] != NULL && file_procs[p - 1]->pid != -1 && file_procs[p - 1]->prio < file_procs[p]->prio)
        {
            if (proc_actif == p)
                proc_actif--;
            else if (proc_actif == p - 1)
                proc_actif++;
            tmp = file_procs[p - 1];
            file_procs[p - 1] = file_procs[p];
            file_procs[p] = tmp;
        }
    }
    for (; p < NBPROC - 1; p++)
    {
        if (file_procs[p + 1] != NULL && file_procs[p] != NULL && file_procs[p + 1]->pid != -1 && file_procs[p + 1]->prio >= file_procs[p]->prio)
        {
            if (proc_actif == p)
                proc_actif++;
            else if (proc_actif == p + 1)
                proc_actif--;
            tmp = file_procs[p + 1];
            file_procs[p + 1] = file_procs[p];
            file_procs[p] = tmp;
        }
    }
    ordonnance();

    return oldprio;
}