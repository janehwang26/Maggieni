/** snake API for C **/
// need to add the elongating of snek after nomming
#include "snek_api.h"
#include <string.h>
#include <time.h>

int CURR_FRAME = 0;
int SCORE = 0;
int MOOGLE_FLAG = 0;
int MOOGLES_EATEN = 0;
int TIME_OUT = ((BOARD_SIZE * 4) - 4) * CYCLE_ALLOWANCE;

GameBoard* init_board(){
	srand(time(0));
	GameBoard* gameBoard = (GameBoard*)(malloc(sizeof(GameBoard)));

	for (int i = 0; i < BOARD_SIZE; i++){
		for (int j = 0; j < BOARD_SIZE; j++){
			gameBoard->cell_value[i][j] = 0;
			gameBoard->occupancy[i][j] = 0;
		}
	}
	gameBoard->occupancy[0][0] = 1; //snake initialized
	gameBoard->snek = init_snek(0, 0);
	return gameBoard;
}

Snek* init_snek(int a, int b){
	Snek* snek = (Snek *)(malloc(sizeof(Snek)));

	snek->head = (SnekBlock *)malloc(sizeof(SnekBlock));
	snek->head->coord[x] = a;
	snek->head->coord[y] = b;

	snek->tail = (SnekBlock *)malloc(sizeof(SnekBlock));
	snek->tail->coord[x] = a;
	snek->tail->coord[y] = b;

	snek->tail->next = NULL;
	snek->head->next = snek->tail;
	
	snek->length = 1;

	return snek;
}

int hits_edge(int axis, int direction, GameBoard* gameBoard){
	// Replaced gameBoard->snek->head[y] with gameBoard->snek->head->coord[y]
	if (((axis == AXIS_Y) && ((direction == UP && gameBoard->snek->head->coord[y] + UP < 0) || (direction == DOWN && gameBoard->snek->head->coord[y] + DOWN > BOARD_SIZE - 1)))
	   || (axis = AXIS_X && ((direction == LEFT && gameBoard->snek->head->coord[x] + LEFT < 0) || (direction == RIGHT && gameBoard->snek->head->coord[x] + RIGHT > BOARD_SIZE-1))))
	{
		return 1;
	} else {
		return 0;
	}

}


int hits_self(int axis, int direction, GameBoard *gameBoard){
	int new_x, new_y;
	if (axis == AXIS_X){
		new_x = gameBoard->snek->head->coord[x] + direction;
		new_y = gameBoard->snek->head->coord[y];
	} else if (axis == AXIS_Y){
		new_x = gameBoard->snek->head->coord[x];
		new_y = gameBoard->snek->head->coord[y] + direction;
	}
	return gameBoard->occupancy[new_y][new_x]; //1 if occupied
}

int time_out(){
	return (MOOGLE_FLAG == 1 && CURR_FRAME > TIME_OUT);

}

int is_failure_state(int axis, int direction, GameBoard *gameBoard){
	return (hits_self(axis, direction, gameBoard) || hits_edge(axis, direction, gameBoard) || time_out());
}

void populate_moogles(GameBoard *gameBoard){
	if (MOOGLE_FLAG == 0){
		int r1 = rand() % BOARD_SIZE;
		int r2 = rand() % BOARD_SIZE;
		
		int r3 = rand() % (BOARD_SIZE * 10);
		if (r3 == 0){
			gameBoard->cell_value[r1][r2] = MOOGLE_POINT * HARRY_MULTIPLIER;
			MOOGLE_FLAG = 1;
		} else if (r3 < BOARD_SIZE){
			gameBoard->cell_value[r1][r2] = MOOGLE_POINT;
			MOOGLE_FLAG = 1;
		}
	}
}

void eat_moogle(GameBoard* gameBoard, int head_x, int head_y) {
	SCORE = SCORE + gameBoard->cell_value[head_y][head_x];
	gameBoard->cell_value[head_y][head_x] = 0;

	gameBoard->snek->length ++;
	MOOGLES_EATEN ++;
	MOOGLE_FLAG = 0;
	CURR_FRAME = 0;
}

