#ifndef CSV_UTILS_H
#define CSV_UTILS_H

#include "fatfs.h"

typedef struct {
	char id[25];
    char id_embebed[20];
    char id_tag[16];
    char timestamp[24];
} TimeRecord;


// Escribe un registro CSV, añade cabecera si está vacío.
void writeCSVRecord(FIL* file, TimeRecord* record);

#endif /* CSV_UTILS_H */
