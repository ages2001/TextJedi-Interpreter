#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>


const char keywords[16][9] = {"asString", "asText", "from", "input", "insert", "int", "locate",
                              "new", "output", "override", "read", "size", "subs", "text",
                              "to", "write"};

int exitProgram(int code, char *errorMsg);

void freeMemory();

char ignoreSpacesInOperation(FILE *filePtr, char ch);

char ignoreSpacesOutsideOperation(FILE *filePtr, char ch);

bool isSpace(char ch);

int parseWord(FILE *filePtr, char first, char *str);

char parseNumber(FILE *filePtr, char first, int *number);

bool isKeyword(char *word);

bool isCommandKeyword(char *word);

char runKeyword(FILE *filePtr, char ch, char *word);

char checkComment(FILE *filePtr);

char spaceAndCommentOperations(int checkOperation, FILE *filePtr, char ch);

int isTextIdentifier(char *identifierName);

int isIntegerIdentifier(char *identifierName);

int isCharIdentifier(char *identifierName);

char callStringFunction(FILE *filePtr, char ch, char *funcName, char *strValue);

char callIntFunction(FILE *filePtr, char ch, char *funcName, int *numberValue);

char callCharFunction(FILE *filePtr, char ch, char *strValue);

char runCommand(FILE *filePtr, char ch, char *word);

char parseString(FILE *filePtr, char ch, char *str);

char returnStringValue(FILE *filePtr, char ch, char *strValue);

char returnIntegerValue(FILE *filePtr, char ch, int *intValue);

char returnCharValue(FILE *filePtr, char ch, char *charValue);

bool isStringFunction(char *funcName);

bool isIntFunction(char *funcName);

bool isCharFunction(char *funcName);

char *stringSubtraction(char *str1, char *str2);

void overrideFunction(char *myText, int location, char *ovrText);

int locateFunction(char *bigText, char *smallText, int start);

char *replaceChar(const char* myText, char oldChar, char newChar);

FILE *readFilePtr, *readCommentFilePtr, *writeCommandFilePtr;

char **textIdentifiers, **intIdentifiers, **charIdentifiers;
char **textValues, *charValues;
int *intValues;

int intListSize = 0, textListSize = 0, charListSize = 0;
int intValueListSize = 0, textValueListSize = 0, charValueListSize = 0;

int lineNumber = 1;

