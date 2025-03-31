#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h> 
#include <ctype.h>
#define MAX_JOUEURS 10
#define SCORE_MAX 100
#define FICHIER_SCORES "scores.txt"

#define RESET "\033[0m"
#define ROUGE "\033[31m"
#define VERT "\033[32m"
#define JAUNE "\033[33m"
#define BLEU "\033[34m"
#define CYAN "\033[36m"
#define GRAS "\033[1m"

typedef struct {
    char nom[50];
    int banque;
    int points_tour;
    int est_IA;
} Player;
typedef struct {
    Player joueurs[MAX_JOUEURS];
    int nb_joueurs;
    int joueur_courant;
} Game;
int lancerDe() {
    return rand() % 6 + 1;
}
void initJoueur(Player *joueur, const char *nom, int est_IA) {
    strncpy(joueur->nom, nom, sizeof(joueur->nom) - 1);
    joueur->nom[sizeof(joueur->nom) - 1] = '\0';
    joueur->banque = 0;
    joueur->points_tour = 0;
    joueur->est_IA = est_IA;
}
int commenceParIA(const char *nom) {
    return (tolower(nom[0]) == 'i' && tolower(nom[1]) == 'a');
}
void initJeu(Game *jeu, int argc, char *argv[]) {
    printf(GRAS BLEU "\n=== Bienvenue dans le jeu de cochons ! ===\n" RESET);
    if (argc > 1) {
        jeu->nb_joueurs = argc - 1;
        if (jeu->nb_joueurs > MAX_JOUEURS) {
            printf(ROUGE "Trop de joueurs ! Maximum : %d\n" RESET, MAX_JOUEURS);
            exit(1);
        }
        for (int i = 0; i < jeu->nb_joueurs; i++) {
            const char *nom = argv[i + 1];
            int est_IA = commenceParIA(nom);
            initJoueur(&jeu->joueurs[i], nom, est_IA);
        }
    } else {
        do {
            printf(JAUNE "Combien de joueurs (2 à %d) ? " RESET, MAX_JOUEURS);
            scanf("%d", &jeu->nb_joueurs);
            getchar();
        } while (jeu->nb_joueurs < 2 || jeu->nb_joueurs > MAX_JOUEURS);
        int nb_humains;
        do {
            printf(JAUNE "Combien de joueurs humains ? " RESET);
            scanf("%d", &nb_humains);
            getchar();
        } while (nb_humains < 0 || nb_humains > jeu->nb_joueurs);
        for (int i = 0; i < jeu->nb_joueurs; i++) {
            char nom[50];
            if (i < nb_humains) {
                printf(JAUNE "Nom du joueur humain %d (max 12 caractères) : " RESET, i + 1);
                fgets(nom, sizeof(nom), stdin);
                nom[strcspn(nom, "\n")] = '\0';
                if (strlen(nom) > 12) {
                    nom[12] = '\0';
                    printf(ROUGE "Nom trop long, il a été tronqué à 12 caractères.\n" RESET);
                    int c;
                    while ((c = getchar()) != '\n' && c != EOF); // vider le buffer
                }
                initJoueur(&jeu->joueurs[i], nom, 0);
            } else {
                snprintf(nom, sizeof(nom), "IA %d", i - nb_humains + 1);
                initJoueur(&jeu->joueurs[i], nom, 1);
            }
        }
    }
    jeu->joueur_courant = 0;
}
void afficherScores(Game *jeu) {
    printf(GRAS CYAN "\n--- Scores actuels ---\n" RESET);
    for (int i = 0; i < jeu->nb_joueurs; i++) {
        printf("%s%-10s%s : %s%3d points%s\n", BLEU, jeu->joueurs[i].nom, RESET, JAUNE, jeu->joueurs[i].banque, RESET);
    }
    printf(CYAN "------------------------\n" RESET);
}
void tourHumain(Player *joueur) {
    joueur->points_tour = 0;
    char choix;
    while (1) {
        int lancer = lancerDe();
        printf(GRAS "\nVous avez lancé : " VERT "%d" RESET "\n", lancer);
        if (lancer == 1) {
            printf(ROUGE "Pas de chance ! Vous perdez les points de ce tour.\n" RESET);
            joueur->points_tour = 0;
            break;
        } else {
            joueur->points_tour += lancer;
            printf(CYAN "Points ce tour : " JAUNE "%d" CYAN ", Banque : " JAUNE "%d\n" RESET, joueur->points_tour, joueur->banque);
            printf(JAUNE "Continuer ? [r]oll / [b]ank : " RESET);
            scanf(" %c", &choix);
            getchar();
            if (choix == 'b') {
                joueur->banque += joueur->points_tour;
                printf(VERT "Points ajoutés à votre banque : %d\n" RESET, joueur->points_tour);
                break;
            }
        }
    }
}
void tourIA(Player *joueur, Game *jeu) {
    joueur->points_tour = 0;
    printf(GRAS CYAN "\n===========================\n" RESET);
    printf(GRAS " Tour de l’IA : " BLEU "%s\n" RESET, joueur->nom);
    printf(GRAS CYAN "===========================\n" RESET);
    sleep(1);
    while (1) {
        int lancer = lancerDe();
        printf(" %s a lancé : " VERT "%d\n" RESET, joueur->nom, lancer);
        sleep(1);
        if (lancer == 1) {
            printf(ROUGE " Pas de chance pour %s !\n" RESET, joueur->nom);
            joueur->points_tour = 0;
            break;
        }
        joueur->points_tour += lancer;
        int total = joueur->banque + joueur->points_tour;
        int meilleur_autre = 0;
        for (int i = 0; i < jeu->nb_joueurs; i++) {
            if (&jeu->joueurs[i] != joueur && jeu->joueurs[i].banque > meilleur_autre) {
                meilleur_autre = jeu->joueurs[i].banque;
            }
        }
        printf(" Points ce tour : " JAUNE "%d" RESET ", Total provisoire : " JAUNE "%d\n" RESET, joueur->points_tour, total);
        sleep(1);
        if (joueur->banque >= 85 && joueur->points_tour >= 3) {
            printf(VERT " %s sécurise la victoire !\n" RESET, joueur->nom);
            joueur->banque += joueur->points_tour;
            break;
        } else if (joueur->banque > meilleur_autre + 30 && joueur->points_tour >= 10) {
            printf(VERT " %s ne prend pas de risque.\n" RESET, joueur->nom);
            joueur->banque += joueur->points_tour;
            break;
        } else if (joueur->points_tour >= 15) {
            printf(VERT " %s banque normalement.\n" RESET, joueur->nom);
            joueur->banque += joueur->points_tour;
            break;
        }
    }
    sleep(1);
}
int comparerScores(const void *a, const void *b) {
    const Player *pa = *(const Player **)a;
    const Player *pb = *(const Player **)b;
    return pb->banque - pa->banque;
}
void sauvegarderScore(Player *gagnant) {
    FILE *f = fopen(FICHIER_SCORES, "a");
    if (f) {
        fprintf(f, "%s a gagné avec %d points\n", gagnant->nom, gagnant->banque);
        fclose(f);
    }
}
void afficherFinDePartie(Game *jeu, int gagnant_index) {
    printf(GRAS VERT "\n=== FIN DE PARTIE ===\n" RESET);
    printf(GRAS "Gagnant : " BLEU "%s" RESET " avec " JAUNE "%d points\n" RESET,
           jeu->joueurs[gagnant_index].nom,
           jeu->joueurs[gagnant_index].banque);
    Player *classement[MAX_JOUEURS];
    for (int i = 0; i < jeu->nb_joueurs; i++) {
        classement[i] = &jeu->joueurs[i];
    }
    qsort(classement, jeu->nb_joueurs, sizeof(Player *), comparerScores);
    printf(GRAS CYAN "\n--- Classement final ---\n" RESET);
    for (int i = 0; i < jeu->nb_joueurs; i++) {
        printf("%d. %s%-10s%s : " JAUNE "%d pts\n" RESET, i + 1, BLEU, classement[i]->nom, RESET, classement[i]->banque);
    }
    sauvegarderScore(&jeu->joueurs[gagnant_index]);
}
int main(int argc, char *argv[]) {
    srand(time(NULL));
    char rejouer = 'o';
    while (rejouer == 'o' || rejouer == 'O') {
        Game jeu;
        initJeu(&jeu, argc, argv);
        printf(GRAS CYAN "\n--- Début de la partie ---\n" RESET);
        afficherScores(&jeu);
        while (1) {
            Player *joueur = &jeu.joueurs[jeu.joueur_courant];
            printf(GRAS "\n>>> " CYAN "Nouveau tour de " BLEU "%s" RESET "\n", joueur->nom);
            if (joueur->est_IA) {
                tourIA(joueur, &jeu);
            } else {
                tourHumain(joueur);
            }
            afficherScores(&jeu);
            if (joueur->banque >= SCORE_MAX) {
                afficherFinDePartie(&jeu, jeu.joueur_courant);
                break;
            }
            jeu.joueur_courant = (jeu.joueur_courant + 1) % jeu.nb_joueurs;
            printf("\n");
        }
        printf(JAUNE "\nVoulez-vous rejouer avec les mêmes joueurs ? (o/n) : " RESET);
        scanf(" %c", &rejouer);
        getchar();
    }
    printf(GRAS BLEU "\nMerci d'avoir joué ! À bientôt !\n\n" RESET);
    return 0;
}