#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_PAIRS 256
#define MAX_LINE_LENGTH 256
#define MAX_SENTENCE_LENGTH 256
#define SEPARATOR "---"
#define QUESTION_PREFIX "Q: "
#define ANSWER_PREFIX "A: "


void tokenize(const char* sentence, char words[][MAX_LINE_LENGTH], int *wordCount) {
    *wordCount = 0;
    char temp[MAX_LINE_LENGTH];
    strcpy(temp, sentence);
    char* token = strtok(temp, " \t\n\r");
    while (token != NULL) {
        strcpy(words[*wordCount], token);
        (*wordCount)++;
        token = strtok(NULL, " \t\n\r");
    }
}

int findUniqueChars(char questions[][MAX_LINE_LENGTH], char answers[][MAX_LINE_LENGTH], int numPairs, char uniqueChars[]) {
    int count = 0;
    for (int i = 0; i < numPairs; i++) {
        for (int j = 0; questions[i][j]; j++) {
            unsigned char c = (unsigned char)questions[i][j];
            if (strchr(uniqueChars, c) == NULL && isascii(c)) {
                uniqueChars[count++] = c;
            }
        }
        for (int j = 0; answers[i][j]; j++) {
            unsigned char c = (unsigned char)answers[i][j];
            if (strchr(uniqueChars, c) == NULL && isascii(c)) {
                uniqueChars[count++] = c;
            }
        }
    }
    uniqueChars[count] = '\0';
    return count;
}

int maxWordLength(char sentences[][MAX_LINE_LENGTH], int numPairs) {
    int maxLen = 0;
    for (int i = 0; i < numPairs; i++) {
        int len = 0;
        for (int j = 0; sentences[i][j]; j++) {
            if (sentences[i][j] == ' ') {
                if (len > maxLen) maxLen = len;
                len = 0;
            } else {
                len++;
            }
        }
        if (len > maxLen) maxLen = len;
    }
    return maxLen;
}

int maxSentenceLength(char questions[][MAX_LINE_LENGTH], char answers[][MAX_LINE_LENGTH], int numPairs) {
    int maxLen = 0;
    for (int i = 0; i < numPairs; i++) {
        int lenQ = 0, lenA = 0;
        for (int j = 0; questions[i][j]; j++)
            if (questions[i][j] == ' ') lenQ++;
        lenQ++;
        for (int j = 0; answers[i][j]; j++)
            if (answers[i][j] == ' ') lenA++;
        lenA++;
        if (lenQ > maxLen) maxLen = lenQ;
        if (lenA > maxLen) maxLen = lenA;
    }
    return maxLen;
}

void oneHotEncodeWord(char *word, char uniqueChars[], int uniqueCharCount, int maxWordLen, int *encoding) {
    int wordLen = strlen(word);
    int padding = maxWordLen - wordLen;
    for (int i = 0; i < maxWordLen * uniqueCharCount; i++) {
        encoding[i] = 0;
    }
    for (int i = 0; i < wordLen; i++) {
        char c = word[i];
        char *ptr = strchr(uniqueChars, c);
        if (ptr != NULL) {
            int charIndex = ptr - uniqueChars;
            encoding[(padding + i) * uniqueCharCount + charIndex] = 1;
        }
    }
}

void generateSentenceEmbedding(char sentence[][MAX_LINE_LENGTH], int wordCount, char uniqueChars[], int uniqueCharCount, int maxWordLen, int maxSentenceLen, int *sentenceEmbedding) {
    int index = 0;
    for (int i = 0; i < maxSentenceLen; i++) {
        if (i < wordCount) {
            int *wordEncoding = malloc(maxWordLen * uniqueCharCount * sizeof(int));
            oneHotEncodeWord(sentence[i], uniqueChars, uniqueCharCount, maxWordLen, wordEncoding);
            for (int j = 0; j < maxWordLen * uniqueCharCount; j++) {
                sentenceEmbedding[index++] = wordEncoding[j];
            }
            free(wordEncoding);
        } else {
            for (int j = 0; j < maxWordLen * uniqueCharCount; j++) {
                sentenceEmbedding[index++] = 0;
            }
        }
    }
}

