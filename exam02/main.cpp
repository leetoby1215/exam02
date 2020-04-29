#include "mbed.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#define UINT14_MAX        16383
#define FXOS8700CQ_SLAVE_ADDR0 (0x1E<<1) // with pins SA0=0, SA1=0
#define FXOS8700CQ_SLAVE_ADDR1 (0x1D<<1) // with pins SA0=1, SA1=0
#define FXOS8700CQ_SLAVE_ADDR2 (0x1C<<1) // with pins SA0=0, SA1=1
#define FXOS8700CQ_SLAVE_ADDR3 (0x1F<<1) // with pins SA0=1, SA1=1
#define FXOS8700Q_STATUS 0x00
#define FXOS8700Q_OUT_X_MSB 0x01
#define FXOS8700Q_OUT_Y_MSB 0x03
#define FXOS8700Q_OUT_Z_MSB 0x05
#define FXOS8700Q_M_OUT_X_MSB 0x33
#define FXOS8700Q_M_OUT_Y_MSB 0x35
#define FXOS8700Q_M_OUT_Z_MSB 0x37
#define FXOS8700Q_WHOAMI 0x0D
#define FXOS8700Q_XYZ_DATA_CFG 0x0E
#define FXOS8700Q_CTRL_REG1 0x2A
#define FXOS8700Q_M_CTRL_REG1 0x5B
#define FXOS8700Q_M_CTRL_REG2 0x5C
#define FXOS8700Q_WHOAMI_VAL 0xC7
#define duration 10

I2C i2c( PTD9,PTD8);
Serial pc(USBTX, USBRX);
InterruptIn btn(SW3);
DigitalOut led(LED1);
EventQueue queue(32 * EVENTS_EVENT_SIZE);
int m_addr = FXOS8700CQ_SLAVE_ADDR1;
void FXOS8700CQ_readRegs(int addr, uint8_t * data, int len);
void FXOS8700CQ_writeRegs(uint8_t * data, int len);

float t[3];

void detect() {
    bool is_over_5_cm = false;
    float dis_x = 0;
    float dis_y = 0;
    for (int i = 0; i <= duration * 10; i++) {
        dis_x += t[0] * 0.1 * 0.1 / 2;
        dis_y += t[1] * 0.1 * 0.1 / 2;
        if (dis_x * dis_x + dis_y * dis_y > 25) {
            is_over_5_cm = true;
            led = false;
        } else {
            is_over_5_cm = false;
            led = true;
        }
        printf("[%1.4f,%1.4f,%1.4f,%d,%1.4f,%1.4f]\r\n", t[0], t[1], t[2], is_over_5_cm,dis_x,dis_y);
        wait(0.1);
    }
    led = true;
}

void btn_fall() {
    queue.call(&detect);
}

int main() {
    pc.baud(115200);

    uint8_t who_am_i, data[2], res[6];
    int16_t acc16;

    FXOS8700CQ_readRegs(FXOS8700Q_CTRL_REG1, &data[1], 1);
    data[1] |= 0x01;
    data[0] = FXOS8700Q_CTRL_REG1;
    FXOS8700CQ_writeRegs(data, 2);
    FXOS8700CQ_readRegs(FXOS8700Q_WHOAMI, &who_am_i, 1);

    Thread thread(osPriorityNormal);
    thread.start(callback(&queue, &EventQueue::dispatch_forever));
    btn.fall(&btn_fall);
    led = true;

    while (true) {
        FXOS8700CQ_readRegs(FXOS8700Q_OUT_X_MSB, res, 6);

        acc16 = (res[0] << 6) | (res[1] >> 2);
        if (acc16 > UINT14_MAX/2)
            acc16 -= UINT14_MAX;
        t[0] = ((float)acc16) / 4096.0f * 9.8;
        acc16 = (res[2] << 6) | (res[3] >> 2);
        if (acc16 > UINT14_MAX/2)
            acc16 -= UINT14_MAX;
        t[1] = ((float)acc16) / 4096.0f * 9.8;

        acc16 = (res[4] << 6) | (res[5] >> 2);
        if (acc16 > UINT14_MAX/2)
            acc16 -= UINT14_MAX;
        t[2] = ((float)acc16) / 4096.0f * 9.8;
    }
}

void FXOS8700CQ_readRegs(int addr, uint8_t * data, int len) {
    char t = addr;
    i2c.write(m_addr, &t, 1, true);
    i2c.read(m_addr, (char *)data, len);
}

void FXOS8700CQ_writeRegs(uint8_t * data, int len) {
    i2c.write(m_addr, (char *)data, len);
}