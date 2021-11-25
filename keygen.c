#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

char get_char() {
    char c = (rand() % (90 - 64 + 1)) + 64;
    if (c == '@') c = ' ';
    return c;
}

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Error: you must enter exactly one command line argument, whcih specifies the key size.\n");
        return 1;
    }

    int size = atoi(argv[1]);
    
    srand(time(NULL));

    char *key = calloc(size + 2, sizeof(char));

    for (int i = 0; i < size; i++) {
        key[i] = get_char();
    }

    key[size] = '\n';
    key[size + 1] = '\0';


    printf("%s", key);

    free(key);

    return 0;
}

