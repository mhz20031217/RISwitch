#include <stdio.h>
#include <assert.h>

int main() {
    unsigned char img[4096];

    FILE *fp = fopen("csr.bin", "rb");
    assert(fp);
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    fread(img, size, 1, fp);

    FILE *txt = fopen("csr.c", "w");

    fprintf(txt, "static unsigned char csr_img[] = {\n  ");
    for (long i = 0; i < size; i ++) {
        fprintf(txt, "0x%02x, ", img[i]);
    }
    fprintf(txt, "\n};\nstatic size_t csr_img_size = %ld;\n", size);
    fprintf(txt, "static size_t csr_img_instr_count = %ld;\n", size / 4);
    fclose(fp);
    fclose(txt);
}
