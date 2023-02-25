/*
	Canvas pour algorithmes de jeux à 2 joueurs
	
	joueur 0 : humain
	joueur 1 : ordinateur
			
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// Paramètres du jeu
#define LARGEUR_MAX 7 		// nb max de fils pour un noeud (= nb max de coups possibles)

#define TEMPS 20		// temps de calcul pour un coup avec MCTS (en secondes)

// macros
#define AUTRE_JOUEUR(i) (1-(i))
#define min(a, b)       ((a) < (b) ? (a) : (b))
#define max(a, b)       ((a) < (b) ? (b) : (a))

// Critères de fin de partie
typedef enum {NON, MATCHNUL, ORDI_GAGNE, HUMAIN_GAGNE } FinDePartie;

#define HAUTEUR_PLATEAU 6
#define LARGEUR_PLATEAU 7
#define VALP 4

// Definition du type Etat (état/position du jeu)
typedef struct EtatSt {

	int joueur; // à qui de jouer ? 
	char plateau[HAUTEUR_PLATEAU][LARGEUR_PLATEAU];	

} Etat;

// Definition du type Coup
typedef struct {
	int ligne;
	int colonne;
} Coup;

// Copier un état 
Etat * copieEtat( Etat * src ) {
	Etat * etat = (Etat *)malloc(sizeof(Etat));

	etat->joueur = src->joueur;
	
	int i,j;	
	for (i=0; i< HAUTEUR_PLATEAU; i++)
		for ( j=0; j<LARGEUR_PLATEAU; j++)
			etat->plateau[i][j] = src->plateau[i][j];
	
	return etat;
}

// Etat initial 
Etat * etat_initial( void ) {
	Etat * etat = (Etat *)malloc(sizeof(Etat));
	
	// TODO: à compléter avec la création de l'état initial
	
	/* par exemple : */
	int i,j;	
	for (i=0; i< HAUTEUR_PLATEAU; i++)
		for ( j=0; j<LARGEUR_PLATEAU; j++)
			etat->plateau[i][j] = ' ';
	
	return etat;
}


void afficheJeu(Etat * etat) {
	//Affichage d'un puissance 4
	int i,j;
	printf("   |");
	for ( j = 0; j < LARGEUR_PLATEAU; j++) 
		printf(" %d |", j);
	printf("\n");
	printf("--------------------------------");
	printf("\n");
	
	for(i=0; i < HAUTEUR_PLATEAU; i++) {
		printf(" %d |", i);
		for ( j = 0; j < LARGEUR_PLATEAU; j++) 
			printf(" %c |", etat->plateau[i][j]);
		printf("\n");
		printf("--------------------------------");
		printf("\n");
	}
}


// Nouveau coup 
// TODO: adapter la liste de paramètres au jeu
Coup * nouveauCoup(int j) {
	Coup * coup = (Coup *)malloc(sizeof(Coup));
	coup->colonne = j;
	
	return coup;
}

// Demander à l'humain quel coup jouer 
Coup * demanderCoup () {
	int j;
	printf("\n quelle colonne ? ");
	scanf("%d",&j); 
	
	return nouveauCoup(j);
}

// Modifier l'état en jouant un coup 
// retourne 0 si le coup n'est pas possible
int jouerCoup( Etat * etat, Coup * coup ) {
	if ( etat->plateau[0][coup->colonne] != ' ' ) {
		return 0;
	}
	else {
		for(int i=HAUTEUR_PLATEAU-1; i>=0; i--) {
			if(etat->plateau[i][coup->colonne] == ' ') {
				etat->plateau[i][coup->colonne] = etat->joueur ? 'R' : 'J';
				coup->ligne = i; 
				break;
			}
		}
		
		// à l'autre joueur de jouer
		etat->joueur = AUTRE_JOUEUR(etat->joueur); 	

		return 1;
	}	
}

// Retourne une liste de coups possibles à partir d'un etat 
// (tableau de pointeurs de coups se terminant par NULL)
Coup ** coups_possibles(Etat * etat ) {
	
	Coup ** coups = (Coup **) malloc((1+LARGEUR_MAX) * sizeof(Coup *) );
	
	int k = 0;
	
	int i,j;
	for(j=0; j<LARGEUR_PLATEAU; j++) {
		for (i=0; i<HAUTEUR_PLATEAU; i++) {
			if (etat->plateau[i][j] == ' ') {
				coups[k] = nouveauCoup(j); 
				k++;
				break;
			}
		}
	}
	coups[k] = NULL;
	return coups;
}


