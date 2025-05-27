#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 256
#define THRESHOLD 80

char uniqueChars[100];
char words[50][MAX_LINE_LENGTH];
char inputSentence[MAX_LINE_LENGTH];

char **questions;
char **answers;
char **kb;

int num_pairs = 0;
int word_embedding_dim;
int max_sentence_length;
int embedding_length;
int max_word_length;
int wordCount = 0;

void oneHotEncodeWord(char *word, char uniqueChars[], int uniqueCharCount, int *encoding) {
    int wordLen = strlen(word);
    int padding = max_word_length - wordLen;
    int i, j;

    for (i = 0; i < max_word_length * uniqueCharCount; i++) {
        encoding[i] = 0;
    }

    for (i = 0; i < wordLen; i++) {
        char c = word[i];
        char *ptr = strchr(uniqueChars, c);
        if (ptr != NULL) {
            int charIndex = ptr - uniqueChars;
            encoding[(padding + i) * uniqueCharCount + charIndex] = 1;
        }
    }
}

void tokenize(char *sentence, char words[][MAX_LINE_LENGTH], int *wordCount) {
    *wordCount = 0;
    char *token = strtok(sentence, " ");
    while (token != NULL) {
        strcpy(words[*wordCount], token);
        (*wordCount)++;
        token = strtok(NULL, " ");
    }
}

void generateSentenceEmbedding(char words[][MAX_LINE_LENGTH], int wordCount, int *sentenceEmbedding, int uniqueCharCount) {
    int i, j, embeddingIndex = 0;

    for (i = 0; i < max_sentence_length; i++) {
        if (i < wordCount) {
            int *wordEncoding = malloc(max_word_length * uniqueCharCount * sizeof(int));
            oneHotEncodeWord(words[i], uniqueChars, uniqueCharCount, wordEncoding);
            for (j = 0; j < max_word_length * uniqueCharCount; j++) {
                sentenceEmbedding[embeddingIndex++] = wordEncoding[j];
            }
            free(wordEncoding);
        } else {
            for (j = 0; j < max_word_length * uniqueCharCount; j++) {
                sentenceEmbedding[embeddingIndex++] = 0;
            }
        }
    }
}

void load_embeddings(const char *filename) {
    FILE *file = fopen(filename, "r");
    char line[30000];
    int index = 0;

    int i;

    if (!file) {
        perror("Error opening file");
        exit(1);
    }

    printf("[DEBUG] Dosya open: %s\n", filename);

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "uniqueChars: ", 13) == 0) {
            strncpy(uniqueChars, line + 13, sizeof(uniqueChars) - 1);
            uniqueChars[strcspn(uniqueChars, "\n")] = '\0';
        } else if (strncmp(line, "Max sentence length", 19) == 0) {
            max_sentence_length = atoi(strchr(line, ':') + 1);
        } else if (strncmp(line, "Max word length", 15) == 0) {
            max_word_length = atoi(strchr(line, ':') + 1);
        } else if (strncmp(line, "Max embedding length", 20) == 0) {
            embedding_length = atoi(strchr(line, ':') + 1);
        } else if (strncmp(line, "Dimension of word embeddings", 28) == 0) {
            word_embedding_dim = atoi(strchr(line, ':') + 1);
        } else if (strncmp(line, "Number of question-answer pairs", 31) == 0) {
            num_pairs = atoi(strchr(line, ':') + 1);

            questions = malloc(num_pairs * sizeof(char *));
            answers = malloc(num_pairs * sizeof(char *));
            kb = malloc(num_pairs * sizeof(char *));
            for (i = 0; i < num_pairs; i++) {
                questions[i] = malloc((embedding_length + 1) * sizeof(char));
                answers[i] = malloc((embedding_length + 1) * sizeof(char));
                kb[i] = malloc((embedding_length + 1) * sizeof(char));
            }
        } else if (strncmp(line, "Question: ", 10) == 0) {
            strncpy(questions[index], line + 10, embedding_length);
            questions[index][embedding_length] = '\0';
        } else if (strncmp(line, "Answer: ", 8) == 0) {
            strncpy(answers[index], line + 8, embedding_length);
            answers[index][embedding_length] = '\0';
            index++;
        }
    }

    fclose(file);
    printf("[DEBUG] load embed working well.\n");
}

