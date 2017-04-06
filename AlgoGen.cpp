// AlgoGen.cpp : définit le point d'entrée pour l'application console.
//

#pragma GCC optimize("-O3")
#pragma GCC optimize("inline")
#pragma GCC optimize("omit-frame-pointer")
#pragma GCC optimize("unroll-loops")

//#include "stdafx.h"

#include <iostream>
#include <chrono>
//#include <string>
//#include <algorithm>
//#include <math.h>

using namespace std;
using namespace std::chrono;

//TO DO VIRER LE RANDOM DANS LE CROSS

#ifdef WITH_LOG
    #define LOG(...)   cerr , __VA_ARGS__ , endl
    #define LOGS(...)  cerr , __VA_ARGS__

    template <typename T>
    ostream& operator,(ostream& out, const T& t)
    {
        out << t;

        return out;
    }

    // Overloaded version to handle all those special std::endl and others...
    ostream& operator,(ostream& out, ostream&(*f)(ostream&))
    {
        out << f;

        return out;
    }
#else
    #define LOG(...)
    #define LOGS(...)
#endif


// ***********************************************************

static unsigned int g_seed;
inline void fast_srand( int seed ) {
	//Seed the generator
	g_seed = seed;
}
inline int fastrand() {
	//fastrand routine returns one integer, similar output value range as C lib.
	g_seed = (214013 * g_seed + 2531011);
	return (g_seed >> 16) & 0x7FFF;
}
inline int fastRandInt(int maxSize) {
	return fastrand() % maxSize;
}
inline int fastRandInt(int a, int b) {
	return(a + fastRandInt(b - a));
}
inline double fastRandDouble() {
	return static_cast<double>(fastrand()) / 0x7FFF;
}
inline double fastRandDouble(double a, double b) {
	return a + (static_cast<double>(fastrand()) / 0x7FFF)*(b - a);
}

// ****************************************************************************************

constexpr int DEPTH = 20;
constexpr int POOL = 51;
constexpr int MUTATION = 2;

constexpr int CROSSPT1 = 10;
constexpr int POOLDIV2 = 25;

//Etat de jeu
static int actualGS;
static int tempGS;

static int actualIndividu = 0;
static int actualCoup = 0;

static int* fitness1 = new int[POOL];
static int* fitness2 = new int[POOL];

static int * actualFitness = new int;
static int * nextFitness = new int;
static int * tmpFitness = new int;

//Generate a completely random starting population
static int ** move1 = new int*[POOL];
static int ** move2 = new int*[POOL];

static int ** actualMove = new int*;
static int ** nextMove = new int*;
static int ** tmpMove = new int*;
static int poolFE;
static int poolFEpp;

//index du meilleur mouvement
static int bestMove = 0;

//une copie temporaire du meilleur mouvement et sa fitness
static int *copyBest = new int[DEPTH];
static int fitnessCopyBest = 0;

static int aIndex;
static int bIndex;
static int firstIndex;
static int secondIndex;

static int generation = 0;
static /*const*/ int crossPt1 /*= DEPTH / 2*/;

/*static int* s;
static int* s2;
static int* d;
static int* d2;
static int* dend;
static int* dend2;*/


high_resolution_clock::time_point start;
#define NOW high_resolution_clock::now()
#define TIME duration_cast<duration<double>>(NOW - start).count()

//TO DO on doit copier l'état du jeu dans temp
inline void CopyStaticActualGameState()
{
	tempGS = actualGS;
}

//TO DO on doit jouer le coup suivant
//Et calculer la fitness
inline void SimulateOnTempGSWithActualIndividuAndCoup(/*int individu, int coup*/)
{
	if (actualCoup == 0) tempGS = 0;

	//Si c'est impair on additionne
	//si c'est pair on soustrait
	if (actualMove[actualIndividu][actualCoup] % 2)
	{
		tempGS -= actualMove[actualIndividu][actualCoup];
	}
	else
	{
		tempGS += actualMove[actualIndividu][actualCoup];
	}

	if (actualCoup == DEPTH - 1)
	{
		actualFitness[actualIndividu] = tempGS;
	}

	//tempGs s'en retrouve modifié
}

//TO DO on doit générer un individu au hasard (remplacer fastRandInt(10000))
inline void randomize()
{
	for (actualCoup = 0; actualCoup < DEPTH; ++actualCoup)
	{
		actualMove[actualIndividu][actualCoup] = fastRandInt(10000);
		//on peut déjà simuler la fitness ici
		SimulateOnTempGSWithActualIndividuAndCoup(/*actualIndividu, actualCoup*/);

		if (actualFitness[bestMove] < actualFitness[actualIndividu]) {
			bestMove = actualIndividu;
		}
	}
}

