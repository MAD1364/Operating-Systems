#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

#define CARDS 52
#define DENOMINATIONS 13

node_t* deck_front;                      // use to represent deck of cards
node_t* deck_back;
int denominations[DENOMINATIONS];        // use to know what cards are in deck
FILE *log;                               // use to log thread activity

int seed;                                // use for seeding random number generator
int cards[6];                            // each player has up to two cards
int win;                                 // use to determine whether a player has won a round
int player;                              // use to indicate active player
int turn;                                // use to switch activity between dealer and players
int rounds;                              // use to count the rounds played
int game_complete[3];                       // use to indicate game is complete
int start;                               // use to indicate all players are available

pthread_mutex_t deck_lock;               // use to lock access to deck
pthread_mutex_t file_lock;               // use to lock access to file
pthread_mutex_t output_lock;             // use to lock access to stdout
pthread_mutex_t win_lock;                // use to lock access to win variable
pthread_mutex_t turn_lock;               // use to lock access to turn variable
pthread_mutex_t player_lock;             // use to lock access to player variable
pthread_mutex_t round_lock;              // use to lock access to the rounds variable
pthread_mutex_t start_lock;              // use to lock access to start variable
pthread_mutex_t dealer_lock;             // use in conjunction with condition variable
pthread_mutex_t p_lock[3];               // use in conjunction with condition variable

pthread_cond_t turn_cond;                // use to signal whether dealer is active or players are active
pthread_cond_t player_cond;              // use to signal the next player to take a turn
pthread_cond_t start_cond;               // use to signal to dealer all players are ready to play
pthread_cond_t dealer_cond;              // use to signal to dealer its turn
pthread_cond_t p_cond[3];                // use to signal to player one its turn

void* dealer(void* arg) {
    long tid = (long)arg;                // convert argument passed in pthread_create to a long from void pointer

    pthread_mutex_lock(&start_lock);
    while (!start) pthread_cond_wait(&start_cond, &start_lock);
    pthread_mutex_unlock(&start_lock);

    while (1)
    {
        pthread_mutex_lock(&dealer_lock);                                 // card dealer must wait for its turn
  	while (turn != 0) pthread_cond_wait(&dealer_cond, &dealer_lock);  // wait for card dealer's turn
        pthread_mutex_unlock(&dealer_lock);

        if (rounds == 3) break;                                           // three rounds of game have been played

        pthread_mutex_lock(&deck_lock);                                   // lock access to deck so card dealer has exclusive access
        pthread_mutex_lock(&output_lock);                                 // lock access to stdout
        pthread_mutex_lock(&file_lock);                                   // lock access to output file
        pthread_mutex_lock(&player_lock);                                 // lock access to player variable
        pthread_mutex_lock(&turn_lock);                                   // lock access to turn variable

        player = rounds + 1;  // use to indicate which player takes a turn first in round to come

        int i;                // use to execute for loops for shuffling deck

        if (deck_front != NULL) clearDeck(&deck_front, &deck_back);  // clear deck to restart for next round

        for (i = 0; i < DENOMINATIONS; ++i) denominations[i] = 4;    // four of each suit
        for (i = 0; i < CARDS; ++i)                                  // shuffle deck
        {
            int new_card;

            do {
                new_card = rand()%13;                  // generate a random number in the range [0, 12]
            } while (denominations[new_card] == 0);    // find a denomination that has not been utilized completely

            push(&deck_front, &deck_back, new_card+1); // put the new card on the deck
            --denominations[new_card];                 // account for the use of a denomination of kind new_card
        }

        fprintf(log, "DEALER: shuffle\n");

        cards[0] = pop(&deck_front, &deck_back);   // deal card to player one
        cards[2] = pop(&deck_front, &deck_back);   // deal card to player two
        cards[4] = pop(&deck_front, &deck_back);   // deal card to player three

        fprintf(log, "DEALER: deal a card to each player\n");

        turn = 1;

        pthread_mutex_lock(&p_lock[player-1]);
        pthread_cond_signal(&p_cond[player-1]);
        pthread_mutex_unlock(&p_lock[player-1]);

        pthread_mutex_unlock(&player_lock);
        pthread_mutex_unlock(&file_lock);
        pthread_mutex_unlock(&output_lock);
        pthread_mutex_unlock(&deck_lock);
        pthread_mutex_unlock(&turn_lock);
    }

    pthread_exit((void*)tid);
}

