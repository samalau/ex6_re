#include "ex6_re.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

# define INT_BUFFER 128

// ================================================
// Basic struct definitions from ex6.h assumed:
//   PokemonData { int id; char *name; PokemonType TYPE; int hp; int attack; EvolutionStatus CAN_EVOLVE; }
//   PokemonNode { PokemonData* data; PokemonNode* left, *right; }
//   OwnerNode   { char* ownerName; PokemonNode* pokedexRoot; OwnerNode *next, *prev; }
//   OwnerNode* ownerHead;
//   const PokemonData pokedex[];
// ================================================

// --------------------------------------------------------------
// 1) Safe integer reading
// --------------------------------------------------------------

void trimWhitespace(char *str)
{
	// Remove leading spaces/tabs/\r
	int start = 0;
	while (str[start] == ' ' || str[start] == '\t' || str[start] == '\r')
		start++;

	if (start > 0)
	{
		int idx = 0;
		while (str[start])
			str[idx++] = str[start++];
		str[idx] = '\0';
	}

	// Remove trailing spaces/tabs/\r
	int len = (int)strlen(str);
	while (len > 0 && (str[len - 1] == ' ' || str[len - 1] == '\t' || str[len - 1] == '\r'))
	{
		str[--len] = '\0';
	}
}

char *myStrdup(const char *src)
{
	if (!src)
		return NULL;
	size_t len = strlen(src);
	char *dest = (char *)malloc(len + 1);
	if (!dest)
	{
		printf("Memory allocation failed in myStrdup.\n");
		return NULL;
	}
	strcpy(dest, src);
	return dest;
}

