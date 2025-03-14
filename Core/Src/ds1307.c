/**
 * @copyright (c) 2021 Noel Dom https://www.youtube.com/channel/UCINCDcQylATh2wS5BAYIBPg.
 *
 * @brief	Device Driver for DS1307 Real-time clock (RTC)
 * @file	ds1307.c
 * @version 0.1.0
 * @date	2021
 * @author 	Noel Dominguez.
 */


#include "ds1307.h"

#define DS1307_ADDRES		0x68
#define DS1307_SECONDS 		0x00
#define DS1307_MINUTES		0x01
#define DS1307_HOURS		0x02
#define DS1307_DAY			0X03
#define	DS1307_DATE			0x04
#define DS1307_MONTH		0x05
#define	DS1307_YEAR			0x06
#define	DS1307_CONTROL		0x07
#define DS1307_REG_UTC_HR	0x08
#define DS1307_REG_UTC_MIN	0x09
#define DS1307_REG_CENT    	0x10
#define DS1307_REG_RAM   	0x11
#define DS1307_TIMEOUT		1000


#define DS1307_HANDLER		hi2c2

typedef enum{
	DS1307_1HZ,
	DS1307_4096HZ,
	DS1307_8192HZ,
	DS1307_32768HZ
}ds1307_rate_t;


static ds1307_err_t ds1307_write_byte(uint8_t ds1307_reg_addres, uint8_t data);
static uint8_t ds1307_read_byte(uint8_t ds1307_reg_addres);
static uint8_t ds1307_bcd_decode(uint8_t data);
static uint8_t ds1307_bcd_encode(uint8_t data);

/**
 * @brief Write byte data from an specific address ds1307 RTC
 * 
 * @param ds1307_reg_addres: REG Address you can see more in datasheet page 4 figure 2
 * @param data 
 * @return ds1307_err_t: 0 if everything is ok
 */
static ds1307_err_t ds1307_write_byte(uint8_t ds1307_reg_addres, uint8_t data){

	uint8_t buff[2] = {ds1307_reg_addres,data};
	ds1307_err_t ret_val;
	ret_val = HAL_I2C_Master_Transmit(&DS1307_HANDLER, DS1307_ADDRES << 1, buff, 2, DS1307_TIMEOUT);

	return ret_val;
}
/**
 * @brief Read byte data from an specific address ds1307 RTC
 * 
 * @param ds1307_reg_addres: REG Address you can see more in datasheet page 4 figure 2
 * @return data: data read from reg address
 */
static uint8_t ds1307_read_byte(uint8_t ds1307_reg_addres){

	uint8_t data;
	HAL_I2C_Master_Transmit(&DS1307_HANDLER, DS1307_ADDRES << 1, &ds1307_reg_addres, 1, DS1307_TIMEOUT);
	HAL_I2C_Master_Receive(&DS1307_HANDLER, DS1307_ADDRES << 1, &data, 1, DS1307_TIMEOUT);
	return data;
}
/**
 * @brief BCD decode
 * 
 * @param data: Value to convert
 * @return uint8_t: data converted
 */
static uint8_t ds1307_bcd_decode(uint8_t data){
	return (((data & 0xf0) >> 4) * 10) + (data & 0x0f);
}
/**
 * @brief BCD Encode
 * 
 * @param data: Value to convert
 * @return uint8_t: data converted
 */
static uint8_t ds1307_bcd_encode(uint8_t data){
	return (data % 10 + ((data / 10) << 4));
}

/**
 * @brief Init ds1307
 * 
 */
void ds1307_init(void){
	ds1307_set_clock_halt(0);
}

/**
 * @brief To start the time and calendar, we must set the stop bit of the clock (CH) in 0, to stop, put the bit in 1
 * more information see datasheet on page 4
 * 
 * @param halt: 0 init, 1 stop
 */
