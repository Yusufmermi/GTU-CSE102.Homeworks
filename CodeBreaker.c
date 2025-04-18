#include<stdio.h>
#include<stdlib.h>
#include <time.h>
#include <math.h>

void adminmode();
void playermode();
void rules();
void menu();
void endgame();
int* generate_code();
int* get_guess();

void menu() {
    char select;
    
    printf("-------Welcome to CODEBREAKER game------\n\n");
    printf("1- Press P for start game:\n");
    printf("2- Press A for admin mode:\n"); 
    printf("3- Press X for exit game:\n");
    printf("Awaiting order... : ");
    scanf(" %c", &select);
    
    switch (select) {
        case 'P':
        case 'p':
            playermode();
            break;

        case 'A':
        case 'a': {
            int password;
            printf("\nEnter your admin password: ");
            scanf("%d", &password);
            if(password != 1453){
                printf("Unauthorized access!\n");
                break;
            }
            printf("<Welcome to admin mode>\n");
            adminmode();
            menu();
            break;
        }
        case 'X':
        case 'x':
            printf("Exiting...\n");
            break;
            
        default:
            printf("Invalid selection!\n");
            menu();
    }
}

void adminmode() {
    int code_length, digit_range, allowduplicate, attemp_number, point_c, point_m, point_w;
    FILE *config = fopen("vault_config.txt", "w");
    
    if (config == NULL) {
        printf("Error opening config file!\n");
        return;
    }

    printf("Admin mode activated.\n");

    do {
        printf("Enter code length (Optimal: 4): ");
        while (scanf("%d", &code_length) != 1) {
            printf("\nInvalid value. Enter valid value again: ");
            while (getchar() != '\n');
        }
    } while (code_length < 0 || code_length > 10);
    fprintf(config, "code_length: %d\n", code_length);

    do {
        printf("Enter digit range (0-9): ");
        while (scanf("%d", &digit_range) != 1) {
            printf("\nInvalid value. Enter valid value again: ");
            while (getchar() != '\n');
        }
    } while (digit_range < 0 || digit_range > 9);
    fprintf(config, "digit_range: %d\n", digit_range);

    do {
        printf("Allow duplicate? (0 = No, 1 = Yes): ");
        while (scanf("%d", &allowduplicate) != 1) {
            printf("\nInvalid value. Enter valid value again: ");
            while (getchar() != '\n');
        }
    } while (allowduplicate < 0 || allowduplicate > 1);
    fprintf(config, "allow_duplicate: %d\n", allowduplicate);

    do {
        printf("Enter number of attempts (0-50): ");
        while (scanf("%d", &attemp_number) != 1) {
            printf("\nInvalid value. Enter valid value again: ");
            while (getchar() != '\n');
        }
    } while (attemp_number < 0 || attemp_number > 50);
    fprintf(config, "attempt_number: %d\n", attemp_number);

    do {
        printf("Enter point for correct digit in CORRECT place: ");
        while (scanf("%d", &point_c) != 1) {
            printf("\nInvalid value. Enter valid value again: ");
            while (getchar() != '\n');
        }
    } while (point_c < 0);
    fprintf(config, "point_correct_place: %d\n", point_c);

    do {
        printf("Enter point for correct digit in WRONG place: ");
        while (scanf("%d", &point_m) != 1) {
            printf("\nInvalid value. Enter valid value again: ");
            while (getchar() != '\n');
        }
    } while (point_m < 0);
    fprintf(config, "point_wrong_place: %d\n", point_m);

    do {
        printf("Enter a negative penalty point for wrong digit(Must be negative): ");
        while (scanf("%d", &point_w) != 1) {
            printf("\nInvalid value. Enter valid value again: ");
            while (getchar() != '\n');
        }
    } while (point_w > 0); // Penalty can be 0 too
    fprintf(config, "point_wrong_digit: %d\n", point_w);

    fclose(config);
    printf("\n\nRules re-designed and saved to file...\n");
}

