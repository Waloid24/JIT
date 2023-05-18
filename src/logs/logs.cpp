#include "logs.hpp"

FILE * openFile(const char * nameFile, const char * format)
{
    MY_ASSERT (nameFile == nullptr, "Wrong pointer to name file with source code");

    FILE *file = fopen (nameFile, format);
    MY_ASSERT (file == nullptr, "Unable to open the source file");
    setbuf (file, NULL);

    return file;
}

FILE * _$logOF (const char * nameFile)
{
    FILE * logfile = nullptr;

    if (nameFile == nullptr)
    {
        logfile = fopen ("logSt.txt", "a+");
    }
    else
    {
        logfile = fopen (nameFile, "a+");
    }
    MY_ASSERT (logfile == nullptr, "Unable to open the file");
    setbuf(logfile, nullptr);

    return logfile;
}

