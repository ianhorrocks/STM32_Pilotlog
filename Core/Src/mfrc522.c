#include "main.h"
#include "mfrc522.h"

uint8_t MFRC522_Rd(uint8_t address)
{
    unsigned int i, ucAddr;
    unsigned int ucResult = 0;
    HAL_GPIO_WritePin(MFRC522_SCK_GPIO_Port, MFRC522_SCK_Pin, 0);
    HAL_GPIO_WritePin(MFRC522_CS_GPIO_Port, MFRC522_CS_Pin, 0);
    ucAddr = ((address << 1) & 0x7E) | 0x80;

    for(i=8; i>0; i--)
    {
    	HAL_GPIO_WritePin(MFRC522_MOSI_GPIO_Port, MFRC522_MOSI_Pin, ((ucAddr & 0x80) == 0x80));
        HAL_GPIO_WritePin(MFRC522_SCK_GPIO_Port, MFRC522_SCK_Pin, 1);
        ucAddr <<= 1;
        HAL_GPIO_WritePin(MFRC522_SCK_GPIO_Port, MFRC522_SCK_Pin, 0);
    }
    for(i=8; i>0; i--)
    {
    	HAL_GPIO_WritePin(MFRC522_SCK_GPIO_Port, MFRC522_SCK_Pin, 1);
        ucResult <<= 1;
        ucResult |= (short)HAL_GPIO_ReadPin(MFRC522_MISO_GPIO_Port, MFRC522_MISO_Pin);
        HAL_GPIO_WritePin(MFRC522_SCK_GPIO_Port, MFRC522_SCK_Pin, 0);
    }
    HAL_GPIO_WritePin(MFRC522_CS_GPIO_Port, MFRC522_CS_Pin, 1);
    HAL_GPIO_WritePin(MFRC522_SCK_GPIO_Port, MFRC522_SCK_Pin, 1);
    return ucResult;
}

void MFRC522_Wr(uint8_t address, uint8_t value)
{
	uint8_t i, ucAddr;
	HAL_GPIO_WritePin(MFRC522_SCK_GPIO_Port, MFRC522_SCK_Pin, 0);
	HAL_GPIO_WritePin(MFRC522_CS_GPIO_Port, MFRC522_CS_Pin, 0);
    ucAddr = ((address << 1) & 0x7E);

    for(i=8; i>0; i--)
    {
        HAL_GPIO_WritePin(MFRC522_MOSI_GPIO_Port, MFRC522_MOSI_Pin, ((ucAddr & 0x80) == 0x80));
        HAL_GPIO_WritePin(MFRC522_SCK_GPIO_Port, MFRC522_SCK_Pin, 1);
        ucAddr <<= 1;
        HAL_GPIO_WritePin(MFRC522_SCK_GPIO_Port, MFRC522_SCK_Pin, 0);
    }
    for(i=8; i>0; i--)
    {
        HAL_GPIO_WritePin(MFRC522_MOSI_GPIO_Port, MFRC522_MOSI_Pin, ((value & 0x80) == 0x80));
        HAL_GPIO_WritePin(MFRC522_SCK_GPIO_Port, MFRC522_SCK_Pin, 1);
        value <<= 1;
        HAL_GPIO_WritePin(MFRC522_SCK_GPIO_Port, MFRC522_SCK_Pin, 0);
    }
    HAL_GPIO_WritePin(MFRC522_CS_GPIO_Port, MFRC522_CS_Pin, 1);
    HAL_GPIO_WritePin(MFRC522_SCK_GPIO_Port, MFRC522_SCK_Pin, 1);
}

static void MFRC522_Clear_Bit(uint8_t addr, uint8_t mask)
{
	uint8_t tmp = 0;
    tmp = MFRC522_Rd(addr) ;
    MFRC522_Wr(addr, tmp&~mask);
}

static void MFRC522_Set_Bit(uint8_t addr, uint8_t mask)
{
	uint8_t tmp = 0;
    tmp = MFRC522_Rd(addr);
    MFRC522_Wr(addr, tmp | mask);
}

