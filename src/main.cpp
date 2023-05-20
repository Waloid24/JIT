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
    MY_ASSERT (argc != 2, "You should enter 2 arguments: executable file and bytecode file");

    compilerInfo_t compilerInfo = {};

    compilerInfo.byteCode = readCode (argv[1]);

    dumpCode        (&compilerInfo);

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

    graphvizDumpIR (compilerInfo);

    fillJmpsCalls (&compilerInfo);

    JITConstructor (&compilerInfo);

    JITCompile (&compilerInfo);

    dumpx86MachineCode (compilerInfo);

    runCode (compilerInfo);

    JITDestructor (&compilerInfo);

    return 0;
}

void dumpx86MachineCode (compilerInfo_t compilerInfo)
{
    FILE * dumpFile = openFile ("dumpx86.txt", "w");

    size_t numWrittenElems = fwrite (compilerInfo.machineCode.buf, compilerInfo.machineCode.len, 1, dumpFile);

    printf ("%zu (%zu) array elements were successfully written\n", numWrittenElems, compilerInfo.machineCode.len);

    return;
}

void runCode (compilerInfo_t compilerInfo)
{
    // int mprotectResult = mprotect (compilerInfo.machineCode.buf, compilerInfo.machineCode.len + 1, PROT_EXEC | PROT_READ | PROT_WRITE);
    
    int mprotectResult = mprotect (compilerInfo.machineCode.buf, compilerInfo.machineCode.len*sizeof(char) + 1, PROT_EXEC | PROT_READ | PROT_WRITE);

    if (mprotectResult == -1)
    {
        MY_ASSERT (1, "Error in mprotect");
    }

    void (* executableBuffer)(void) = (void (*)(void))(compilerInfo.machineCode.buf);
    MY_ASSERT (executableBuffer == nullptr, "gg vp");

    clock_t begin = clock ();
    // for (int i = 0; i < 1000; i++)
        executableBuffer();

    clock_t end = clock ();
    printf ("Elapsed time(secs): %lf\n", (double)(end - begin) / CLOCKS_PER_SEC);

    mprotectResult = mprotect (compilerInfo.machineCode.buf, compilerInfo.machineCode.len + 1, PROT_READ | PROT_WRITE);

    if (mprotectResult == -1)
    {
        MY_ASSERT (1, "Error in mprotect");
    }

}