int main(int argc, char *argv[]) {
    char ch;

    textIdentifiers = (char **) malloc(sizeof(char *) * 1); // text (string) identifier names list for checking later
    intIdentifiers = (char **) malloc(sizeof(char *) * 1); // integer identifier names list for checking later
    charIdentifiers = (char **) malloc(sizeof(char *) * 1); // char identifier names list for checking later

    textValues = (char **) malloc(sizeof(char *) * 1); // text (string) identifiers' values stored
    intValues = (int *) malloc(sizeof(int) * 1); // integer identifiers' values stored
    charValues = (char *) malloc(sizeof(char) * 1); // char identifiers' values stored

    if ((textIdentifiers == NULL || intIdentifiers == NULL) || (textValues == NULL || intValues == NULL)) {
        printf("Memory allocation failed!");
        exit(1);
    }

    if (argc < 2) { // get file name from command prompt
        printf("You should enter file name from command prompt!\nExample: TextJedi myProg.tj\n");
        return -1;
    }

    char *fileName = argv[1];

    readFilePtr = fopen(fileName, "r");

    if (readFilePtr == NULL) {
        printf("File '%s' not found!\n", fileName);
        exitProgram(-1, "Cannot open the file!");
    }

    do {
        ch = fgetc(readFilePtr);
        ch = spaceAndCommentOperations(0, readFilePtr, ch);

        if (isalpha(ch) != 0) { // if ch is a letter
            char word[31];

            ch = parseWord(readFilePtr, ch, word);

            if (ch < 0)
                exitProgram(-1, "Identifier or keyword cannot longer than 30 characters!");

            if (isKeyword(word)) { // if word is a keyword
                if (isStringFunction(word) || isIntFunction(word))
                    exitProgram(-1, "Non-void function cannot be used without expression!");

                if (isSpace(ch) || ch == '/') {
                    ch = spaceAndCommentOperations(1, readFilePtr, ch);

                    ch = runKeyword(readFilePtr, ch, word);
                } else
                    exitProgram(-1, "Syntax error!");

                ch = spaceAndCommentOperations(1, readFilePtr, ch);

                if (ch == ';')
                    continue;
                exitProgram(-1, "Invalid character or missing ';' !");
            } else if (isTextIdentifier(word) != -1) { // if word is a text identifier
                int oldIdentifierIndex = isTextIdentifier(word);
                ch = spaceAndCommentOperations(1, readFilePtr, ch);

                if (ch == ':') {
                    ch = fgetc(readFilePtr);

                    if (ch == '=') {
                        ch = fgetc(readFilePtr);
                        ch = spaceAndCommentOperations(1, readFilePtr, ch);

                        char text[10000]; // Unlimited size
                        ch = returnStringValue(readFilePtr, ch, text);

                        if (ch == '+' || ch == '-') {
                            char text2[10000]; // Unlimited size

                            if (ch == '+') { // Addition
                                ch = fgetc(readFilePtr);
                                ch = spaceAndCommentOperations(1, readFilePtr, ch);

                                ch = returnStringValue(readFilePtr, ch, text2);
                                strcpy(textValues[oldIdentifierIndex], text);
                                strcat(textValues[oldIdentifierIndex], text2); // text + text2
                            } else { // Subtraction
                                ch = fgetc(readFilePtr);
                                ch = spaceAndCommentOperations(1, readFilePtr, ch);

                                ch = returnStringValue(readFilePtr, ch, text2);
                                if (strlen(text) < strlen(text2))
                                    exitProgram(-1,
                                                "Second string cannot bigger than the first string in subtraction operation!");

                                char *text3 = stringSubtraction(text, text2); // text - text2
                                strcpy(textValues[oldIdentifierIndex], text3);
                                free(text3);
                            }
                        } else if (ch == ';')
                            strcpy(textValues[oldIdentifierIndex], text);
                        else
                            exitProgram(-1, "Invalid assignment!");
                    } else
                        exitProgram(-1, "Invalid operator ':' !");

                    if (ch == ';')
                        continue;
                    exitProgram(-1, "Missing ';' !");
                } else
                    exitProgram(-1, "Operator ':=' missing!");
            } else if (isIntegerIdentifier(word) != -1) { // if word is a integer identifier
                int oldIdentifierIndex = isIntegerIdentifier(word);
                ch = spaceAndCommentOperations(1, readFilePtr, ch);

                if (ch == ':') {
                    ch = fgetc(readFilePtr);

                    if (ch == '=') {
                        ch = fgetc(readFilePtr);
                        ch = spaceAndCommentOperations(1, readFilePtr, ch);

                        int number;
                        ch = returnIntegerValue(readFilePtr, ch, &number);

                        if (ch == '+' || ch == '-') {
                            int number2, number3;

                            if (ch == '+') { // Addition
                                ch = fgetc(readFilePtr);
                                ch = spaceAndCommentOperations(1, readFilePtr, ch);

                                ch = returnIntegerValue(readFilePtr, ch, &number2);
                                number3 = number + number2;
                            } else { // Subtraction
                                ch = fgetc(readFilePtr);
                                ch = spaceAndCommentOperations(1, readFilePtr, ch);

                                ch = returnIntegerValue(readFilePtr, ch, &number2);
                                number3 = number - number2;
                                if (number3 < 0)
                                    exitProgram(-1, "Negative values are not allowed!");
                            }
                            intValues[oldIdentifierIndex] = number3;
                        } else if (ch == ';')
                            intValues[oldIdentifierIndex] = number;

                        else if (isSpace(ch) || ch == '/')
                            ch = spaceAndCommentOperations(1, readFilePtr, ch);

                        else
                            exitProgram(-1, "Invalid assignment!");
                    } else
                        exitProgram(-1, "Invalid operator ':' !");

                    if (ch == ';')
                        continue;
                    exitProgram(-1, "Missing ';' !");
                } else
                    exitProgram(-1, "Operator ':=' missing!");
            } else if (isCharIdentifier(word) != -1) { // if word is a char identifier
                int oldIdentifierIndex = isCharIdentifier(word);
                ch = spaceAndCommentOperations(1, readFilePtr, ch);

                if (ch == ':') {
                    ch = fgetc(readFilePtr);

                    if (ch == '=') {
                        ch = fgetc(readFilePtr);
                        ch = spaceAndCommentOperations(1, readFilePtr, ch);

                        char character;
                        ch = returnCharValue(readFilePtr, ch, &character);
                        charValues[oldIdentifierIndex] = character;

                        ch = spaceAndCommentOperations(1, readFilePtr, ch);
                    } else
                        exitProgram(-1, "Invalid operator ':' !");

                    if (ch == ';')
                        continue;
                    exitProgram(-1, "Missing ';' !");
                }
            } else {
                char errorMsg[100] = "Unknown word '";
                strcat(errorMsg, word);
                strcat(errorMsg, "' !");
                exitProgram(-1, errorMsg);
            }
        } else if (isdigit(ch)) { // if ch is a number
            exitProgram(-1, "Invalid usage of number!");
        } else if (ch == ';')
            continue;

        else if (isSpace(ch) || ch == '/')
            ch = spaceAndCommentOperations(0, readFilePtr, ch);

        else if (ch == '+')
            exitProgram(-1, "No operation chosen before using '+' operator!");

        else if (ch == '-')
            exitProgram(-1, "No operation chosen before using '-' operator!");

        else if (ch == ':') {
            ch = fgetc(readFilePtr);

            if (ch == '=')
                exitProgram(-1, "No operation chosen before using ':=' operator!");
            else
                exitProgram(-1, "Invalid operator ':' !");
        } else if (ch == EOF)
            break;

        else {
            printf("Unknown character found '%c' !\n", ch);
            exitProgram(-1, "Invalid character!");
        }

    } while (ch != EOF);

    printf("Operations completed successfully!");
    freeMemory();
    fclose(readFilePtr);
    return 0;
}