int advance_frame(int axis, int direction, GameBoard *gameBoard){
	if (is_failure_state(axis, direction, gameBoard)){
		return 0;
	} else {
		// update the occupancy grid and the snake coordinates
		int head_x, head_y;
		// figure out where the head should now be
		if (axis == AXIS_X) {
			head_x = gameBoard->snek->head->coord[x] + direction;
			head_y = gameBoard->snek->head->coord[y];
		} else if (axis == AXIS_Y){
			head_x = gameBoard->snek->head->coord[x];
			head_y = gameBoard->snek->head->coord[y] + direction;
		}
		
		int tail_x = gameBoard->snek->tail->coord[x];
		int tail_y = gameBoard->snek->tail->coord[y];

		// update the occupancy grid for the head
		gameBoard->occupancy[head_y][head_x] = 1;

		if (gameBoard->snek->length > 1) { //make new head
			SnekBlock *newBlock = (SnekBlock *)malloc(sizeof(SnekBlock));
			newBlock->coord[x] = gameBoard->snek->head->coord[x];
			newBlock->coord[y] = gameBoard->snek->head->coord[y];
			newBlock->next = gameBoard->snek->head->next;

			gameBoard->snek->head->coord[x] = head_x;
			gameBoard->snek->head->coord[y] = head_y;
			gameBoard->snek->head->next = newBlock;
	
			if (gameBoard->cell_value[head_y][head_x] > 0){  //eat something
				eat_moogle(gameBoard, head_x, head_y);
			} else { //did not eat
				//delete the tail
				gameBoard->occupancy[tail_y][tail_x] = 0;
				SnekBlock *currBlock = gameBoard->snek->head;
				while (currBlock->next != gameBoard->snek->tail){
					currBlock = currBlock->next;
				} //currBlock->next points to tail

				currBlock->next = NULL;
				free(gameBoard->snek->tail);
				gameBoard->snek->tail = currBlock;
			}

		} else if ((gameBoard->snek->length == 1) && gameBoard->cell_value[head_y][head_x] == 0){ // change both head and tail coords, head is tail
			gameBoard->occupancy[tail_y][tail_x] = 0;
			gameBoard->snek->head->coord[x] = head_x;
			gameBoard->snek->head->coord[y] = head_y;
			gameBoard->snek->tail->coord[x] = head_x;
			gameBoard->snek->tail->coord[y] = head_y;
			
		} else { //snake is length 1 and eats something
			eat_moogle(gameBoard, head_x, head_y);
			gameBoard->snek->head->coord[x] = head_x;
			gameBoard->snek->head->coord[y] = head_y;
		}

		// update the score and board
		SCORE = SCORE + LIFE_SCORE;
		if (MOOGLE_FLAG == 1){
			CURR_FRAME ++;
		}

		// populate moogles
		populate_moogles(gameBoard);
		return 1;
	} 
}

void show_board(GameBoard* gameBoard) {
	fprintf(stdout, "\033[2J"); // clear terminal ANSI code
	fprintf(stdout, "\033[0;0H"); // reset cursor position
	
	char blank = 	43;
	char snek = 	83;
	char moogle = 	88;

	for (int i = 0; i < BOARD_SIZE; i++){
		for (int j = 0; j < BOARD_SIZE; j++){
			if (gameBoard->occupancy[i][j] == 1){
				//snake is here
				fprintf(stdout, "%c", snek);
			} else if (gameBoard->cell_value[i][j] > 0) {
				//there be a moogle
				fprintf(stdout, "%c", moogle);
			} else {
				//nothing to see here
				fprintf(stdout, "%c", blank);
			}
		} //new line
		fprintf(stdout, "\n");
		
	}

	fprintf(stdout, "\n\n");
	
	if (MOOGLE_FLAG == 1){
		fprintf(stdout, "!..ALERT, MOOGLE IN VICINITY..!\n\n");
	}
	fprintf(stdout, "SCORE: %d\n", SCORE);
	fprintf(stdout, "YOU HAVE EATEN %d MOOGLES\n\n", MOOGLES_EATEN);

	fprintf(stdout, "SNEK HEAD\t(%d, %d)\n", gameBoard->snek->head->coord[x], gameBoard->snek->head->coord[y]);
	fprintf(stdout, "SNEK TAIL\t(%d, %d)\n", gameBoard->snek->tail->coord[x], gameBoard->snek->tail->coord[y]);
	fprintf(stdout, "LENGTH \t%d\n", gameBoard->snek->length);
	fprintf(stdout, "CURR FRAME %d vs TIME OUT %d\n", CURR_FRAME, TIME_OUT);


	fflush(stdout);
}