int readIntSafe(const char *prompt)
{
	char buffer[INT_BUFFER];
	int value;
	int success = 0;

	while (!success)
	{
		printf("%s", prompt);

		// If we fail to read, treat it as invalid
		if (!fgets(buffer, sizeof(buffer), stdin))
		{
			printf("Invalid input.\n");
			clearerr(stdin);
			continue;
		}

		// 1) Strip any trailing \r or \n
		//    so "123\r\n" becomes "123"
		size_t len = strlen(buffer);
		if (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r'))
			buffer[--len] = '\0';
		if (len > 0 && (buffer[len - 1] == '\r' || buffer[len - 1] == '\n'))
			buffer[--len] = '\0';

		// 2) Check if empty after stripping
		if (len == 0)
		{
			printf("Invalid input.\n");
			continue;
		}

		// 3) Attempt to parse integer with strtol
		char *endptr;
		value = (int)strtol(buffer, &endptr, 10);

		// If endptr didn't point to the end => leftover chars => invalid
		// or if buffer was something non-numeric
		if (*endptr != '\0')
		{
			printf("Invalid input.\n");
		}
		else
		{
			// We got a valid integer
			success = 1;
		}
	}
	return value;
}

// --------------------------------------------------------------
// 2) Utility: Get type name from enum
// --------------------------------------------------------------
const char *getTypeName(PokemonType type)
{
	switch (type)
	{
	case GRASS:
		return "GRASS";
	case FIRE:
		return "FIRE";
	case WATER:
		return "WATER";
	case BUG:
		return "BUG";
	case NORMAL:
		return "NORMAL";
	case POISON:
		return "POISON";
	case ELECTRIC:
		return "ELECTRIC";
	case GROUND:
		return "GROUND";
	case FAIRY:
		return "FAIRY";
	case FIGHTING:
		return "FIGHTING";
	case PSYCHIC:
		return "PSYCHIC";
	case ROCK:
		return "ROCK";
	case GHOST:
		return "GHOST";
	case DRAGON:
		return "DRAGON";
	case ICE:
		return "ICE";
	default:
		return "UNKNOWN";
	}
}

// --------------------------------------------------------------
// Utility: getDynamicInput (for reading a line into malloc'd memory)
// --------------------------------------------------------------
char *getDynamicInput()
{
	char *input = NULL;
	size_t size = 0, capacity = 1;
	input = (char *)malloc(capacity);
	if (!input)
	{
		printf("Memory allocation failed.\n");
		return NULL;
	}

	int c;
	while ((c = getchar()) != '\n' && c != EOF)
	{
		if (size + 1 >= capacity)
		{
			capacity *= 2;
			char *temp = (char *)realloc(input, capacity);
			if (!temp)
			{
				printf("Memory reallocation failed.\n");
				free(input);
				return NULL;
			}
			input = temp;
		}
		input[size++] = (char)c;
	}
	input[size] = '\0';

	// Trim any leading/trailing whitespace or carriage returns
	trimWhitespace(input);

	return input;
}

// Function to print a single Pokemon node
void printPokemonNode(PokemonNode *node)
{
	if (!node)
		return;
	printf("ID: %d, Name: %s, Type: %s, HP: %d, Attack: %d, Can Evolve: %s\n",
		   node->data->id,
		   node->data->name,
		   getTypeName(node->data->TYPE),
		   node->data->hp,
		   node->data->attack,
		   (node->data->CAN_EVOLVE == CAN_EVOLVE) ? "Yes" : "No");
}

void traverseDFS(PokemonNode *root, int order) {
	if (!root) {
		printf("Pokedex is empty.\n"); return;
   	}
	switch(order) {
		case PRE_ORDER: {
			printPokemonNode(root);
			if (root->left != root) traverseDFS(root->left, PRE_ORDER);
			if (root->right != root) traverseDFS(root->right, PRE_ORDER);
			return;
		}
		case IN_ORDER: {
			if (root->left != root) traverseDFS(root->left, IN_ORDER);
			printPokemonNode(root);
			if (root->right != root) traverseDFS(root->right, IN_ORDER);
			return;
		}
		case POST_ORDER: {
			if (root->left != root) traverseDFS(root->left, POST_ORDER);
			if (root->right != root) traverseDFS(root->right, POST_ORDER);
			printPokemonNode(root);
			return;
		}
		default: return;
	}
}

// --------------------------------------------------------------
// Display Menu
// --------------------------------------------------------------
void displayMenu(OwnerNode *owner)
{
    if (!owner->pokedexRoot)
    {
        printf("Pokedex is empty.\n");
        return;
    }

    printf("Display:\n");
    printf("1. BFS (Level-Order)\n");
    printf("2. Pre-Order\n");
    printf("3. In-Order\n");
    printf("4. Post-Order\n");
    printf("5. Alphabetical (by name)\n");

    int choice = readIntSafe("Your choice: ");

    switch (choice)
    {
    case 1:
        // displayBFS(owner->pokedexRoot);
        break;
    case 2:
		traverseDFS(owner->pokedexRoot, PRE_ORDER);
        break;
    case 3:
        traverseDFS(owner->pokedexRoot, IN_ORDER);
        break;
    case 4:
		traverseDFS(owner->pokedexRoot, POST_ORDER);
        break;
    case 5:
        // displayAlphabetical(owner->pokedexRoot);
        break;
    default:
        printf("Invalid choice.\n");
    }
}

// --------------------------------------------------------------
// Sub-menu for existing Pokedex
// --------------------------------------------------------------
void enterExistingPokedexMenu()
{
	printf("\nExisting Pokedexes:\n");
	if (!ownerHead) {
		return;
	}
	OwnerNode* cur = NULL;
	choosePokedexByNumber(&cur, (char)0);
	if (!cur) {
		return;
	}
	printf("\nEntering %s's Pokedex...\n", cur->ownerName);
	int subChoice;
	do
	{
		printf("\n-- %s's Pokedex Menu --\n", cur->ownerName);
		printf("1. Add Pokemon\n");
		printf("2. Display Pokedex\n");
		printf("3. Release Pokemon (by ID)\n");
		printf("4. Pokemon Fight!\n");
		printf("5. Evolve Pokemon\n");
		printf("6. Back to Main\n");

		subChoice = readIntSafe("Your choice: ");

		switch (subChoice)
		{
		case 1:
			// addPokemon(cur);
			break;
		case 2:
			displayMenu(cur);
			break;
		case 3:
			// freePokemon(cur);
			break;
		case 4:
			// pokemonFight(cur);
			break;
		case 5:
			// evolvePokemon(cur);
			break;
		case 6:
			printf("Back to Main Menu.\n");
			break;
		default:
			printf("Invalid choice.\n");
		}
	} while (subChoice != 6);
}

void freePokemonNode(PokemonNode *node) {
	node->data = NULL;
	node->left = NULL;
	node->right = NULL;
}

// OwnerNode *ownerNode;
// PokemonData *starterData;
// PokemonNode *starterNode;

// --------------------------------------------------------------
// New Pokedex
// --------------------------------------------------------------
void openPokedexMenu(void) {
	printf("Your name: ");
	char *ownerName = getDynamicInput();
	if (!ownerName) {  // placeholder
		return;
	}
	// trimWhitespace(yourName);
	if (findOwnerByName(ownerName)) {
		printf("Owner '%s' already exists. Not creating a new Pokedex.\n", ownerName);
		free(ownerName);
		ownerName = NULL;
	} else {
		int pokemon = readIntSafe("Choose Starter:\n1. Bulbasaur\n2. Charmander\n3. Squirtle\nYour choice: ");
		if (pokemon < 1 || 3 < pokemon) {
			free(ownerName);
			ownerName = NULL;
			return;  // placeholder
		}
		pokemon = (pokemon * 3) - 3;
		PokemonData *starterData = (PokemonData *)malloc(sizeof(PokemonData));
		if (!(starterData)) {
			return;  // placeholder
		}
		PokemonNode *starter = (PokemonNode *)malloc(sizeof(PokemonNode));
		if (!(starter)) {
			return;  // placeholder
		}
		starter = createPokemonNode(&pokedex[pokemon]);
		starter->left = starter->right = starter;
		OwnerNode *ownerNode = createOwner(ownerName, starter);
		if (!ownerNode) {
			return;  // placeholder
		}
		if (ownerHead) {
			linkOwnerInCircularList(ownerNode);
		}
		else {
			ownerNode->next = ownerNode->prev = ownerHead = ownerNode;
		}
	}
	return;
}

void linkOwnerInCircularList(OwnerNode *newOwner) {
	newOwner->prev = ownerHead->prev;
	newOwner->next = ownerHead;
	ownerHead->prev->next = newOwner;
	ownerHead->prev = newOwner;
}

// void removeOwnerFromCircularList(OwnerNode *target) {

// }

// void printOwnersCircular(void) {

// }

OwnerNode *findOwnerByName(const char *name) {
	if (!ownerHead) {return NULL;}
	OwnerNode *node = ownerHead;
	while(strcmp(node->ownerName, name) != 0) {
		if (node->next == ownerHead) {return NULL;}
		node = node->next;
	}
	return node;
}

void choosePokedexByNumber(OwnerNode **cur, char del) {
	if (!ownerHead) {
		return;
	}
	*cur = ownerHead;
	int ind = 0;
	do {
		printf("%d. %s\n", ++ind, (*cur)->ownerName);
		*cur = (*cur)->next;
	} while (*cur != ownerHead);
	int sel = readIntSafe(del ? "Choose a Pokedex to delete by number: " : "Choose a Pokedex by number: ");
	if (sel < 1 || sel > ind) {
		*cur = NULL;
		return; // placeholder
	}
	ind = 0;
	while (++ind != sel) {
		*cur = (*cur)->next;
	}
}

void deletePokedex(void) {
	if (!ownerHead) {
		return;
	}
	OwnerNode *cur = NULL;
	choosePokedexByNumber(&cur, (char)1);
	if (!cur) {
		return;
	}
	cur->next->prev = cur->prev;
	cur->prev->next = cur->next;
	if (cur == ownerHead) {
		cur = NULL;
		ownerHead = ownerHead->next == ownerHead ? NULL : ownerHead->next;
	}
	// TODO: FREE & NULL
}

// void mergePokedexMenu(void) {

// }

OwnerNode *createOwner(char *ownerName, PokemonNode *starter) {
	OwnerNode *owner = (OwnerNode *)malloc(sizeof(OwnerNode));
	if (!(ownerName || starter)) {
		return NULL;  // placeholder
	}
	owner->ownerName = ownerName;
	owner->pokedexRoot = starter;
	owner->prev = ownerHead ? ownerHead->prev : owner;
	owner->next = ownerHead ? ownerHead : owner;
	return owner;
}

// void freeAllOwners(void) {

// }

// void freeOwnerNode(OwnerNode *owner) {

// }

// void addPokemon(OwnerNode *owner) {

// }

// PokemonNode *insertPokemonNode(PokemonNode *root, PokemonNode *newNode) {

// }

PokemonNode *createPokemonNode(const PokemonData *data) {
	PokemonNode *poke = (PokemonNode*)malloc(sizeof(PokemonNode));
	poke->data = data;
	poke->left = NULL;  // assign in caller
	poke->right = NULL;  // assign in caller
	return poke;
}

// PokemonNode *removeNodeBST(PokemonNode *root, int id) {

// }

// PokemonNode *removePokemonByID(PokemonNode *root, int id) {

// }

// void freePokemonTree(PokemonNode *root) {

// }

// void freePokemon(OwnerNode *owner) {

// }

// void freePokemonNode(PokemonNode *node) {

// }

// --------------------------------------------------------------
// Main Menu
// --------------------------------------------------------------
void mainMenu()
{
	int choice;
	do
	{
		printf("\n=== Main Menu ===\n");
		printf("1. New Pokedex\n");
		printf("2. Existing Pokedex\n");
		printf("3. Delete a Pokedex\n");
		printf("4. Merge Pokedexes\n");
		printf("5. Sort Owners by Name\n");
		printf("6. Print Owners in a direction X times\n");
		printf("7. Exit\n");
		choice = readIntSafe("Your choice: ");

		switch (choice)
		{
		case 1:
			openPokedexMenu();
			break;
		case 2:
			enterExistingPokedexMenu();
			break;
		case 3:
			deletePokedex();
			break;
		case 4:
			// mergePokedexMenu();
			break;
		case 5:
			// sortOwners();
			break;
		case 6:
			// printOwnersCircular();
			break;
		case 7:
			printf("Goodbye!\n");
			break;
		default:
			printf("Invalid.\n");
		}
	} while (choice != 7);
}

int main()
{
	mainMenu();
	// freeAllOwners();
	return 0;
}