int exitProgram(int code, char *errorMsg) { // prints error reason and exits with error code
    printf("ERROR: %s (at line %d)", errorMsg, lineNumber);
    freeMemory();
    fclose(readFilePtr);
    exit(code);
}

void freeMemory() {
    for (int i = 0; i < textListSize; i++)
        free(textIdentifiers[i]);

    for (int i = 0; i < textValueListSize; i++)
        free(textValues[i]);

    for (int i = 0; i < intListSize; i++)
        free(intIdentifiers[i]);

    free(textIdentifiers);
    free(textValues);
    free(intIdentifiers);
    free(intValues);
}

char ignoreSpacesInOperation(FILE *filePtr, char ch) { // space, tab and new line checker while in operation
    do {
        if (ch == '\n')
            lineNumber++;
        ch = fgetc(filePtr);

    } while ((ch != EOF) && isSpace(ch));

    if (ch == EOF)
        exitProgram(-1, "End of file before operations complete!");

    return ch;
}

char ignoreSpacesOutsideOperation(FILE *filePtr, char ch) { // space, tab and new line checker while no operation
    do {
        if (ch == '\n')
            lineNumber++;
        ch = fgetc(filePtr);

    } while ((ch != EOF) && isSpace(ch));

    return ch;
}

bool isSpace(char ch) { // checks ch is space, new line or tab
    return (ch == ' ' || ch == '\n' || (ch == '\t'));
}

int parseWord(FILE *filePtr, char first, char *str) { // reads a word characters and adds to string array (str) from file
    char ch = fgetc(filePtr);
    int size = 1;
    str[0] = first;

    while (ch != EOF && (isalpha(ch) || isdigit(ch) || ch == '_')) {
        str[size++] = ch;
        if (size > 30) {
            return -1;
        }

        ch = fgetc(filePtr);
    }
    str[size] = '\0';
    return ch;
}

char parseNumber(FILE *filePtr, char first, int *number) { // reads digits and converts to integer number from file
    char digit = fgetc(filePtr);
    int size = 1;
    char digits[10000];
    digits[0] = first;

    while (digit != EOF && isdigit(digit)) {
        if (size > UINT_MAX) // Limited to C unsigned int size
            exitProgram(-1, "A number cannot bigger than C unsigned int size!");
        digits[size++] = digit;
        digit = fgetc(filePtr);
    }
    digit = spaceAndCommentOperations(1, filePtr, digit);

    digits[size] = '\0';
    char *endDigits;
    *number = strtol(digits, &endDigits, 10); // string to integer
    return digit;
}

bool isKeyword(char *word) { // checks word is a keyword
    int numberOfKeywords = sizeof(keywords) / sizeof(keywords[0]);

    for (int i = 0; i < numberOfKeywords; i++) {
        if (strcmp(word, keywords[i]) == 0)
            return true;
    }

    return false;
}

bool isCommandKeyword(char *word) { // checks word is a command
    bool check = (strcmp(word, keywords[3]) == 0) ||
                 (strcmp(word, keywords[7]) == 0) ||
                 (strcmp(word, keywords[8]) == 0) ||
                 (strcmp(word, keywords[10]) == 0) ||
                 (strcmp(word, keywords[15]) == 0);
    return check;
}

char runKeyword(FILE *filePtr, char ch, char *word) { // starts keyword operation
    if (isalpha(ch) != 0 || ch == '"' || ch == '\'') {
        if (!isCommandKeyword(word))
            exitProgram(-1, "Invalid keyword usage!");

        ch = runCommand(filePtr, ch, word);
    } else {
        printf("Invalid character '%c' !\n", ch);
        exitProgram(-1, "Invalid character or operator after keyword!");
    }

    return ch;
}

char checkComment(FILE *filePtr) { // checks '/' and '*' for detecting comments
    char ch = fgetc(readFilePtr);

    if (ch == '*') {
        ch = fgetc(filePtr);

        while (ch != EOF) {
            if (ch == '\n')
                lineNumber++;
            else if (ch == '*') {
                ch = fgetc(filePtr);
                if (ch == '/') {
                    return fgetc(filePtr);
                } else if (ch == '\n')
                    lineNumber++;
            }
            ch = fgetc(filePtr);
        }
        exitProgram(-1, "End of file before closing comment!");
    } else
        exitProgram(-1, "Invalid operator '/' !");
}

char spaceAndCommentOperations(int checkOperation, FILE *filePtr, char ch) { // processes and passes space, tab or new line
    while (isSpace(ch) || ch == '/') {
        if (isSpace(ch)) {
            if (checkOperation == 1) // this checks if it is inside operation or not
                ch = ignoreSpacesInOperation(filePtr, ch);
            else
                ch = ignoreSpacesOutsideOperation(filePtr, ch);
        } else
            ch = checkComment(filePtr);
    }
    return ch;
}

