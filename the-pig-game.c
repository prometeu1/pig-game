#include <stdio.h>      // Pour les fonctions d'entrée/sortie standard
#include <stdlib.h>     // Pour les fonctions comme rand() et system()
#include <string.h>     // Pour les opérations sur les chaînes de caractères
#include <time.h>       // Pour initialiser le générateur de nombres aléatoires
#include <ctype.h>      // Pour les fonctions de test et conversion de caractères

#ifdef _WIN32          // Vérif si compilation sous Windows
#include <windows.h>    // Inclut les API Windows
#include <process.h>    // Pour les fonctions liées aux processus sous Windows
#define sleep(seconds) Sleep(seconds * 1000)  // Adapte sleep pour Windows (ms -> s)
#define getpid() _getpid()                    // Adapte getpid pour Windows
#else                  // Si pas sous Windows, on utilise les headers UNIX
#include <unistd.h>     // Pour sleep() et getpid() sous UNIX
#endif

#define MAX_PLAYERS 10  // Nb max de joueurs autorisés
#define MAX_SCORE 100   // Score à atteindre pour gagner
#define SCORES_FILE "scores.txt"  // Fichier où sauvegarder les scores
#define MAX_NAME 12     // Longueur max pour les noms des joueurs

// Codes de couleur ANSI pour rendre le jeu plus joli
#define RESET "\033[0m"      // Réinitialise la couleur
#define RED "\033[31m"       // Rouge pour les erreurs
#define GREEN "\033[32m"     // Vert pour les succès
#define YELLOW "\033[33m"    // Jaune pour les questions
#define BLUE "\033[34m"      // Bleu pour les noms
#define CYAN "\033[36m"      // Cyan pour les infos
#define BOLD "\033[1m"       // Texte en gras

// Structure pour stocker les infos d'un joueur
typedef struct {
    char name[50];       // Nom du joueur
    int bank;            // Points accumulés (sécurisés)
    int turn_points;     // Points accumulés pendant le tour actuel (non sécurisés)
    int is_AI;           // 1 si IA, 0 si humain
} Player;

// Structure pour gérer l'état du jeu
typedef struct {
    Player players[MAX_PLAYERS];  // Tableau des joueurs
    int num_players;              // Nb de joueurs dans la partie
    int current_player;           // Index du joueur actuel
} Game;

// Lance un dé (1 à 6)
int rollDie() {
    return rand() % 6 + 1;  // Génère un nb aléatoire entre 1 et 6
}

// Initialise un joueur avec ses valeurs de départ
void initPlayer(Player *player, const char *name, int is_AI) {
    strncpy(player->name, name, sizeof(player->name) - 1);  // Copie le nom avec protection
    player->name[sizeof(player->name) - 1] = '\0';  // S'assure que la chaîne est bien terminée
    player->bank = 0;        // Commence avec 0 point en banque
    player->turn_points = 0; // Commence avec 0 point pour le tour
    player->is_AI = is_AI;   // Définit si c'est une IA ou non
}

// Vérifie si le nom commence par "IA" (insensible à la casse)
int startsWithAI(const char *name) {
    return (tolower(name[0]) == 'i' && tolower(name[1]) == 'a');  // Compare les 2 premiers caractères
}

// Vérifie si un nom est déjà pris par un autre joueur
int isNameTaken(Game *game, const char *name, int exclude_index) {
    for (int i = 0; i < game->num_players; i++) {  // Parcourt tous les joueurs
        if (i != exclude_index && strcmp(game->players[i].name, name) == 0) {  // Compare les noms
            return 1;  // Nom déjà pris
        }
    }
    return 0;  // Nom disponible
}

// Lit un nombre avec validation entre min et max
int readNumber(int min, int max, const char *message) {
    int number;       // Pour stocker le nombre saisi
    char buffer[100]; // Buffer pour lire l'entrée
    do {
        printf("%s", message);  // Affiche le message de demande
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {  // Lit l'entrée
            printf(RED "Erreur de lecture.\n" RESET);  // Gère l'erreur de lecture
            continue;  // Redemande
        }
        if (sscanf(buffer, "%d", &number) == 1 && number >= min && number <= max) {  // Vérifie validité
            return number;  // Retourne le nb si valide
        }
        printf(RED "Entrée invalide. Veuillez entrer un nombre entre %d et %d.\n" RESET, min, max);  // Message d'erreur
    } while (1);  // Boucle jusqu'à entrée valide
}

