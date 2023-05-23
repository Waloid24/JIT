#include "../include/dumpIr.hpp"
#include "../include/lexer.hpp"


#define dumpline(text, ...)\
		fprintf (IRdumpFile, text, ##__VA_ARGS__)

static int isPushPop (int cmd)
{
    if (cmd == 1 || cmd == 2)
    {
        return 1;
    }
    return 0;
}

int NUMBER_PICTURES = 1;

static bool isJump (ir_t irCommand)
{
    if (irCommand.cmd == CMD_JMP || irCommand.cmd == CMD_JE || 
        irCommand.cmd == CMD_JBE || irCommand.cmd == CMD_JB || 
        irCommand.cmd == CMD_JGE || irCommand.cmd == CMD_JA ||
        irCommand.cmd == CMD_JMP || irCommand.cmd == CMD_JNE)
    {
        return true;
    }
    return false;
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
            else if (isJump(compilerInfo.irInfo.irArray[i]) || compilerInfo.irInfo.irArray[i].cmd == CMD_CALL) 
            {
                dumpline("struct%zu [\nlabel = \"<index> index: %zu|<ip>ip: %zu|<x86ip>x86ip: %zu (%lx)|<name>name: %s|<size> size(native): %d|<arg> argument: %ld\", style = \"filled\", fillcolor = \"cyan\" \n];\n", i, i, compilerInfo.irInfo.irArray[i].nativeIP, compilerInfo.irInfo.irArray[i].x86ip, compilerInfo.irInfo.irArray[i].x86ip, name, compilerInfo.irInfo.irArray[i].nativeSize, compilerInfo.irInfo.irArray[i].argument);
            }
            else if (isPushPop(compilerInfo.irInfo.irArray[i].cmd)) 
            {
                if (compilerInfo.irInfo.irArray[i].argument_type == NUMBER)
                {
                    dumpline("struct%zu [\nlabel = \"<index> index: %zu|<ip>ip: %zu|<x86ip>x86ip: %zu (%lx)|<name>name: %s|<size> size(native): %d|<arg> argument: %ld\", style = \"filled\", fillcolor = \"green\" \n];\n", i, i, compilerInfo.irInfo.irArray[i].nativeIP, compilerInfo.irInfo.irArray[i].x86ip, compilerInfo.irInfo.irArray[i].x86ip, compilerInfo.irInfo.irArray[i].name, compilerInfo.irInfo.irArray[i].nativeSize, compilerInfo.irInfo.irArray[i].argument);
                }
                else if (compilerInfo.irInfo.irArray[i].argument_type == MEM_NUM)
                {
                    dumpline("struct%zu [\nlabel = \"<index> index: %zu|<ip>ip: %zu|<x86ip>x86ip: %zu (%lx)|<name>name: %s|<size> size(native): %d|<arg> argument [MEM]: %ld\", style = \"filled\", fillcolor = \"green\" \n];\n", i, i, compilerInfo.irInfo.irArray[i].nativeIP, compilerInfo.irInfo.irArray[i].x86ip, compilerInfo.irInfo.irArray[i].x86ip, compilerInfo.irInfo.irArray[i].name, compilerInfo.irInfo.irArray[i].nativeSize, compilerInfo.irInfo.irArray[i].argument);
                }
                else if (compilerInfo.irInfo.irArray[i].argument_type == MEM_REG)
                {
                    dumpline("struct%zu [\nlabel = \"<index> index: %zu|<ip>ip: %zu|<x86ip>x86ip: %zu (%lx)|<name>name: %s|<size> size(native): %d|<reg_arg> reg [MEM]: %d\", style = \"filled\", fillcolor = \"green\" \n];\n", i, i, compilerInfo.irInfo.irArray[i].nativeIP, compilerInfo.irInfo.irArray[i].x86ip, compilerInfo.irInfo.irArray[i].x86ip, compilerInfo.irInfo.irArray[i].name, compilerInfo.irInfo.irArray[i].nativeSize, compilerInfo.irInfo.irArray[i].reg_type);
                }

                else if (compilerInfo.irInfo.irArray[i].argument_type == REGISTER)
                {
                    dumpline("struct%zu [\nlabel = \"<index> index: %zu|<ip>ip: %zu|<x86ip>x86ip: %zu (%lx)|<name>name: %s|<size> size(native): %d|<reg_arg> reg: %d\", style = \"filled\", fillcolor = \"green\" \n];\n", i, i, compilerInfo.irInfo.irArray[i].nativeIP, compilerInfo.irInfo.irArray[i].x86ip, compilerInfo.irInfo.irArray[i].x86ip, compilerInfo.irInfo.irArray[i].name, compilerInfo.irInfo.irArray[i].nativeSize, compilerInfo.irInfo.irArray[i].reg_type);
                }
                else if (compilerInfo.irInfo.irArray[i].argument_type == NUM_REG)
                {
                    dumpline("struct%zu [\nlabel = \"<index> index: %zu|<ip>ip: %zu|<x86ip>x86ip: %zu (%lx)|<name>name: %s|<size> size(native): %d|<arg> argument: %ld|<reg_arg> reg: %d\", style = \"filled\", fillcolor = \"green \" \n];\n", i, i, compilerInfo.irInfo.irArray[i].nativeIP, compilerInfo.irInfo.irArray[i].x86ip, compilerInfo.irInfo.irArray[i].x86ip, compilerInfo.irInfo.irArray[i].name, compilerInfo.irInfo.irArray[i].nativeSize, compilerInfo.irInfo.irArray[i].argument, compilerInfo.irInfo.irArray[i].reg_type);
                }
                else if (compilerInfo.irInfo.irArray[i].argument_type == MEM_NUM_REG)
                {
                    dumpline("struct%zu [\nlabel = \"<index> index: %zu|<ip>ip: %zu|<x86ip>x86ip: %zu (%lx)|<name>name: %s|<size> size(native): %d|<arg> argument [MEM]: %ld|<reg_arg> reg [MEM]: %d\", style = \"filled\", fillcolor = \"green\" \n];\n", i, i, compilerInfo.irInfo.irArray[i].nativeIP, compilerInfo.irInfo.irArray[i].x86ip, compilerInfo.irInfo.irArray[i].x86ip, compilerInfo.irInfo.irArray[i].name, compilerInfo.irInfo.irArray[i].nativeSize, compilerInfo.irInfo.irArray[i].argument, compilerInfo.irInfo.irArray[i].reg_type);
                }
                else 
                {
                    MY_ASSERT (1, "Error type of push/pop");
                }
            }
            else if (compilerInfo.irInfo.irArray[i].cmd == CMD_MOV)
            {
                dumpline("struct%zu [\nlabel = \"<index> index: %zu|<ip>ip: %zu|<x86ip>x86ip: %zu (%lx)|<name>name: %s|<size> size(native): %d|<arg> argument: %ld|<reg_arg> reg: %d\", style = \"filled\", fillcolor = \"yellow\" \n];\n", i, i, compilerInfo.irInfo.irArray[i].nativeIP, compilerInfo.irInfo.irArray[i].x86ip, compilerInfo.irInfo.irArray[i].x86ip, compilerInfo.irInfo.irArray[i].name, compilerInfo.irInfo.irArray[i].nativeSize, compilerInfo.irInfo.irArray[i].argument, compilerInfo.irInfo.irArray[i].reg_type);
            }
            else 
            {
                dumpline("struct%zu [\nlabel = \"<index> index: %zu|<ip>ip: %zu|<x86ip>x86ip: %zu (%lx)|<name>name: %s|<size> size(native): %d|<arg> argument: %ld\", style = \"filled\", fillcolor = \"gray\" \n];\n", i, i, compilerInfo.irInfo.irArray[i].nativeIP, compilerInfo.irInfo.irArray[i].x86ip, compilerInfo.irInfo.irArray[i].x86ip, compilerInfo.irInfo.irArray[i].name, compilerInfo.irInfo.irArray[i].nativeSize, compilerInfo.irInfo.irArray[i].argument);
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

    sprintf(cmd, "dot -T png ./graph/ir_dump.dot -o ./graph/IR_DUMP%d.png", NUMBER_PICTURES);
    NUMBER_PICTURES++;
    system(cmd);
}