void MFRC522_Reset(void)
{
	HAL_GPIO_WritePin(MFRC522_RST_GPIO_Port, MFRC522_RST_Pin, 1);
    HAL_Delay(1);
    HAL_GPIO_WritePin(MFRC522_RST_GPIO_Port, MFRC522_RST_Pin, 0);
    HAL_Delay(1);
    HAL_GPIO_WritePin(MFRC522_RST_GPIO_Port, MFRC522_RST_Pin, 1);
    HAL_Delay(1);
    MFRC522_Wr(COMMANDREG, PCD_RESETPHASE);
    HAL_Delay(1);
}

void MFRC522_AntennaOn(void)
{
	uint8_t stt = 0;
    stt = MFRC522_Rd(TXCONTROLREG);
    MFRC522_Set_Bit(TXCONTROLREG, 0x03);
    stt = stt;
}

void MFRC522_AntennaOff(void)
{
    MFRC522_Clear_Bit(TXCONTROLREG, 0x03);
}

void MFRC522_Init(void)
{
	HAL_GPIO_WritePin(MFRC522_SCK_GPIO_Port, MFRC522_SCK_Pin, 0);
	HAL_GPIO_WritePin(MFRC522_MOSI_GPIO_Port, MFRC522_MOSI_Pin, 0);
    HAL_GPIO_WritePin(MFRC522_CS_GPIO_Port, MFRC522_CS_Pin, 1);
    HAL_GPIO_WritePin(MFRC522_RST_GPIO_Port, MFRC522_RST_Pin, 1);
    MFRC522_Reset();
    MFRC522_Wr(TMODEREG, 0x8D);
    MFRC522_Wr(TPRESCALERREG, 0x3E);
    MFRC522_Wr(TRELOADREGL, 30);
    MFRC522_Wr(TRELOADREGH, 0);
    MFRC522_Wr(TXAUTOREG, 0x40);
    MFRC522_Wr(MODEREG, 0x3D);
    MFRC522_AntennaOff();
    MFRC522_AntennaOn();
}

uint8_t MFRC522_ToCard(uint8_t cmd, uint8_t *dat, uint8_t len, uint8_t *back_dat, unsigned *back_len)
{
	uint8_t _status = MI_ERR;
	uint8_t irqEn = 0x00;
	uint8_t waitIRq = 0x00;
	uint8_t lastBits;
    uint8_t n;
    unsigned i;

    switch(cmd)
    {
        case PCD_AUTHENT:
            irqEn = 0x12;
            waitIRq = 0x10;
            break;

        case PCD_TRANSCEIVE:
            irqEn = 0x77;
            waitIRq = 0x30;
            break;

        default:
            break;
    }
    MFRC522_Wr(COMMIENREG, irqEn | 0x80);
    MFRC522_Clear_Bit(COMMIRQREG, 0x80);
    MFRC522_Set_Bit(FIFOLEVELREG, 0x80);
    MFRC522_Wr(COMMANDREG, PCD_IDLE);

    for(i=0; i<len; i++)
    {
        MFRC522_Wr(FIFODATAREG, dat[i]);
    }
    MFRC522_Wr(COMMANDREG, cmd);
    if(cmd == PCD_TRANSCEIVE)
    {
        MFRC522_Set_Bit(BITFRAMINGREG, 0x80);
    }
    i = 0xFFFF;
    do
    {
        n = MFRC522_Rd(COMMIRQREG);
        i--;
    }while(i && !(n & 0x01) && !(n & waitIRq));

    MFRC522_Clear_Bit(BITFRAMINGREG, 0x80);
    if(i != 0)
    {
        if(!(MFRC522_Rd(ERRORREG) & 0x1B))
        {
            _status = MI_OK;
            if(n & irqEn & 0x01)
            {
                _status = MI_NOTAGERR;
            }
            if(cmd == PCD_TRANSCEIVE)
            {
                n = MFRC522_Rd(FIFOLEVELREG);
                lastBits = MFRC522_Rd(CONTROLREG) & 0x07;
                if(lastBits)
                {
                    *back_len = (n-1) * 8 + lastBits;
                }
                else
                {
                    *back_len = n * 8;
                }
                if(n == 0)
                {
                    n = 1;
                }
                if(n > 16)
                {
                    n = 16;
                }
                for(i=0; i<n; i++)
                {
                	back_dat[i] = MFRC522_Rd(FIFODATAREG);
                }
                back_dat[i] = 0;
            }
        }
        else
        {
            _status = MI_ERR;
        }
    }
    return _status;
}

