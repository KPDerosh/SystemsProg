#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

int deckCounter = 0;
/*method to init the deck of cards*/
void initDeck(int deck[]){
	int i; 
	int k;
	int counter =0;
	for(k =0; k < 4; k++){
		for(i = 1; i <= 13; i++){
			deck[counter] = i;
			counter++;
		}
	}
}
void printSomething(char *something){
	char printBuffer[1024];
	sprintf(printBuffer, "%s", something);
	write(1, printBuffer, strlen(printBuffer));
}
/*shuffles the deck using rand()*/
void shuffleDeck(int deck[]){
	int i, j, temp, size = 52;

	for(i = 0; i < size; i++){
		j = rand() % size;
		temp = deck[i];
		deck[i] = deck[j];
		deck[j] = temp;
	}
	printSomething("Shuffling Deck...\n");
	deckCounter = 0;
}

void printNumber(int i){
	char printBuffer[100];
	sprintf(printBuffer, "%d", i);
	write(1, printBuffer, strlen(printBuffer));
}
void printDeck(int deck[]){
	int i;
	for(i = 0; i < 52; i++){
		printNumber(deck[i]);
		printSomething(" ");
	}
	printSomething("\n");
}
void printWinner(int i){
	char printBuffer[100];
	sprintf(printBuffer, "%s%d\n","The winning player was player: ", i);
	write(1, printBuffer, strlen(printBuffer));
}

int dealHand(int deck[], int player){
	pid_t pid;
	int sum = 0, i, writeVal;
	char *const parmList[] = {"player", 0};
	int p2c[2], c2p[2];
	player = player + 1;
	/*if all cards will be used on this hand shuffle new deck*/
	if(deckCounter > 48){
		shuffleDeck(deck);
	}
	printSomething("Dealing Hand to player: "); printNumber(player); printSomething("\n");
	printSomething("Dealt: ");
	for(i = 0; i < 4; i++){
		printNumber(deck[deckCounter + i]);
		printSomething(" ");
	}
	printSomething("\n");
	pipe(p2c);
	pipe(c2p);

	/*create child process*/
	pid = fork(); 
	if(pid == -1){
		printSomething("Error in fork");
	}
	if(pid != 0){	
		/*parent*/
		int returnStatus;
		close(p2c[0]);
		for(i = 0; i < 4; i++){
			writeVal = deck[deckCounter];
			write(p2c[1], &writeVal, sizeof(int));
			deckCounter++;
		}
		waitpid(pid, &returnStatus, 0);
		
		read(c2p[0], &sum, sizeof(int));
	
	} else if(pid == 0){/*child*/
		close(0);
		dup(p2c[0]);
		close(p2c[0]);
		close(p2c[1]);
		close(1);
		dup(c2p[1]);
		close(c2p[1]);
		close(c2p[0]);
		execvp("./player", parmList);
	}
	printSomething("player "); printNumber(player); printSomething("'s sum was ");printNumber(sum);printSomething("\n");
	printSomething("\n");
	return sum;
}


/*main function accepts args input filename, brightness value, output file name*/
int main(int argc, char *cdArgs[]){
	int deck[52], i = 0, round = 1, tiedPlayers = 0, numOfPlayers, winningPlayer, winnerFound = 0, playersLeft, lowestSum = 1000, lowestSumIndex = 0;
	int sums[100];

	printSomething("Initializing Deck...\n");
	initDeck(deck);
	
	/*deck is initialized*/
	/*shuffle deck up*/
	printSomething("Shuffling deck...\n");
	shuffleDeck(deck);

	/*we get one command line arg to tell how many players there are.*/
	printSomething("Getting number of players...\n");
	numOfPlayers = atoi(cdArgs[1]);
	printSomething("Number of players: ");
	printNumber(numOfPlayers);
	printSomething("\n\n");
	/*deal first hand*/
	for(i = 0; i < numOfPlayers; i++){
		sums[i] = dealHand(deck, i);
		if(sums[i] < lowestSum){
			lowestSum = sums[i];
			lowestSumIndex = i;
		}
	}
	sums[lowestSumIndex] = -1;
	printSomething("Player "); printNumber(lowestSumIndex + 1); printSomething(" was eliminated.\n\n");
	printSomething("Round ");
	printNumber(round);
	printSomething(" is over.\n\n");

	while(winnerFound == 0){
		playersLeft = 0;

		for(i = 0; i < numOfPlayers; i++){
			if(sums[i] > 0){
				playersLeft = playersLeft + 1;
				winningPlayer = i + 1;
			}
		}

		if(playersLeft == 1){
			winnerFound = 1;
			break;
		}

		for(i = 0; i < numOfPlayers; i++){
			if(sums[i] != -1){
				sums[i] = dealHand(deck, i);
			}
		}
		lowestSum = 1000;
		for(i = 0; i < numOfPlayers; i++){
			if(sums[i] == lowestSum){
				tiedPlayers++;
			}
			if(sums[i] < lowestSum && sums[i] != -1){
				lowestSum = sums[i];
				lowestSumIndex = i;
				tiedPlayers = 0;
			}
		}
		sums[lowestSumIndex] = -1;
		printSomething("Player "); printNumber(lowestSumIndex + 1); printSomething(" was eliminated.\n\n");
		round++;
		printSomething("Round ");
		printNumber(round);
		printSomething(" is over.\n\n");
	}
	
	printWinner(winningPlayer);
	
	return 0;
}