//TO DO on doit être sûr que la fitness soit calculée à la fin du simulate
inline void Evaluate()
{
	for (actualIndividu = 0; actualIndividu < POOL; ++actualIndividu)
	{
		CopyStaticActualGameState();
		for (actualCoup = 0; actualCoup < DEPTH; ++actualCoup)
		{
			//On doit simuler les coups de bout en bout
			//Et renseigner la fitness
			SimulateOnTempGSWithActualIndividuAndCoup(/*actualIndividu, actualCoup*/);
		}

		if (actualFitness[bestMove] < actualFitness[actualIndividu]) {
			bestMove = actualIndividu;
		}
	}
}

//TO DO 
inline void copyBestMove()
{
	for (actualCoup = 0; actualCoup < DEPTH; ++actualCoup)
	{
		nextMove[0][actualCoup] = actualMove[bestMove][actualCoup];
	}
}

//TO DO mutate proprement
inline void mutateCopyBestMove()
{
	nextMove[0][fastRandInt(DEPTH)] = fastRandInt(10000);
}

//TO DO mutate proprement
inline void mutateNewIndividuInNextMove()
{
	nextMove[poolFE][fastRandInt(DEPTH)] = fastRandInt(10000);
	nextMove[poolFEpp][fastRandInt(DEPTH)] = fastRandInt(10000);
}

//TO DO Simulate proprement
inline void SimulateOnTempGSWithPoolFE()
{
	CopyStaticActualGameState();

	for (actualCoup = 0; actualCoup < DEPTH; ++actualCoup)
	{
		if (actualCoup == 0) tempGS = 0;

		//Si c'est impair on additionne
		//si c'est pair on soustrait
		if (nextMove[poolFE][actualCoup] % 2)
		{
			tempGS -= nextMove[poolFE][actualCoup];
		}
		else
		{
			tempGS += nextMove[poolFE][actualCoup];
		}

		if (actualCoup == DEPTH - 1)
		{
			nextFitness[poolFE] = tempGS;
		}
	}
}

//TO DO Simulate proprement
inline void SimulateOnTempGSWithPoolFEpp()
{
	CopyStaticActualGameState();

	for (actualCoup = 0; actualCoup < DEPTH; ++actualCoup)
	{
		if (actualCoup == 0) tempGS = 0;

		//Si c'est impair on additionne
		//si c'est pair on soustrait
		if (nextMove[poolFEpp][actualCoup] % 2)
		{
			tempGS -= nextMove[poolFEpp][actualCoup];
		}
		else
		{
			tempGS += nextMove[poolFEpp][actualCoup];
		}

		if (actualCoup == DEPTH - 1)
		{
			nextFitness[poolFEpp] = tempGS;
		}
	}
}

inline void SimulateOnTempGSWithCopyBest(/*int individu, int coup*/)
{
	CopyStaticActualGameState();

	for (actualCoup = 0; actualCoup < DEPTH; ++actualCoup)
	{
		if (actualCoup == 0) tempGS = 0;

		//Si c'est impair on additionne
		//si c'est pair on soustrait
		if (nextMove[0][actualCoup] % 2)
		{
			tempGS -= nextMove[0][actualCoup];
		}
		else
		{
			tempGS += nextMove[0][actualCoup];
		}

		if (actualCoup == DEPTH - 1)
		{
			nextFitness[0] = tempGS;
		}
	}

	//tempGs s'en retrouve modifié
}

//TO DO croisement ok ?
inline void MergeActualMoveIndex1WithIndex2IntoNextMove()
{
	poolFEpp = poolFE + 1;
	for (actualCoup = 0; actualCoup < DEPTH; ++actualCoup) {
		if (fastRandInt(2)) {
			nextMove[poolFE][actualCoup] = actualMove[firstIndex][actualCoup];
			nextMove[poolFEpp][actualCoup] = actualMove[secondIndex][actualCoup];
		}
		else {
			nextMove[poolFE][actualCoup] = actualMove[secondIndex][actualCoup];
			nextMove[poolFEpp][actualCoup] = actualMove[firstIndex][actualCoup];
		}
	}
}

