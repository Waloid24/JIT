#include "../include/lexer.hpp"
#include "../include/translateCommand.hpp"
#include "../include/optimizer.hpp"
#include "../include/translateCommand.hpp"
#include <string.h>
#include <sys/mman.h>
#include <time.h>

void graphvizDumpIR (compilerInfo_t compilerInfo);
void dumpCode       (compilerInfo_t * compilerInfo);
void runCode        (compilerInfo_t compilerInfo);
void dumpx86MachineCode (compilerInfo_t compilerInfo);

int main (int argc, char * argv[])
{
    // MY_ASSERT (argc != 2, "You should enter 2 arguments: executable file and bytecode file");

    compilerInfo_t compilerInfo = {};

    compilerInfo.byteCode = readCode ("scanf.bin");

    // dumpCode        (&compilerInfo);

    createIRArray   (&compilerInfo);

    fillIRArray     (&compilerInfo);

    printf ("numCmds = %zu\n", compilerInfo.irInfo.sizeArray);

    if (compilerInfo.irInfo.sizeArray == 0)
    {
        printf ("You send an empty file\n");
        free (compilerInfo.byteCode.buf);
        return 0;
    }

    // optimizeIR (&compilerInfo);

    // graphvizDumpIR (compilerInfo);

    fillJmpsCalls (&compilerInfo);

    // graphvizDumpIR (compilerInfo);

    JITConstructor (&compilerInfo);

    JITCompile (&compilerInfo);

    dumpx86MachineCode (compilerInfo);

    runCode (compilerInfo);

    JITDestructor (&compilerInfo);

    return 0;
}

void dumpx86MachineCode (compilerInfo_t compilerInfo)
{
    FILE * dumpFile = openFile ("dumpx86.bin", "w");

    size_t numWrittenElems = fwrite (compilerInfo.machineCode.buf, compilerInfo.machineCode.len, 1, dumpFile);

    fclose (dumpFile);

    return;
}

void runCode (compilerInfo_t compilerInfo)
{
    int mprotectResult = mprotect (compilerInfo.machineCode.buf, compilerInfo.machineCode.len*sizeof(char) + 1, PROT_EXEC | PROT_READ | PROT_WRITE);

    if (mprotectResult == -1)
    {
        MY_ASSERT (1, "Error in mprotect");
    }

    void (* executableBuffer)(void) = (void (*)(void))(compilerInfo.machineCode.buf);
    MY_ASSERT (executableBuffer == nullptr, "Error in buffer casting");

    clock_t begin = clock ();
    // for (int i = 0; i < 1000; i++)

    printf ("EXECUTABLE BUFFER!!!\n");
        executableBuffer();

    clock_t end = clock ();
    printf ("Elapsed time(secs): %lf\n", (double)(end - begin) / CLOCKS_PER_SEC);

    mprotectResult = mprotect (compilerInfo.machineCode.buf, compilerInfo.machineCode.len*sizeof(char) + 1, PROT_READ | PROT_WRITE);

    if (mprotectResult == -1)
    {
        MY_ASSERT (1, "Error in mprotect");
    }

}

void dumpCode (compilerInfo_t * compilerInfo)
{
    FILE * logfile = openFile ("./logs/logCpu.txt", "a");

    fprintf (logfile, "\n$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");

    for (size_t i = 0; i < compilerInfo->byteCode.sizeBuf/sizeof(int); i++)
    {
        fprintf (logfile, "code[%zu] = %d\n", i, (compilerInfo->byteCode.buf)[i]);
    }

    fprintf (logfile, "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
    fclose (logfile);
}