#include <stdint.h>
#include <stdio.h>

static uint8_t data[4096*1024];
static size_t size;

void read_binary(const char *file_name)
{
    
    FILE *f = fopen(file_name, "rb");
    size = fread(data, 1, 4096 * 1024, f);
    fclose(f);
}

void gen_program(const char *file_name)
{
    FILE *f = fopen(file_name, "w");
    fprintf(f,"#include <stdint.h>\n");
    fprintf(f,"uint8_t program_data[] = {\n");
    for(size_t i = 0; i < size; i++){
        fprintf(f,"0x%x,\n",data[i]);
    }
    fprintf(f,"\n};\n");
    
}

int main(int argc, char *argv[])
{
    if (argc != 3){
        printf("bad argument\n");
        return -1;
    }
    read_binary(argv[1]);
    gen_program(argv[2]);
    return 0;
}