#define dumpline(text, ...)\
		fprintf (IRdumpFile, text, ##__VA_ARGS__)

static int isJump (int cmd)
{
    if (cmd == 9 || (cmd >= 12 && cmd <= 17) )
    {
        return 1;
    }
    return 0;
}

static int isPushPop (int cmd)
{
    if (cmd == 1 || cmd == 2)
    {
        return 1;
    }
    return 0;
}

void graphvizDumpIR (compilerInfo_t compilerInfo) 
{
    MY_ASSERT (compilerInfo.irInfo.irArray == nullptr, "There is no access to the commands array");

    FILE* IRdumpFile = fopen("./graph/ir_dump.dot", "w");
    MY_ASSERT (IRdumpFile == nullptr, "Unable to open the file");

    dumpline("digraph {\n");
    dumpline("rankdir=LR;\n");
    dumpline("node [ shape=record ];\n");

    size_t sizeArr = compilerInfo.irInfo.sizeArray;

    for (size_t i = 0; i < sizeArr; i++) 
    {
        const char* name = compilerInfo.irInfo.irArray[i].name;
        if (compilerInfo.irInfo.irArray[i].cmd != 0)
        {
            if (compilerInfo.irInfo.irArray[i].cmd == CMD_TRASH)
            {
                dumpline("struct%zu [\nlabel = \"<index> index: %zu|<name>name: %s\", style = \"filled\", fillcolor = \"red\" \n];\n", i, i, compilerInfo.irInfo.irArray[i].name);
            }
            else if (isJump(compilerInfo.irInfo.irArray[i].cmd)) 
            {
                dumpline("struct%zu [\nlabel = \"<index> index: %zu|<ip>ip: %zu|<name>name: %s|<size> size(native): %d|<arg> argument: %ld\", style = \"filled\", fillcolor = \"cyan\" \n];\n", i, i, compilerInfo.irInfo.irArray[i].nativeIP, name, compilerInfo.irInfo.irArray[i].nativeSize, compilerInfo.irInfo.irArray[i].argument);
            }
            else if (isPushPop(compilerInfo.irInfo.irArray[i].cmd)) 
            {
                if (compilerInfo.irInfo.irArray[i].argument_type == NUMBER)
                {
                    dumpline("struct%zu [\nlabel = \"<index> index: %zu|<ip>ip: %zu|<name>name: %s|<size> size(native): %d|<arg> argument: %ld\", style = \"filled\", fillcolor = \"green\" \n];\n", i, i, compilerInfo.irInfo.irArray[i].nativeIP, compilerInfo.irInfo.irArray[i].name, compilerInfo.irInfo.irArray[i].nativeSize, compilerInfo.irInfo.irArray[i].argument);
                }
                else if (compilerInfo.irInfo.irArray[i].argument_type == MEM_NUM)
                {
                    dumpline("struct%zu [\nlabel = \"<index> index: %zu|<ip>ip: %zu|<name>name: %s|<size> size(native): %d|<arg> argument [MEM]: %ld\", style = \"filled\", fillcolor = \"green\" \n];\n", i, i, compilerInfo.irInfo.irArray[i].nativeIP, compilerInfo.irInfo.irArray[i].name, compilerInfo.irInfo.irArray[i].nativeSize, compilerInfo.irInfo.irArray[i].argument);
                }
                else if (compilerInfo.irInfo.irArray[i].argument_type == MEM_REG)
                {
                    dumpline("struct%zu [\nlabel = \"<index> index: %zu|<ip>ip: %zu|<name>name: %s|<size> size(native): %d|<reg_arg> reg [MEM]: %d\", style = \"filled\", fillcolor = \"green\" \n];\n", i, i, compilerInfo.irInfo.irArray[i].nativeIP, compilerInfo.irInfo.irArray[i].name, compilerInfo.irInfo.irArray[i].nativeSize, compilerInfo.irInfo.irArray[i].reg_type);
                }
                else if (compilerInfo.irInfo.irArray[i].argument_type == REGISTER)
                {
                    dumpline("struct%zu [\nlabel = \"<index> index: %zu|<ip>ip: %zu|<name>name: %s|<size> size(native): %d|<reg_arg> reg: %d\", style = \"filled\", fillcolor = \"green\" \n];\n", i, i, compilerInfo.irInfo.irArray[i].nativeIP, compilerInfo.irInfo.irArray[i].name, compilerInfo.irInfo.irArray[i].nativeSize, compilerInfo.irInfo.irArray[i].reg_type);
                }
                else if (compilerInfo.irInfo.irArray[i].argument_type == NUM_REG)
                {
                    dumpline("struct%zu [\nlabel = \"<index> index: %zu|<ip>ip: %zu|<name>name: %s|<size> size(native): %d|<arg> argument: %ld|<reg_arg> reg: %d\", style = \"filled\", fillcolor = \"green\" \n];\n", i, i, compilerInfo.irInfo.irArray[i].nativeIP, compilerInfo.irInfo.irArray[i].name, compilerInfo.irInfo.irArray[i].nativeSize, compilerInfo.irInfo.irArray[i].argument, compilerInfo.irInfo.irArray[i].reg_type);
                }
                else if (compilerInfo.irInfo.irArray[i].argument_type == MEM_NUM_REG)
                {
                    dumpline("struct%zu [\nlabel = \"<index> index: %zu|<ip>ip: %zu|<name>name: %s|<size> size(native): %d|<arg> argument [MEM]: %ld|<reg_arg> reg [MEM]: %d\", style = \"filled\", fillcolor = \"green\" \n];\n", i, i, compilerInfo.irInfo.irArray[i].nativeIP, compilerInfo.irInfo.irArray[i].name, compilerInfo.irInfo.irArray[i].nativeSize, compilerInfo.irInfo.irArray[i].argument, compilerInfo.irInfo.irArray[i].reg_type);
                }
                else 
                {
                    MY_ASSERT (1, "Error type of push/pop");
                }
            }
            else if (compilerInfo.irInfo.irArray[i].cmd == CMD_MOV)
            {
                dumpline("struct%zu [\nlabel = \"<index> index: %zu|<ip>ip: %zu|<name>name: %s|<size> size(native): %d|<arg> argument: %d|<reg_arg> reg: %d\", style = \"filled\", fillcolor = \"yellow\" \n];\n", i, i, compilerInfo.irInfo.irArray[i].nativeIP, compilerInfo.irInfo.irArray[i].name, compilerInfo.irInfo.irArray[i].nativeSize, compilerInfo.irInfo.irArray[i].argument, compilerInfo.irInfo.irArray[i].reg_type);
            }
            else 
            {
                dumpline("struct%zu [\nlabel = \"<index> index: %zu|<ip>ip: %zu|<name>name: %s|<size> size(native): %d|<arg> argument: %d\", style = \"filled\", fillcolor = \"gray\" \n];\n", i, i, compilerInfo.irInfo.irArray[i].nativeIP, compilerInfo.irInfo.irArray[i].name, compilerInfo.irInfo.irArray[i].nativeSize, compilerInfo.irInfo.irArray[i].argument);
            }
            if (i == 0)
            {
                continue;
            }

            dumpline("struct%zu -> struct%zu [weight=100];\n", i-1, i);
        }
    }
    dumpline("}\n");

    fclose (IRdumpFile);

    char cmd[100] = {0};

    sprintf(cmd, "dot -T png ./graph/ir_dump.dot -o ./graph/IR_DUMP.png");
    system(cmd);
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