void ds1307_set_clock_halt(uint8_t halt){
	uint8_t ch = (halt ? 1 << 7 : 0);
	ds1307_write_byte(DS1307_SECONDS, ch | (ds1307_read_byte(DS1307_SECONDS) & 0x7F));
}
/**
 * @brief ds1307_get_clock_halt more info datasheet page 4
 * 
 * @return uint8_t 
 */
uint8_t ds1307_get_clock_halt(void){
	return (ds1307_read_byte(DS1307_SECONDS) & 0x80) >> 7;
}

/**
 * @brief ds1307_set_hour
 * 
 * @param hour 
 * @return ds1307_err_t 
 */
ds1307_err_t ds1307_set_hour(uint8_t hour){
	return ds1307_write_byte(DS1307_HOURS,ds1307_bcd_encode(hour & 0x3F));
}
/**
 * @brief ds1307_get_hour
 * 
 * @return uint8_t 
 */
uint8_t ds1307_get_hour(void){
	return ds1307_bcd_decode(ds1307_read_byte(DS1307_HOURS) & 0x3F);
}

/**
 * @brief ds1307_set_second
 * 
 * @param second 
 */
void ds1307_set_second(uint8_t second){
	uint8_t val = ds1307_get_clock_halt();
	ds1307_write_byte(DS1307_SECONDS, ds1307_bcd_encode(second | val));

}
/**
 * @brief ds1307_get_second
 * 
 * @return uint8_t 
 */
uint8_t ds1307_get_second(void){
	return ds1307_bcd_decode(ds1307_read_byte(DS1307_SECONDS) & 0x7F);
}
/**
 * @brief ds1307_set_minutes
 * 
 * @param minutes 
 */
void ds1307_set_minutes(uint8_t minutes){
	ds1307_write_byte(DS1307_MINUTES, ds1307_bcd_encode(minutes));
}
/**
 * @brief ds1307_get_minutes
 * 
 * @return uint8_t 
 */
uint8_t ds1307_get_minutes(void){
	return ds1307_bcd_decode(ds1307_read_byte(DS1307_MINUTES));

}
/**
 * @brief ds1307_set_day
 * 
 * @param day 
 */
void ds1307_set_day(uint8_t day){
	ds1307_write_byte(DS1307_DAY, ds1307_bcd_encode(day));
}
/**
 * @brief ds1307_get_day
 * 
 * @return ds1307_days_t 
 */
ds1307_days_t ds1307_get_day(void){
	return ds1307_read_byte(ds1307_bcd_decode(DS1307_DAY));
}


/**
 * @brief ds1307_set_date
 * 
 * @param date 
 */
void ds1307_set_date(uint8_t date){
	ds1307_write_byte(DS1307_DATE, ds1307_bcd_encode(date));
}
/**
 * @brief 
 * 
 * @return uint8_t 
 */
uint8_t ds1307_get_date(void){
	return ds1307_bcd_decode(ds1307_read_byte(DS1307_DATE));
}

/**
 * @brief ds1307_set_month
 * 
 * @param month 
 */
void ds1307_set_month(ds1307_months_t month){
	ds1307_write_byte(DS1307_MONTH, ds1307_bcd_encode(month));
}
/**
 * @brief ds1307_get_month
 * 
 * @return ds1307_months_t 
 */
ds1307_months_t ds1307_get_month(void){
	return ds1307_read_byte(ds1307_bcd_decode(DS1307_MONTH));
}
/**
 * @brief ds1307_set_year
 * 
 * @param year setup year
 */
void ds1307_set_year(uint16_t year){
	ds1307_write_byte(DS1307_REG_CENT, year / 100);
	ds1307_write_byte(DS1307_YEAR, ds1307_bcd_encode(year % 100));
}
/**
 * @brief ds1307_get_year
 * 
 * @return uint16_t: current year
 */
uint16_t ds1307_get_year(void){
	uint16_t cent = ds1307_read_byte(DS1307_REG_CENT) * 100;
	return ds1307_bcd_decode(ds1307_read_byte(DS1307_YEAR)) + cent;

}
/**
 * @brief Set your time zone more info at https://everytimezone.com/
 * 
 * @param hr: current hour
 * @param min: current min
 */