void playermode() {
    int code,code_length, digit_range, allowduplicate, attemp_number, point_c, point_m, point_w;

    generate_code();
    FILE *config = fopen("vault_config.txt","r");
    fscanf(config, "code_length: %d\n", &code_length);
    fscanf(config, "digit_range: %d\n", &digit_range);
    fscanf(config, "allow_duplicate: %d\n", &allowduplicate);
    fscanf(config, "attempt_number: %d\n", &attemp_number);
    fscanf(config, "point_correct_place: %d\n", &point_c);
    fscanf(config, "point_wrong_place: %d\n", &point_m);
    fscanf(config, "point_wrong_digit: %d\n", &point_w);
    fclose(config);

    FILE *log = fopen("game_log.txt","w");

    if (log == NULL) {
        printf("Error opening log file!\n");
        return;
    }
    fprintf(log, "-- Vault Codebreaker Game Log ---\n");
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    fprintf(log, "\nGame date & time: %02d-%02d-%d %02d:%02d:%02d", 
           tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900,
           tm.tm_hour, tm.tm_min, tm.tm_sec);
    rules();
    FILE *vault = fopen("vault_code.txt","r");
    if (vault == NULL) {
        printf("Error opening vault file!\n");
        return;
    }
    fscanf(vault, "Vault code is: %d", &code);
    fclose(vault);
    fprintf(log, "\nVault code is: %d\n", code);
    fprintf(log, "Code length: %d\n", code_length);
    fprintf(log, "Digit range: %d\n", digit_range);
    fprintf(log, "Allow duplicate: %d\n", allowduplicate);
    fprintf(log, "Attempt number: %d\n\n\n", attemp_number);

    printf("\n\n<<<<Ready for CODEBREAK?>>>>\n");
    int *user_guess;
    int number_array[code_length];
    int guess_array[code_length];
    int penalty = 0;
    for (int j = code_length - 1; j >= 0; j--) { // Code => array
        number_array[j] = code % 10;
        code /= 10;
    }
    
        for(int i = 0; i < attemp_number; i++) {
            if(i == attemp_number){printf("\nThis is your last chance");}
            user_guess = get_guess();
            int temp_guess = *user_guess;
            for(int k = code_length - 1; k >=0; k--) {
                guess_array[k] = temp_guess % 10;

                temp_guess /= 10;
            }
            fprintf(log, "\nAttempt number: %d ", i);
            printf("Your attemps: %d / %d\n",i+1,attemp_number);
            fprintf(log, "%d => Feedback: ", *user_guess);
            free(user_guess); // Clearing memory for memory leak
            
            int count = 0;
            int correct_number = 0;
            int miss_number = 0;
            for(int t = 0; t < code_length; t++) {
                count = 0;
                for(int z = 0; z < code_length; z++) {
                    if(guess_array[t] == number_array[z]){
                    count++;}
                }
                
                if (guess_array[t] == number_array[t]) {
                    fprintf(log, "C ");
                    printf("C ");
                    correct_number++;
                }
                else if(count > 0)  {
                    fprintf(log, "M ");
                    printf("M ");
                    miss_number++;
                } else {
                    fprintf(log, "W ");
                    printf("W ");
                    penalty++;
                }
                
                temp_guess /= 10;
            }

            int total_points = 0;
            printf("\nTotal penalty: %d ", penalty);
            total_points = (point_c * correct_number) + (point_m * miss_number) + (point_w * penalty);
            printf("|| Score: %d\n", total_points);
                if(correct_number == code_length) {
                    printf("\n----Good job detective you found code all correctly!----\nYour inspect points is %d / %d.\nYou are ",total_points, point_c*correct_number);
                    fprintf(log, "\n----Good job detective you found code all correctly!----\nYour inspect points is %d.\nYou are ",total_points);
                }
                if(total_points >= (code_length * point_c * 0.9)){
                    printf("code master");
                    fprintf(log, " code master");}
                else if (total_points >= (code_length * point_c * 0.7)){
                    printf("cipher hunter");
                    fprintf(log, "cipher hunter");}
                else if (total_points >= (code_length * point_c * 0.5)){
                    printf("number sleuth");
                    fprintf(log, "number sleuth");}
                else if (total_points >= (code_length * point_c * 0.3)){
                    printf("safe kicker");
                    fprintf(log, "safe kicker");}
                else if (total_points >= (code_length * point_c * 0.1)){
                    printf("rookie");
                    fprintf(log, "rookie");}
                else{
                    printf("a code potato, press alt+f4 you loser");
                    fprintf(log, "a code potato, press alt+f4 you loser");}
                fclose(log);
                FILE *score = fopen("score.txt","w");
                if (score == NULL) {
                    printf("Error opening score file!\n");
                    return;
                }
                fprintf(score, "\n--Your last score--\n         %d ", total_points);
                fclose(score);
                break;
                }
            



            
            printf("\nYou used all of your attemps.") ;
            }

