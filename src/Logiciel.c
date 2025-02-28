#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include "utils.h"
#include <sys/wait.h>

#define MAX_KEYS 100
#define MAX_KEY_LENGTH 1024
#define KEY_FILE "keys.txt"

// Struct to store a key, its usage status, and its number
typedef struct {
    unsigned char key[MAX_KEY_LENGTH];
    bool used;
    int number;
} Key;

// Global variables
Key keys[MAX_KEYS];
int key_count = 0;
int next_key_number = 1;
FILE *log_file;

// Function prototypes
void log_action(const char *action);
void print_help();
void list_keys();
void generate_key(int n);
void delete_key(int number);
void decrypt_encrypt(const char *in, const char *out, const unsigned char *key, const char *method, const char *iv);
void crack(const char *in, const char *method, int length, const char *dico);
void load_keys();
void save_keys();

// Log function
void log_action(const char *action) {
    fprintf(log_file, "%s\n", action);
    fflush(log_file);
}

// Load keys from file
void load_keys() {
    FILE *file = fopen(KEY_FILE, "r");
    if (!file) {
        // Create the file if it doesn't exist
        file = fopen(KEY_FILE, "w");
        if (!file) {
            perror("Error creating key file");
            return;
        }
        fclose(file);
        printf("Key file '%s' created.\n", KEY_FILE);
        return;
    }

    // Read existing keys from the file
    key_count = 0;
    while (fscanf(file, "%d %s %d", &keys[key_count].number, keys[key_count].key, (int *)&keys[key_count].used) == 3) {
        if (keys[key_count].number >= next_key_number) {
            next_key_number = keys[key_count].number + 1;
        }
        key_count++;
    }
    fclose(file);
}

// Save keys to file
void save_keys() {
    FILE *file = fopen(KEY_FILE, "w");
    if (!file) {
        perror("Error opening key file");
        return;
    }
    for (int i = 0; i < key_count; i++) {
        fprintf(file, "%d %s %d\n", keys[i].number, keys[i].key, keys[i].used);
    }
    fclose(file);
}

// Display help
void print_help() {
    printf("Available commands:\n");
    printf("  help : Displays the list of commands\n");
    printf("  list-keys : Lists all generated keys and their usage status\n");
    printf("  gen-key <n> : Generates a key of length n\n");
    printf("  del-key <number> : Deletes the key with the specified number\n");
    printf("  encrypt <in> <out> <key> <method (xor,cbc-crypt)> [<iv> write <none> if not used] : Encrypts input using a specified key and method\n");
    printf("  decrypt <in> <out> <key> <method (xor,cbc-uncrypt)> [<iv> write <none> if not used] : Decrypts input using a specified key and method\n");
    printf("  crack <in> <method (c1,all)> <length> <dico> : Attempts to crack the input file\n");
    printf("  quit : Exits the program\n");
}

// List keys
void list_keys() {
    printf("Keys:\n");
    for (int i = 0; i < key_count; i++) {
        printf("  %d: %s (used: %s)\n", keys[i].number, keys[i].key, keys[i].used ? "yes" : "no");
    }
}

// Generate a key of specified length
void generate_key(int n) {
    if (key_count >= MAX_KEYS) {
        printf("Error: Maximum number of keys reached.\n");
        return;
    }
    if (n > MAX_KEY_LENGTH) {
        printf("Error: Key length exceeds maximum allowed size.\n");
        return;
    }
    unsigned char *key = gen_key(n);
    strncpy((char *)keys[key_count].key, (char *)key, MAX_KEY_LENGTH);
    keys[key_count].used = false;
    keys[key_count].number = next_key_number++;
    key_count++;
    printf("Generated key: %s\n", key);
    log_action("Key generated");
}

// Delete a specified key
void delete_key(int number) {
    for (int i = 0; i < key_count; i++) {
        if (keys[i].number == number) {
            for (int j = i; j < key_count - 1; j++) {
                keys[j] = keys[j + 1];
            }
            key_count--;
            printf("Key %d deleted.\n", number);
            log_action("Key deleted");
            return;
        }
    }
    printf("Error: Key not found.\n");
}