// Definition du type Noeud 
typedef struct NoeudSt {
		
	int joueur; // joueur qui a joué pour arriver ici
	Coup * coup;   // coup joué par ce joueur pour arriver ici
	
	Etat * etat; // etat du jeu
			
	struct NoeudSt * parent; 
	struct NoeudSt * enfants[LARGEUR_MAX]; // liste d'enfants : chaque enfant correspond à un coup possible
	int nb_enfants;	// nb d'enfants présents dans la liste
	
	// POUR MCTS:
	int nb_victoires;
	int nb_simus;
	
} Noeud;


// Créer un nouveau noeud en jouant un coup à partir d'un parent 
// utiliser nouveauNoeud(NULL, NULL) pour créer la racine
Noeud * nouveauNoeud (Noeud * parent, Coup * coup ) {
	Noeud * noeud = (Noeud *)malloc(sizeof(Noeud));
	
	if ( parent != NULL && coup != NULL ) {
		noeud->etat = copieEtat ( parent->etat );
		jouerCoup ( noeud->etat, coup );
		noeud->coup = coup;			
		noeud->joueur = AUTRE_JOUEUR(parent->joueur);		
	}
	else {
		noeud->etat = NULL;
		noeud->coup = NULL;
		noeud->joueur = 0; 
	}
	noeud->parent = parent; 
	noeud->nb_enfants = 0; 
	
	// POUR MCTS:
	noeud->nb_victoires = 0;
	noeud->nb_simus = 0;

	return noeud; 	
}

// Ajouter un enfant à un parent en jouant un coup
// retourne le pointeur sur l'enfant ajouté
Noeud * ajouterEnfant(Noeud * parent, Coup * coup) {
	Noeud * enfant = nouveauNoeud (parent, coup ) ;
	parent->enfants[parent->nb_enfants] = enfant;
	parent->nb_enfants++;
	return enfant;
}

void freeNoeud ( Noeud * noeud) {
	if ( noeud->etat != NULL)
		free (noeud->etat);
		
	while ( noeud->nb_enfants > 0 ) {
		freeNoeud(noeud->enfants[noeud->nb_enfants-1]);
		noeud->nb_enfants --;
	}
	if ( noeud->coup != NULL)
		free(noeud->coup); 

	free(noeud);
}
	

// Test si l'état est un état terminal 
// et retourne NON, MATCHNUL, ORDI_GAGNE ou HUMAIN_GAGNE
FinDePartie testFin( Etat * etat ) {

	// TODO...
	
	/* par exemple	*/
	
	// tester si un joueur a gagné
	int i,j,k,n = 0;
	for ( i=0;i < HAUTEUR_PLATEAU; i++) {
		for(j=0; j < LARGEUR_PLATEAU; j++) {
			if ( etat->plateau[i][j] != ' ') {
				n++;	// nb coups joués
			
				//colonnes
				k=0;
				while ( k < VALP && i+k < HAUTEUR_PLATEAU && etat->plateau[i+k][j] == etat->plateau[i][j] ) 
					k++;
				if ( k == VALP ) 
					return etat->plateau[i][j] == 'R'? ORDI_GAGNE : HUMAIN_GAGNE;

				//lignes
				k=0;
				while ( k < VALP && j+k < LARGEUR_PLATEAU && etat->plateau[i][j+k] == etat->plateau[i][j] ) 
					k++;
				if ( k == VALP ) 
					return etat->plateau[i][j] == 'R'? ORDI_GAGNE : HUMAIN_GAGNE;

				/* diagonales*/
				k=0;
				while ( k < VALP && i+k < HAUTEUR_PLATEAU && j+k < LARGEUR_PLATEAU && etat->plateau[i+k][j+k] == etat->plateau[i][j] ) 
					k++;
				if ( k == VALP ) 
					return etat->plateau[i][j] == 'R'? ORDI_GAGNE : HUMAIN_GAGNE;

				k=0;
				while ( k < VALP && i+k < HAUTEUR_PLATEAU && j-k >= 0 && etat->plateau[i+k][j-k] == etat->plateau[i][j] ) 
					k++;
				if ( k == VALP ) 
					return etat->plateau[i][j] == 'R'? ORDI_GAGNE : HUMAIN_GAGNE;
			}
		}
	}

	// et sinon tester le match nul	
	if ( n == HAUTEUR_PLATEAU*LARGEUR_PLATEAU ) 
		return MATCHNUL;
		
	return NON;
}

