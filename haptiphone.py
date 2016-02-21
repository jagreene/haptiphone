from cmd2 import Cmd
from matplotlib import pyplot as plt
import usb.core
import time
import math

class haptiphone(Cmd):

    def preloop(self):
        print("Welcome to Haptiphone!")
        print("To see a list of fuctions type help")
        self.completekey ='tab'
        self.TOGGLE_LED1 = 1
        self.TOGGLE_LED2 = 2
        self.READ_SW1 = 3
        self.ENC_READ_REG = 5
        self.TOGGLE_LED3 = 8
        self.READ_SW2 = 9
        self.READ_SW3 = 10
        self.dev = usb.core.find(idVendor = 0x6666, idProduct = 0x0003)
        if self.dev is None:
            raise ValueError('no USB device found matching idVendor = 0x6666 and idProduct = 0x0003')
        self.dev.set_configuration()

        # AS5048A Register Map
        self.ENC_NOP = 0x0000
        self.ENC_CLEAR_ERROR_FLAG = 0x0001
        self.ENC_PROGRAMMING_CTRL = 0x0003
        self.ENC_OTP_ZERO_POS_HI = 0x0016
        self.ENC_OTP_ZERO_POS_LO = 0x0017
        self.ENC_DIAG_AND_AUTO_GAIN_CTRL = 0x3FFD
        self.ENC_MAGNITUDE = 0x3FFE
        self.ENC_ANGLE_AFTER_ZERO_POS_ADDER = 0x3FFF
        self.SET_MOTOR_MAX = 11
        self.SET_MOTOR_COAST = 12

        self.SET_MOTOR_BRAKE = 13
        self.SET_MOTOR_HALF = 14
        self.SET_CONTROLLERS = 15
        # self.SET_CONSTANTS = 16
        self.SET_WALL_LEFT = 16
        self.SET_WALL_LEFT = 16
        self.SET_WALL_RIGHT = 17
        self.SET_SPRING_ = 18
        self.SET_SPRING_CONSTANT = 19
        self.SET_DAMPER_CONSTANT = 20
        self.SET_TEXTURE_CONSTANT = 21

    def do_close(self, *args):
        self.dev = None

    def do_toggle_led1(self, *args):
        try:
            self.dev.ctrl_transfer(0x40, self.TOGGLE_LED1)
        except usb.core.USBError:
            print "Could not send TOGGLE_LED1 vendor request."

    def do_toggle_led2(self, *args):
        try:
            self.dev.ctrl_transfer(0x40, self.TOGGLE_LED2)
        except usb.core.USBError:
            print "Could not send TOGGLE_LED2 vendor request."

    def do_toggle_led3(self, *args):
        try:
            self.dev.ctrl_transfer(0x40, self.TOGGLE_LED3)
        except usb.core.USBError:
            print "Could not send TOGGLE_LED3 vendor request."

    def read_sw1(self):
        try:
            ret = self.dev.ctrl_transfer(0xC0, self.READ_SW1, 0, 0, 1)
        except usb.core.USBError:
            print "Could not send READ_SW1 vendor request."
        else:
            return int(ret[0])

    def do_read_sw1(self, arsgs):
        print self.read_sw1();

    def read_sw2(self):
        try:
            ret = self.dev.ctrl_transfer(0xC0, self.READ_SW2, 0, 0, 1)
        except usb.core.USBError:
            print "Could not send READ_SW2 vendor request."
        else:
            return int(ret[0])

    def do_read_sw2(self, arsgs):
        print self.read_sw2();

    def do_read_sw3(self, *args):
        try:
            ret = self.dev.ctrl_transfer(0xC0, self.READ_SW3, 0, 0, 1)
        except usb.core.USBError:
            print "Could not send READ_SW3 vendor request."
        else:
            return int(ret[0])

    def do_read_sw3(self, *args):
        print self.read_sw3();

    def read_reg(self, address):
        try:
            ret = self.dev.ctrl_transfer(0xC0, self.ENC_READ_REG, address, 0, 2)
        except usb.core.USBError:
            print "Could not send ENC_READ_REG vendor request."
        else:
            return ret

    def do_read_reg(self, reg):
        print self.read_reg(int(reg));

    def read_ang(self, *args):
        try:
            ret = self.dev.ctrl_transfer(0xC0, self.ENC_READ_REG, self.ENC_ANGLE_AFTER_ZERO_POS_ADDER, 0, 2)
        except usb.core.USBError:
            print "Could not send ENC_ANGLE_AFTER_ZERO_POS_ADDER vendor request."
        else:
            byte1, byte2 = ret
            return ((byte2 & 0x3f) << 8) + byte1

    def do_read_ang(self, *args):
        print self.read_ang();

    def do_set_controllers(self, *args):
        input = args[0].split(' ')
        controllers = 0b00000000;
        if "spring" in input:
            controllers |= 0b10001000;
        if "dampen" in input:
            controllers |= 0b01000100;
        if "wall" in input:
            controllers |= 0b00100010;
        if "texture" in input:
            controllers |= 0b00010001;
        try:
            ret = self.dev.ctrl_transfer(0xC0, self.SET_CONTROLLERS, controllers)
        except usb.core.USBError:
            print "Could not send ENC_ANGLE_AFTER_ZERO_POS_ADDER vendor request."
        else:
            print "Controllers Set!"

    def do_set_constants(self, constant, *args):
        input = args[0].split(' ')
        controllers = 0b00000000;
        if "spring" in input:
            controllers |= 0b10001000;
        if "dampen" in input:
            controllers |= 0b01000100;
        if "wall" in input:
            controllers |= 0b00100010;
        if "texture" in input:
            controllers |= 0b00010001;
        try:
            ret = self.dev.ctrl_transfer(0xC0, self.SET_CONSTANTS, controllers)
        except usb.core.USBError:
            print "Could not send ENC_ANGLE_AFTER_ZERO_POS_ADDER vendor request."
        else:
            print "Controllers Set!"

    def do_mot_max(self, *args):
        try:
            ret = self.dev.ctrl_transfer(0x40, self.SET_MOTOR_MAX)
        except usb.core.USBError:
            print "Could not send SET_MOTOR_MAX vendor request."

    def do_mot_half(self, *args):
        try:
            ret = self.dev.ctrl_transfer(0x40, self.SET_MOTOR_HALF)
        except usb.core.USBError:
            print "Could not send SET_MOTOR_HALF vendor request."

    def do_mot_var(self, speed, *args):
        speed = math.floor(float(speed)*(2**16))
        print speed
        try:
            ret = self.dev.ctrl_transfer(0x40, self.SET_MOTOR_VAR, int(speed))
        except usb.core.USBError:
            print "Could not send SET_MOTOR_MAX vendor request."

    def do_mot_coast(self, *args):
        try:
            ret = self.dev.ctrl_transfer(0x40, self.SET_MOTOR_COAST)
        except usb.core.USBError:
            print "Could not send SET_MOTOR_COAST vendor request."

    def spindown(self, *args):
        readings = range(2000)
        self.do_mot_max()
        time.sleep(1)
        self.do_mot_coast()
        idx = 0
        for idx in range(len(readings)):
            readings[idx] = self.read_ang()

        return readings

    def _cdf(self, arr):
        SET_MOTOR_HALF = 0
        total = 0

        for val in arr:
            total = total + val
            yield total

    def cdf(self, arr):
        return [v for v in self._cdf(arr)]

    def do_run_spindown(self, *args):
        readings = self.spindown()
        nexts = readings[1:]
        prevs = readings[:-1]
        diff = [v[1] - v[0] for v in zip(prevs, nexts)]
        diff2 = [v if v > 0 else -1 * v for v in diff]
        diff3 = [v if v < 8000 else 0 for v in diff2]
        integral = self.cdf(diff3)

        fd = open('diff.txt', 'w')
        fi = open('int.txt', 'w')
        fd.write(str(diff))
        fi.write(str(integral))

        fd.close()
        fi.close()

        plt.plot(diff, 'b')
        plt.title('Spin Down Differential')
        plt.xlabel("Time (~0.2 microseconds)")
        plt.ylabel('Position Delta (encoder units)')

        plt.figure()

        plt.plot(integral, 'r')
        plt.title('Spin Down Integral')
        plt.xlabel("Time (~0.2 microseconds)")
        plt.ylabel('Total Position Delta (encoder units)')

        plt.show()

    def do_run_calibration_curve(self, *args):
        count = 36
        measures = 100
        angles = [360 * idx / count for idx in range(count)]
        # print(angles)

        exes = [angles[p // measures] for p in range(count * measures)]
        wise = []
        for angle in angles:
            measurements = range(measures)
            try:
                a = input("Calibrating for: " + str(angle) + " press ENTER when ready")
                print("Running calibration, please wait...")
            except:
                print("Running calibration, please wait...")

            for idx in range(measures):
                measurements[idx] = self.read_ang()

            wise = wise + measurements

        data = str(exes) + '\n' + str(wise)
        f = open('Calibration.txt', 'w')
        f.write(data)
        f.close()
        plt.plot(exes, wise, '.')
        plt.title('Angle Calibration Curve')
        plt.xlabel('Angle (Degrees)')
        plt.ylabel('Encoder Value (14-bit integer)')
        plt.show()

    # Manual
    def do_run_calibration_2(self, *args):
        raw_input("Press ENTER to begin measuring")

        measurements = []
        while True:
            try:
                val = self.read_ang()
                measurements.append(val)
            except:
                break

        m0s = [m[0] for m in measurements]
        m1s = [m[1] for m in measurements]
        m2s = [m[2] for m in measurements]
        m3s = [m[3] for m in measurements]

        plt.plot(m1s, 'b')
        plt.show()

if __name__ == '__main__':
    haptiphoneCLI = haptiphone()
    haptiphoneCLI.cmdloop();