int isTextIdentifier(char *identifierName) { // if word is a text (string) variable
    for (int i = 0; i < textListSize; i++) {
        if (strcmp(textIdentifiers[i], identifierName) == 0)
            return i;
    }
    return -1;
}

int isIntegerIdentifier(char *identifierName) { // if word is an integer variable
    for (int i = 0; i < intListSize; i++) {
        if (strcmp(intIdentifiers[i], identifierName) == 0)
            return i;
    }
    return -1;
}

int isCharIdentifier(char *identifierName) { // if word is a char variable
    for (int i = 0; i < charListSize; i++) {
        if (strcmp(charIdentifiers[i], identifierName) == 0)
            return i;
    }
    return -1;
}

char callStringFunction(FILE *filePtr, char ch, char *funcName, char *strValue) { // run string-return function
    ch = spaceAndCommentOperations(1, filePtr, ch);
    if (ch != '(')
        exitProgram(-1, "Syntax error!");
    ch = fgetc(filePtr);
    ch = spaceAndCommentOperations(1, filePtr, ch);

    if (strcmp(funcName, keywords[0]) == 0) { // asString
        int numberValue;
        ch = returnIntegerValue(filePtr, ch, &numberValue);

        snprintf(strValue, sizeof(strValue), "%d", numberValue); // integer to string
    } else if (strcmp(funcName, keywords[4]) == 0) { // insert
        char myTextIdentifier[31], myText[10000], insertText[10000];
        int identifierIndex = -1;
        if (ch == '"')
            ch = returnStringValue(filePtr, ch, myText);
        else {
            ch = parseWord(filePtr, ch, myTextIdentifier);

            identifierIndex = isTextIdentifier(myTextIdentifier);
            if (identifierIndex < 0)
                exitProgram(-1, "Invalid identifier!");
            strcpy(myText, textValues[identifierIndex]);
        }
        if (ch != ',')
            exitProgram(-1, "Invalid 'insert' function usage!");

        ch = fgetc(filePtr);
        ch = spaceAndCommentOperations(1, filePtr, ch);

        int location;
        ch = returnIntegerValue(filePtr, ch, &location);
        if (location > strlen(myText))
            exitProgram(-1, "Invalid 'location' integer value!");

        if (ch != ',')
            exitProgram(-1, "Invalid 'insert' function usage!");

        ch = fgetc(filePtr);
        ch = spaceAndCommentOperations(1, filePtr, ch);

        ch = returnStringValue(filePtr, ch, insertText);

        int textLen = strlen(myText), insertTextLen = strlen(insertText);
        char temp[textLen + insertTextLen + 1];

        strncpy(temp, myText, location);
        temp[location] = '\0';
        strcat(temp, insertText);
        strcat(temp, myText + location);
        strcpy(strValue, temp);

        if (identifierIndex != -1)
            strcpy(textValues[identifierIndex], temp);
    } else if (strcmp(funcName, keywords[9]) == 0) { // override
        char myTextIdentifier[31], myText[10000], ovrText[10000];
        int identifierIndex = -1;
        if (ch == '"')
            ch = returnStringValue(filePtr, ch, myText);
        else {
            ch = parseWord(filePtr, ch, myTextIdentifier);

            identifierIndex = isTextIdentifier(myTextIdentifier);
            if (identifierIndex < 0)
                exitProgram(-1, "Invalid identifier!");
            strcpy(myText, textValues[identifierIndex]);
        }
        if (ch != ',')
            exitProgram(-1, "Invalid 'override' function usage!");

        ch = fgetc(filePtr);
        ch = spaceAndCommentOperations(1, filePtr, ch);

        int location;
        ch = returnIntegerValue(filePtr, ch, &location);
        if (location >= strlen(myText))
            exitProgram(-1, "Invalid 'location' integer value!");

        if (ch != ',')
            exitProgram(-1, "Invalid 'override' function usage!");

        ch = fgetc(filePtr);
        ch = spaceAndCommentOperations(1, filePtr, ch);

        ch = returnStringValue(filePtr, ch, ovrText);
        overrideFunction(myText, location, ovrText);
        strcpy(strValue, myText);

        if (identifierIndex != -1)
            strcpy(textValues[identifierIndex], strValue);
    } else if (strcmp(funcName, keywords[12]) == 0) { // subs
        char text[10000];
        ch = returnStringValue(filePtr, ch, text);
        if (ch != ',')
            exitProgram(-1, "Invalid 'subs' function usage!");

        ch = fgetc(filePtr);
        ch = spaceAndCommentOperations(1, filePtr, ch);

        int begin, end;
        ch = returnIntegerValue(filePtr, ch, &begin);
        if (begin > strlen(text))
            exitProgram(-1, "Invalid 'begin' integer value!");

        if (ch != ',')
            exitProgram(-1, "Invalid 'subs' function usage!");

        ch = fgetc(filePtr);
        ch = spaceAndCommentOperations(1, filePtr, ch);

        ch = returnIntegerValue(filePtr, ch, &end);
        if (end < begin || end > strlen(text))
            exitProgram(-1, "Invalid 'end' integer value!");

        int len = end - begin + 1;
        strncpy(strValue, text + begin, len - 1);
        strValue[len - 1] = '\0';
    }

    if (ch != ')')
        exitProgram(-1, "Syntax error!");
    ch = fgetc(filePtr);
    return ch;
}