Noeud * createChildren(Noeud * node) {
	Coup ** coups = coups_possibles(node->etat);
	int k = 0;
	while ( coups[k] != NULL) {
		ajouterEnfant(node, coups[k]);
		k++;
	}
	return node;
}

int getNbVictoires(Noeud * node) {
	if(node->nb_enfants == 0) {
		switch(testFin(node->etat)) {
			case 1:
				node->nb_victoires = 0;
				node->nb_simus = 1;
				break;
			case 2:
				node->nb_victoires = 1;
				node->nb_simus = 1;
				break;
			case 3:
				node->nb_victoires = 0;
				node->nb_simus = 1;
				break;
		}
		return node->nb_victoires;
	}
	else {
		int sum = 0;
		for(int i=0; i<node->nb_enfants; i++) {
			sum += getNbVictoires(node->enfants[i]);
		}
		return sum;
	}
}

int getNbSimulations(Noeud * node) {
	if(node->nb_enfants == 0) {
		node->nb_simus = 1;
		return node->nb_simus;
	}
	else {
		int sum = 0;
		for(int i=0; i<node->nb_enfants; i++) {
			sum += getNbSimulations(node->enfants[i]);
		}
		return sum;
	}
}

int findBestCoupID(Noeud * node) {
	int max = -1000;
	int bestCoupID = 0;
	for(int i=0; i<node->nb_enfants; i++) {
		if(testFin(node->enfants[i]->etat) == ORDI_GAGNE) return i;
		int heuristique = node->enfants[i]->nb_victoires*100 - node->enfants[i]->nb_simus;
		if(heuristique > max) {
			max = heuristique;
			bestCoupID = i;
		}
	}
	return bestCoupID;
}