int get_score() {
	return SCORE;
}

void end_game(GameBoard **board){
	//fprintf(stdout, "\033[2J");
	//fprintf(stdout, "\033[0;0H"); 
	fprintf(stdout, "\n\n\n--!!---GAME OVER---!!--\n\nYour score: %d\n\n\n\n", SCORE);
	fflush(stdout);
	// need to free all allocated memory
	// first snek
	SnekBlock **snekHead = &((*board)->snek->head);
	SnekBlock *curr;
	SnekBlock *prev;
	while ((*snekHead)->next != NULL) {
		curr = *snekHead;
		while (curr->next != NULL){
			prev = curr;
			curr = curr->next;
		}
		prev->next = NULL;
		free(curr);
	}
	free(*snekHead);
	free((*board)->snek);
	free(*board);
}

/* A utility function to check if the vertex v can be added at 
   index 'pos' in the Hamiltonian Cycle constructed so far (stored 
   in 'path[]') */
bool isSafe(int v, bool graph[V][V], int path[], int pos) 
{ 
    /* Check if this vertex is an adjacent vertex of the previously 
       added vertex. */
    if (graph[path[pos-1]][v] == 0) 
        return false; 
  
    /* Check if the vertex has already been included. 
      This step can be optimized by creating an array of size V */
    for (int i = 0; i < pos; i++) 
        if (path[i] == v){
          return false;
        } 
    return true; 
}

/* A recursive utility function to solve hamiltonian cycle problem */
bool hamCycleUtil(bool graph[V][V], int path[], int pos) 
{ 
    /* base case: If all vertices are included in Hamiltonian Cycle */
    if (pos == V) 
    { 
        // And if there is an edge from the last included vertex to the 
        // first vertex 
        if (graph[path[pos-1]][path[0]] == 1 ) 
           return true; 
        else
          return false; 
    } 
  
    // Try different vertices as a next candidate in Hamiltonian Cycle. 
    // We don't try for 0 as we included 0 as starting point in hamCycle() 
    for (int v = 1; v < V; v++) 
    { 
        /* Check if this vertex can be added to Hamiltonian Cycle */
        if (isSafe(v, graph, path, pos)) 
        { 
            path[pos] = v; 
  
            /* recur to construct rest of the path */
            if (hamCycleUtil (graph, path, pos+1) == true) 
                return true; 
  
            /* If adding vertex v doesn't lead to a solution, 
               then remove it */
            path[pos] = -1; 
        } 
    } 
  
    /* If no vertex can be added to Hamiltonian Cycle constructed so far, 
       then return false */
    return false; 
} 
  
/* This function solves the Hamiltonian Cycle problem using Backtracking. 
  It mainly uses hamCycleUtil() to solve the problem. It returns false 
  if there is no Hamiltonian Cycle possible, otherwise return true and 
  prints the path. Please note that there may be more than one solutions, 
  this function prints one of the feasible solutions. */
bool hamCycle(bool graph[V][V]) 
{ 
    int *path = new int[V]; 
    for (int i = 0; i < V; i++) 
        path[i] = -1; 
  
    /* Let us put vertex 0 as the first vertex in the path. If there is 
       a Hamiltonian Cycle, then the path can be started from any point 
       of the cycle as the graph is undirected */
    path[0] = 0; 
    if ( hamCycleUtil(graph, path, 1) == false ) 
    { 
        printf("\nSolution does not exist"); 
        return false; 
    } 
  
    printSolution(path); 
    return true; 
} 

/* A utility function to print solution */
void printSolution(int path[]) 
{ 
    printf ("Solution Exists:"
            " Following is one Hamiltonian Cycle \n"); 
    for (int i = 0; i < V; i++) 
        printf(" %d ", path[i]); 
  
    // Let us print the first vertex again to show the complete cycle 
    printf(" %d ", path[0]); 
    printf("\n"); 
} 
  