void rules() {

    int code_length, digit_range, allowduplicate, attemp_number, point_c, point_m, point_w;

    FILE *config = fopen("vault_config.txt","r");

    if (config == NULL) {
        printf("Error opening config file!\n");
        return;
    }
    printf("\n--RULES--\n");
    fscanf(config, "code_length: %d\n", &code_length);
    printf("Code lenght: %d\n", code_length);
    fscanf(config, "digit_range: %d\n", &digit_range);
    printf("Digit range: %d\n", digit_range);
    fscanf(config, "allow_duplicate: %d\n", &allowduplicate);
    printf("Is duplicats allowed(Yes:1 No:0): %d\n", allowduplicate);
    fscanf(config, "attempt_number: %d\n", &attemp_number);
    printf("Your attempt rights: %d\n", attemp_number);
    fscanf(config, "point_correct_place: %d\n", &point_c);
    fscanf(config, "point_wrong_place: %d\n", &point_m);
    fscanf(config, "point_wrong_digit: %d\n", &point_w); 
    printf("Points of\n-Correct: %d\n-Miss: %d\n-Penalty:%d\n", point_c, point_m, point_w);

    fclose(config);
}

int* generate_code() {
    int code_length, digit_range, allowduplicate;
    FILE *config = fopen("vault_config.txt","r");
    fscanf(config, "code_length: %d\n", &code_length);
    fscanf(config, "digit_range: %d\n", &digit_range);
    fscanf(config, "allow_duplicate: %d\n", &allowduplicate);
    fclose(config);

    srand(time(NULL));
    int *code = (int*)malloc(sizeof(int) * code_length);
    for (int i = 0; i < code_length; i++) {
        code[i] = rand() % (digit_range + 1);
        if (!allowduplicate) {
            for (int j = 0; j < i; j++) {
                if (code[i] == code[j]) {
                    i--;
                    break;
                }
            }
        }
    }
    FILE *vault = fopen("vault_code.txt","w");
    if (vault == NULL) {
        printf("Error opening vault file!\n");
        free(code);
        return NULL;
    }
    
    fprintf(vault, "Vault code is: ");
    for (int i = 0; i < code_length; i++) {
        fprintf(vault, "%d", code[i]);
    }
    fclose(vault);
    
    return code;
}

int* get_guess() {
    int code_lenght;
    int *guess = (int*)malloc(sizeof(int));
    FILE *config= fopen("vault_config.txt","r");
    fscanf(config,"code_length: %d", &code_lenght);
    fclose(config);
    int up = pow(10, code_lenght - 1);
    int down = pow(10, code_lenght) - 1;

        printf("Enter your guess: ");
        while(getchar() != '\n');
        while (1) {
            if(scanf("%d", guess) != 1) {
                printf("\nInvalid value. Enter valid value again: ");
                while (getchar() != '\n');
            } else if (*guess < up || *guess > down) {
                printf("\nInvalid value. Enter valid value again: ");
                while (getchar() != '\n');
            } else {
                break;
            }
        }
            return guess;
}

int main() {
    menu();
    return 0;
}