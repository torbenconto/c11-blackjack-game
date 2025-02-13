#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DECK_SIZE 52
#define HEADER "C11 Blackjack V1 - Torben Conto 2025\n"

typedef enum {
    CLUBS,
    DIAMONDS,
    HEARTS,
    SPADES,
} suit_t;

typedef struct {
    suit_t suit;
    int value;
} card_t;

typedef struct {
    card_t *cards;
    int total_cards;
    int top;
} deck_t;

typedef struct {
    card_t *cards;
    size_t total_cards;
} hand_t;

void construct(deck_t *deck, int deck_count) {
    deck->total_cards = deck_count * DECK_SIZE;
    deck->cards = malloc(deck->total_cards * sizeof(card_t));
    if (!deck->cards) {
        printf("Memory allocation failed!\n");
        exit(1);
    }

    int cardIndex = 0;
    for (int d = 0; d < deck_count; d++) {
        for (int suit = 0; suit < 4; suit++) {
            for (int value = 1; value <= 13; value++) {
                deck->cards[cardIndex].value = value;
                deck->cards[cardIndex].suit = suit;
                cardIndex++;
            }
        }
    }
    deck->top = deck->total_cards - 1;
}

void shuffle(deck_t *deck) {
    for (int i = 0; i < deck->total_cards; i++) {
        int randIndex = rand() % deck->total_cards;
        card_t temp = deck->cards[i];
        deck->cards[i] = deck->cards[randIndex];
        deck->cards[randIndex] = temp;
    }
}

card_t draw(deck_t *deck) {
    if (deck->top < 0) {
        printf("Deck is empty! Reshuffling...\n");
        shuffle(deck);
        deck->top = deck->total_cards - 1;
    }
    return deck->cards[deck->top--];
}

void free_deck(deck_t *deck) {
    free(deck->cards);
    deck->cards = NULL;
}

void free_hand(hand_t *hand) {
    free(hand->cards);
    hand->cards = NULL;
    hand->total_cards = 0;
}

void append_card(hand_t *hand, card_t card) {
    card_t *temp = realloc(hand->cards, (hand->total_cards + 1) * sizeof(card_t));
    if (!temp) {
        printf("Memory allocation failed!\n");
        exit(1);
    }
    hand->cards = temp;
    hand->cards[hand->total_cards] = card;
    hand->total_cards++;
}

int hand_total(hand_t *hand) {
    int sum = 0;
    int ace_count = 0;

    for (size_t i = 0; i < hand->total_cards; i++) {
        int cardValue = hand->cards[i].value;

        if (cardValue == 1) { // Ace
            ace_count++;
            sum += 11;
        } else if (cardValue > 10) { // Face cards
            sum += 10;
        } else {
            sum += cardValue;
        }
    }

    while (sum > 21 && ace_count > 0) {
        sum -= 10;
        ace_count--;
    }

    return sum;
}

void print_hand(hand_t *hand, size_t n_cards) {
    const char *suit_names[] = {"Clubs", "Diamonds", "Hearts", "Spades"};
    const char *face_cards[] = {"Ace", "2", "3", "4", "5", "6", "7", "8", "9", "10", "Jack", "Queen", "King"};

    if (n_cards > hand->total_cards) {
        n_cards = hand->total_cards;
    }

    for (size_t i = 0; i < n_cards; i++) {
        printf("%s of %s", face_cards[hand->cards[i].value - 1], suit_names[hand->cards[i].suit]);
        if (i < n_cards - 1) {
            printf(", ");
        }
    }
    printf("\n");
}

