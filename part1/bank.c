#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "account.h"
#include "string_parser.h"

int main(int argc, char const *argv[]){
    if(argc == 2){ //checking for command line argument && (strcmp(argv[1], "f") == 0)
        //opening file to read
        FILE *inFPtr;
        inFPtr = fopen (argv[1], "r");
        if(inFPtr == NULL) {
            printf("Error opening file");
            return 1;
        }

        // Redirect stdout to output.txt
        /**
        FILE *outFPtr = freopen("output.txt", "w", stdout);
        if(outFPtr == NULL) {
            printf("Error redirecting stdout to output.txt");
            fclose(inFPtr); // Close the input file
            return 1;
        }
        */

        //declare line_buffer
        size_t len = 128;
        char* line_buf = malloc (len);
        if (line_buf == NULL) {
            printf("Error allocating memory for line_buf");
            fclose(inFPtr); // Close the file 
            return 1;
        }

        command_line large_token_buffer;
        command_line small_token_buffer;

        int numAcc = 0;
        int line_num = 0;
        // Read the first line to get the number of accounts
        if (getline(&line_buf, &len, inFPtr) != -1) {
            numAcc = atoi(line_buf); // Convert the first line to an integer
            line_num++;
        } else {
            printf("Error reading the number of accounts");
            fclose(inFPtr); // Close the file
            free(line_buf); // Free the line buffer
            return 1;
        }

        account accounts[numAcc];

        for (int i = 0; i < numAcc; i++) {
            // Skip the index line (line 1 for each account)
            getline(&line_buf, &len, inFPtr);
            printf("Account %d index line: %s\n", i + 1, line_buf);

            for (int j = 0; j < 4; j++) {
                getline(&line_buf, &len, inFPtr);
                large_token_buffer = str_filler(line_buf, "\n");

                // Assign the parsed values to the appropriate struct fields
                if (j == 0) { 
                    // Account number
                    strncpy(accounts[i].account_number, large_token_buffer.command_list[0], 16);
                    accounts[i].account_number[16] = '\0'; // Null-terminate
                    printf("Account %d - Account Number: %s\n", i + 1, accounts[i].account_number);
                } else if (j == 1) {
                    // Password
                    strncpy(accounts[i].password, large_token_buffer.command_list[0], 8);
                    accounts[i].password[8] = '\0'; // Null-terminate
                    printf("Account %d - Password: %s\n", i + 1, accounts[i].password);
                } else if (j == 2) {
                    // Balance
                    accounts[i].balance = atof(large_token_buffer.command_list[0]); // Convert to double
                    printf("Account %d - Balance: %.2f\n", i + 1, accounts[i].balance);
                } else if (j == 3) {
                    // Reward rate
                    accounts[i].reward_rate = atof(large_token_buffer.command_list[0]); // Convert to double
                    printf("Account %d - Reward Rate: %.3f\n", i + 1, accounts[i].reward_rate);
                }

                line_num++;
                // Free memory for the token buffer
                free_command_line(&large_token_buffer);
            }
            printf("\nBreak!\n\n");
        }
        printf("Line Num: %d\n", line_num);

        printf("Printing all account details for verification:\n");
        for (int i = 0; i < numAcc; i++) {
            printf("Account %d:\n", i + 1);
            printf("  Account Number: %s\n", accounts[i].account_number);
            printf("  Password: %s\n", accounts[i].password);
            printf("  Balance: %.2f\n", accounts[i].balance);
            printf("  Reward Rate: %.3f\n", accounts[i].reward_rate);
            printf("-------------------------\n");
}


        fclose(inFPtr);
        //free line buffer
        free (line_buf);
        printf("End of file\nBye Bye\n");

        // Restore stdout back to the console
        /**
        fclose(outFPtr); // Close the output file
        freopen("/dev/tty", "w", stdout); // Restores stdout to terminal on UNIX-based systems
        return 0;
        */
    }
    else{
        printf("Invalid number of arguments\n");
        return 1;
    }
}


