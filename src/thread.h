#ifndef __THREAD_H__
#define __THREAD_H__

/* identifiant de thread
 * NB: pourra être un entier au lieu d'un pointeur si ca vous arrange,
 *     mais attention aux inconvénient des tableaux de threads
 *     (consommation mémoire, cout d'allocation, ...).
 */

typedef struct thread * thread_t;

/* recuperer l'identifiant du thread courant.
 */
extern thread_t thread_self(void);

/* creer un nouveau thread qui va exécuter la fonction func avec
 * l'argument funcarg.  renvoie 0 en cas de succès, -1 en cas
 * d'erreur.
 */
extern int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg);

/* passer la main à un autre thread.
 */
extern int thread_yield(void);

/* attendre la fin d'exécution d'un thread.
 * la valeur renvoyée par le thread est placée dans *retval.
 * si retval est NULL, la valeur de retour est ignorée.
 */
extern int thread_join(thread_t thread, void **retval);

/* terminer le thread courant en renvoyant la valeur de retour retval.
 * cette fonction ne retourne jamais.
 */
extern void thread_exit(void *retval)__attribute__ ((__noreturn__));


/* envoyer un signal a un thread
 */
void thread_kill (thread_t thr, int sig);

/* attribuer une fonction de traitement a un signal
*/
void thread_signal(thread_t thr, int sig, void(*sig_func)(int));

/* initialiser la table de traitement des signaux
*/
void thread_initSigTab(thread_t thr);

/* traiter les signaux d'un thread
*/
void thread_sigTreat(thread_t thr);
#endif /* __THREAD_H__ */
