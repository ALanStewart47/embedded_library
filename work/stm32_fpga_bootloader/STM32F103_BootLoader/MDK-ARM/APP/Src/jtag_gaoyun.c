///******************************************************************************
// * @file jtag_gaoyun.c
// * @brief STM32(HAL) 位带 JTAG 主机, 对 Gowin GW1N-9C 内部用户 Flash 做 ISP 编程。
// *        移植自 GD32 参考(参考/boot参考/jtag_gaoyun.c)。JTAG TAP 状态机与高云 ISP
// *        指令(0x11/0x41/0x15/0x75/0x71/0x3A/0x3C/0x02...)与参考完全一致, 仅适配:
// *          - GPIO 位带: gpio_bit_write/gpio_input_bit_get -> STM32 BSRR/IDR 直写
// *          - 时钟使能/SWJ: rcu/AFIO_PCF0 -> __HAL_RCC_*/__HAL_AFIO_REMAP_SWJ_NOJTAG
// *          - 延时: delay_1ms -> HAL_Delay; delay_us 复用 uart.c 中实现
// *****************************************************************************/
#include "jtag_gaoyun.h"
#include "fpga_update.h"    

 
#define TMS_GPIO_Port   GPIOA
#define TMS_Pin         GPIO_PIN_4
#define TCK_GPIO_Port   GPIOA
#define TCK_Pin         GPIO_PIN_5
#define TDI_GPIO_Port   GPIOA
#define TDI_Pin         GPIO_PIN_6
#define TDO_GPIO_Port   GPIOA
#define TDO_Pin         GPIO_PIN_7

 
#define TCK_HIGH   (TCK_GPIO_Port->BSRR = TCK_Pin)
#define TCK_LOW    (TCK_GPIO_Port->BSRR = (uint32_t)TCK_Pin << 16)
#define TMS_HIGH   (TMS_GPIO_Port->BSRR = TMS_Pin)
#define TMS_LOW    (TMS_GPIO_Port->BSRR = (uint32_t)TMS_Pin << 16)
#define TDI_HIGH   (TDI_GPIO_Port->BSRR = TDI_Pin)
#define TDI_LOW    (TDI_GPIO_Port->BSRR = (uint32_t)TDI_Pin << 16)
#define READ_TDO() (((TDO_GPIO_Port->IDR & TDO_Pin) != 0U) ? 1U : 0U)
#define DELAY()    ((void)0)                
#define delay_1ms(x)  HAL_Delay(x)

extern void delay_us(uint32_t us);          
extern unsigned char rx_buffer[];           
extern void set_BootLoader_flag(void);      

typedef enum {
    TEST_LOGIC_RESET, RUN_TEST_IDLE,
    SELECT_DR_SCAN, CAPTURE_DR, SHIFT_DR, EXIT1_DR, PAUSE_DR, EXIT2_DR, UPDATE_DR,
    SELECT_IR_SCAN, CAPTURE_IR, SHIFT_IR, EXIT1_IR, PAUSE_IR, EXIT2_IR, UPDATE_IR
} TAPState;

TAPState currentState = TEST_LOGIC_RESET;
uint16_t cnt, cnt_max, cnt_max_decimal;

void my_gpio_init(void)
{
    GPIO_InitTypeDef g = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();
     

    g.Mode  = GPIO_MODE_OUTPUT_PP;
    g.Speed = GPIO_SPEED_FREQ_HIGH;
    g.Pull  = GPIO_NOPULL;
    g.Pin = TMS_Pin; HAL_GPIO_Init(TMS_GPIO_Port, &g);
    g.Pin = TCK_Pin; HAL_GPIO_Init(TCK_GPIO_Port, &g);
    g.Pin = TDI_Pin; HAL_GPIO_Init(TDI_GPIO_Port, &g);

    g.Mode = GPIO_MODE_INPUT;
    g.Pull = GPIO_PULLUP;
    g.Pin  = TDO_Pin; HAL_GPIO_Init(TDO_GPIO_Port, &g);
}