// Calcule et joue un coup de l'ordinateur avec MCTS-UCT
// en tempsmax secondes
void ordijoue_mcts(Etat * etat, int tempsmax) {

	clock_t tic, toc;
	tic = clock();
	int temps;

	Coup * meilleur_coup;
	
	// Créer l'arbre de recherche
	Noeud * racine = nouveauNoeud(NULL, NULL);	
	racine->etat = copieEtat(etat); 
	
	// créer les premiers noeuds:
	racine = createChildren(racine);

	//meilleur_coup = coups[ rand()%k ]; // choix aléatoire
	
	int iter = 0;
	Etat * etat_possible; 
	do {
		// à compléter par l'algorithme MCTS-UCT... 
		/*int i = 0;
		while(i<k){
			enfant = racine->enfants[i]; 
			etat_possible = enfant->etat;

			if (ORDI_GAGNE == testFin(etat_possible)){
				meilleur_coup = enfant->coup; 
			} else {
				int l = enfant->coup->ligne; 
				int c = enfant->coup->colonne; 

				//Vertical
				int v = 0;
				//Un jeton associé 
				if(l+1 < HAUTEUR_PLATEAU && etat_possible->plateau[l+1][c] == etat_possible->plateau[l][c]){
					v++; 
					//deux jetons associés 
					if(l+2<HAUTEUR_PLATEAU && etat_possible->plateau[l+2][c] == etat_possible->plateau[l][c]){
						v++; 
					}
				}
				
				//Horizontal
				int h = 0; 

				//Droite
				if(c+1<LARGEUR_MAX && etat_possible->plateau[l][c+1] == etat_possible->plateau[l][c]){
					h++; 
					if(c+2<LARGEUR_MAX && etat_possible->plateau[l][c+1] == etat_possible->plateau[l][c]){
						h++;
					}
				}

				//Gauche
				if(c-1>=0 && etat_possible->plateau[l][c-1] == etat_possible->plateau[l][c]){
					h++; 
					if(c-2>=0 && etat_possible->plateau[l][c-2] == etat_possible->plateau[l][c]){
						h++; 
					}
				}

				//Diagonale 
				int d = 0;
				//Bas-Droite 
				if(c+1<LARGEUR_PLATEAU && l+1<HAUTEUR_PLATEAU && etat_possible->plateau[l+1][c+1] == etat_possible->plateau[l][c]){
					d++; 
					if(c+2<LARGEUR_PLATEAU && l+2<HAUTEUR_PLATEAU && etat_possible->plateau[l+2][c+1] == etat_possible->plateau[l][c]){
						d++;
					}
				}
				//Bas-Gauche
				if(c-1>=0 && l+1<HAUTEUR_PLATEAU && etat_possible->plateau[l+1][c-1] == etat_possible->plateau[l][c]){
					d++; 
					if(c-2>=0 && l+2<HAUTEUR_PLATEAU && etat_possible->plateau[l+2][c-2] == etat_possible->plateau[l][c]){
						d++; 
					}
				}
				//Haut-Gauche
				if(c-1>=0 && l-1>=0 && etat_possible->plateau[l-1][c-1] == etat_possible->plateau[l][c]){
					d++; 
					if(c-2>=0 && l-2>0 && etat_possible->plateau[l-2][i-2] == etat_possible->plateau[l][c]){
						d++; 
					}
				}
				//Haut-Droit
				if(c+1<LARGEUR_PLATEAU && l-1>=0 && etat_possible->plateau[l-1][c+1] == etat_possible->plateau[l][c]){
					d++; 
					if(c+2<LARGEUR_PLATEAU && l-2>=0 && etat_possible->plateau[l-2][c-2] == etat_possible->plateau[l][c]){
						d++; 
					}
				}
				enfant->nb_simus = h+v+d; 
			}

			i = i+1;
		}

		i = 0; 
		int nbc = rand()%k; 
		int max = racine->enfants[nbc]->nb_simus;
		while(i<k){ 
			enfant = racine->enfants[i]; 
			if(max<enfant->nb_simus){
				nbc = i; 
				max = enfant->nb_simus; 
			}
			i=i+1;
		}
		meilleur_coup = racine->enfants[nbc]->coup; 
		*/

		//Tous les fils parcourus dans le temps imparti
		if(iter == racine->nb_enfants) break;

		//Selection + Expansion + Simulation pour chaque fils de la racine à chaque iteration
		Noeud * tempo = racine->enfants[iter];
		while(testFin(tempo->etat) == NON) {
			if(tempo->nb_enfants != 0) tempo = tempo->enfants[0];
			else {
				tempo = createChildren(tempo);
				tempo = tempo->enfants[0];
			}
		}
		//MAJ Feuille
		switch(testFin(tempo->etat)) {
			case 1:
				tempo->nb_victoires = 0;
				tempo->nb_simus = 1;
				break;
			case 2:
				tempo->nb_victoires = 1;
				tempo->nb_simus = 1;
				break;
			case 3:
				tempo->nb_victoires = 0;
				tempo->nb_simus = 1;
				break;
		}

		//Rétropropagation
		while(tempo->parent != racine) {
			tempo = tempo->parent;
			tempo->nb_victoires = getNbVictoires(tempo);
			tempo->nb_simus = getNbSimulations(tempo);
		}
		
		toc = clock();
		temps = (int)( ((double) (toc - tic)) / CLOCKS_PER_SEC );
		iter ++;

		printf("Temps : %d | Victoires : %d | Simulations : %d\n", temps, tempo->nb_victoires, tempo->nb_simus);
		//system("PAUSE");
	} while ( temps < tempsmax );

	// On choisi le meilleur fils avec l'heuristique : nbVictoires * 100 - nbSimulations
	meilleur_coup = racine->enfants[findBestCoupID(racine)]->coup;
	// Jouer le meilleur premier coup
	jouerCoup(etat, meilleur_coup );
	
	// Penser à libérer la mémoire :
	freeNoeud(racine);
}

int main(void) {
	Coup * coup;
	FinDePartie fin;
	
	// initialisation
	Etat * etat = etat_initial(); 
	
	// Choisir qui commence : 
	printf("Qui commence (0 : humain, 1 : ordinateur) ? ");
	scanf("%d", &(etat->joueur) );
	
	// boucle de jeu
	do {
		printf("\n");
		afficheJeu(etat);
		
		if ( etat->joueur == 0 ) {
			// tour de l'humain
			
			do {
				coup = demanderCoup();
			} while ( !jouerCoup(etat, coup) );
									
		}
		else {
			// tour de l'Ordinateur
			ordijoue_mcts( etat, TEMPS );	
			//etat->joueur = AUTRE_JOUEUR(etat->joueur);
		}
		
		fin = testFin( etat );
		//fin = NON;
	}	while ( fin == NON ) ;

	printf("\n");
	afficheJeu(etat);
		
	if ( fin == ORDI_GAGNE )
		printf( "** L'ordinateur a gagné **\n");
	else if ( fin == MATCHNUL )
		printf(" Match nul !  \n");
	else
		printf( "** BRAVO, l'ordinateur a perdu  **\n");
	system("pause");
	return 0;
}