char callIntFunction(FILE *filePtr, char ch, char *funcName, int *numberValue) { // run integer-return function
    ch = spaceAndCommentOperations(1, filePtr, ch);
    if (ch != '(')
        exitProgram(-1, "Syntax error!");
    ch = fgetc(filePtr);
    ch = spaceAndCommentOperations(1, filePtr, ch);

    if (strcmp(funcName, keywords[1]) == 0) { // asText
        if (ch == '"') {
            ch = fgetc(filePtr);
            ch = returnIntegerValue(filePtr, ch, numberValue);
            if (ch != '"')
                exitProgram(-1, "String cannot convert to integer!");
            ch = fgetc(filePtr);
        } else
            ch = returnIntegerValue(filePtr, ch, numberValue);
    } else if (strcmp(funcName, keywords[6]) == 0) { // locate
        char bigText[10000], smallText[10000];
        int start;

        ch = returnStringValue(filePtr, ch, bigText);
        if (ch != ',')
            exitProgram(-1, "Invalid 'locate' function usage!");

        ch = fgetc(filePtr);
        ch = spaceAndCommentOperations(1, filePtr, ch);

        ch = returnStringValue(filePtr, ch, smallText);
        if (ch != ',')
            exitProgram(-1, "Invalid 'locate' function usage!");

        ch = fgetc(filePtr);
        ch = spaceAndCommentOperations(1, filePtr, ch);

        ch = returnIntegerValue(filePtr, ch, &start);

        *numberValue = locateFunction(bigText, smallText, start);
    } else if (strcmp(funcName, keywords[11]) == 0) { // size
        char text[10000];
        ch = returnStringValue(filePtr, ch, text);
        *numberValue = strlen(text);
    }

    ch = spaceAndCommentOperations(1, filePtr, ch);
    if (ch != ')')
        exitProgram(-1, "Syntax error!");
    ch = fgetc(filePtr);
    return ch;
}

char callCharFunction(FILE *filePtr, char ch, char *strValue) { // run char function
    if (ch != '(')
        exitProgram(-1, "Invalid function usage!");

    ch = fgetc(filePtr);
    char myTextIdentifier[31];
    ch = parseWord(filePtr, ch, myTextIdentifier);

    int textIdentifierIndex = isTextIdentifier(myTextIdentifier);
    if (textIdentifierIndex == -1)
        exitProgram(-1, "Invalid identifier!");

    ch = spaceAndCommentOperations(1, filePtr, ch);
    if (ch != ',')
        exitProgram(-1, "Invalid function usage!");
    ch = fgetc(filePtr);
    ch = spaceAndCommentOperations(1, filePtr, ch);

    char oldChar;
    ch = returnCharValue(filePtr, ch, &oldChar);
    if (ch != ',')
        exitProgram(-1, "Invalid function usage!");
    ch = fgetc(filePtr);
    ch = spaceAndCommentOperations(1, filePtr, ch);

    char newChar;
    ch = returnCharValue(filePtr, ch, &newChar);
    if (ch != ')')
        exitProgram(-1, "Invalid function usage!");

    char *newTextValue = replaceChar(textValues[textIdentifierIndex], oldChar, newChar);
    strcpy(textValues[textIdentifierIndex], newTextValue);
    strcpy(strValue, newTextValue);

    ch = fgetc(filePtr);
    ch = spaceAndCommentOperations(1, filePtr, ch);
    return ch;
}

