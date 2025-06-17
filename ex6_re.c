#include "ex6_re.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

# define INT_BUFFER 128

# define FIRST_STARTER 1 
# define LAST_STARTER 3

# define MERGE_DST 1
# define MERGE_SRC 2

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

void trimWhitespace(char *str) {
	// Remove leading spaces/tabs/\r
	int start = 0;
	while (str[start] == ' ' || str[start] == '\t' || str[start] == '\r')
		start++;
	if (start > 0) {
		int idx = 0;
		while (str[start])
			str[idx++] = str[start++];
		str[idx] = '\0';
	}

	// Remove trailing spaces/tabs/\r
	int len = (int)strlen(str);
	while (len > 0 && (str[len - 1] == ' ' || str[len - 1] == '\t' || str[len - 1] == '\r'))
		str[--len] = '\0';
}

char *myStrdup(const char *src) {
	if (!src) return NULL;
	size_t len = strlen(src);
	char *dest = (char *)malloc(len + 1);
	if (!dest) {
		printf("Memory allocation failed in myStrdup.\n");
		return NULL;
	}
	strcpy(dest, src);
	return dest;
}

int readIntSafe(const char *prompt) {
	char buffer[INT_BUFFER];
	int value;
	int success = 0;
	while (!success) {
		printf("%s", prompt);
		// If we fail to read, treat it as invalid
		if (!fgets(buffer, sizeof(buffer), stdin)) {
			// if (feof(stdin)) return -1;
			printf("Invalid input.\n");
			clearerr(stdin);
			continue;
		}
		// 1) Strip any trailing \r or \n
		// so "123\r\n" becomes "123"
		size_t len = strlen(buffer);
		if (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r'))
			buffer[--len] = '\0';
		if (len > 0 && (buffer[len - 1] == '\r' || buffer[len - 1] == '\n'))
			buffer[--len] = '\0';
		// 2) Check if empty after stripping
		if (len == 0) {
			printf("Invalid input.\n");
			continue;
		}
		// 3) Attempt to parse integer with strtol
		char *endptr;
		value = (int)strtol(buffer, &endptr, 10);
		// If endptr didn't point to the end => leftover chars => invalid
		// or if buffer was something non-numeric
		if (*endptr != '\0') printf("Invalid input.\n");
		else success = 1;  // We got a valid integer
	}
	return value;
}

// --------------------------------------------------------------
// 2) Utility: Get type name from enum
// --------------------------------------------------------------
const char *getTypeName(PokemonType type) {
	switch (type) {
	case GRASS: return "GRASS";
	case FIRE: return "FIRE";
	case WATER: return "WATER";
	case BUG: return "BUG";
	case NORMAL: return "NORMAL";
	case POISON: return "POISON";
	case ELECTRIC: return "ELECTRIC";
	case GROUND: return "GROUND";
	case FAIRY: return "FAIRY";
	case FIGHTING: return "FIGHTING";
	case PSYCHIC: return "PSYCHIC";
	case ROCK: return "ROCK";
	case GHOST: return "GHOST";
	case DRAGON: return "DRAGON";
	case ICE: return "ICE";
	default: return "UNKNOWN";
	}
}

// --------------------------------------------------------------
// Utility: getDynamicInput (for reading a line into malloc'd memory)
// --------------------------------------------------------------
char *getDynamicInput(void) {
	char *input = NULL;
	size_t size = 0, capacity = 1;
	input = (char *)malloc(capacity);
	if (!input) {
		printf("Memory allocation failed.\n");
		return NULL;
	}
	int c;
	while ((c = getchar()) != '\n' && c != EOF) {
		if (size + 1 >= capacity) {
			capacity *= 2;
			char *temp = (char *)realloc(input, capacity);
			if (!temp) {
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

char readDirection(const char *prompt) {
    printf("%s", prompt);
    char *input = getDynamicInput();
    if (!input) return '\0';
    char dir = tolower(input[0]);
    free(input);
    if (dir == 'f' || dir == 'b') return dir;
    return '\0';
}

void swapOwnerData(OwnerNode *a, OwnerNode *b) {
    char *tmpName = a->ownerName;
    PokemonNode *tmpRoot = a->pokedexRoot;
    a->ownerName = b->ownerName;
    a->pokedexRoot = b->pokedexRoot;
    b->ownerName = tmpName;
    b->pokedexRoot = tmpRoot;
}

void sortOwners(void) {
    if (!ownerHead || ownerHead->next == ownerHead) return;
    int swapped;
    do {
        swapped = 0;
        OwnerNode *cur = ownerHead;
        do {
            OwnerNode *n = cur->next;
            if (strcmp(cur->ownerName, n->ownerName) > 0) {
                swapOwnerData(cur, n);
                swapped = 1;
            }
            cur = n;
        } while (cur->next != ownerHead);
    } while (swapped);
	OwnerNode *min = ownerHead;
    OwnerNode *iter = ownerHead->next;
    while (iter != ownerHead) {
        if (strcmp(iter->ownerName, min->ownerName) < 0) min = iter;
        iter = iter->next;
    }
    ownerHead = min;
}

PokemonNode* pokemonCircleToTree(PokemonNode *root) {
    if (!root) return NULL;
    PokemonNode *t = root;
    PokemonNode *treeRoot = NULL;
    PokemonNode *next;
    do {
        next = t->right;
        PokemonNode *clone = (PokemonNode *)malloc(sizeof(PokemonNode));
        if (!clone) {
            return treeRoot;
        }
        *clone = *t;
        clone->left = clone->right = NULL;
        if (!treeRoot) {
            treeRoot = clone;
        } else {
            PokemonNode *p = treeRoot;
            for (;;) {
                if (clone->data->id < p->data->id) {
                    if (!p->left) {
						p->left = clone;
						break;
					}
                    p = p->left;
                } else {
                    if (!p->right) {
						p->right = clone;
						break;
					}
                    p = p->right;
                }
            }
        }
        t = next;
    } while (t != root);
    return treeRoot;
}

void freePokemonTree(PokemonNode **root) {
    if (!*root) return;
    freePokemonTree(&(*root)->left);
    freePokemonTree(&(*root)->right);
    free(*root);
    *root = NULL;
}

void printPokemonNode(PokemonNode *node) {
	if (!node || !node->data) return;
	printf("ID: %d, Name: %s, Type: %s, HP: %d, Attack: %d, Can Evolve: %s\n",
		node->data->id,
		node->data->name,
		getTypeName(node->data->TYPE),
		node->data->hp,
		node->data->attack,
		(node->data->CAN_EVOLVE == CAN_EVOLVE) ? "Yes" : "No");
}

void BFSGeneric(PokemonNode *root, VisitNodeFunc visit) {
    if (!root || !visit) return;
    Queue q;
    initQueue(&q);
    enqueue(&q, root);
    while (q.front) {
        PokemonNode *node = dequeue(&q);
        visit(node);
        if (node->left) enqueue(&q, node->left);
        if (node->right) enqueue(&q, node->right);
    }
    freeQueue(&q);
}

void preOrderGeneric(PokemonNode *root, VisitNodeFunc visit) {
    if (!root) return;
    visit(root);
    preOrderGeneric(root->left, visit);
    preOrderGeneric(root->right, visit);
}

void inOrderGeneric(PokemonNode *root, VisitNodeFunc visit) {
    if (!root) return;
    inOrderGeneric(root->left, visit);
	visit(root);
    inOrderGeneric(root->right, visit);
}

void postOrderGeneric(PokemonNode *root, VisitNodeFunc visit) {
    if (!root) return;
    postOrderGeneric(root->left, visit);
    postOrderGeneric(root->right, visit);
	visit(root);
}

void preOrderTraversal(PokemonNode *root) {
	if (root==NULL){
		printf("Pokedex is empty.\n");
		return;
   }
	preOrderGeneric(root, printPokemonNode);
}

void inOrderTraversal(PokemonNode *root) {
	if (root==NULL){
		printf("Pokedex is empty.\n");
		return;
   }
	inOrderGeneric(root, printPokemonNode);
}

void postOrderTraversal(PokemonNode *root) {
	if (root==NULL){
		printf("Pokedex is empty.\n");
		return;
   }
	postOrderGeneric(root, printPokemonNode);
}

// --------------------------------------------------------------
// Display Menu
// --------------------------------------------------------------
void displayMenu(OwnerNode *owner) {
    if (!owner->pokedexRoot) {
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
	PokemonNode *treeRoot = pokemonCircleToTree(owner->pokedexRoot);
    switch (choice) {
	case 1:
		displayBFS(treeRoot);
		break;
	case 2:
		preOrderTraversal(treeRoot);
		break;
	case 3:
		inOrderTraversal(treeRoot);
		break;
	case 4:
		postOrderTraversal(treeRoot);
		break;
	case 5:
		displayAlphabetical(treeRoot);
		break;
	default:
		printf("Invalid choice.\n");
    }
	freePokemonTree(&treeRoot);
}

void displayAlphabetical(PokemonNode *root) {
    if (!root) {
        printf("Pokedex is empty.\n");
        return;
    }
    NodeArray na;
    initNodeArray(&na, 16);
    collectAll(root, &na);
    qsort(na.nodes, na.size, sizeof(PokemonNode *), compareByNameNode);
    for (int i = 0; i < na.size; i++)
        printPokemonNode(na.nodes[i]);
    free(na.nodes);
}

static void initNodeArray(NodeArray *na, int cap) {
    na->nodes = malloc(cap * sizeof(PokemonNode *));
    na->size = 0;
    na->capacity = cap;
}
static void addNode(NodeArray *na, PokemonNode *node) {
    if (na->size == na->capacity) {
        na->capacity *= 2;
        na->nodes = realloc(na->nodes, na->capacity * sizeof(PokemonNode *));
    }
    na->nodes[na->size++] = node;
}

static void collectAll(PokemonNode *root, NodeArray *na) {
    if (!root) return;
    collectAll(root->left, na);
    addNode(na, root);
    collectAll(root->right, na);
}

static int compareByNameNode(const void *a, const void *b) {
    const PokemonNode *const *p1 = a;
    const PokemonNode *const *p2 = b;
    return strcmp((*p1)->data->name, (*p2)->data->name);
}

// --------------------------------------------------------------
// Sub-menu for existing Pokedex
// --------------------------------------------------------------
void enterExistingPokedexMenu(void) {
	printf("\nExisting Pokedexes:\n");
	if (!ownerHead) return;
	OwnerNode* owner = NULL;
	ownerByNumber(&owner, (char)0);
	if (!owner) return;
	printf("\nEntering %s's Pokedex...\n", owner->ownerName);
	int subChoice;
	do {
		printf("\n-- %s's Pokedex Menu --\n", owner->ownerName);
		printf("1. Add Pokemon\n");
		printf("2. Display Pokedex\n");
		printf("3. Release Pokemon (by ID)\n");
		printf("4. Pokemon Fight!\n");
		printf("5. Evolve Pokemon\n");
		printf("6. Back to Main\n");
		subChoice = readIntSafe("Your choice: ");
		switch (subChoice) {
		case 1:
			addPokemon(owner);
			break;
		case 2:
			displayMenu(owner);
			break;
		case 3:
			freePokemon(owner);
			break;
		case 4:
			pokemonFight(owner);
			break;
		case 5:
			evolvePokemon(owner);
			break;
		case 6:
			printf("Back to Main Menu.\n");
			break;
		default: printf("Invalid choice.\n");
		}
	} while (subChoice != 6);
}

void initQueue(Queue *q) {
    q->front = q->rear = NULL;
}

void enqueue(Queue *q, PokemonNode *p) {
    QueueNode *n = (QueueNode *)malloc(sizeof(QueueNode));
    if (!n) return;
    n->pokemon = p;
    n->next = NULL;
    if (!q->rear) q->front = q->rear = n;
    else q->rear = q->rear->next = n;
}

PokemonNode *dequeue(Queue *q) {
    if (!q->front) return NULL;
    QueueNode *n = q->front;
    PokemonNode *p = n->pokemon;
    if (!(q->front = n->next)) q->rear = NULL;
    free(n);
    return p;
}

void freeQueue(Queue *q) {
    while (q->front) dequeue(q);
}

PokemonNode *searchPokemonBFS(PokemonNode *root, int id) {
    if (!root) {printf("FUCK IRAN\n"); return NULL;}
    Queue q;
    initQueue(&q);
    enqueue(&q, root);
    while (q.front) {
        PokemonNode *node = dequeue(&q);
        if (node->data->id == id) {
            freeQueue(&q);
            return node;
        }
        if (node->left) enqueue(&q, node->left);
        if (node->right) enqueue(&q, node->right);
    }
    return NULL;
}

PokemonNode *removeNodeBST(PokemonNode *root, int id) {
    if (!root) {printf("FUCK YOU\n"); return NULL;}
    if (id < root->data->id) root->left = removeNodeBST(root->left, id);
    else if (id > root->data->id) root->right = removeNodeBST(root->right, id);
    else {
        if (!root->left && !root->right) {
            freePokemonNode(root);
            return NULL;
        } else if (!root->left) {
            PokemonNode *temp = root->right;
            freePokemonNode(root);
            return temp;
        } else if (!root->right) {
            PokemonNode *temp = root->left;
            freePokemonNode(root);
            return temp;
        } else {
            PokemonNode *succ = root->right;
            while (succ->left) succ = succ->left;
            root->data = succ->data;
            root->right = removeNodeBST(root->right, succ->data->id);
        }
    }
    return root;
}

PokemonNode *removePokemonByID(PokemonNode *root, int id) {
    if (!searchPokemonBFS(root, id)) 
        return root;
    return removeNodeBST(root, id);
}

void freePokemon(OwnerNode *owner) {
    int id = readIntSafe("Enter Pokemon ID to release: ");
    if (id < 1 || id > 151 || !owner->pokedexRoot) return;
    PokemonNode *cur = owner->pokedexRoot, *target = NULL;
    do {
        if (cur->data->id == id) { target = cur; break; }
        cur = cur->right;
    } while (cur != owner->pokedexRoot);
    if (!target) {
        printf("No Pokemon with ID %d found.\n", id);
        return;
    }
    PokemonNode *newRoot;
    if (target->left == target && target->right == target) {
        newRoot = NULL;
    } else {
        newRoot = (target == owner->pokedexRoot) ? target->right : owner->pokedexRoot;
        target->left->right = target->right;
        target->right->left = target->left;
    }
    freePokemonNode(target);
    target = NULL;
    owner->pokedexRoot = newRoot;
}

void freePokemonNode(PokemonNode *node) {
    if (!node) return;
    node->data = NULL;
    node->left = node->right = NULL;
    free(node);
}

void displayBFS(PokemonNode *root) {
    if (!root) {
        printf("(?) DEBUG: Pokedex is empty. ___OR___ No Pokemon to release.\n");
        return;
    }
    Queue q;
    initQueue(&q);
    enqueue(&q, root);
    while (q.front) {
        PokemonNode *node = dequeue(&q);
        printPokemonNode(node);
        if (node->left) enqueue(&q, node->left);
        if (node->right) enqueue(&q, node->right);
    }
    freeQueue(&q);
}

void pokemonFight(OwnerNode *owner) {
    if (!owner->pokedexRoot) {
        printf("Pokedex is empty.\n");
        return;
    }
    PokemonNode *tree = pokemonCircleToTree(owner->pokedexRoot);
    int id1 = readIntSafe("Enter ID of the first Pokemon: ");
    int id2 = readIntSafe("Enter ID of the second Pokemon: ");
    PokemonNode *p1 = searchPokemonBFS(tree, id1);
    PokemonNode *p2 = searchPokemonBFS(tree, id2);
    if (!p1 || !p2) {
        printf("One or both Pokemon IDs not found.\n");
        freePokemonTree(&tree);
        return;
    }
    double score1 = p1->data->attack * 1.5 + p1->data->hp * 1.2;
    double score2 = p2->data->attack * 1.5 + p2->data->hp * 1.2;
    printf("Pokemon 1: %s (Score = %.2f)\n", p1->data->name, score1);
    printf("Pokemon 2: %s (Score = %.2f)\n", p2->data->name, score2);
    if (score1 > score2)
        printf("%s wins!\n", p1->data->name);
    else if (score2 > score1)
        printf("%s wins!\n", p2->data->name);
    else
        printf("It's a tie!\n");
    freePokemonTree(&tree);
}

void evolvePokemon(OwnerNode *owner) {
    if (!owner->pokedexRoot) {
        printf("Cannot evolve. Pokedex empty.\n");
        return;
    }
    int id = readIntSafe("Enter ID of Pokemon to evolve: ");
    PokemonNode *tree = pokemonCircleToTree(owner->pokedexRoot);
    PokemonNode *found = searchPokemonBFS(tree, id);
    freePokemonTree(&tree);
    if (!found) {
        printf("Pokemon with ID %d not found.\n", id);
        return;
    }
    int newId = id + 1;
    owner->pokedexRoot = removePokemonByID(owner->pokedexRoot, id);
    PokemonNode *c = createPokemonNode(&pokedex[newId - 1]);
    c->left = c->right = c;
    if (!owner->pokedexRoot)
        owner->pokedexRoot = c;
    else
        insertPokemonNode(owner->pokedexRoot, c);
    printf("Pokemon evolved from %s (ID %d) to %s (ID %d).\n",
           found->data->name, id,
           pokedex[newId - 1].name, newId);
}

// --------------------------------------------------------------
// New Pokedex
// --------------------------------------------------------------
int computeStarterID(menuChoice) {return (menuChoice * 3) - 3;}

void openPokedexMenu(void) {
	printf("Your name: ");
	char *ownerName = getDynamicInput();
	if (!ownerName) return;
	if (findOwnerByName(ownerName)) {
		printf("Owner '%s' already exists. Not creating a new Pokedex.\n", ownerName);
		free(ownerName);
		return;
	}
	int menuChoice = readIntSafe(
		"Choose Starter:\n"
		"1. Bulbasaur\n2. Charmander\n3. Squirtle\n"
		"Your choice: ");
	if (menuChoice < FIRST_STARTER || menuChoice > LAST_STARTER) {
		free(ownerName);
		return;
	}
	PokemonNode *starter = createPokemonNode(&pokedex[computeStarterID(menuChoice)]);
	if (!starter) {
		free(ownerName);
		return;
	}
	starter->left = starter->right = starter;
	OwnerNode *ownerNode = createOwner(ownerName, starter);
	if (!ownerNode) {
		freePokemonNode(starter);
		free(ownerName);
		return;
	}
	if (!ownerHead) ownerHead = ownerNode->prev = ownerNode->next = ownerNode;
	else linkOwnerInCircularList(ownerNode);
}

void linkOwnerInCircularList(OwnerNode *newOwner) {
	newOwner->prev = ownerHead->prev;
	newOwner->next = ownerHead;
	ownerHead->prev->next = newOwner;
	ownerHead->prev = newOwner;
}

void removeOwnerFromCircularList(OwnerNode *owner) {
    if (!owner || !ownerHead) return;
    if (owner->next == owner && owner->prev == owner) {
        ownerHead = NULL;
        return;
    }
    if (owner->prev && owner->next) {
        owner->prev->next = owner->next;
        owner->next->prev = owner->prev;
    }
    if (ownerHead == owner)
        ownerHead = owner->next;
}

void printOwnersCircular(OwnerNode *owner) {
    if (! (ownerHead && owner)) return;
	char direction = toupper(readDirection("Enter direction (F or B): "));
    if (!(direction || direction == 'F' || direction == 'B')) return;
	while (getchar() != '\n') {continue;}
    int repeatCount = 0;
	// ?? EXPECT RE-PROMPT ??
    if (repeatCount = readIntSafe("How many prints? "), repeatCount <= 0) return;
    OwnerNode *t = ownerHead;
    for (int i = 0; i < repeatCount; i++) {
        printf("[%d] %s\n", i + 1, t->ownerName);
        t = (direction == 'F') ? t->next : t->prev;
    }
}

OwnerNode *findOwnerByName(const char *name) {
	if (!ownerHead) return NULL;
	OwnerNode *node = ownerHead;
	while(strcmp(node->ownerName, name) != 0) {
		if (node->next == ownerHead) return NULL;
		node = node->next;
	}
	return node;
}

void ownerByNumber(OwnerNode **owner, char ifDelete) {
	if (!ownerHead) return;
	*owner = ownerHead;
	int ind = 0;
	do {
		printf("%d. %s\n", ++ind, (*owner)->ownerName);
		*owner = (*owner)->next;
	} while (*owner != ownerHead);
	int select = readIntSafe(
		ifDelete ? "Choose a Pokedex to delete by number: "
			: "Choose a Pokedex by number: ");
	if (select < 1 || select > ind) {
		*owner = NULL;
		printf("DEBUG PRINT: OWNER DOESN'T EXIST\n");
		return;
	}
	ind = 0;
	while (++ind != select) *owner = (*owner)->next;
}

void deletePokedex(void) {
	if (!ownerHead) return;
	OwnerNode *owner = NULL;
	ownerByNumber(&owner, (char)1);
	if (!owner) return;
	if (owner == ownerHead)
		ownerHead = ownerHead->next != ownerHead ?
			ownerHead->next
			: NULL;
	freeOwnerNode(owner);
	free(owner);
	owner = NULL;
}

void ownerByName(OwnerNode **owner, char whichOwner) {
	if (!ownerHead) {
		*owner = NULL;
		return; 
	}
	printf("%s", 
		whichOwner == MERGE_DST
			? "Enter name of first owner: "
			: "Enter name of second owner: ");  // MERGE_SRC
	char *name = getDynamicInput();
	if (!name) {
		*owner = NULL;
		return; 
	}
	OwnerNode *cur = ownerHead;
	do {
		if (strcmp(cur->ownerName, name) == 0) {
			*owner = cur;
			free(name);
			return;
		}
		cur = cur->next;
	} while (cur != ownerHead);
	free(name);
	*owner = NULL;
}

void mergePokedexMenu(void) {
    if (!ownerHead || ownerHead->next == ownerHead) return;
    OwnerNode *dst = NULL, *src = NULL;
    ownerByName(&dst, MERGE_DST);
    ownerByName(&src, MERGE_SRC);
    if (!dst || !src) return;
    if (!src->pokedexRoot) {
        freeOwnerNode(src);
        free(src);
        return;
    }
    PokemonNode *dstTree = pokemonCircleToTree(dst->pokedexRoot);
    PokemonNode *srcTree = pokemonCircleToTree(src->pokedexRoot);
    if (!srcTree) {
		freePokemonTree(&dstTree);
		freeOwnerNode(src);
		free(src);
		src = NULL;
		return;
    }
    Queue q;
    initQueue(&q);
    enqueue(&q, srcTree);
    while (q.front) {
        PokemonNode *n = dequeue(&q);
        if (!searchPokemonBFS(dstTree, n->data->id)) {
            PokemonNode *c = createPokemonNode(n->data);
			if (!c) break;
            c->left = c->right = c;
            if (!dst->pokedexRoot) dst->pokedexRoot = c;
            else insertPokemonNode(dst->pokedexRoot, c);
        }
        if (n->left) enqueue(&q, n->left);
        if (n->right) enqueue(&q, n->right);
    }
    freeQueue(&q);
    freePokemonTree(&srcTree);
    freePokemonTree(&dstTree);
    freeOwnerNode(src);
    free(src);
}

OwnerNode *createOwner(char *ownerName, PokemonNode *starter) {
	if (!(ownerName || starter)) return NULL; 
	OwnerNode *owner = (OwnerNode *)malloc(sizeof(OwnerNode));
	if (!owner) return 	NULL;
	owner->ownerName = ownerName;
	owner->pokedexRoot = starter;
	owner->prev = ownerHead ? ownerHead->prev : owner;
	owner->next = ownerHead ? ownerHead : owner;
	return owner;
}

void freeAllOwners(void) {
    if (!ownerHead) return;
    OwnerNode *head = ownerHead;
    OwnerNode *curr = head->next;
    OwnerNode *next;
    while (curr != head) {
        next = curr->next;
        freeOwnerNode(curr);
        free(curr);
        curr = next;
    }
    freeOwnerNode(head);
    free(head);
    ownerHead = NULL;
}

void freeOwnerNode(OwnerNode *owner) {
	if (!owner) return;
	free(owner->ownerName);
	owner->ownerName = NULL;
	removeOwnerFromCircularList(owner);
	PokemonNode *root = owner->pokedexRoot;
	PokemonNode *pokemon = root->right;
	PokemonNode *next;
	while (pokemon != root) {
		pokemon->left->right = pokemon->right;
		pokemon->right->left = pokemon->left;
		next = pokemon->right;
		freePokemonNode(pokemon);
		pokemon = next;
	}
	freePokemonNode(root);
	owner->pokedexRoot = NULL;
	// free owner in caller
}

void addPokemon(OwnerNode *owner) {
	int id = readIntSafe("Enter ID to add: ");
	if (id < 1 || id > 151) {
		printf("DEBUG PRINT: INVALID ID\n");
		return;
	}
	if (searchPokemonBFS(owner->pokedexRoot, id)) {
		printf("Pokemon with ID %d is already in the Pokedex. No changes made.\n", id);
		return;
	}
	PokemonNode *pokemon = createPokemonNode(&pokedex[id-1]);
	if (!pokemon) return;
	pokemon->left = pokemon->right = pokemon;
	insertPokemonNode(owner->pokedexRoot, pokemon);
	return;
}

PokemonNode *insertPokemonNode(PokemonNode *root, PokemonNode *newNode) {
	newNode->right = root;
	newNode->left = root->left;
	root->left->right = newNode;
	root->left = newNode;
	return newNode;
}

PokemonNode *createPokemonNode(const PokemonData *data) {
	PokemonNode *poke = (PokemonNode*)malloc(sizeof(PokemonNode));
	if (!poke) return NULL;
	poke->data = data;
	poke->left = poke->right = NULL;  // assign in caller
	return poke;
}

// --------------------------------------------------------------
// Main Menu
// --------------------------------------------------------------
void mainMenu(void) {
	int choice;
	do {
		printf("\n=== Main Menu ===\n");
		printf("1. New Pokedex\n");
		printf("2. Existing Pokedex\n");
		printf("3. Delete a Pokedex\n");
		printf("4. Merge Pokedexes\n");
		printf("5. Sort Owners by Name\n");
		printf("6. Print Owners in a direction X times\n");
		printf("7. Exit\n");
		choice = readIntSafe("Your choice: ");
		switch (choice) {
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
			mergePokedexMenu();
			break;
		case 5:
			sortOwners();
			break;
		case 6:
			printOwnersCircular(ownerHead);
			break;
		case 7:
			printf("Goodbye!\n");
			break;
		default: printf("Invalid.\n");
		}
	} while (choice != 7);
}

int main(void) {
	mainMenu();
	freeAllOwners();
	return 0;
}