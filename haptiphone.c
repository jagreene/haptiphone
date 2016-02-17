#include <p24FJ128GB206.h>
#include <stdint.h>
#include "config.h"
#include "common.h"
#include "ui.h"
#include "usb.h"
#include "pin.h"
#include "spi.h"
#include "oc.h"
#include "node.h"

#define TOGGLE_LED1         1
#define TOGGLE_LED2         2
#define READ_SW1            3
#define ENC_WRITE_REG       4
#define ENC_READ_REG        5

#define ENC_READ_ANG        6

#define TOGGLE_LED3         8
#define READ_SW2            9
#define READ_SW3            10

#define SET_MOTOR_MAX       11
#define SET_MOTOR_COAST     12
#define SET_MOTOR_BRAKE     13
#define SET_MOTOR_HALF      14
#define SET_CONTROLLERS      15

#define REG_MAG_ADDR        0x3FFE
#define REG_ANG_ADDR        0x3FFF

// Typically between 1-20kHz based on Aaron Hoover.  Value in Hz
#define MOTOR_FREQ          10e3

_PIN *ENC_SCK, *ENC_MISO, *ENC_MOSI;
_PIN *ENC_NCS;

_PIN *OC_PIN_1, *OC_PIN_2;

char FEEDBACK_FLAGS;

WORD enc_readReg(WORD address) {
    WORD cmd, result;
    cmd.w = 0x4000|address.w; //set 2nd MSB to 1 for a read
    cmd.w |= parity(cmd.w)<<15; //calculate even parity for

    pin_clear(ENC_NCS);
    spi_transfer(&spi1, cmd.b[1]);
    spi_transfer(&spi1, cmd.b[0]);
    pin_set(ENC_NCS);

    pin_clear(ENC_NCS);
    result.b[1] = spi_transfer(&spi1, 0);
    result.b[0] = spi_transfer(&spi1, 0);
    pin_set(ENC_NCS);
    return result;
}

uint8_t motor_setSpeed() {


}
//void ClassRequests(void) {
//    switch (USB_setup.bRequest) {
//        default:
//            USB_error_flags |= 0x01;                    // set Request Error Flag
//    }
//}

