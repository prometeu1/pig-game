#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MAX_JOUEURS 10


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
    int score_max;               
} Game;


void initJoueur(Player *joueur, const char *nom, int est_IA) {
    strncpy(joueur->nom, nom, sizeof(joueur->nom) - 1);
    joueur->nom[sizeof(joueur->nom) - 1] = '\0';  
    joueur->banque = 0;
    joueur->points_tour = 0;
    joueur->est_IA = est_IA;
}


void initJeu(Game *jeu, int nb_joueurs, int score_max) {
    jeu->nb_joueurs = nb_joueurs;
    jeu->score_max = score_max;
    jeu->joueur_courant = 0; 

   
    for (int i = 0; i < nb_joueurs; i++) {
        char nom[50];
        int est_IA;

        if (i % 2 == 0) {  
            printf("Entrez le nom du joueur %d: ", i + 1);
            fgets(nom, sizeof(nom), stdin);
            nom[strcspn(nom, "\n")] = '\0';  
            est_IA = 0;  
        } else {
            snprintf(nom, sizeof(nom), "IA %d", i + 1);
            est_IA = 1;  
        }

        initJoueur(&jeu->joueurs[i], nom, est_IA);
    }
}

int main() {
    Game jeu;

    printf("Bienvenue au jeu de cochons!\n");
    int nb_joueurs = 2;  
    int score_max = 100;  

    initJeu(&jeu, nb_joueurs, score_max);

    printf("Le jeu commence!\n");
    for (int i = 0; i < jeu.nb_joueurs; i++) {
        printf("Joueur %d: %s\n", i + 1, jeu.joueurs[i].nom);
    }

    return 0;
}
