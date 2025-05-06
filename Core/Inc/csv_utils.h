// csv_utils.h

#ifndef CSV_UTILS_H
#define CSV_UTILS_H

#include "fatfs.h"

typedef struct {
	char id[25];
    char id_embebido[20];
    char id_usuario[9];
    char tiempo_inicial[20];
    char tiempo_final[20];
    char tiempo_total[20];
} TimeRecord;

void writeCSVRecord(FIL* file, TimeRecord* record);

#endif /* CSV_UTILS_H */
