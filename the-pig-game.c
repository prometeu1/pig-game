#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAX_JOUEURS 10
#define SCORE_MAX 100

// Structure pour un joueur
typedef struct {
char nom[50];
int banque;
int points_tour;
int est_IA;
} Player;

// Structure pour le jeu
typedef struct {
Player joueurs[MAX_JOUEURS];
int nb_joueurs;
int joueur_courant;
} Game;

// Lancer de dé
int lancerDe() {
return rand() % 6 + 1;
}

// Initialisation d’un joueur
void initJoueur(Player *joueur, const char *nom, int est_IA) {
strncpy(joueur->nom, nom, sizeof(joueur->nom) - 1);
joueur->nom[sizeof(joueur->nom) - 1] = '\0';
joueur->banque = 0;
joueur->points_tour = 0;
joueur->est_IA = est_IA;
}

// Initialisation du jeu avec N joueurs
void initJeu(Game *jeu) {
printf("Bienvenue au jeu de cochons !\n");

do {
printf("Combien de joueurs (2 à %d) ? ", MAX_JOUEURS);
scanf("%d", &jeu->nb_joueurs);
getchar(); // consomme le \n
} while (jeu->nb_joueurs < 2 || jeu->nb_joueurs > MAX_JOUEURS);

int nb_humains;
do {
printf("Combien de joueurs humains ? ");
scanf("%d", &nb_humains);
getchar(); // consomme le \n
} while (nb_humains < 0 || nb_humains > jeu->nb_joueurs);

jeu->joueur_courant = 0;

for (int i = 0; i < jeu->nb_joueurs; i++) {
char nom[50];

if (i < nb_humains) {
printf("Nom du joueur humain %d : ", i + 1);
fgets(nom, sizeof(nom), stdin);
nom[strcspn(nom, "\n")] = '\0'; // Enlève le \n
initJoueur(&jeu->joueurs[i], nom, 0);
} else {
snprintf(nom, sizeof(nom), "IA %d", i - nb_humains + 1);
initJoueur(&jeu->joueurs[i], nom, 1);
}
}
}

// Affiche les scores
void afficherScores(Game *jeu) {
printf("\n--- Scores actuels ---\n");
for (int i = 0; i < jeu->nb_joueurs; i++) {
printf("%s : %d points\n", jeu->joueurs[i].nom, jeu->joueurs[i].banque);
}
printf("----------------------\n");
}

// Tour du joueur humain
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
printf("Vous avez maintenant %d points en banque.\n", joueur->banque);
break;
}
}
}
}

// Tour IA avec stratégie
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
printf(" %s décide de banker. Total en banque : %d\n", joueur->nom, joueur->banque);
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
sleep(1);
} else {
tourHumain(joueur);
}

afficherScores(&jeu);

if (joueur->banque >= SCORE_MAX) {
printf("\n%s a gagné avec %d points ! Félicitations !\n", joueur->nom, joueur->banque);
break;
}

jeu.joueur_courant = (jeu.joueur_courant + 1) % jeu.nb_joueurs;
}

return 0;
}