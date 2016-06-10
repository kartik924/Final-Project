#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define LOWERCASE 32

void ParseArgs(int argc, char *argv[], int *flagD, int *flagT, int *flagF,
 char *termin, char **key, char **fName) {
   int counter, charC;
   char *argVal;

   for (counter = 1; counter < argc; counter++) {
      
      if (*(argv[counter]) == '-') {
         argVal = strdup(argv[counter]);

         for (charC = 1; charC < strlen(argVal); charC++) {
            if (argVal[charC] == 'd')
               *flagD = 1;
            else if (argVal[charC] == 'f')
               *flagF = 1;
            else if (argVal[charC] == 't') {
               *flagT = 1;

               if (counter == argc - 1)
                  fprintf(stderr,
                   "look: option requires an argument -- t\n");
               else {
                  counter++;
                  if (strlen(argv[counter]) > 1) {
                     fprintf(stderr,
                      "look: invalid termination character\n");
                     exit(2);
                  }
                  *termin = *(argv[counter]);
               }
            }

            else {
               fprintf(stderr, "look: illegal option -- %c\n", argVal[charC]);
               fprintf(stderr,
                "usage: look [-df] [-t char] string [file ...]\n");
               exit(2);
            }

         }
      }
      else {
         if (!(*key))
            *key = strdup(argv[counter]);
         else {
            *fName = strdup(argv[counter]);
            break;
         }
      }
   }
   if (strcmp(*fName, "/usr/share/dict/words") == 0)
      *flagD = *flagF = 1;
   if (!(*key)) {
      fprintf(stderr, "usage: look [-df] [-t char] string [file ...]\n");
      exit(2);
   }

}

char *GetLine(FILE *file) {
   fpos_t position;
   char *line, c;
   int counter, i;
   counter = 0;

   fgetpos(file, &position);
   
   c = fgetc(file);
   while (c != '\n' && c != EOF) {
      counter++;
      c = fgetc(file);
   }

   fsetpos(file, &position);
   line = calloc((counter + 1), sizeof(char));
   
   for (i = 0; i < counter; i++) {
      *(line + i) = fgetc(file);
   }
   *(line + i) = 0;
   return line;
}

char *GetLineAt(char *fName, int index) {
   int counter;
   char *line, end;
   FILE *file;

   file = fopen(fName, "r");

   end = counter = 0;
   for (counter = 0; counter < index; counter++) {
      line = GetLine(file);
      end = fgetc(file);
   }
   fclose(file);
   return line;
}

int CountLines(char *fName) {
   int counter;
   FILE *file;
   char end;

   file = fopen(fName, "r");

   if (file == NULL) {
      fprintf(stderr, "look: %s: No such file or directory\n", fName);
      exit(2);
   }

   end = counter = 0;

   do {
      GetLine(file);
      end = fgetc(file);
      counter++;
   }
   while (end != EOF);

   fclose(file);
   return counter;

}

void AdjustWord(char **line, int flagD, int flagF) {
   char newLine[strlen(*line) + 1];
   int oldC, newC;

   oldC = newC = 0;

   for (oldC = 0; oldC < strlen(*line); oldC++) {
      newLine[newC] = ((*line)[oldC]);

      if (flagD)
         if (!isalnum(newLine[newC]))
            continue;
      
      if (flagF)
         if (isupper(newLine[newC]))
            newLine[newC] += LOWERCASE;

      newC++;
   }

   newLine[newC] = 0;

   if (flagD && !isalnum(newLine[strlen(newLine) - 1]))
      newLine[strlen(newLine)- 1] = 0;

   *line = strdup(newLine);
}

void PrintResults(char *fName, char *key, int flagD, int flagF, int middle) {
   int fLength, counter;
   char *temp, *temp2;

   fLength = CountLines(fName);
   counter = middle;
   
   // temp = GetLineAt(fName, counter);
   // AdjustWord(&temp, flagD, flagF);
   // counter--;
   

   do {
      temp = GetLineAt(fName, counter);
      AdjustWord(&temp, flagD, flagF);
      if (strncmp(temp, key, strlen(key))) {
         counter++;
         break;
      }
      counter--;
      if (counter == 0) {
         counter = 1;
         break;
      }
   }
   while ((strncmp(temp, key, strlen(key)) == 0) && counter >= 1);

   while (counter <= fLength) {
      temp = GetLineAt(fName, counter);
      temp2 = strdup(temp);
      AdjustWord(&temp, flagD, flagF);
      if (strncmp(temp, key, strlen(key)) == 0)
         printf("%s\n", temp2);
      else
         break;
      counter++;
   }
}

int main(int argc, char *argv[]) {
   int flagD, flagT, flagF, ndx, first, middle, last, found;
   char termin, *key, *fName, *temp;

   fName = strdup("/usr/share/dict/words");
   key = NULL;
   flagD = flagT = flagF = termin = found = 0;

   ParseArgs(argc, argv, &flagD, &flagT, &flagF, &termin, &key, &fName);
   if (termin) {
      temp = strchr(key, termin);
      if (temp) {
         ndx = (int) (temp - key);
         key[ndx + 1] = 0;
      }
         
   }
   
   AdjustWord(&key, flagD, flagF);
   last = CountLines(fName);
   first = 1;
   

   while (first <= last) {
      middle = (first + last) / 2;
      temp = GetLineAt(fName, middle);
      AdjustWord(&temp, flagD, flagF);
      if (strncmp(temp, key, strlen(key)) < 0)
         first = middle + 1;
      else if (strncmp(temp, key, strlen(key)) == 0) {
         found = 1;
         break;
      }
      else if (strncmp(temp, key, strlen(key)) > 0)
         last = middle - 1;
   }

   if (!found)
      return 1;

   PrintResults(fName, key, flagD, flagF, middle);
      
   return 0;
}