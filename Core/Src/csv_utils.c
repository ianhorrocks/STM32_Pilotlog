// csv_utils.c

#include "csv_utils.h"
#include <stdio.h>

void writeCSVRecord(FIL* file, TimeRecord* record) {
    // Crear una línea con el nuevo registro
    char write_buffer[100];
    if (f_size(file) == 0) {
        char header[] = "id,id_embebido,id_usuario,tiempo_inicial,tiempo_final,tiempo_total\n";
        f_puts(header, file);
    }
    sprintf(write_buffer, "%s,%s,%s,%s,%s,%s\n",
            record->id,
            record->id_embebido,
			record->id_usuario,
            record->tiempo_inicial,
            record->tiempo_final,
            record->tiempo_total);

    f_lseek(file, f_size(file));  // Ir al final del archivo
    f_puts(write_buffer, file);
}