void generateKnowledgeBase() {
    int i, j;
    for (i = 0; i < num_pairs; i++) {
        for (j = 0; j < embedding_length; j++) {
            kb[i][j] = ((questions[i][j] - '0') ^ (answers[i][j] - '0')) + '0';
        }
        kb[i][embedding_length] = '\0';
    }
    printf("[DEBUG] Knowledge base working well.\n");
}

float calculateMatchingScore(char *a, char *b) {
    int i, mismatches = 0;
    for (i = 0; i < embedding_length; i++) {
        if (a[i] == '1') {
            mismatches++;
        }
    }
    return (1.0 - ((float)mismatches / embedding_length)) * 100.0;
}

void kelime_cevir(char* binaryEmbedding) {
    int uniqueCharCount = strlen(uniqueChars);
    int i, j, k;

    printf("Matched Answer (decoded): ");

    for (i = 0; i < max_sentence_length; i++) {
        for (j = 0; j < max_word_length; j++) {
            int baseIndex = (i * max_word_length + j) * uniqueCharCount;
            for (k = 0; k < uniqueCharCount; k++) {
                if (binaryEmbedding[baseIndex + k] == '1') {
                    printf("%c", uniqueChars[k]);
                    break;
                }
            }
        }
        printf(" ");  
    }
    printf("\n");
}


int main() {
    int i, j;
    int uniqueCharCount;
    int *sentenceEmbedding;
    char *queryEmbeddingChar;
    char *retrievedAnswer;
    float maxMatchingScore;
    int bestMatchIndex;

    printf("[DEBUG] Program started.\n");

    load_embeddings("embeddings.txt");

    printf("Enter a sentence to encode: ");
    fgets(inputSentence, MAX_LINE_LENGTH, stdin);
    inputSentence[strcspn(inputSentence, "\n")] = 0;

    tokenize(inputSentence, words, &wordCount);

    uniqueCharCount = strlen(uniqueChars);
    sentenceEmbedding = malloc(max_sentence_length * max_word_length * uniqueCharCount * sizeof(int));
    if (sentenceEmbedding == NULL) {
        perror("Memory failed");
        return 1;
    }

    generateSentenceEmbedding(words, wordCount, sentenceEmbedding, uniqueCharCount);

    queryEmbeddingChar = malloc((embedding_length + 1) * sizeof(char));
    for (i = 0; i < embedding_length; i++) {
        queryEmbeddingChar[i] = sentenceEmbedding[i] + '0';
    }
    queryEmbeddingChar[embedding_length] = '\0';

    retrievedAnswer = malloc((embedding_length + 1) * sizeof(char));
    maxMatchingScore = 0.0;
    bestMatchIndex = -1;

    generateKnowledgeBase();

    for (i = 0; i < num_pairs; i++) {
        for (j = 0; j < embedding_length; j++) {
            retrievedAnswer[j] = ((kb[i][j] - '0') ^ (queryEmbeddingChar[j] - '0')) + '0';
        }
        retrievedAnswer[embedding_length] = '\0';

        float score = calculateMatchingScore(retrievedAnswer, answers[i]);

        if (score >= THRESHOLD && (bestMatchIndex == -1 || score > maxMatchingScore)) {
            maxMatchingScore = score;
            bestMatchIndex = i;
        }
    }

    if (bestMatchIndex != -1) {
        printf("Max matching score: %.1f\n", maxMatchingScore);
        printf("Matching score: %.2f%%\n", maxMatchingScore);
        kelime_cevir(answers[bestMatchIndex]);
    } else {
        printf("No matching answer found above threshold.\n");
    }

    for (i = 0; i < num_pairs; i++) {
        free(questions[i]);
        free(answers[i]);
        free(kb[i]);
    }
    free(questions);
    free(answers);
    free(kb);
    free(sentenceEmbedding);
    free(queryEmbeddingChar);
    free(retrievedAnswer);

    return 0;
}
