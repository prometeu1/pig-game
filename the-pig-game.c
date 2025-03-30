#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int lancerDe() {
    static int initialized = 0;
    if (!initialized) {
        srand(time(NULL));
        initialized = 1;
    }

    return rand() % 6 + 1;
}

int main() {
    int resultat;

    printf("Le jeu de cochons\n");
    printf("Appuyez sur Entrée pour lancer le dé");
    getchar(); 

    resultat = lancerDe();
    printf("Vous avez lancé : %d\n", resultat);

    if (resultat == 1) {
        printf("Dommage ! Vous perdez tous vos points pour ce tour.\n");
    } else {
        printf("Vous avez gagné %d points pour ce tour.\n", resultat);
    }

    return 0;
}
