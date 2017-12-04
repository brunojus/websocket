#include "blacklist.h"


int verifyDenyTerm(const char* buffer) {
	//returns -1 if found a deny term
	// returns 1 otherwise
	
	FILE* fterms;
	fterms = fopen ("./websocket/deny_terms.txt", "r");
	char term[256];
	
	if (fterms == NULL) { //arquivo nao existe
		printf("File deny_terms.txt not found\n");
	}
	else { //verifying whitelist
		while (fgets(term, sizeof(term), fterms)) {
			int size = strlen (term);
			term[size-1] = '\0'; //apagar o \n do fgets
			if (strstr(buffer,term)) { //TERM FOUND IN BUFFER
				printf("Deny Term found: %s\n", term);
				return -1;
			}
		}
		fclose (fterms);
	}
	printf("No deny terms found\n");
	return 1;
}