inline void MergeActualMoveIndex1WithIndex2IntoNextMoveCross1Pt()
{
    //mettre une valeur en dur à la place de fastrandint fait gagner 2m individu
    //crossPt1 = 10/*fastRandInt(DEPTH)*/;
	poolFEpp = poolFE + 1;
	/*for (actualCoup = 0; actualCoup < crossPt1; ++actualCoup) {		
			nextMove[poolFE][actualCoup] = actualMove[firstIndex][actualCoup];
			nextMove[poolFEpp][actualCoup] = actualMove[secondIndex][actualCoup];
	}
	
	for (actualCoup = crossPt1; actualCoup < DEPTH; ++actualCoup) {		
			nextMove[poolFE][actualCoup] = actualMove[secondIndex][actualCoup];
			nextMove[poolFEpp][actualCoup] = actualMove[firstIndex][actualCoup];
	}	
	
	int * startPtr = nextMove[poolFE];
	int * startPtrpp = nextMove[poolFEpp];
	int * endPtr = nextMove[poolFE][crossPt1];
	int * endPtrpp = nextMove[poolFEpp];
	
	for (startPtr; startPtr != endPtr; ++startPtr) {		
			startPtr[po[actualCoup] = actualMove[firstIndex][actualCoup];
			nextMove[poolFEpp][actualCoup] = actualMove[secondIndex][actualCoup];
	}*/
	
	/*const int* s = src;
    int* d = dst;
    const int*const dend = dst + n;
    while ( d != dend )
	    *d++ = *s++;
	  */
	  
	const int* s = actualMove[firstIndex];
	const int* s2 = actualMove[secondIndex];
    int* d = nextMove[poolFE];
    int* d2 = nextMove[poolFEpp];    
    const int*const dend = d + CROSSPT1;
    const int*const dend2 = d + DEPTH;
    
    /*s = actualMove[firstIndex];
	s2 = actualMove[secondIndex];
    d = nextMove[poolFE];
    d2 = nextMove[poolFEpp];
    dend = nextMove[poolFE] + crossPt1;
    dend2 = nextMove[poolFE] + DEPTH;*/
    
    while ( d != dend )
    {
	    *d++ = *s++;
	    *d2++ = *s2++;
    }
    
    while ( d != dend2 )
    {
	    *d2++ = *s++;
	    *d++ = *s2++;
    }
	
}

void debugMove1()
{
	cerr << "Move 1 : " << endl;

	for (actualIndividu = 0; actualIndividu < POOL; ++actualIndividu)
	{
		for (actualCoup = 0; actualCoup < DEPTH; ++actualCoup)
		{
			cerr << move1[actualIndividu][actualCoup] << " ";
		}

		cerr << endl << " => " << fitness1[actualIndividu] << endl;
	}
}

void debugMove2()
{
	cerr << "Move 2 : " << endl;

	for (actualIndividu = 0; actualIndividu < POOL; ++actualIndividu)
	{
		for (actualCoup = 0; actualCoup < DEPTH; ++actualCoup)
		{
			cerr << move2[actualIndividu][actualCoup] << " ";
		}

		cerr << endl << " => " << fitness2[actualIndividu] << endl;
	}
}