// Perform decryption or encryption
void decrypt_encrypt(const char *in, const char *out, const unsigned char *key, const char *method, const char *iv) {
    printf("Decrypting/encrypting file %s to %s using key %s and method %s\n", in, out, key, method);

    int pid = fork();
    if (pid == 0) { // Child process
        char *args[12];
        args[0] = "./sym_crypt";
        args[1] = "-i";
        args[2] = (char *)in;
        args[3] = "-o";
        args[4] = (char *)out;
        args[5] = "-k";
        args[6] = (char *)key;
        args[7] = "-m";
        args[8] = (char *)method;
        if (strcmp(iv,"none")!=0) {
            args[9] = "-v";
            args[10] = (char *)iv;
            args[11] = NULL;
        }else{
            args[9] = NULL;
        }

        execvp(args[0], args);
        perror("Error executing sym_crypt");
        exit(EXIT_FAILURE);
    } else { // Parent process
        waitpid(pid, NULL, 0);
    }

    log_action("Decryption/Encryption performed");
}

// Attempt to crack a file
void crack(const char *in, const char *method, int length, const char *dico) {
    printf("Cracking file %s with method %s, length %d, and dictionary %s\n", in, method, length, dico);

    int pid = fork();
    if (pid == 0) { // Child process
        char length_str[16];
        snprintf(length_str, sizeof(length_str), "%d", length);
        char str[20];  // Déclare un tableau de caractères suffisamment grand pour contenir la chaîne
        // Utilisation de sprintf pour convertir l'entier en chaîne
        sprintf(str, "%d", length);
        char *args[10];
        args[0] = "./break_code";
        args[1] = "-i";
        args[2] = (char *)in;
        args[3] = "-m";
        args[4] = (char *)method;
        args[5] = "-d";
        args[6] = (char *)dico;
        args[7] = "-k";
        args[8] = str;
        args[9] = NULL;

        execvp(args[0], args);
        perror("Error executing break_code");
        exit(EXIT_FAILURE);
    } else { // Parent process
        waitpid(pid, NULL, 0);
    }

    log_action("Crack attempt performed");
}

// Main program
int main() {
    srand(time(NULL));
    log_file = fopen("log.txt", "a");
    if (!log_file) {
        perror("Error opening log file");
        return EXIT_FAILURE;
    }

    char command[256];
    printf("Key Management System\nType 'help' for a list of commands.\n\n");
    load_keys();
    while (1) {
        printf("> ");
        if (!fgets(command, sizeof(command), stdin)) {
            break;
        }

        // Remove newline
        command[strcspn(command, "\n")] = 0;

        if (strcmp(command, "help") == 0) {
            print_help();
        } else if (strcmp(command, "list-keys") == 0) {
            list_keys();
        } else if (strncmp(command, "gen-key", 7) == 0) {
            int n;
            if (sscanf(command, "gen-key %d", &n) == 1) {
                generate_key(n);
            } else {
                printf("Usage: gen-key <n>\n");
            }
        } else if (strncmp(command, "del-key", 7) == 0) {
            int number;
            if (sscanf(command, "del-key %d", &number) == 1) {
                delete_key(number);
            } else {
                printf("Usage: del-key <number>\n");
            }
        } else if (strncmp(command, "encrypt", 7) == 0) {
            char in[256], out[256], method[256], iv[256];
            unsigned char key[MAX_KEY_LENGTH];
            if (sscanf(command, "encrypt %s %s %s %s %s", in, out, key, method, iv) >= 4) {
                decrypt_encrypt(in, out, key, method, iv);
            } else {
                printf("encrypt <in> <out> <key> <method (xor,cbc-crypt)> [<iv> write <none> if not used]\n");
            }
        } else if (strncmp(command, "decrypt", 7) == 0) {
            char in[256], out[256], method[256], iv[256];
            unsigned char key[MAX_KEY_LENGTH];
            if (sscanf(command, "decrypt %s %s %s %s %s", in, out, key, method, iv) >= 4) {
                decrypt_encrypt(in, out, key, method, iv);
            } else {
                printf("decrypt <in> <out> <key> <method (xor,cbc-uncrypt)> [<iv> write <none> if not used]\n");
            }
        } else if (strncmp(command, "crack", 5) == 0) {
            char in[256], out[256], dico[256];
            int length;
            if (sscanf(command, "crack %s %s %d %s", in, out, &length, dico) == 4) {
                crack(in, out, length, dico);
            } else {
                printf("Usage: crack <in> <method (c1,all)> <length> <dico>\n");
            }
        } else if (strcmp(command, "quit") == 0) {
            printf("Exiting program.\n");
            save_keys();
            break;
        } else {
            printf("Unknown command. Type 'help' for a list of commands.\n");
        }
    }

    fclose(log_file);
    return 0;
}
