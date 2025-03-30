#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MAX_JOUEURS 10
#define SCORE_MAX 100


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


void initJeu(Game *jeu) {
jeu->nb_joueurs = 2;
jeu->joueur_courant = 0;

char nom[50];
printf("Bienvenue au jeu de cochons!\n");
printf("Entrez le nom du joueur 1: ");
fgets(nom, sizeof(nom), stdin);
nom[strcspn(nom, "\n")] = '\0'; 
initJoueur(&jeu->joueurs[0], nom, 0); 

initJoueur(&jeu->joueurs[1], "IA 2", 1); 
}


void tourHumain(Player *joueur) {
joueur->points_tour = 0;
char choix;

while (1) {
int lancer = lancerDe();
printf("Vous avez lancé : %d\n", lancer);

if (lancer == 1) {
printf("Pas de chance ! Vous perdez les points de ce tour.\n");
joueur->points_tour = 0;
break;
} else {
joueur->points_tour += lancer;
printf("Vous avez %d points ce tour, et %d en banque.\n", joueur->points_tour, joueur->banque);
printf("Continuer à lancer ? [r]oll ou [b]ank: ");
scanf(" %c", &choix);
getchar(); 

if (choix == 'b') {
joueur->banque += joueur->points_tour;
printf("Vous avez %d points en banque.\n", joueur->banque);
break;
}
}
}
}


void tourIA(Player *joueur) {
joueur->points_tour = 0;
printf("%s joue...\n", joueur->nom);

while (1) {
int lancer = lancerDe();
printf(" %s a lancé : %d\n", joueur->nom, lancer);

if (lancer == 1) {
printf(" Pas de chance pour %s !\n", joueur->nom);
joueur->points_tour = 0;
break;
} else {
joueur->points_tour += lancer;
printf(" %s a %d points ce tour, et %d en banque.\n", joueur->nom, joueur->points_tour, joueur->banque);
if (joueur->points_tour >= 15) {
joueur->banque += joueur->points_tour;
printf(" %s décide de banker. Nouveau total : %d points.\n", joueur->nom, joueur->banque);
break;
}
}
}
}

int main() {
srand(time(NULL));
Game jeu;
initJeu(&jeu);

while (1) {
Player *joueur = &jeu.joueurs[jeu.joueur_courant];

printf("\n==> C’est le tour de %s\n", joueur->nom);

if (joueur->est_IA) {
tourIA(joueur);
} else {
tourHumain(joueur);
}


if (joueur->banque >= SCORE_MAX) {
printf("\n%s a gagné avec %d points ! Bravo !\n", joueur->nom, joueur->banque);
break;
}


jeu.joueur_courant = (jeu.joueur_courant + 1) % jeu.nb_joueurs;
}

return 0;
}