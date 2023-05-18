#include "../include/processFileWithCode.hpp"

const int SIGNATURE_SIZE = 4;
typedef int elem_t;

static size_t size (const char * argConsole)
{
    struct stat code = {};
    stat (argConsole, &code);

    return (size_t) code.st_size;
}

static size_t readNumStrs (FILE * binFile)
{
    MY_ASSERT (binFile == nullptr, "There is no access to binary file");

    fseek (binFile, SIGNATURE_SIZE*sizeof(char), SEEK_SET);

    int nStrs = -1;
    fread (&nStrs, sizeof(int), 1, binFile);
    MY_ASSERT (nStrs < 0, "The file is empty");

    return (size_t)nStrs;
}

static void writeTextToBuf (code_t * code, FILE * codeFile)
{
    MY_ASSERT (code == nullptr, "There is no access to the struct with the necessary information");
    MY_ASSERT (codeFile == nullptr, "There is no access to the code file");

    code->buf = (int *) calloc (code->sizeBuf, sizeof(char));
    MY_ASSERT (code->buf == nullptr, "Unable to allocate new memory");

    fread (code->buf, sizeof(char), code->sizeBuf,  codeFile);
}

code_t readCode (const char * nameFile)
{
    MY_ASSERT (nameFile == nullptr, "You did not pass the file name");
    code_t fileInfo = {};
    fileInfo.sizeBuf = size (nameFile) - SIGNATURE_SIZE*sizeof(char);

    FILE * codeFile = openFile (nameFile, "r");

    fileInfo.nStrs = readNumStrs (codeFile);
    printf ("fileInfo.nStrs = %zu\n", fileInfo.nStrs);
    writeTextToBuf (&fileInfo, codeFile);

    fclose (codeFile);

    return fileInfo;
}