int main() {
    FILE *fp = fopen("database.txt", "r");
    if (!fp) {
        perror("Dosya açılamadı");
        return 1;
    }

    char questions[MAX_PAIRS][MAX_LINE_LENGTH];
    char answers[MAX_PAIRS][MAX_LINE_LENGTH];
    int numPairs = 0;

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\r\n")] = 0;

        if (strncmp(line, QUESTION_PREFIX, strlen(QUESTION_PREFIX)) == 0) {
            strcpy(questions[numPairs], line + strlen(QUESTION_PREFIX));
        } else if (strncmp(line, ANSWER_PREFIX, strlen(ANSWER_PREFIX)) == 0) {
            strcpy(answers[numPairs], line + strlen(ANSWER_PREFIX));
            numPairs++;
            if (numPairs >= MAX_PAIRS) break;
        }
    }
    fclose(fp);

    char uniqueChars[256] = {0};
    int uniqueCharCount = findUniqueChars(questions, answers, numPairs, uniqueChars);

    int maxWordLen = maxWordLength(questions, numPairs);
    int maxWordLenA = maxWordLength(answers, numPairs);
    if (maxWordLenA > maxWordLen) maxWordLen = maxWordLenA;

    int maxSentenceLen = maxSentenceLength(questions, answers, numPairs);

    fp = fopen("embeddings.txt", "w");
    if (!fp) {
        perror("Embeeding cant open");
        return 1;
    }

    fprintf(fp, "Creating embedding...\n");
    fprintf(fp, "uniqueChars: ");
    for (int i = 0; i < uniqueCharCount; i++) {
        fprintf(fp, "%c", uniqueChars[i]);
    }
    fprintf(fp, "\n");
    fprintf(fp, "Max sentence length (words): %d\n", maxSentenceLen);
    fprintf(fp, "Max word length (chars): %d\n", maxWordLen);
    fprintf(fp, "Max embedding length (words*chars*uniqueChars): %d\n", maxSentenceLen * maxWordLen * uniqueCharCount);
    fprintf(fp, "Dimension of word embeddings: %d\n", maxWordLen * uniqueCharCount);
    fprintf(fp, "Number of question-answer pairs: %d\n\n", numPairs);

    for (int i = 0; i < numPairs; i++) {
        char questionWords[MAX_SENTENCE_LENGTH][MAX_LINE_LENGTH];
        char answerWords[MAX_SENTENCE_LENGTH][MAX_LINE_LENGTH];
        int questionWordCount = 0, answerWordCount = 0;

        tokenize(questions[i], questionWords, &questionWordCount);
        tokenize(answers[i], answerWords, &answerWordCount);

        int *questionEmbedding = malloc(maxSentenceLen * maxWordLen * uniqueCharCount * sizeof(int));
        int *answerEmbedding = malloc(maxSentenceLen * maxWordLen * uniqueCharCount * sizeof(int));

        generateSentenceEmbedding(questionWords, questionWordCount, uniqueChars, uniqueCharCount, maxWordLen, maxSentenceLen, questionEmbedding);
        generateSentenceEmbedding(answerWords, answerWordCount, uniqueChars, uniqueCharCount, maxWordLen, maxSentenceLen, answerEmbedding);

        fprintf(fp, "Question: ");
        for (int j = 0; j < maxSentenceLen * maxWordLen * uniqueCharCount; j++) {
            fprintf(fp, "%d", questionEmbedding[j]);
        }
        fprintf(fp, "\n");

        fprintf(fp, "Answer: ");
        for (int j = 0; j < maxSentenceLen * maxWordLen * uniqueCharCount; j++) {
            fprintf(fp, "%d", answerEmbedding[j]);
        }
        fprintf(fp, "\n");

        if (i < numPairs - 1) {
            fprintf(fp, "%s\n", SEPARATOR);
        }

        free(questionEmbedding);
        free(answerEmbedding);
    }

    fclose(fp);
    printf("Embedding file is created successfully.\n");
    return 0;
}
