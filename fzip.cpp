#include <string>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

char *inputFilePath;
char *outputFilePath;
const char * extractFile;
//You must fill out your name and id below
char * studentName = (char *) "Troy Wise";
char * studentCWID = (char *) "890597990";

//Do not change this section in your submission
char * usageString =
        (char *) "To archive a file: 		fzip -a INPUT_FILE_NAME  OUTPUT_FILE_NAME\n"
                "To archive a directory: 	fzip -a INPUT_DIR_NAME   OUTPUT_DIR_NAME\n"
                "To extract a file: 		fzip -x INPUT_FILE_NAME  OUTPUT_FILE_NAME\n"
                "To extract a directory: 	fzip -x INPUT_DIR_NAME   OUTPUT_DIR_NAME\n";

bool isExtract = false;
void parseArg(int argc, char *argv[], char ** inputName, char ** outputName) {
    if (argc >= 2 && strncmp("-n", argv[1], 2) == 0) {
        printf("Student Name: %s\n", studentName);
        printf("Student CWID: %s\n", studentCWID);
        exit(EXIT_SUCCESS);
    }

    if (argc != 4) {
        fprintf(stderr, "Incorrect arguements\n%s", usageString);
        exit(EXIT_FAILURE);
    }

    *inputName  = argv[2];
    *outputName = argv[3];
    if (strncmp("-a", argv[1], 2) == 0) {
        isExtract = false;
    } else if (strncmp("-x", argv[1], 2) == 0) {
        isExtract = true;
    } else {
        fprintf(stderr, "Incorrect arguements\n%s", usageString);
        exit(EXIT_FAILURE);
    }
}
//END OF: Do not change this section

char * cat(char * A, char * B){
    char *combi = (char *)malloc(strlen(A)+strlen(B)+1);
    strcpy(combi,A); strcat(combi,B);
    return combi;
}

void DirectoryScan(char *inputFilePath, int fileDesOut){
    DIR *dir = opendir(inputFilePath);
    struct dirent *dirEntry = nullptr;
    char *fileName;
    char *buff;
    int fType = 1;
    int dType = 0;
    size_t fileNameSize;
    int fileDesIn;
    size_t inFileSize;

    if(dir!=NULL){
        int pLen;
        pLen = (int) strlen(inputFilePath);
        write(fileDesOut, &dType, sizeof(int));     //write a new directory type, name size and name
        write(fileDesOut, &pLen, sizeof(int));
        write(fileDesOut, inputFilePath, sizeof(pLen+1));
        while((dirEntry = readdir(dir)) != NULL) {
            fileName = cat(cat(inputFilePath, (char *) "/"), (char *) dirEntry->d_name);
            fileNameSize = strlen(fileName);
            if (dirEntry->d_type == DT_DIR)
                DirectoryScan(inputFilePath, fileDesOut);
            else
                write(fileDesOut, &fType, sizeof(int));   //append file type, name size and name
            write(fileDesOut, &fileNameSize, sizeof(int));
            write(fileDesOut, inputFilePath, fileNameSize + 1);

            fileDesIn = open(inputFilePath, O_RDONLY);
            inFileSize = (size_t) lseek(fileDesIn, 0, SEEK_END);  //seek to the end to find # of bytes
            lseek(fileDesIn, 0, SEEK_SET);      //return pointer to the beginning
            write(fileDesOut, &inFileSize, sizeof(size_t));     //write file size

            buff = (char *) malloc(inFileSize + 1);             //allocate buffer size to write file data
            ssize_t tempIn = read(fileDesIn, buff, inFileSize);  //read to buffer
            write(fileDesOut, buff, (size_t) tempIn);           //write buffer contents to File Descriptor
        }
    }}

/**
 *
 * Your program should archive or extract based on the flag passed in.
 * Both when extracting and archiving, it should print the output file/dir path as the last line.
 *
 * @param argc the number of args
 * @param argv the arg string table
 * @return
 */
int main(int argc, char** argv) {
    char * inputName, * outputName;
    parseArg(argc, argv, &inputName, &outputName);
    char *fName;
    int fType=1;
    short fnSize;
    int fileDesIn;
    int fileDesOut;
    errno = 0;

    if (isExtract) {
        printf("Extracting %s\n", inputFilePath);
        fileDesIn = open(inputFilePath, O_RDONLY);
        while (read(fileDesIn, &fType, sizeof(int)) < 1) { //if still reading bytes, read more
            read(fileDesIn, &fnSize, sizeof(int));

            fName = (char *) malloc(sizeof(fName + 1)); //take into account extra bit for null character
            read(fileDesIn, fName, sizeof(fName + 1));

           extractFile = cat(fName, cat(outputFilePath, (char *)'/'));
        }
        if (fType == 1) { //file type == 1
            size_t fSize;
            read(fileDesIn, &fSize, sizeof(size_t));

            //read file contents with a buffer
            char *buff;
            buff = (char *) malloc(fSize + 1); //allocate memory amount including null pointer
            read(fileDesIn, buff, fSize);


            fileDesOut = open(extractFile, O_WRONLY | O_CREAT, 0744);
            write(fileDesOut, buff, fSize);
            close(fileDesOut);
        }
        //	this should be the same as the `path` var above but
        //	without the .fzip

        printf("%s\n", outputFilePath);//relative or absolute path
        printf("Archiving %s\n", inputFilePath);
    }

    else {
//archive code
        fileDesOut = open(outputFilePath, O_WRONLY | O_CREAT, 0744);
        DirectoryScan(inputFilePath, fileDesOut);

        close(fileDesOut);
        printf("%s.fzip\n", inputFilePath);
    }
    return EXIT_SUCCESS;
}



