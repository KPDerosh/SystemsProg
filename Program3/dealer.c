#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

int deckCounter = 0;

/*method used to print a string to stdout*/
void printSomething(char *something){
	char printBuffer[1024];
	sprintf(printBuffer, "%s", something);
	write(1, printBuffer, strlen(printBuffer));
}

/*Method to print an integer to stdout*/
void printNumber(int i){
	char printBuffer[100];
	sprintf(printBuffer, "%d", i);
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
	printSomething("Initializing Deck...\n");
}

/*print deck for debug purposes*/
void printDeck(int deck[]){
	int i;
	for(i = 0; i < 52; i++){
		printNumber(deck[i]);
		printSomething(" ");
	}
	printSomething("\n");
}

/*print the winner of the game.*/
void printWinner(int i){
	char printBuffer[100];
	sprintf(printBuffer, "%s%d\n","The winning player was player: ", i);
	write(1, printBuffer, strlen(printBuffer));
}

/*creates a parent child process. The child executes player program with
piped information then the child player program pipes information back the parent
That is the how we get the sum of the integers piped to player process.*/
int dealHand(int deck[], int player){
	pid_t pid;
	int sum = 0, i, writeVal;
	char *const parmList[] = {"player", 0};
	int p2c[2], c2p[2];
	player = player + 1;	/*make player not zero indexed*/

	/*if all cards will be used on this hand shuffle new deck*/
	/*since 52 is always divisable by 4 we will never need to shuffle mid dealing*/
	/*we do need to shuffle when deck counter > 52 which is how many cards we have*/
	if(deckCounter > 48){
		shuffleDeck(deck);
	}

	printSomething("Dealing Hand to player: "); printNumber(player); printSomething("\n");
	printSomething("Dealt: ");

	/*print next 4 cards which are cards to be dealt out.*/
	for(i = 0; i < 4; i++){
		printNumber(deck[deckCounter + i]);
		printSomething(" ");
	}
	printSomething("\n");

	/*create pipes*/
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
		close(p2c[0]);	/*close uneeded pipe ends*/
		/*write to parent write end so we can replace stdin with parent read end.*/
		/*this way when player reads from stdin it's actually the parent pipe read end.*/
		for(i = 0; i < 4; i++){
			writeVal = deck[deckCounter];
			write(p2c[1], &writeVal, sizeof(int));
			deckCounter++;
		}
		/*wait for the process to execute all things.*/
		waitpid(pid, &returnStatus, 0);

		/*read the sum that the player process piped back*/
		read(c2p[0], &sum, sizeof(int));
	
	} else if(pid == 0){/*child*/
		close(0);		/*close stdin to replace with parent read pipe.*/
		dup(p2c[0]);	/*replaces fd 0 with read pipe end.*/
		close(p2c[0]);	/*close none needed pipe ends.*/
		close(p2c[1]);
		close(1);		/*close stdout and replace with child write end.*/
		dup(c2p[1]);	/*replace fd 1 with child write end.*/
		close(c2p[1]);	/*close non needed pipe ends.*/
		close(c2p[0]);
		execvp("./player", parmList);	/*run player process*/
	}
	/*print the sum that was retrieved from pipe.*/
	printSomething("player "); printNumber(player); printSomething("'s sum was ");printNumber(sum);printSomething("\n");
	printSomething("\n");
	/*return to game logic*/
	return sum;
}

/*main function accepts args input filename, brightness value, output file name*/
int main(int argc, char *cdArgs[]){
	/*declare integers*/
	int deck[52];
	int i = 0, round = 1, tiedPlayers = 0, numOfPlayers, winningPlayer, winnerFound = 0, playersLeft, lowestSum = 1000, lowestSumIndex = 0;
	int sums[100];
	int tiedPlayersArray[1000];	/*keep and array of indexs of the tied players*/
	/*Initialize and shuffle deck.*/
	initDeck(deck);
	shuffleDeck(deck);

	/*we get one command line arg to tell how many players there are.*/
	printSomething("Getting number of players...\n");
	numOfPlayers = atoi(cdArgs[1]);
	printSomething("Number of players: ");
	printNumber(numOfPlayers);
	printSomething("\n\n");
	playersLeft = numOfPlayers;
	

	while(winnerFound == 0){
		/*deal hands to remaining players. Only if sum is not = -1 are they still in game and should be dealt cards*/
		for(i = 0; i < numOfPlayers; i++){
			if(sums[i] != -1){
				sums[i] = dealHand(deck, i);
			}
		}

		/*set really high sum to check for lowest sum*/
		lowestSum = 1000;

		/*find lowest sum and set index so that person can be kicked from game*/
		for(i = 0; i < numOfPlayers; i++){
			
			if(sums[i] < lowestSum && sums[i] != -1){
				lowestSum = sums[i];
				lowestSumIndex = i;
				tiedPlayers = 0;
			}
			if(sums[i] == lowestSum){
				tiedPlayersArray[tiedPlayers] = i;
				tiedPlayers++;
			}
		}
		sums[lowestSumIndex] = -1;
		playersLeft--;
		tiedPlayers = 0;
		printSomething("Player "); printNumber(lowestSumIndex + 1); printSomething(" was eliminated.\n\n");
		printSomething("Round ");
		printNumber(round);
		printSomething(" is over.\n\n");
		round++;
		/*if one player is left break while loop.*/
		/*find winning player if game is over*/
		for(i = 0; i < numOfPlayers; i++){
			if(sums[i] > 0){
				winningPlayer = i + 1;
			}
		}
		if(playersLeft == 1){
			winnerFound = 1;
			break;
		}
	}
	
	printWinner(winningPlayer);
	
	return 0;
}