void transition(TAPState nextState) {
    switch (nextState) {
        case TEST_LOGIC_RESET:
            TMS_HIGH;
            for (int i = 0; i < 8; i++) { TCK_HIGH; DELAY(); TCK_LOW; DELAY(); }
            break;
        case RUN_TEST_IDLE:
            TMS_LOW;
            for (int i = 0; i < 8; i++) { TCK_HIGH; DELAY(); TCK_LOW; DELAY(); }
            break;
        case SELECT_DR_SCAN:
            TMS_HIGH; TCK_HIGH; DELAY(); TCK_LOW; DELAY();
            break;
        case CAPTURE_DR:
        case SHIFT_DR:
            TMS_LOW; TCK_HIGH; DELAY(); TCK_LOW; DELAY();
            break;
        case EXIT1_DR:
            TMS_HIGH; TCK_HIGH; DELAY(); TCK_LOW; DELAY();
            break;
        case PAUSE_DR:
            TMS_HIGH; TCK_HIGH; DELAY(); TCK_LOW; DELAY();
            break;
        case EXIT2_DR:
            TMS_LOW; TCK_HIGH; DELAY(); TCK_LOW; DELAY();
            break;
        case UPDATE_DR:
            TMS_HIGH; TCK_HIGH; DELAY(); TCK_LOW; DELAY();
            break;
        case SELECT_IR_SCAN:
            TMS_HIGH; TCK_HIGH; DELAY(); TCK_LOW; DELAY();
            break;
        case CAPTURE_IR:
        case SHIFT_IR:
            TMS_LOW; TCK_HIGH; DELAY(); TCK_LOW; DELAY();
            break;
        case EXIT1_IR:
            TMS_HIGH; TCK_HIGH; DELAY(); TCK_LOW; DELAY();
            break;
        case PAUSE_IR:
            TMS_HIGH; TCK_HIGH; DELAY(); TCK_LOW; DELAY();
            break;
        case EXIT2_IR:
            TMS_LOW; TCK_HIGH; DELAY(); TCK_LOW; DELAY();
            break;
        case UPDATE_IR:
            TMS_HIGH; TCK_HIGH; DELAY(); TCK_LOW; DELAY();
            break;
    }
    currentState = nextState;
}

uint64_t readTDO(uint32_t width) {
    uint64_t idData = 0;
    for (uint32_t i = 0; i < width; i++) {
        if (i == width - 1) { TMS_HIGH; }
        TCK_HIGH; DELAY();
        idData = (idData << 1) | READ_TDO();
        TCK_LOW; DELAY();
    }
    return idData;
}

void sendInstruction_(uint8_t instruction, uint8_t length) {
    for (uint8_t i = 0; i < length; i++) {
        if ((instruction >> (i)) & 1) { TDI_HIGH; } else { TDI_LOW; }
        if (i == length - 1) { TMS_HIGH; }
        TCK_HIGH; DELAY(); TCK_LOW; DELAY();
    }
}

uint32_t readFPGA_Dat(uint8_t leght_num)
{
    uint32_t idData = 0;
    transition(SELECT_DR_SCAN);
    transition(CAPTURE_DR);
    transition(SHIFT_DR);
    idData = (uint32_t)readTDO(leght_num);
    transition(UPDATE_DR);
    transition(RUN_TEST_IDLE);
    return idData;
}

void sendInstruction(uint8_t instruction) {
    for (int i = 0; i < 8; i++) {
        if (i == 7) TMS_HIGH;
        if ((instruction >> (i)) & 1) { TDI_HIGH; } else { TDI_LOW; }
        TCK_HIGH; DELAY(); TCK_LOW; DELAY();
    }
}

void configureDevice(uint8_t instruction)
{
    transition(SELECT_DR_SCAN);
    transition(SELECT_IR_SCAN);
    transition(CAPTURE_IR);
    transition(SHIFT_IR);
    sendInstruction(instruction);
    transition(UPDATE_IR);
    transition(RUN_TEST_IDLE);
    delay_us(100);
}