int main()
{

	/*int surfaceN; // the number of points used to draw the surface of Mars.
	cin >> surfaceN; cin.ignore();
	for (int i = 0; i < surfaceN; i++) {
		int landX; // X coordinate of a surface point. (0 to 6999)
		int landY; // Y coordinate of a surface point. By linking all the points together in a sequential fashion, you form the surface of Mars.
		cin >> landX >> landY; cin.ignore();
	}*/

	int turn = 0;
	double limit = turn ? 0.085 : 0.970;

#define LIMIT TIME < limit	

	actualMove = move1;
	nextMove = move2;

	actualFitness = fitness1;
	nextFitness = fitness2;

	for (actualIndividu = 0; actualIndividu < POOL; ++actualIndividu)
	{
		move1[actualIndividu] = new int[DEPTH];
		move2[actualIndividu] = new int[DEPTH];
		//generate random individu
		randomize();
	}

//	debugMove1();

	while (turn < 80)
	{
		start = NOW;
		// cerr << "Time " << LIMIT << endl;

		limit = turn ? 0.085 : 0.970;

		//En sortant du randomize on a déjà évalué les individus
		while (LIMIT)
		{
			// New generation
			generation++;
			
			//crossPt1 = fastRandInt(DEPTH);

			//cerr << "On rentre là" << endl;

			// Force the actual best with a mutation to be in the pool
			//int[] copyBest = copy(move1[bestMove]);
			//On copie le meilleur move dans le nextMove[0]
			copyBestMove();
			//On le mutate
			mutateCopyBestMove();
			//Et on simule son score
			SimulateOnTempGSWithCopyBest();

//			debugMove2();

			//Quoiqu'il arrive le bestMove à 0 sera toujours la meilleure solution
			//Si la fitness n'est pas meilleure on reprend l'ancienne		
			if (nextFitness[0] < actualFitness[bestMove]) {								
				copyBestMove();
				nextFitness[0] = actualFitness[bestMove];				
			}
			/*else
			{
				cerr << "Le muté est meilleur que le meilleur " << nextFitness[0] << " " << actualFitness[bestMove] << endl;
			}*/

			bestMove = 0;

			/*counter += 1;
			*/
			poolFE = 1;

			//Randomly pick two separate solutions
			while (poolFE < POOL /*&& LIMIT*/)
			{
				aIndex = fastRandInt(POOL);
				//bIndex;

				//do {
					bIndex = fastRandInt(POOL);
				//} while (bIndex == aIndex);

				firstIndex = actualFitness[aIndex] > actualFitness[bIndex] ? aIndex : bIndex;

				//do {
					aIndex = fastRandInt(POOL);
				//} while (aIndex == firstIndex);

				//do {
					bIndex = fastRandInt(POOL);
				//} while (bIndex == aIndex || bIndex == firstIndex);

				secondIndex = actualFitness[aIndex] > actualFitness[bIndex] ? aIndex : bIndex;

				//int[] child = actualMove[firstIndex]->merge(move1[secondIndex]);

				//Dans nextMove poolFE et poolFE + 1
				//MergeActualMoveIndex1WithIndex2IntoNextMove();
				MergeActualMoveIndex1WithIndex2IntoNextMoveCross1Pt();

				if (poolFE > POOLDIV2/*!fastRandInt(MUTATION)*/) {
					mutateNewIndividuInNextMove();
				}

				SimulateOnTempGSWithPoolFE();
				SimulateOnTempGSWithPoolFEpp();

				if (nextFitness[poolFE] > nextFitness[bestMove]) {
					bestMove = poolFE;
				}

				if (nextFitness[poolFEpp] > nextFitness[bestMove]) {
					bestMove = poolFEpp;
				}

				//poolFE += 2;
				
				poolFE+=2;
				//poolFE++;

				//	nextMove[poolFE++] = child;

				//counter += 1;
			}

			//On échange les pointeurs
			tmpMove = nextMove;
			nextMove = actualMove;
			actualMove = tmpMove;

			tmpFitness = nextFitness;
			nextFitness = actualFitness;
			actualFitness = tmpFitness;
		}

		cerr << "Best guy evar " << actualFitness[bestMove] << endl;
		for (actualCoup = 0; actualCoup < DEPTH; ++actualCoup)
		{
			cerr << actualMove[bestMove][actualCoup] << " ";
		}

		cerr << "turn : " << turn << endl << " nb generation : " << generation << " individu " << generation * (POOL+1) << endl;
		cout << "0 3" << endl;
		turn++;
		//system("pause");
	}

	cerr << "nb generation : " << generation << endl;
	//system("pause");

	return 0;
}


/*int main()
{
int surfaceN; // the number of points used to draw the surface of Mars.
cin >> surfaceN; cin.ignore();
for (int i = 0; i < surfaceN; i++) {
int landX; // X coordinate of a surface point. (0 to 6999)
int landY; // Y coordinate of a surface point. By linking all the points together in a sequential fashion, you form the surface of Mars.
cin >> landX >> landY; cin.ignore();
}

// game loop
while (1) {
int X;
int Y;
int hSpeed; // the horizontal speed (in m/s), can be negative.
int vSpeed; // the vertical speed (in m/s), can be negative.
int fuel; // the quantity of remaining fuel in liters.
int rotate; // the rotation angle in degrees (-90 to 90).
int power; // the thrust power (0 to 4).
cin >> X >> Y >> hSpeed >> vSpeed >> fuel >> rotate >> power; cin.ignore();

// Write an action using cout. DON'T FORGET THE "<< endl"
// To debug: cerr << "Debug messages..." << endl;


// 2 integers: rotate power. rotate is the desired rotation angle (should be 0 for level 1), power is the desired thrust power (0 to 4).
cout << "0 3" << endl;
}
}*/

