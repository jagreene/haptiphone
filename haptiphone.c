#include <p24FJ128GB206.h>
#include <stdint.h>
#include <stdio.h>
#include "config.h"
#include "common.h"
#include "ui.h"
#include "usb.h"
#include "pin.h"
#include "spi.h"
#include "oc.h"
#include "math.h"
#include "node.h"
#include "uart.h"

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
#define SET_MOTOR_VAR       13
#define SET_MOTOR_BRAKE     14
#define SET_MOTOR_HALF      15
#define SET_CONTROLLERS     16

// SET values for feedback constants
#define SET_WALL_LEFT           17
#define SET_WALL_RIGHT          18
#define SET_SPRING_EQUILIBRIUM  19
#define SET_SPRING_CONSTANT     20
#define SET_DAMPER_CONSTANT     21
#define SET_TEXTURE_CONSTANT    22

#define REG_MAG_ADDR        0x3FFE
#define REG_ANG_ADDR        0x3FFF

// Typically between 1-20kHz based on Aaron Hoover.  Value in Hz
#define MOTOR_FREQ          10e3

#define ENC_PER_HALF_DEGREE 91

#define SPRING_FLAG         0x8
#define DAMPENER_FLAG       0x4
#define WALL_FLAG           0x2
#define TEXTURE_FLAG        0x1


// Feedback loop constants
uint16_t wall_edge_left, wall_edge_right,
    spring_equilibrium, spring_constant,
    damper_constant,
    texture_constant;


BYTE FEEDBACK_FLAGS;

_PIN *ENC_SCK, *ENC_MISO, *ENC_MOSI;
_PIN *ENC_NCS;

// Set the two driving pins
_PIN *OC_PIN_1, *OC_PIN_2;

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

//void ClassRequests(void) {
//    switch (USB_setup.bRequest) {
//        default:
//            USB_error_flags |= 0x01;                    // set Request Error Flag
//    }
//}

uint16_t enc_half_degrees(){
    WORD address = (WORD)0x3FFF;
    WORD result = enc_readReg(address);

    // 14 bits divided by 360 degrees * 2
    // uint encRead_per_halfDegree = 91;

    uint16_t enc_x4 = (result.b[1] << 10) + (result.b[0] << 2);
    return enc_x4 / ENC_PER_HALF_DEGREE;
}