// Vide le buffer d'entrée (utile après fgets)
void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);  // Lit tous les caractères jusqu'au prochain '\n'
}

// Initialise le jeu avec les joueurs
void initGame(Game *game, int argc, char *argv[]) {
    printf(BOLD BLUE "\n=== Bienvenue dans le jeu de cochons ! ===\n" RESET);  // Message d'accueil
    if (argc > 1) {  // Mode ligne de commande
        game->num_players = argc - 1;  // Nb de joueurs = nb d'args - 1
        if (game->num_players > MAX_PLAYERS) {  // Vérifie si trop de joueurs
            printf(RED "Trop de joueurs ! Maximum : %d\n" RESET, MAX_PLAYERS);
            exit(1);  // Quitte avec erreur
        }
        for (int i = 0; i < game->num_players; i++) {  // Pour chaque joueur
            const char *name = argv[i + 1];  // Récupère le nom depuis les args
            if (strlen(name) > MAX_NAME || strlen(name) == 0 || isNameTaken(game, name, -1)) {  // Vérifie validité
                printf(RED "Nom invalide ou déjà pris : %s\n" RESET, name);
                exit(1);  // Quitte avec erreur
            }
            int is_AI = startsWithAI(name);  // Détecte si c'est une IA
            initPlayer(&game->players[i], name, is_AI);  // Initialise le joueur
        }
    } else {  // Mode interactif
        game->num_players = readNumber(2, MAX_PLAYERS, YELLOW "Combien de joueurs (2 à 10) ? " RESET);  // Demande nb joueurs
        int num_humans = readNumber(0, game->num_players, YELLOW "Combien de joueurs humains ? " RESET);  // Demande nb humains
        for (int i = 0; i < game->num_players; i++) {  // Pour chaque joueur
            char name[50];  // Buffer pour le nom
            if (i < num_humans) {  // Si c'est un humain
                do {
                    printf(YELLOW "Nom du joueur humain %d (max %d caractères) : " RESET, i + 1, MAX_NAME);  // Demande nom
                    if (fgets(name, sizeof(name), stdin) == NULL) {  // Lit le nom
                        printf(RED "Erreur de lecture.\n" RESET);  // Gère erreur
                        continue;  // Redemande
                    }
                    name[strcspn(name, "\n")] = '\0';  // Enlève le \n de la fin
                    if (strlen(name) == 0) {  // Vérifie que pas vide
                        printf(RED "Le nom ne peut pas être vide.\n" RESET);
                    } else if (strlen(name) > MAX_NAME) {  // Vérifie longueur
                        printf(RED "Nom trop long ! Maximum : %d caractères.\n" RESET, MAX_NAME);
                    } else if (isNameTaken(game, name, i)) {  // Vérifie unicité
                        printf(RED "Ce nom est déjà pris.\n" RESET);
                    } else {
                        break;  // Nom OK, sort de la boucle
                    }
                } while (1);  // Continue jusqu'à nom valide
                initPlayer(&game->players[i], name, 0);  // Initialise joueur humain
            } else {  // Si c'est une IA
                snprintf(name, sizeof(name), "IA%d", i - num_humans + 1);  // Génère un nom auto (IA1, IA2...)
                initPlayer(&game->players[i], name, 1);  // Initialise joueur IA
            }
        }
    }
    game->current_player = 0;  // Premier joueur commence
}

// Affiche les scores actuels
void displayScores(Game *game) {
    printf(BOLD CYAN "\n--- Scores actuels ---\n" RESET);  // Titre
    for (int i = 0; i < game->num_players; i++) {  // Pour chaque joueur
        printf("%s%-10s%s : %s%3d points%s\n", BLUE, game->players[i].name, RESET, YELLOW, game->players[i].bank, RESET);  // Affiche score
    }
    printf(CYAN "------------------------\n" RESET);  // Ligne de séparation
}