uint8_t MFRC522_Request(uint8_t reqMode, uint8_t *TagType)
{
	uint8_t _status;
    unsigned backBits;
    MFRC522_Wr(BITFRAMINGREG, 0x07);
    TagType[0] = reqMode;
    _status = MFRC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);
    if((_status != MI_OK) || (backBits != 0x10))
    {
        _status = MI_ERR;
    }
    return _status;
}

void MFRC522_CRC(uint8_t *dataIn, uint8_t length, uint8_t *dataOut)
{
	uint8_t i, n;
    MFRC522_Clear_Bit(DIVIRQREG, 0x04);
    MFRC522_Set_Bit(FIFOLEVELREG, 0x80);
    for(i=0; i<length; i++)
    {
        MFRC522_Wr(FIFODATAREG, *dataIn++);
    }
    MFRC522_Wr(COMMANDREG, PCD_CALCCRC);
    i = 0xFF;
    do
    {
        n = MFRC522_Rd(DIVIRQREG);
        i--;
    }
    while(i && !(n & 0x04));
    dataOut[0] = MFRC522_Rd(CRCRESULTREGL);
    dataOut[1] = MFRC522_Rd(CRCRESULTREGM);
}

uint8_t MFRC522_SelectTag(uint8_t *serNum)
{
	uint8_t i;
	uint8_t _status;
	uint8_t size;
    unsigned recvBits;
    uint8_t buffer[9];
    buffer[0] = PICC_SElECTTAG;
    buffer[1] = 0x70;

    for(i=2; i<7; i++)
    {
        buffer[i] = *serNum++;
    }
    MFRC522_CRC(buffer, 7, &buffer[7]);
    _status = MFRC522_ToCard(PCD_TRANSCEIVE, buffer, 9, buffer, &recvBits);
    if((_status == MI_OK) && (recvBits == 0x18))
    {
        size = buffer[0];
    }
    else
    {
        size = 0;
    }
    return size;
}

void MFRC522_Halt(void)
{
    unsigned unLen;
    uint8_t buff[4];
    buff[0] = PICC_HALT;
    buff[1] = 0;
    MFRC522_CRC(buff, 2, &buff[2]);
    MFRC522_Clear_Bit(STATUS2REG, 0x80);
    MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &unLen);
    MFRC522_Clear_Bit(STATUS2REG, 0x08);
}

uint8_t MFRC522_AntiColl(uint8_t *serNum)
{
	uint8_t _status;
	uint8_t i;
	uint8_t serNumCheck = 0;
    unsigned unLen;
    MFRC522_Wr(BITFRAMINGREG, 0x00);
    serNum[0] = PICC_ANTICOLL;
    serNum[1] = 0x20;
    MFRC522_Clear_Bit(STATUS2REG, 0x08);
    _status = MFRC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);
    if(_status == MI_OK)
    {
        for(i=0; i<4; i++)
        {
            serNumCheck ^= serNum[i];
        }
        if(serNumCheck != serNum[4])
        {
            _status = MI_ERR;
        }
    }
    return _status;
}

uint8_t MFRC522_IsCard(uint8_t *TagType)
{
    if(MFRC522_Request(PICC_REQIDL, TagType) == MI_OK)
        return 1;
    else
        return 0;
}

uint8_t MFRC522_ReadCardSerial(uint8_t *str)
{
	uint8_t _status;
    _status = MFRC522_AntiColl(str);
    str[4] = 0;
    if(_status == MI_OK)
        return 1;
    else
        return 0;
}

uint8_t MFRC522_Compare_UID(uint8_t *l, uint8_t *u)
{
	for(uint8_t i=0; i<4; i++)
	{
		if(l[i] != u[i])
		{
			return 0;
		}
	}
	return 1;
}