char runCommand(FILE *filePtr, char ch, char *word) { // run command-keywords
    if (strcmp(word, keywords[3]) == 0) { // input
        char identifier[31];
        ch = parseWord(filePtr, ch, identifier);

        if (isSpace(ch) || ch == '/')
            ch = spaceAndCommentOperations(1, filePtr, ch);

        int identifierIndex = isTextIdentifier(identifier);

        if (identifierIndex < 0)
            exitProgram(-1, "Invalid identifier!");

        char promptKeyword[31];
        ch = parseWord(filePtr, ch, promptKeyword);

        if (strcmp(promptKeyword, "prompt") == 0) {
            if (isSpace(ch) || ch == '/')
                ch = spaceAndCommentOperations(1, filePtr, ch);

            char promptText[101];
            char text[101];

            if (ch == ';') {
                gets(text);
                if (strlen(text) > 100)
                    exitProgram(-1, "Buffer characters cannot longer than 100!");
                strcpy(textValues[identifierIndex], text);
            } else if (ch == '"') {
                ch = parseString(filePtr, ch, promptText);

                printf("%s", promptText);
                gets(text);
                if (strlen(text) > 100)
                    exitProgram(-1, "Buffer characters cannot longer than 100!");
                strcpy(textValues[identifierIndex], text);
            } else if (isalpha(ch) != 0) {
                char identifier2[31];
                ch = parseWord(filePtr, ch, identifier2);

                int identifier2Index = isTextIdentifier(identifier2);

                if (identifier2Index != -1) {
                    printf("%s", textValues[identifier2Index]);
                    gets(text);
                    if (strlen(text) > 100)
                        exitProgram(-1, "Buffer characters cannot longer than 100!");
                } else if (isIntegerIdentifier(identifier2) != -1)
                    exitProgram(-1, "Integer identifier value cannot set to a text identifier!");

                else
                    exitProgram(-1, "Invalid identifier!");

                strcpy(textValues[identifierIndex], text);
            } else
                exitProgram(-1, "Syntax error!");
        } else
            exitProgram(-1, "Invalid 'input' keyword usage!");
    } else if (strcmp(word, keywords[7]) == 0) { // new
        char dataType[31];
        ch = parseWord(filePtr, ch, dataType);

        if (isSpace(ch) || ch == '/')
            ch = spaceAndCommentOperations(1, filePtr, ch);

        if (isalpha(ch) != 0) {
            if (strcmp(dataType, "text") == 0) { // if data type is text (string)
                char identifier[31];
                ch = parseWord(filePtr, ch, identifier);
                if (ch < 0)
                    exitProgram(-1, "Identifier cannot longer than 30 characters!");

                else if ((isTextIdentifier(identifier) != -1) || isIntegerIdentifier(identifier) != -1 ||
                        isCharIdentifier(identifier) != -1) {
                    printf("Repeating identifier: %s\n", identifier);
                    exitProgram(-1, "Identifier is already created before!");
                } else if ((strcmp(identifier, "text") == 0) || (strcmp(identifier, "int") == 0))
                    exitProgram(-1, "Identifier name cannot have the same name with a data type!");

                textIdentifiers[textListSize] = (char *) malloc(31 * sizeof(char));
                strcpy(textIdentifiers[textListSize++], identifier);

                textIdentifiers = (char **) realloc(textIdentifiers, (textListSize + 1) * sizeof(char *));

                textValues[textValueListSize] = (char *) malloc(10000 * sizeof(char));
                strcpy(textValues[textValueListSize++], "");

                textValues = (char **) realloc(textValues, (textValueListSize + 1) * sizeof(char *));
            } else if (strcmp(dataType, "int") == 0) { // if data type is integer
                char identifier[31];
                ch = parseWord(filePtr, ch, identifier);

                if (ch < 0)
                    exitProgram(-1, "Identifier cannot longer than 30 characters!");

                if ((isTextIdentifier(identifier) != -1) || isIntegerIdentifier(identifier) != -1 ||
                isCharIdentifier(identifier) != -1) {
                    printf("Repeating identifier: %s\n", identifier);
                    exitProgram(-1, "Identifier is already created before!");
                }

                intIdentifiers[intListSize] = (char *) malloc(31 * sizeof(char));
                strcpy(intIdentifiers[intListSize++], identifier);

                intIdentifiers = (char **) realloc(intIdentifiers, (intListSize + 1) * sizeof(char *));

                intValues[intValueListSize++] = -1;

                intValues = (int *) realloc(intValues, (intValueListSize + 1) * sizeof(int));
            } else if (strcmp(dataType, "char") == 0) {
                char identifier[31];
                ch = parseWord(filePtr, ch, identifier);

                if (ch < 0)
                    exitProgram(-1, "Identifier cannot longer than 30 characters!");

                if ((isTextIdentifier(identifier) != -1) || isIntegerIdentifier(identifier) != -1 ||
                isCharIdentifier(identifier) != -1) {
                    printf("Repeating identifier: %s\n", identifier);
                    exitProgram(-1, "Identifier is already created before!");
                }

                charIdentifiers[charListSize] = (char *) malloc(31 * sizeof(char));
                strcpy(charIdentifiers[charListSize++], identifier);

                charIdentifiers = (char **) realloc(charIdentifiers, (charListSize + 1) * sizeof(char *));

                charValues[charValueListSize++] = '\0';

                charValues = (char *) realloc(charValues, (charValueListSize + 1) * sizeof(char));
            } else
                exitProgram(-1, "Invalid data type!");
        } else
            exitProgram(-1, "Syntax error!");
    } else if (strcmp(word, keywords[8]) == 0) { // output
        if (ch == '"') {
            char text[10000]; // unlimited size
            ch = parseString(filePtr, ch, text);
            printf("%s\n", text);
        } else if (ch == '\'') {
            ch = fgetc(filePtr);
            printf("%c\n", ch);
            ch = fgetc(filePtr);
            if (ch != '\'')
                exitProgram(-1, "Invalid 'output' usage!");
            ch = fgetc(filePtr);
        } else {
            char identifier[31];
            ch = parseWord(filePtr, ch, identifier);

            if (isTextIdentifier(identifier) != -1) {
                int identifierIndex = isTextIdentifier(identifier);
                printf("%s\n", textValues[identifierIndex]);
            } else if (isIntegerIdentifier(identifier) != -1) {
                int identifierIndex = isIntegerIdentifier(identifier);
                printf("%d\n", intValues[identifierIndex]);
            } else if (isCharIdentifier(identifier) != -1) {
                int identifierIndex = isCharIdentifier(identifier);
                printf("%c\n", charValues[identifierIndex]);
            } else
                exitProgram(-1, "Invalid identifier or text!");
        }
    } else if (strcmp(word, keywords[10]) == 0) { // read
        char identifier[31];
        ch = parseWord(filePtr, ch, identifier);

        if (isSpace(ch) || ch == '/')
            ch = spaceAndCommentOperations(1, filePtr, ch);

        int identifierIndex = isTextIdentifier(identifier);

        if (identifierIndex < 0)
            exitProgram(-1, "Invalid identifier!");

        char fromKeyword[31];
        ch = parseWord(filePtr, ch, fromKeyword);

        if (isSpace(ch) || ch == '/')
            ch = spaceAndCommentOperations(1, filePtr, ch);

        if (isalpha(ch) == 0)
            exitProgram(-1, "Invalid file name after 'from' keyword!");

        if (strcmp(fromKeyword, keywords[2]) == 0) { // verify keyword is 'from'
            char fileName[10000]; // unlimited file name character
            ch = parseWord(filePtr, ch, fileName);

            strcat(fileName, ".txt");

            readCommentFilePtr = fopen(fileName, "r");

            if (readCommentFilePtr == NULL) {
                printf("File '%s' not found or file error!\n", fileName);
                exitProgram(-1, "Cannot open the file!");
            }

            char readCh, fileText[10000]; // unlimited file size
            int size = 0;
            readCh = fgetc(readCommentFilePtr);

            while (readCh != EOF) {
                fileText[size++] = readCh;
                readCh = fgetc(readCommentFilePtr);
            }
            fileText[size] = '\0';
            strcpy(textValues[identifierIndex], fileText);
            fclose(readCommentFilePtr);
        } else
            exitProgram(-1, "Invalid 'read' keyword usage!");
    } else if (strcmp(word, keywords[15]) == 0) { // write
        char printText[10000];

        if (ch == '"') {
            ch = parseString(filePtr, ch, printText);

            if (isSpace(ch) || ch == '/')
                ch = spaceAndCommentOperations(1, filePtr, ch);
        } else {
            char identifier[31];
            ch = parseWord(filePtr, ch, identifier);

            if (isSpace(ch) || ch == '/')
                ch = spaceAndCommentOperations(1, filePtr, ch);

            int identifierIndex = isTextIdentifier(identifier);

            if (identifierIndex < 0)
                exitProgram(-1, "Invalid identifier!");
            strcpy(printText, textValues[identifierIndex]);
        }
        char toKeyword[31];
        ch = parseWord(filePtr, ch, toKeyword);

        if (isSpace(ch) || ch == '/')
            ch = spaceAndCommentOperations(1, filePtr, ch);

        if (isalpha(ch) == 0)
            exitProgram(-1, "Invalid file name after 'from' keyword!");

        if (strcmp(toKeyword, keywords[14]) == 0) { // verify keyword is 'to'
            char fileName[10000]; // unlimited file name character
            ch = parseWord(filePtr, ch, fileName);
            strcat(fileName, ".txt");

            writeCommandFilePtr = fopen(fileName, "w");

            if (writeCommandFilePtr == NULL)
                exitProgram(-1, "File error!");

            fprintf(writeCommandFilePtr, "%s", printText);
            fclose(writeCommandFilePtr);
        } else
            exitProgram(-1, "Invalid 'write' keyword usage!");
    } else
        exitProgram(-1, "Invalid keyword usage!");

    if (isSpace(ch) || ch == '/')
        ch = spaceAndCommentOperations(1, filePtr, ch);

    return ch;
}

