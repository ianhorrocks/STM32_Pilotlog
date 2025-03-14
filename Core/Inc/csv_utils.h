// csv_utils.h

#ifndef CSV_UTILS_H
#define CSV_UTILS_H

#include "fatfs.h"

typedef struct {
    uint32_t id;
    uint32_t id_aeronave;
    uint32_t id_usuario;
    char tiempo_inicial[20];
    char tiempo_final[20];
    char tiempo_total[20];
} TimeRecord;

void writeCSVRecord(FIL* file, TimeRecord* record);

#endif /* CSV_UTILS_H */