void VendorRequests(void) {
    WORD32 address;
    WORD result;
    uint16_t half_degrees;
    uint8_t sig;

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
        case ENC_READ_ANG:
            half_degrees = enc_half_degrees();
            half_degrees = half_degrees >> 8;
            sig = (uint8_t)half_degrees;
            BD[EP0IN].address[0] = sig;
            BD[EP0IN].bytecount = 1;
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

        // Forces motor to go to max speed by setting p1 to 1, p2 to 0 and the input oc to 0xffff (100% duty cycle)
        case SET_MOTOR_HALF:
            pin_write(OC_PIN_1, 0xffff);
            pin_write(OC_PIN_2, 0x0);
            BD[EP0IN].bytecount = 0;         // set EP0 IN byte count to 0
            BD[EP0IN].status = 0xC8;         // send packet as DATA1, set UOWN bit
            break;

        case SET_MOTOR_MAX:
            pin_write(OC_PIN_1, 0xffff);
            pin_write(OC_PIN_2, 0x0);
            BD[EP0IN].bytecount = 0;         // set EP0 IN byte count to 0
            BD[EP0IN].status = 0xC8;         // send packet as DATA1, set UOWN bit
            break;

        case SET_MOTOR_COAST:
            pin_write(OC_PIN_1, 0x0);
            pin_write(OC_PIN_2, 0x0);
            BD[EP0IN].bytecount = 0;         // set EP0 IN byte count to 0
            BD[EP0IN].status = 0xC8;         // send packet as DATA1, set UOWN bit
            break;

        case SET_MOTOR_VAR:
            pin_write(OC_PIN_1, (uint16_t)USB_setup.wValue.w);
            pin_write(OC_PIN_2, 0x0);
            BD[EP0IN].bytecount = 0;         // set EP0 IN byte count to 0
            BD[EP0IN].status = 0xC8;         // send packet as DATA1, set UOWN bit
            break;

        // Hard brake
        case SET_MOTOR_BRAKE:
            pin_write(OC_PIN_1, 0xffff);
            pin_write(OC_PIN_2, 0xffff);
            BD[EP0IN].bytecount = 0;         // set EP0 IN byte count to 0
            BD[EP0IN].status = 0xC8;         // send packet as DATA1, set UOWN bit
            break;

        case SET_CONTROLLERS:
            // feedback byte is spring damper wall texture, duplicated for error control
            // right shift to only get one copy of nibble
            FEEDBACK_FLAGS = USB_setup.wValue.w >> 4;
            BD[EP0IN].bytecount = 0;         // set EP0 IN byte count to 0
            BD[EP0IN].status = 0xC8;         // send packet as DATA1, set UOWN bit
            break;

        case SET_WALL_LEFT:
            wall_edge_left = USB_setup.wValue.w;
            BD[EP0IN].bytecount = 0;         // set EP0 IN byte count to 0
            BD[EP0IN].status = 0xC8;         // send packet as DATA1, set UOWN bit
            break;
        case SET_WALL_RIGHT:
            wall_edge_right = USB_setup.wValue.w;
            BD[EP0IN].bytecount = 0;         // set EP0 IN byte count to 0
            BD[EP0IN].status = 0xC8;         // send packet as DATA1, set UOWN bit
            break;
        case SET_SPRING_EQUILIBRIUM:
            spring_equilibrium = USB_setup.wValue.w;
            BD[EP0IN].bytecount = 0;         // set EP0 IN byte count to 0
            BD[EP0IN].status = 0xC8;         // send packet as DATA1, set UOWN bit
            break;
        case SET_SPRING_CONSTANT:
            spring_constant = USB_setup.wValue.w;
            BD[EP0IN].bytecount = 0;         // set EP0 IN byte count to 0
            BD[EP0IN].status = 0xC8;         // send packet as DATA1, set UOWN bit
            break;
        case SET_DAMPER_CONSTANT:
            damper_constant = USB_setup.wValue.w;
            BD[EP0IN].bytecount = 0;         // set EP0 IN byte count to 0
            BD[EP0IN].status = 0xC8;         // send packet as DATA1, set UOWN bit
            break;
        case SET_TEXTURE_CONSTANT:
            texture_constant = USB_setup.wValue.w;
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

int16_t wall(uint16_t half_degrees){
    // Half degrees
    // uint16_t wall_edge_left = 50; // End of wall
    // uint16_t wall_edge_right = 670; // End of wall

    uint16_t wall_begin_left = wall_edge_left + 16; // Start of wall
    uint16_t wall_begin_right = wall_edge_right - 16; // Start of wall
    if(half_degrees < wall_edge_left){
        led_write(&led1, true);
        // led_write(&led2, false);
        // led_write(&led3, false);
        return 0x8001; // Set max speed positive direction 0111 1111 1111 1111
    }
    else if(half_degrees < wall_begin_left){
        led_write(&led1, false);
        // led_write(&led2, false);
        // led_write(&led3, false);
        //  Must be positive, so at most 2**14
        return -1 * (1 << (wall_begin_left - half_degrees)); // Hacky exponential.  Doubles for each half degree step
    }
    else if(half_degrees > wall_edge_right){
        led_write(&led1, false);
        // led_write(&led2, true);
        // led_write(&led3, false);
        return 0x7fff; // Set max speed negative direction 1000 0000
    }
    else if(half_degrees > wall_begin_right){
        led_write(&led1, false);
        // led_write(&led2, false);
        // led_write(&led3, false);
        return (1 << (half_degrees - wall_begin_right));
    }
    else {
        // led_write(&led1, false);
        // led_write(&led2, false);
        // led_write(&led3, true);
    }

    return 0; // Not hitting wall, return 0
}

int16_t spring(uint16_t half_degrees){
    // uint16_t spring_equilibrium = 360;
    // uint16_t spring_constant = 10;

    // Negative sign accounted for in order of subtraction
    return spring_constant * (half_degrees - spring_equilibrium);
}

int16_t damper(int16_t velocity){
    // int16_t damper_constant = -10;

    return velocity * damper_constant;
}

int16_t texture(uint16_t half_degrees){

    // adjust frequency from 720 half degrees to 45 half degrees to radians
    return (int16_t)(texture_constant*sin(half_degrees*(720.0/45.0/114.6)));
}

void iteration(){
    int16_t velocity = 0; //get_avg_diff(positions);
    uint16_t position = enc_half_degrees(); //positions->val;

    int16_t feedback = 0;
    // led_write(&led2, true);
    if((FEEDBACK_FLAGS & SPRING_FLAG) == SPRING_FLAG){
        feedback += spring(position);
    }
    if((FEEDBACK_FLAGS & DAMPENER_FLAG) == DAMPENER_FLAG){
        feedback += damper(velocity);
    }
    if((FEEDBACK_FLAGS & TEXTURE_FLAG) == TEXTURE_FLAG){
        feedback += texture(position);
    }
    if((FEEDBACK_FLAGS & WALL_FLAG) == WALL_FLAG){
        led_write(&led1, true);
        // led_write(&led2, false);

        int16_t wall_val = wall(position);
        if(abs(wall_val) > abs(feedback)){
            feedback = wall_val;
        }
    }

    //Set motor to feedback speed.  Reverse these as necessary.
    if(feedback > 0){
        feedback = feedback << 1;
        led_write(&led1, true);
        led_write(&led2, false);
        pin_write(OC_PIN_1, (uint16_t)feedback);
        pin_write(OC_PIN_2, 0x0);
    }
    else{
        feedback = (-1*feedback) << 1;
        led_write(&led1, false);
        led_write(&led2, true);
        pin_write(OC_PIN_1, 0x0);
        pin_write(OC_PIN_2, (uint16_t)feedback);//(uint16_t)feedback);
    }
}

int16_t main(void) {
    init_clock();
    init_uart(); // Why is this erroring?
    init_ui();
    init_pin();
    init_spi();
    InitUSB();
    init_oc();

    int count = 0;
    // Turn on USB interupt
    // U1IE = 0xFF;
    // U1EIE = 0xFF;
    // IFS5bits.USB1IF = 0;
    // IEC5bits.USB1IE = 1;

    node* positions = init_list(10);

    ENC_MISO = &D[1];
    ENC_MOSI = &D[0];
    ENC_SCK = &D[2];
    ENC_NCS = &D[3];

    pin_digitalOut(ENC_NCS);
    pin_set(ENC_NCS);

    spi_open(&spi1, ENC_MISO, ENC_MOSI, ENC_SCK, 5e6, 1);

    // OC setup code

    OC_PIN_1 = &D[7];
    OC_PIN_2 = &D[8];
    oc_pwm(&oc1, OC_PIN_1, NULL, MOTOR_FREQ, 0x0);
    oc_pwm(&oc2, OC_PIN_2, NULL, MOTOR_FREQ, 0x0);

    wall_edge_left = 100;
    wall_edge_right = 620;
    spring_equilibrium = wall_edge_right - 16;
    // MAX difference = 500 degrees
    // 2**13 * 3
    spring_constant = 50;
    damper_constant = -10;
    texture_constant = 17000;


    // oc_pwm(&oc1, OC_PIN_1, NULL, MOTOR_FREQ, 0xffff);
    // oc_pwm(&oc2, OC_PIN_2, NULL, MOTOR_FREQ, 0xffff);
    FEEDBACK_FLAGS = WALL_FLAG | SPRING_FLAG | TEXTURE_FLAG;
    // led_write(&led2, true);
    // led_write(&led3, true);
    while (true) {
        //positions = add_node(enc_half_degrees(), positions); // update position list
        count++;
        if (count%1000 == 0){
            led_toggle(&led3);
        }
        iteration();
    }
}