void play_hand(deck_t *deck, hand_t *hand) {
    while (1) {
        printf("Your total: %d\n", hand_total(hand));
        printf("1: Hit Me!\n2: Stand!\n");

        int action;
        if (scanf("%d", &action) != 1) {
            printf("Invalid input! Try again.\n");
            while (getchar() != '\n');
            continue;
        }

        if (action == 1) {
            append_card(hand, draw(deck));
            printf("Your Hand: ");
            print_hand(hand, hand->total_cards);

            if (hand_total(hand) > 21) {
                printf("Bust! You lose this hand.\n");
                return;
            }
        } else if (action == 2) {
            break;
        } else {
            printf("Invalid action!\n");
        }
    }
}

int is_blackjack(hand_t *hand) {
    if (hand->total_cards != 2) return 0;

    int has_ace = 0, has_ten_value = 0;

    for (size_t i = 0; i < hand->total_cards; i++) {
        if (hand->cards[i].value == 1) has_ace = 1;
        if (hand->cards[i].value >= 10) has_ten_value = 1;
    }

    return has_ace && has_ten_value;
}

int main(void) {
    srand(time(NULL));

    printf(HEADER);

    printf("How many decks would you like to play with?: ");
    int deck_count;
    if (scanf("%d", &deck_count) != 1 || deck_count <= 0 || deck_count > 4) {
        printf("Invalid deck count!\n");
        return 1;
    }

    deck_t deck;
    construct(&deck, deck_count);
    shuffle(&deck);

    printf("Deck constructed with %d cards of size %zu bytes.\n", deck.total_cards, deck.total_cards * sizeof(card_t));

    while (1) {
        hand_t player_hand = {NULL, 0};
        hand_t dealer_hand = {NULL, 0};

        printf("Dealing Cards...\n");

        append_card(&player_hand, draw(&deck));
        append_card(&dealer_hand, draw(&deck));
        append_card(&player_hand, draw(&deck));
        append_card(&dealer_hand, draw(&deck));

        printf("Your Hand: ");
        print_hand(&player_hand, player_hand.total_cards);
        printf("Dealer Showing: ");
        print_hand(&dealer_hand, 1);

        // Check for Blackjack
        int player_blackjack = is_blackjack(&player_hand);
        int dealer_blackjack = is_blackjack(&dealer_hand);

        if (player_blackjack || dealer_blackjack) {
            printf("Dealer Reveals: ");
            print_hand(&dealer_hand, dealer_hand.total_cards);

            if (player_blackjack && dealer_blackjack) {
                printf("Push! Both you and the dealer have Blackjack.\n");
            } else if (player_blackjack) {
                printf("Blackjack! You win!\n");
            } else {
                printf("Dealer has Blackjack. You lose!\n");
            }

            free_hand(&player_hand);
            free_hand(&dealer_hand);

            printf("Play again? (1: Yes, 0: No): ");
            int play_again;
            if (scanf("%d", &play_again) != 1 || play_again == 0) {
                break;
            }
            continue;
        }

        play_hand(&deck, &player_hand);
        printf("Dealer Reveals: ");
        print_hand(&dealer_hand, dealer_hand.total_cards);

        while (hand_total(&dealer_hand) < 17) {
            append_card(&dealer_hand, draw(&deck));
            printf("Dealer Hand: ");
            print_hand(&dealer_hand, dealer_hand.total_cards);
        }

        int player_score = hand_total(&player_hand);
        int dealer_score = hand_total(&dealer_hand);

        if (player_score > 21) {
            printf("Bust! You lose this hand.\n");
        } else if (dealer_score > 21 || player_score > dealer_score) {
            printf("You Win!\n");
        } else if (player_score == dealer_score) {
            printf("Push! It's a tie.\n");
        } else {
            printf("You Lost!\n");
        }

        free_hand(&player_hand);
        free_hand(&dealer_hand);

        printf("Play again? (1: Yes, 0: No): ");
        int play_again;
        if (scanf("%d", &play_again) != 1 || play_again == 0) {
            break;
        }
    }

    free_deck(&deck);
    printf("Freed deck of size %zu bytes.\n", deck.total_cards * sizeof(card_t));
    printf("Thanks for playing!\n");
    return 0;
}