void VendorRequests(void) {
    WORD32 address;
    WORD result;

    switch (USB_setup.bRequest) {
        case TOGGLE_LED1:
            led_toggle(&led1);
            BD[EP0IN].bytecount = 0;         // set EP0 IN byte count to 0
            BD[EP0IN].status = 0xC8;         // send packet as DATA1, set UOWN bit
            break;
        case TOGGLE_LED2:
            led_toggle(&led2);
            BD[EP0IN].bytecount = 0;         // set EP0 IN byte count to 0
            BD[EP0IN].status = 0xC8;         // send packet as DATA1, set UOWN bit
            break;
        case READ_SW1:
            BD[EP0IN].address[0] = (uint8_t)sw_read(&sw1);
            BD[EP0IN].bytecount = 1;         // set EP0 IN byte count to 1
            BD[EP0IN].status = 0xC8;         // send packet as DATA1, set UOWN bit
            break;
        case ENC_READ_REG:
            result = enc_readReg(USB_setup.wValue);
            BD[EP0IN].address[0] = result.b[0];
            BD[EP0IN].address[1] = result.b[1];
            BD[EP0IN].bytecount = 2;         // set EP0 IN byte count to 1
            BD[EP0IN].status = 0xC8;         // send packet as DATA1, set UOWN bit
            break;
        case TOGGLE_LED3:
            led_toggle(&led3);
            BD[EP0IN].bytecount = 0;         // set EP0 IN byte count to 0
            BD[EP0IN].status = 0xC8;         // send packet as DATA1, set UOWN bit
            break;
        case READ_SW2:
            BD[EP0IN].address[0] = (uint8_t)sw_read(&sw2);
            BD[EP0IN].bytecount = 1;         // set EP0 IN byte count to 1
            BD[EP0IN].status = 0xC8;         // send packet as DATA1, set UOWN bit
            break;
        case READ_SW3:
            BD[EP0IN].address[0] = (uint8_t)sw_read(&sw3);
            BD[EP0IN].bytecount = 1;         // set EP0 IN byte count to 1
            BD[EP0IN].status = 0xC8;         // send packet as DATA1, set UOWN bit
            break;
        // case ENC_READ_ANG:              // Basicalyl ENC_READ_REG but with angle address already set
        //     result = enc_readReg(REG_ANG_ADDR);
        //     BD[EP0IN].address[0] = result.b[0];
        //     BD[EP0IN].address[1] = result.b[1];
        //     BD[EP0IN].bytecount = 2;         // set EP0 IN byte count to 1
        //     BD[EP0IN].status = 0xC8;         // send packet as DATA1, set UOWN bit
        //     break;

        // Forces motor to go to max speed by setting p1 to 1, p2 to 0 and the input oc to 0xffff (100% duty cycle)
        case SET_MOTOR_HALF:
            // pin_write(&D[6], 0xffff);
            // pin_write(&D[7], 0x0);
            oc_pwm(&oc1, OC_PIN_1, NULL, MOTOR_FREQ, 0xa000);
            oc_pwm(&oc2, OC_PIN_2, NULL, MOTOR_FREQ,    0x0);
            break;

        case SET_MOTOR_MAX:
            // pin_write(&D[6], 0xffff);
            // pin_write(&D[7], 0x0);
            oc_pwm(&oc1, OC_PIN_1, NULL, MOTOR_FREQ, 0xffff);
            oc_pwm(&oc2, OC_PIN_2, NULL, MOTOR_FREQ,    0x0);
            break;

        case SET_MOTOR_COAST:
            // pin_write(&D[7], 0x0);
            // pin_write(&D[8], 0x0);
            oc_pwm(&oc1, OC_PIN_1, NULL, MOTOR_FREQ, 0x0);
            oc_pwm(&oc2, OC_PIN_2, NULL, MOTOR_FREQ, 0x0);
            BD[EP0IN].bytecount = 0;         // set EP0 IN byte count to 0
            BD[EP0IN].status = 0xC8;         // send packet as DATA1, set UOWN bit
            break;

        // Hard brake
        case SET_MOTOR_BRAKE:
            // pin_write(&D[7], 0xffff);
            // pin_write(&D[8], 0xffff);
            oc_pwm(&oc1, OC_PIN_1, NULL, MOTOR_FREQ, 0xffff);
            oc_pwm(&oc2, OC_PIN_2, NULL, MOTOR_FREQ, 0xffff);
            BD[EP0IN].bytecount = 0;         // set EP0 IN byte count to 0
            BD[EP0IN].status = 0xC8;         // send packet as DATA1, set UOWN bit
            break;

        case SET_CONTROLLERS:
            FEEDBACK_FLAGS = USB_setup.wValue.w << 4;
            BD[EP0IN].bytecount = 0;         // set EP0 IN byte count to 0
            BD[EP0IN].status = 0xC8;         // send packet as DATA1, set UOWN bit
            break;
        default:
            USB_error_flags |= 0x01;    // set Request Error Flag
    }
}

void VendorRequestsIn(void) {
    switch (USB_request.setup.bRequest) {
        default:
            USB_error_flags |= 0x01;                    // set Request Error Flag
    }
}

void VendorRequestsOut(void) {
//    WORD32 address;
//
//    switch (USB_request.setup.bRequest) {
//        case ENC_WRITE_REGS:
//            enc_writeRegs(USB_request.setup.wValue.b[0], BD[EP0OUT].address, USB_request.setup.wLength.b[0]);
//            break;
//        default:
//            USB_error_flags |= 0x01;                    // set Request Error Flag
//    }
}

int16_t main(void) {
    WORD result;
    uint16_t angle;
    uint16_t mask;

    init_clock();
    init_ui();
    init_pin();
    init_spi();

    node* P = init_list(10);
    node* I = init_list(10);
    node* D = init_list(10);

    ENC_MISO = &D[1];
    ENC_MOSI = &D[0];
    ENC_SCK = &D[2];
    ENC_NCS = &D[3];

    pin_digitalOut(ENC_NCS);
    pin_set(ENC_NCS);

    spi_open(&spi1, ENC_MISO, ENC_MOSI, ENC_SCK, 5e6, 1);

    // OC setup code
    init_oc();

    // Set the two driving pins
    OC_PIN_1 = &D[7];
    OC_PIN_2 = &D[8];


    InitUSB();                              // initialize the USB registers and serial interface engine
    while (USB_USWSTAT!=CONFIG_STATE) {     // while the peripheral is not configured...
        ServiceUSB();                       // ...service USB requests
    }
    while (1) {
        ServiceUSB();                       // service any pending USB requests
    }
}
