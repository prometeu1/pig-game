#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>

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

int commenceParIA(const char *nom) {
return (tolower(nom[0]) == 'i' && tolower(nom[1]) == 'a');
}

void initJeu(Game *jeu, int argc, char *argv[]) {
if (argc > 1) {
jeu->nb_joueurs = argc - 1;
if (jeu->nb_joueurs > MAX_JOUEURS) {
printf("Trop de joueurs ! Maximum autorisé : %d\n", MAX_JOUEURS);
exit(1);
}
for (int i = 0; i < jeu->nb_joueurs; i++) {
const char *nom = argv[i + 1];
int est_IA = commenceParIA(nom);
initJoueur(&jeu->joueurs[i], nom, est_IA);
}
} else {
do {
printf("Combien de joueurs (2 à %d) ? ", MAX_JOUEURS);
scanf("%d", &jeu->nb_joueurs);
getchar();
} while (jeu->nb_joueurs < 2 || jeu->nb_joueurs > MAX_JOUEURS);

int nb_humains;
do {
printf("Combien de joueurs humains ? ");
scanf("%d", &nb_humains);
getchar();
} while (nb_humains < 0 || nb_humains > jeu->nb_joueurs);

for (int i = 0; i < jeu->nb_joueurs; i++) {
char nom[50];
if (i < nb_humains) {
printf("Nom du joueur humain %d : ", i + 1);
fgets(nom, sizeof(nom), stdin);
nom[strcspn(nom, "\n")] = '\0';
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
printf("\n--- Scores actuels ---\n");
for (int i = 0; i < jeu->nb_joueurs; i++) {
printf("%s : %d points\n", jeu->joueurs[i].nom, jeu->joueurs[i].banque);
}
printf("----------------------\n");
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
printf("Vous avez maintenant %d points en banque.\n", joueur->banque);
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
printf(" %s décide de banker. Total en banque : %d\n", joueur->nom, joueur->banque);
break;
}
}
}
}

void afficherFinDePartie(Game *jeu, int gagnant_index) {
printf("\n===============================\n");
printf(" Le gagnant est : %s\n", jeu->joueurs[gagnant_index].nom);
printf(" Score final : %d points\n", jeu->joueurs[gagnant_index].banque);
printf("===============================\n");

printf("Classement final :\n");
for (int i = 0; i < jeu->nb_joueurs; i++) {
printf(" - %s : %d points\n", jeu->joueurs[i].nom, jeu->joueurs[i].banque);
}
printf("===============================\n");
}

int main(int argc, char *argv[]) {
srand(time(NULL));
Game jeu;
initJeu(&jeu, argc, argv);

printf("\n--- Début de la partie ---\n");
afficherScores(&jeu);

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
afficherFinDePartie(&jeu, jeu.joueur_courant);
break;
}

jeu.joueur_courant = (jeu.joueur_courant + 1) % jeu.nb_joueurs;
}

return 0;
}