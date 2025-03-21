/**
 * @copyright (c) 2021 Noel Dom https://www.youtube.com/channel/UCINCDcQylATh2wS5BAYIBPg.
 *
 * @brief	Device Driver for DS1307 Real-time clock (RTC)
 * @file	ds1307.h
 * @version 0.1.0
 * @date	2021
 * @author 	Noel Dominguez.
 */
#ifndef DS1307_H_
#define DS1307_H_

#include "i2c.h"
//#include "DBG.h"
#include <stdint.h>


typedef enum {
    Lun = 1,
    Mar,
    Mie,
    Jue,
    Vie,
    Sab,
    Dom
} ds1307_days_t;

typedef enum {
    Enero = 1,
    Febrero,
    Marzo,
    Abril,
    Mayo,
    Junio,
    Julio,
    Agosto,
    Septiembre,
    Octubre,
    Noviembre,
    Diciembre
} ds1307_months_t;


typedef enum{
	DS1307_OK,
	DS1307_ERROR
}ds1307_err_t;

typedef struct {

	uint8_t 		seconds;
	uint8_t 		minutes;
	uint8_t 		hours;
	ds1307_days_t 	day;
	uint8_t 		date;
	ds1307_months_t	month;
	uint16_t 		year;
	int8_t 			t_zone_hour;
	int8_t			t_zone_min;

}ds1307_dev_t;


/**
 * @brief I2C Scanner utility .
 *
 * @param[in]	I2Chnd: i2c handler
 * @param[in]	delay_: delay between each device scan
 *
 */
void start_i2c_scan(I2C_HandleTypeDef *I2Chnd, uint32_t delay_);
void ds1307_init(void);
ds1307_err_t ds1307_set_hour(uint8_t hour);
uint8_t ds1307_get_hour(void);
ds1307_err_t ds1307_set_hour(uint8_t hour);
void ds1307_set_clock_halt(uint8_t halt);
uint8_t ds1307_get_clock_halt(void);
void ds1307_set_second(uint8_t second);
uint8_t ds1307_get_second(void);
void ds1307_set_minutes(uint8_t minutes);
uint8_t ds1307_get_minutes(void);
void ds1307_set_day(uint8_t day);
ds1307_days_t ds1307_get_day(void);
void ds1307_set_date(uint8_t date);
uint8_t ds1307_get_date(void);
void ds1307_set_month(ds1307_months_t month);
ds1307_months_t ds1307_get_month(void);
void ds1307_set_year(uint16_t year);
uint16_t ds1307_get_year(void);
void ds1307_set_time_zone(int8_t hr, uint8_t min);
int8_t ds1307_get_time_zone_hour(void);
int8_t ds1307_get_time_zone_min(void);
void ds1307_update(ds1307_dev_t *ds1307_dev);
void ds1307_config(uint8_t seconds, uint8_t minutes, uint8_t hours,ds1307_days_t day, uint8_t date,
					ds1307_months_t month, uint16_t year, int8_t t_zone_hour, int8_t t_zone_min);

void ds1307_log_uart(ds1307_dev_t *ds1307_dev);


#endif /* DS1307_H_ */