// Gère le tour d'un joueur humain
void humanTurn(Player *player) {
    player->turn_points = 0;  // Commence avec 0 point pour ce tour
    char buffer[10];  // Buffer pour les entrées
    while (1) {  // Boucle de lancer de dé
        int roll = rollDie();  // Lance le dé
        printf(BOLD "\nVous avez lancé : " GREEN "%d" RESET "\n", roll);  // Affiche résultat
        if (roll == 1) {  // Si on fait 1
            printf(RED "Pas de chance ! Vous perdez les points de ce tour.\n" RESET);  // Message de perte
            player->turn_points = 0;  // Perd tous les points du tour
            break;  // Fin du tour
        }
        player->turn_points += roll;  // Ajoute les points du dé
        printf(CYAN "Points ce tour : " YELLOW "%d" CYAN ", Banque : " YELLOW "%d\n" RESET, player->turn_points, player->bank);  // Affiche état
        do {
            printf(YELLOW "Continuer ? [r]oll / [b]ank : " RESET);  // Demande choix
            if (fgets(buffer, sizeof(buffer), stdin) == NULL) {  // Lit choix
                printf(RED "Erreur de lecture.\n" RESET);  // Gère erreur
                continue;  // Redemande
            }
            char choice = tolower(buffer[0]);  // Prend 1er caractère (minuscule)
            if (choice == 'b') {  // Si banque
                player->bank += player->turn_points;  // Ajoute points à la banque
                printf(GREEN "Points ajoutés à votre banque : %d\n" RESET, player->turn_points);  // Confirme
                return;  // Fin du tour
            } else if (choice != 'r') {  // Si ni 'r' ni 'b'
                printf(RED "Choix invalide. Entrez 'r' ou 'b'.\n" RESET);  // Erreur
            } else {  // Si 'r' (relancer)
                break;  // Sort de la boucle de choix
            }
        } while (1);  // Continue jusqu'à choix valide
    }
}

// Gère le tour d'une IA
void aiTurn(Player *player, Game *game) {
    player->turn_points = 0;  // Commence avec 0 point pour ce tour
    printf(BOLD CYAN "\n===========================\n" RESET);  // Séparateur
    printf(BOLD " Tour de l'IA : " BLUE "%s\n" RESET, player->name);  // Annonce tour IA
    printf(BOLD CYAN "===========================\n" RESET);  // Séparateur
    sleep(1);  // Pause pour effet
    while (1) {  // Boucle de lancer
        int roll = rollDie();  // Lance le dé
        printf(" %s a lancé : " GREEN "%d\n" RESET, player->name, roll);  // Affiche résultat
        sleep(1);  // Pause pour effet
        if (roll == 1) {  // Si fait 1
            printf(RED " Pas de chance pour %s !\n" RESET, player->name);  // Message de perte
            player->turn_points = 0;  // Perd les points du tour
            break;  // Fin du tour
        }
        player->turn_points += roll;  // Ajoute points du dé
        int total = player->bank + player->turn_points;  // Calcul total potentiel
        int best_other = 0;  // Pour stocker meilleur score adversaire
        for (int i = 0; i < game->num_players; i++) {  // Cherche meilleur score adverse
            if (&game->players[i] != player && game->players[i].bank > best_other) {
                best_other = game->players[i].bank;  // Met à jour meilleur score
            }
        }
        printf(" Points ce tour : " YELLOW "%d" RESET ", Total provisoire : " YELLOW "%d\n" RESET, player->turn_points, total);  // Affiche état
        sleep(1);  // Pause pour effet
        // Stratégie de l'IA (avec commentaires sur chaque condition)
        if (player->bank >= 85 && player->turn_points >= 3) {  // Proche de la victoire
            printf(GREEN " %s sécurise la victoire !\n" RESET, player->name);  // Explique décision
            player->bank += player->turn_points;  // Banque les points
            break;  // Fin du tour
        } else if (player->bank > best_other + 30 && player->turn_points >= 10) {  // Grande avance
            printf(GREEN " %s ne prend pas de risque.\n" RESET, player->name);  // Explique décision
            player->bank += player->turn_points;  // Banque les points
            break;  // Fin du tour
        } else if (player->turn_points >= 15) {  // Bon gain pour ce tour
            printf(GREEN " %s banque normalement.\n" RESET, player->name);  // Explique décision
            player->bank += player->turn_points;  // Banque les points
            break;  // Fin du tour
        }
    }
    sleep(1);  // Pause avant joueur suivant
}

// Compare les scores pour le tri du classement
int compareScores(const void *a, const void *b) {
    const Player *pa = *(const Player **)a;  // Convertit en Player* 
    const Player *pb = *(const Player **)b;  // Idem
    return pb->bank - pa->bank;  // Compare scores (ordre décroissant)
}

// Sauvegarde le score du gagnant dans un fichier
void saveScore(Player *winner) {
    FILE *f = fopen(SCORES_FILE, "a");  // Ouvre fichier en mode ajout
    if (f) {  // Si ouverture OK
        fprintf(f, "%s a gagné avec %d points\n", winner->name, winner->bank);  // Écrit ligne de score
        fclose(f);  // Ferme fichier
    } else {  // Si pb ouverture
        printf(RED "Erreur : Impossible d'ouvrir le fichier des scores.\n" RESET);  // Message erreur
    }
}