char parseString(FILE *filePtr, char ch, char *str) { // string reader
    int len = 0, i = 0;
    ch = fgetc(filePtr);

    while (str[i] != '\0') // clean previous old string value
        str[i++] = '\0';

    while (ch != EOF && ch != '"') {
        if (ch == '\n')
            lineNumber++;

        str[len++] = ch;
        ch = fgetc(filePtr);
    }
    if (ch == EOF)
        exitProgram(-1, "End of file before string ends! Please limit string with ' \" ' !");

    str[len] = '\0';
    ch = fgetc(filePtr);
    return ch;
}

char returnStringValue(FILE *filePtr, char ch, char *strValue) { // returns text (string) values to expressions
    if (isalpha(ch) != 0) {
        char word[31];
        ch = parseWord(filePtr, ch, word);

        int identifierIndex = isTextIdentifier(word);

        if (identifierIndex != -1)
            strcpy(strValue, textValues[identifierIndex]);
        else if (isStringFunction(word))
            ch = callStringFunction(filePtr, ch, word, strValue);
        else if (isCharFunction(word)) {
            ch = spaceAndCommentOperations(1, filePtr, ch);
            ch = callCharFunction(filePtr, ch, strValue);
        }
        else
            exitProgram(-1, "Invalid assignment!");
    } else if (ch == '"')
        ch = parseString(filePtr, ch, strValue);
    else
        exitProgram(-1, "Invalid assignment!");

    ch = spaceAndCommentOperations(1, readFilePtr, ch);
    return ch;
}