void ds1307_set_time_zone(int8_t hr, uint8_t min){
	ds1307_write_byte(DS1307_REG_UTC_HR, hr);
	ds1307_write_byte(DS1307_REG_UTC_MIN, min);
}
/**
 * @brief Get the actual timezone-hour configured in the rtc
 * 
 * @return int8_t actual time zone
 */
int8_t ds1307_get_time_zone_hour(void){
	return ds1307_read_byte(DS1307_REG_UTC_HR);
}
/**
 * @brief  Get the actual timezone-min configured in the rtc
 * 
 * @return int8_t: actual time zone
 */
int8_t ds1307_get_time_zone_min(void){
	return ds1307_read_byte(DS1307_REG_UTC_MIN);
}


/**
 * @brief Update ds1307 data
 * 
 * @param dev: ds1307 pointer
 */
void ds1307_update(ds1307_dev_t *ds1307_dev){

	ds1307_dev->seconds = ds1307_get_second();
	ds1307_dev->minutes = ds1307_get_minutes();
	ds1307_dev->hours = ds1307_get_hour();
	ds1307_dev->day = ds1307_get_day();
	ds1307_dev->date = ds1307_get_date();
	ds1307_dev->month = ds1307_get_month();
	ds1307_dev->year = ds1307_get_year();
	ds1307_dev->t_zone_hour = ds1307_get_time_zone_hour();
	ds1307_dev->t_zone_min = ds1307_get_time_zone_min();
}
/**
 * @brief Use this function to configure the DS1307, you only need to call this function once on the the first run of your RTC.
 * 
 * @param seconds
 * @param minutes 
 * @param hours 
 * @param day 
 * @param date 
 * @param month 
 * @param year 
 * @param t_zone_hour 
 * @param t_zone_min 
 */
void ds1307_config(uint8_t seconds, uint8_t minutes, uint8_t hours,ds1307_days_t day, uint8_t date,
					ds1307_months_t month, uint16_t year, int8_t t_zone_hour, int8_t t_zone_min)
{
	ds1307_set_second(seconds);
	ds1307_set_minutes(minutes);
	ds1307_set_hour(hours);
	ds1307_set_day(day);
	ds1307_set_date(date);
	ds1307_set_month(month);
	ds1307_set_year(year);
	ds1307_set_time_zone(t_zone_hour, t_zone_min);

}
/**
 * @brief: Display ds1307 info by UART, @todo you need to implement your printf function!
 * 
 * @param: ds1307_dev ds1307 struct pointer
 */
void ds1307_log_uart(ds1307_dev_t *ds1307_dev){
	DBG_print("%d:%d:%d %d/%d/%d\n",ds1307_dev->hours,ds1307_dev->minutes,ds1307_dev->seconds,
			ds1307_dev->date,ds1307_dev->month,ds1307_dev->year);
}
/**
 * @brief This function helps to scan all connected devices on our I2C peripheral
 * 
 * @param I2Chnd: I2C Handles
 * @param delay_: Delay between scanning
 */
void start_i2c_scan(I2C_HandleTypeDef *I2Chnd, uint32_t delay_){

	HAL_StatusTypeDef status;
	uint8_t no_devices = 0;
	DBG_print("\n\r [ I2C Scanner v0.1 ]");

	for (uint8_t i = 0; i < 128; i++){

		status = HAL_I2C_IsDeviceReady(I2Chnd , (uint16_t)(i<<1), 2, 2);

		if ( status != HAL_OK){
			//DBG_print(".\n\r");
		} else {
			DBG_print("\n\rDevice found! Address: 0x%X",i);
			++no_devices;
		}

		HAL_Delay(delay_);

	}

	if (!no_devices){
		DBG_print("\n\r No Devices found!");
	} else {
		DBG_print("\n\r Total Devices found: %d",no_devices);

	}

}