void eraseSRAM()
{
    configureDevice(0x15);
    configureDevice(0x05);
    configureDevice(0x02);

    for (int j = 0; j < 20; j++)
        for (int i = 0; i < 5000; i++) {
            TCK_HIGH;
            DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();
            TCK_LOW;
            DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();
        }
    delay_us(10000);
    configureDevice(0x09);
    configureDevice(0x02);
    delay_us(10000);
    configureDevice(0x3A);
    configureDevice(0x02);

    transition(RUN_TEST_IDLE);
    for (int j = 0; j < 2; j++)
        for (int i = 0; i < 5000; i++) {
            TCK_HIGH;
            DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();
            TCK_LOW;
            DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();
        }
}

void writeSRAM(uint8_t* bitstream, uint32_t length) {
    uint16_t cnt_temp = 0;
    uint8_t it = 0;
    (void)bitstream; (void)length; (void)it;
    if (cnt == 60000) {
        transition(TEST_LOGIC_RESET);
        transition(TEST_LOGIC_RESET);
        transition(TEST_LOGIC_RESET); delay_us(2000);
        transition(RUN_TEST_IDLE);
        transition(TEST_LOGIC_RESET);
        transition(TEST_LOGIC_RESET);
        transition(TEST_LOGIC_RESET); delay_us(2000);
        transition(RUN_TEST_IDLE);

        configureDevice(0x3a);
        configureDevice(0x00);
        read_device_ID();
        read_Status();

        configureDevice(0x3a);
        configureDevice(0x00);
        read_device_ID();
        read_Status();

        eraseSRAM();
        configureDevice(0x11);
        eraseSRAM();

        delay_us(5000);
        read_Status();
        delay_us(5000);
        read_Status();
        it = 1;
    }
    if (cnt == 1) {
        transition(TEST_LOGIC_RESET);
        transition(TEST_LOGIC_RESET);
        transition(TEST_LOGIC_RESET); delay_us(2000);
        transition(RUN_TEST_IDLE);
        read_Status();
        configureDevice(0x15);
        configureDevice(0x12);
        configureDevice(0x17);

        transition(SELECT_DR_SCAN);
        transition(CAPTURE_DR);
        transition(SHIFT_DR);
        for (int i = 0; i < 8; i++) delay_us(1000);
    }
    if (cnt <= cnt_max) {
        if (cnt_temp != cnt) {
            cnt_temp = cnt;
            if (cnt_temp <= cnt_max - 1)
                for (uint32_t i = 0; i < 2048; i++) {
                    for (int j = 0; j < 8; j++) {
                        if ((rx_buffer[i] >> (7 - j)) & 1) { TDI_HIGH; } else { TDI_LOW; }
                        TCK_HIGH; DELAY(); TCK_LOW; DELAY();
                    }
                }
            else {
                for (uint32_t i = 0; i < 2048; i++) {
                    for (int j = 0; j < 8; j++) {
                        if (i == 2048 - 1 && j == 7) { TMS_HIGH; }
                        if (i >= cnt_max_decimal - 1) { TDI_HIGH; }
                        else if ((rx_buffer[i] >> (7 - j)) & 1) { TDI_HIGH; } else { TDI_LOW; }
                        TCK_HIGH; DELAY(); TCK_LOW; DELAY();
                    }
                }
                {
                    cnt = 60000;
                    delay_us(100);
                    transition(UPDATE_DR);
                    transition(RUN_TEST_IDLE);
                    delay_us(100);
                    configureDevice(0x3A);
                    configureDevice(0x02);
                    delay_us(5000);delay_us(5000);delay_us(5000);delay_us(5000);
                    delay_us(5000);delay_us(5000);delay_us(5000);delay_us(5000);
                    read_Status();
                }
            }
        }
    }
}

