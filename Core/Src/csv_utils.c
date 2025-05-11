#include "csv_utils.h"
#include <stdio.h>

void writeCSVRecord(FIL* file, TimeRecord* record) {
    // 1) Si está vacío, escribimos cabecera
    if (f_size(file) == 0) {
        const char header[] = "id,id_embebed,id_tag,timestamp\n";
        f_puts(header, file);
    }

    // 2) Construimos línea CSV
    char buf[100];
    sprintf(buf, "%s,%s,%s,%s\n",
            record->id,
            record->id_embebed,
            record->id_tag,
            record->timestamp);

    // 3) Vamos al final y escribimos
    f_lseek(file, f_size(file));
    f_puts(buf, file);
}