void* players(void* arg) {
    long tid = (long)arg;

    pthread_mutex_lock(&start_lock);
    ++start;
    if (start == 3) pthread_cond_signal(&start_cond);
    pthread_mutex_unlock(&start_lock);

    while (1)
    {
        pthread_mutex_lock(&p_lock[tid-1]);                                      // use to lock access to player variable
        while (player != tid) pthread_cond_wait(&p_cond[tid-1], &p_lock[tid-1]); // wait for this player's turn to start
        pthread_mutex_unlock(&p_lock[tid-1]);

        pthread_mutex_lock(&turn_lock);
        pthread_mutex_lock(&player_lock);
        pthread_mutex_lock(&round_lock);                                     
        pthread_mutex_lock(&file_lock);
        pthread_mutex_lock(&output_lock);
        pthread_mutex_lock(&deck_lock);
        pthread_mutex_lock(&win_lock);

        if (!win)
        {
            fprintf(log, "PLAYER %ld: hand %d\n", tid, cards[2*tid-2]);      // display hand before drawing second card
            cards[2*tid-1] = pop(&deck_front, &deck_back);                   // player draws a card
            fprintf(log, "PLAYER %ld: draws %d\n", tid, cards[2*tid-1]);     // display card player drew this turn
            printf("PLAYER %ld HAND: %d  %d\n", tid, cards[2*tid-2], cards[2*tid-1]);

            if (cards[2*tid-2] == cards[2*tid-1])
            {
                win = 1;
                fprintf(log, "PLAYER %ld: wins\n", tid);
                printf("PLAYER %ld WIN (yes)\n", tid);
                fprintf(log, "PLAYER %ld: round completed\n", tid);

                ++turn;
                ++game_complete[tid-1];
            }
            else
            {
                int r = rand()%2; // generate random number in range [0, 1] to pick card at random to discard

                fprintf(log, "PLAYER %ld: discards %d\n", tid, cards[2*tid-2+r]);
                push(&deck_front, &deck_back, cards[2*tid-2+r]);             // discard card by placing it on the bottom
                if (r == 0) cards[2*tid-2] = cards[2*tid-1];                 // replace first card in hand for logistics
                fprintf(log, "PLAYER %ld: hand %d\n", tid, cards[2*tid-2]);
                printf("PLAYER %ld: WIN (no)\n", tid);
            }

            printf("DECK ");
            listDeck(&deck_front);
        }
        else
        {
            fprintf(log, "PLAYER %ld: round completed\n", tid);

            turn = (turn+1)%4;                                               // players must concede round for dealer
            ++game_complete[tid-1];
            if (turn == 0)                                                   // dealer's turn
            {
                ++rounds;
                win = 0;
                pthread_mutex_lock(&dealer_lock);
                pthread_cond_signal(&dealer_cond);
                pthread_mutex_unlock(&dealer_lock);
            }
        }

        if (player == 2) player = 3;
        else player = (player+1)%3;
        
        pthread_mutex_lock(&p_lock[player-1]);
        if (turn != 0) pthread_cond_signal(&p_cond[player-1]);                 // signal next player in sequence
        pthread_mutex_unlock(&p_lock[player-1]);                               // unlock player lock to allow other players to play

        pthread_mutex_unlock(&turn_lock);
        pthread_mutex_unlock(&player_lock);
        pthread_mutex_unlock(&round_lock);
        pthread_mutex_unlock(&file_lock);
        pthread_mutex_unlock(&output_lock);
        pthread_mutex_unlock(&deck_lock);
        pthread_mutex_unlock(&win_lock);

        if (game_complete[tid-1] == 3) break;
    }

    pthread_exit((void*)tid);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {fprintf(stderr, "ERROR: Too few arguments. Enter seed.\n"); exit(-1);}

    seed = atoi(argv[1]);            // cast first element of argv as an integer
    srand(seed);                     // seed random number generator to produce different sequences with multiple executions of program

    log = fopen("card_log.txt", "w");  // open a file in write mode to log actions of card dealer and players
    deck_front = NULL;               // initialize deck
    deck_back = NULL;

    player = 0;                      // initialize player
    rounds = 0;                      // initialize rounds played
    turn = 0;                        // initialize turn (dealer starts)
    game_complete[0] = 0;            // initialize game
    game_complete[1] = 0;
    game_complete[2] = 0;
    start = 0;

    void* status;                    // can be used to identify return status of a pthread

    /* Initialize mutex and condition variable objects */
    pthread_mutex_init(&turn_lock, NULL);
    pthread_cond_init(&turn_cond, NULL);

    pthread_mutex_init(&player_lock, NULL);
    pthread_cond_init(&player_cond, NULL);

    pthread_mutex_init(&start_lock, NULL);
    pthread_cond_init(&start_cond, NULL);

    pthread_mutex_init(&p_lock[0], NULL);
    pthread_cond_init(&p_cond[0], NULL);

    pthread_mutex_init(&p_lock[1], NULL);
    pthread_cond_init(&p_cond[1], NULL);

    pthread_mutex_init(&p_lock[2], NULL);
    pthread_cond_init(&p_cond[2], NULL);

    pthread_mutex_init(&win_lock, NULL);
    pthread_mutex_init(&deck_lock, NULL);
    pthread_mutex_init(&output_lock, NULL);
    pthread_mutex_init(&file_lock, NULL);
    pthread_mutex_init(&round_lock, NULL);

    pthread_t dealer_t;              // create a pthread handle for card dealer thread
    pthread_t player_h[3];           // create an array of pthread handles for players
    
    pthread_attr_t attr;             // use to ensure threads can be joined as this attribute is not supported on all implementations of pthreads
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    pthread_create(&dealer_t, &attr, dealer, (void*)0);     // create pthread for card dealer function
    pthread_create(&player_h[0], &attr, players, (void*)1); // create pthread for player 1
    pthread_create(&player_h[1], &attr, players, (void*)2); // create pthread for player 2
    pthread_create(&player_h[2], &attr, players, (void*)3); // create pthread for player 3

    pthread_join(dealer_t, &status);     // wait on card dealer thread to complete its task
    pthread_join(player_h[0], &status);  // wait on player one to complete its task
    pthread_join(player_h[1], &status);  // wait on player two to complete its task
    pthread_join(player_h[2], &status);  // wait on player three to complete its task

    fclose(log);                         // close file use to log card dealer and player actions

    /* Deallocate memory allocate for attribute, mutex and condition variable objects */
    pthread_attr_destroy(&attr);
    
    pthread_mutex_destroy(&turn_lock);
    pthread_cond_destroy(&turn_cond);

    pthread_mutex_destroy(&player_lock);
    pthread_cond_destroy(&player_cond);

    pthread_mutex_destroy(&start_lock);
    pthread_cond_destroy(&start_cond);

    pthread_mutex_destroy(&p_lock[0]);
    pthread_cond_destroy(&p_cond[0]);

    pthread_mutex_destroy(&p_lock[1]);
    pthread_cond_destroy(&p_cond[1]);

    pthread_mutex_destroy(&p_lock[2]);
    pthread_cond_destroy(&p_cond[2]);

    pthread_mutex_destroy(&win_lock);
    pthread_mutex_destroy(&deck_lock);
    pthread_mutex_destroy(&output_lock);
    pthread_mutex_destroy(&file_lock);
    pthread_mutex_destroy(&round_lock);

    pthread_exit(NULL);
}
