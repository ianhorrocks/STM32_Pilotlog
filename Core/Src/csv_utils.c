#include "csv_utils.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

// Función CRC32 estándar
uint32_t crc32(const char* data) {
    uint32_t crc = 0xFFFFFFFF;
    while (*data) {
        crc ^= (uint8_t)(*data++);
        for (int i = 0; i < 8; i++) {
            crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
        }
    }
    return ~crc;
}

void writeCSVRecord(FIL* file, TimeRecord* record) {
    // 1) Si está vacío, escribimos cabecera
    if (f_size(file) == 0) {
        const char header[] = "id,id_embebed,id_tag,timestamp,hash\n";
        f_puts(header, file);
    }

    // 2) Construimos línea CSV sin hash
    char line[100];
    sprintf(line, "%s,%s,%s,%s",
            record->id,
            record->id_embebed,
            record->id_tag,
            record->timestamp);

    // 3) Calculamos hash
    uint32_t hash = crc32(line);

    // 4) Creamos línea final con hash
    char finalLine[128];
    snprintf(finalLine, sizeof(finalLine), "%s,%08X\n", line, (unsigned int)hash);


    // 5) Vamos al final y escribimos
    f_lseek(file, f_size(file));
    f_puts(finalLine, file);
}