char returnIntegerValue(FILE *filePtr, char ch, int *intValue) { // returns integer values to expressions
    if (isalnum(ch)) {
        if (isalpha(ch) != 0) {
            char word[31];
            ch = parseWord(filePtr, ch, word);

            int identifierIndex = isIntegerIdentifier(word);

            if (identifierIndex != -1)
                *intValue = intValues[identifierIndex];
            else if (isIntFunction(word))
                ch = callIntFunction(filePtr, ch, word, intValue);
            else
                exitProgram(-1, "Invalid assignment!");
        } else
            ch = parseNumber(filePtr, ch, intValue);
    } else
        exitProgram(-1, "Invalid assignment!");

    ch = spaceAndCommentOperations(1, readFilePtr, ch);
    return ch;
}

char returnCharValue(FILE *filePtr, char ch, char *charValue) { // returns char values to expressions
    if (isalpha(ch) != 0) {
        char word[31];
        ch = parseWord(filePtr, ch, word);

        int identifierIndex = isCharIdentifier(word);
        int identifierIndex2 = isTextIdentifier(word);

        if (identifierIndex != -1)
            *charValue = charValues[identifierIndex];
        else if (identifierIndex2 != -1) {
            ch = spaceAndCommentOperations(1, filePtr, ch);
            if (ch != '[')
                exitProgram(-1, "Invalid assignment!");
            ch = fgetc(filePtr);
            ch = spaceAndCommentOperations(1, filePtr, ch);
            int index;
            ch = returnIntegerValue(filePtr, ch, &index);
            ch = spaceAndCommentOperations(1, filePtr, ch);
            if (ch != ']')
                exitProgram(-1, "Invalid assignment!");
            *charValue = textValues[identifierIndex2][index];
            ch = fgetc(filePtr);
        } else
            exitProgram(-1, "Invalid assignment!");
    } else if (ch == '\'') {
        ch = fgetc(filePtr);
        *charValue = ch;
        ch = fgetc(filePtr);
        if (ch != '\'')
            exitProgram(-1, "Invalid assignment!");
        ch = fgetc(filePtr);
    }
    else
        exitProgram(-1, "Invalid assignment!");

    ch = spaceAndCommentOperations(1, filePtr, ch);
    return ch;
}

bool isStringFunction(char *funcName) { // checks if function is string-return function
    bool check = (strcmp(funcName, keywords[0]) == 0) ||
                 (strcmp(funcName, keywords[4]) == 0) ||
                 (strcmp(funcName, keywords[9]) == 0) ||
                 (strcmp(funcName, keywords[12]) == 0);
    return check;
}

bool isIntFunction(char *funcName) { // checks if function is int-return function
    bool check = (strcmp(funcName, keywords[1]) == 0) ||
                 (strcmp(funcName, keywords[6]) == 0) ||
                 (strcmp(funcName, keywords[11]) == 0);
    return check;
}

bool isCharFunction(char *funcName) { // checks if function is char function
    return (strcmp(funcName, "replaceChar") == 0);
}

char *stringSubtraction(char *str1, char *str2) { // for string subtraction
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    int resultLen = len1 - len2;

    char *result = (char *) malloc((resultLen + 1) * sizeof(char));

    int i = 0;
    int j = 0;
    int k = 0;

    while (str1[i] != '\0') {
        if (str1[i] == str2[j]) {
            i++;
            j++;
        } else {
            result[k] = str1[i];
            i++;
            k++;
        }
    }
    result[k] = '\0';
    return result;
}

void overrideFunction(char *myText, int location, char *ovrText) { // for override built-in function
    int textLen = strlen(myText);
    int ovrTextLen = strlen(ovrText);

    if (location >= textLen)
        return;
    for (int i = 0; i < ovrTextLen; i++) {
        if (location + i >= textLen)
            break;
        myText[location + i] = ovrText[i];
    } // No error when writing operation exceeds the size of myText
}

int locateFunction(char *bigText, char *smallText, int start) { // for locate built-in function
    int bigTextLen = strlen(bigText);
    int smallTextLen = strlen(smallText);

    if (start >= bigTextLen || smallTextLen == 0)
        return 0;

    for (int i = start; i <= bigTextLen - smallTextLen; i++) {
        int j;
        for (j = 0; j < smallTextLen; j++) {
            if (bigText[i + j] != smallText[j])
                break;
        }
        if (j == smallTextLen)
            return i;
    }
    return 0;
}

char *replaceChar(const char* myText, char oldChar, char newChar) { // for replaceChar built-in function
    int size = strlen(myText);
    char *newString = (char*) malloc((size + 1) * sizeof(char));

    for (int i = 0; i < size; i++) {
        if (myText[i] == oldChar)
            newString[i] = newChar;
        else
            newString[i] = myText[i];
    }

    newString[size] = '\0';
    return newString;
}