void eraseflash() {
    configureDevice(0x15);
    configureDevice(0x75);

    transition(RUN_TEST_IDLE);
    transition(SELECT_DR_SCAN);
    transition(CAPTURE_DR);
    transition(SHIFT_DR);
    for (int i = 0; i < 32; i++) {
        if (i == 31) TMS_HIGH;
        TDI_LOW;
        TCK_HIGH; DELAY(); TCK_LOW; DELAY();
    }
    transition(UPDATE_DR);
    transition(RUN_TEST_IDLE);

    for (int i = 0; i < 230000; i++) {  
        TCK_HIGH;
        DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();
        TCK_LOW;
        DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();DELAY();
    }

    configureDevice(0x3A);
    configureDevice(0x02);
    configureDevice(0x3C);

    delay_1ms(15);
    read_Status();
    delay_1ms(15);
    read_Status();
}

void shiftData(uint32_t data) {
    transition(SELECT_DR_SCAN);
    transition(CAPTURE_DR);
    transition(SHIFT_DR);
    for (int i = 0; i < 32; i++) {
        if (i == 32 - 1) TMS_HIGH;
        if ((data >> i) & 1) { TDI_HIGH; } else { TDI_LOW; }
        TCK_HIGH; DELAY(); TCK_LOW; DELAY();
    }
    transition(UPDATE_DR);
    transition(RUN_TEST_IDLE);
    delay_us(20);
}

void programXPage(uint32_t addr, uint8_t *data) {
    configureDevice(0x15);
    configureDevice(0x71);

    shiftData(addr);

    for (int i = 0; i < 64; i++) {      
        uint32_t dataToProgram = 0;
        dataToProgram = (uint32_t)(data[i*4+3] << 0)  + (uint32_t)(data[i*4+2] << 8)
                      + (uint32_t)(data[i*4+1] << 16) + (uint32_t)(data[i*4+0] << 24);
        shiftData(dataToProgram);
    }

    for (int i = 0; i < 9400; i++) {    
        TCK_HIGH; DELAY(); TCK_LOW; DELAY();
    }
}

uint32_t read_device_ID(void)
{
    configureDevice(0x11);
    return readFPGA_Dat(32);
}

uint32_t read_user_ID(void)
{
    configureDevice(0x13);
    return readFPGA_Dat(32);
}

uint32_t read_Status(void)
{
    configureDevice(0x41);
    return readFPGA_Dat(32);
}

void rst_Device(void)
{
    transition(TEST_LOGIC_RESET);
    transition(RUN_TEST_IDLE);
}

void program_internal_flash(uint16_t cnt_num)
{
    uint8_t  data_[256];
    uint32_t addr_temp = (cnt_num - 1) * 64 * (spi_w_handle.block_size_t / 256);

    if (cnt_num == 0)
    {
        rst_Device();
        read_device_ID();
        read_Status();
        eraseSRAM();
        eraseflash();
        read_Status();
    }

    for (int n = 0; n < spi_w_handle.block_size_t / 256; n++)
    {
        if (cnt_num <= cnt_max && cnt_num != 0)
        {
            if (((cnt_num == cnt_max && n != (spi_w_handle.block_size_t / 256) - 1) || (cnt_num < cnt_max)))
            {
                for (int j = 0; j < 256; j++)
                    data_[j] = spi_w_handle.spi_data[j + n * 256];
                if (cnt_num == 1 && n == 0)          
                {
                    data_[0] = 0x47; data_[1] = 0x57; data_[2] = 0x31; data_[3] = 0x4e;
                }
                programXPage(addr_temp + (n) * 64, data_);
            }
            else if (cnt_num == cnt_max)             
            {
                for (int j = 0; j < 256; j++)
                    data_[j] = spi_w_handle.spi_data[j + n * 256];
                programXPage(addr_temp + (n) * 64, data_);

                configureDevice(0x3a);
                configureDevice(0x3c);
                configureDevice(0x02);

                read_device_ID();
                read_Status();
                delay_1ms(20);
                delay_1ms(100);
                read_Status();

                cnt = 60000;

                 
                set_BootLoader_flag();
                delay_1ms(10);
                __disable_irq();
                NVIC_SystemReset();
            }
        }
    }
}