// Affiche le résultat final et le classement
void displayGameEnd(Game *game, int winner_index) {
    printf(BOLD GREEN "\n=== FIN DE PARTIE ===\n" RESET);  // Annonce fin
    printf(BOLD "Gagnant : " BLUE "%s" RESET " avec " YELLOW "%d points\n" RESET,
           game->players[winner_index].name, game->players[winner_index].bank);  // Affiche gagnant
    Player *ranking[MAX_PLAYERS];  // Tableau pour trier joueurs
    for (int i = 0; i < game->num_players; i++) {  // Remplit tableau
        ranking[i] = &game->players[i];  // Stocke les pointeurs vers joueurs
    }
    qsort(ranking, game->num_players, sizeof(Player *), compareScores);  // Trie par score
    printf(BOLD CYAN "\n--- Classement final ---\n" RESET);  // Titre classement
    for (int i = 0; i < game->num_players; i++) {  // Pour chaque joueur
        printf("%d. %s%-10s%s : " YELLOW "%d pts\n" RESET, i + 1, BLUE, ranking[i]->name, RESET, ranking[i]->bank);  // Affiche rang et score
    }
    saveScore(&game->players[winner_index]);  // Sauvegarde le score gagnant
}

// Active le support des couleurs ANSI sous Windows
#ifdef _WIN32
void enableANSISupport() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);  // Récupère handle console
    if (hOut == INVALID_HANDLE_VALUE) return;  // Si erreur, abandonne
    DWORD dwMode = 0;  // Mode console
    if (!GetConsoleMode(hOut, &dwMode)) return;  // Récupère mode actuel
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;  // Active support ANSI
    SetConsoleMode(hOut, dwMode);  // Applique nouveau mode
}
#endif

// Fonction principale
int main(int argc, char *argv[]) {
    #ifdef _WIN32  // Si Windows
    enableANSISupport();  // Active les couleurs ANSI
    #endif

    srand((unsigned)time(NULL) ^ (unsigned)getpid());  // Init générateur aléatoire
    char replay;  // Pour stocker choix de rejouer
    do {  // Boucle de parties
        Game game;  // Crée structure de jeu
        initGame(&game, argc, argv);  // Initialise le jeu
        printf(BOLD CYAN "\n--- Début de la partie ---\n" RESET);  // Annonce début
        displayScores(&game);  // Affiche scores de départ
        while (1) {  // Boucle principale de jeu
            Player *player = &game.players[game.current_player];  // Joueur actuel
            printf(BOLD "\n>>> " CYAN "Nouveau tour de " BLUE "%s" RESET "\n", player->name);  // Annonce joueur
            if (player->is_AI) {  // Si IA
                aiTurn(player, &game);  // Tour IA
            } else {  // Si humain
                humanTurn(player);  // Tour humain
            }
            displayScores(&game);  // Affiche scores après le tour
            if (player->bank >= MAX_SCORE) {  // Si score gagnant atteint
                displayGameEnd(&game, game.current_player);  // Affiche fin de partie
                break;  // Sort de la boucle principale
            }
            game.current_player = (game.current_player + 1) % game.num_players;  // Passe au joueur suivant
            printf("\n");  // Ligne vide pour lisibilité
        }
        do {  // Boucle pour demander si rejouer
            char buffer[10];  // Buffer entrée
            printf(YELLOW "\nVoulez-vous rejouer avec les mêmes joueurs ? (o/n) : " RESET);  // Demande choix
            if (fgets(buffer, sizeof(buffer), stdin) == NULL) {  // Lit choix
                printf(RED "Erreur de lecture.\n" RESET);  // Gère erreur
                continue;  // Redemande
            }
            replay = tolower(buffer[0]);  // Prend 1er caractère (minuscule)
            if (replay != 'o' && replay != 'n') {  // Si réponse invalide
                printf(RED "Choix invalide. Entrez 'o' ou 'n'.\n" RESET);  // Message erreur
            }
        } while (replay != 'o' && replay != 'n');  // Continue jusqu'à réponse valide
    } while (replay == 'o');  // Rejoue si choix = oui

    printf(BOLD BLUE "\nMerci d'avoir joué ! À bientôt !\n\n" RESET);  // Message final
    return 0;  // Fin du programme
}
