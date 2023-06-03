#include "../include/lexer.hpp"
#include "../include/translateCommand.hpp"
#include "../include/optimizer.hpp"
#include "../include/translateCommand.hpp"
#include "../include/dumpIr.hpp"
#include <string.h>
#include <sys/mman.h>
#include <time.h>

void runCode        (compilerInfo_t compilerInfo);

int main (int argc, char * argv[])
{
    MY_ASSERT (argc != 2, "You should enter 2 arguments: executable file and bytecode file");

    compilerInfo_t compilerInfo = {};

    compilerInfo.byteCode = readCode (argv[1]);
    // compilerInfo.byteCode = readCode ("./tests/quadraticEquation.bin");

    // dumpCode        (&compilerInfo);

    createIRArray   (&compilerInfo);

    fillIRArray     (&compilerInfo);

    if (compilerInfo.irInfo.sizeArray == 0)
    {
        printf ("You send an empty file\n");
        free (compilerInfo.byteCode.buf);
        free (compilerInfo.irInfo.irArray);
        return 0;
    }

    optimizeIR (&compilerInfo);

    setIp (&compilerInfo);

    fillJmpsCalls (&compilerInfo);

    // graphvizDumpIR (compilerInfo);

    JITConstructor (&compilerInfo);

    JITCompile (&compilerInfo);

    dumpx86MachineCode (compilerInfo);

    runCode (compilerInfo);

    JITDestructor (&compilerInfo);

    return 0;
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
    // for (int i = 0; i < 1000000; i++)
        executableBuffer();

    clock_t end = clock ();
    printf ("Elapsed time(secs): %lf\n", (double)(end - begin) / CLOCKS_PER_SEC);

    mprotectResult = mprotect (compilerInfo.machineCode.buf, compilerInfo.machineCode.len*sizeof(char) + 1, PROT_READ | PROT_WRITE);

    if (mprotectResult == -1)
    {
        MY_ASSERT (1, "Error in mprotect");
    }
}