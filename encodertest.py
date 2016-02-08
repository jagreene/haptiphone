
import usb.core
import time

class encodertest:

    def __init__(self):
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

    def close(self):
        self.dev = None

    def toggle_led1(self):
        try:
            self.dev.ctrl_transfer(0x40, self.TOGGLE_LED1)
        except usb.core.USBError:
            print "Could not send TOGGLE_LED1 vendor request."

    def toggle_led2(self):
        try:
            self.dev.ctrl_transfer(0x40, self.TOGGLE_LED2)
        except usb.core.USBError:
            print "Could not send TOGGLE_LED2 vendor request."

    def toggle_led3(self):
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

    def read_sw2(self):
        try:
            ret = self.dev.ctrl_transfer(0xC0, self.READ_SW2, 0, 0, 1)
        except usb.core.USBError:
            print "Could not send READ_SW2 vendor request."
        else:
            return int(ret[0])

    def read_sw3(self):
        try:
            ret = self.dev.ctrl_transfer(0xC0, self.READ_SW3, 0, 0, 1)
        except usb.core.USBError:
            print "Could not send READ_SW3 vendor request."
        else:
            return int(ret[0])

    def enc_readReg(self, address):
        try:
            ret = self.dev.ctrl_transfer(0xC0, self.ENC_READ_REG, address, 0, 2)
        except usb.core.USBError:
            print "Could not send ENC_READ_REG vendor request."
        else:
            return ret

    def enc_readAng(self):
        try:
            ret = self.dev.ctrl_transfer(0xC0, self.ENC_READ_REG, self.ENC_ANGLE_AFTER_ZERO_POS_ADDER, 0, 2)
        except usb.core.USBError:
            print "Could not send ENC_ANGLE_AFTER_ZERO_POS_ADDER vendor request."
        else:
            byte1, byte2 = ret
            return ((byte2 & 0x3f) << 8) + byte1

    def angle(self):
        byte1, byte2 = self.dev.ctrl_transfer(0xC0, self.ENC_READ_REG, self.ENC_ANGLE_AFTER_ZERO_POS_ADDER, 0, 2)
        # # X X 0 1 2 3 4 5 6 7 8 9 10 11 12 13
        # t0 = ((byte1 & 0x3f) << 8) + byte2

        # # 0 1 2 3 4 5 6 7 8 9 10 11 12 13 X X
        # t1 = ((byte1) << 6) + (byte2 >> 2)

        # # 0 1 2 3 4 5 6 7 8 9 10 11 12 13 X X
        # t2 = flip_byte(t0, 14)

        # # 13 12 11 10 9 8 7 6 5 4 3 2 1 0 X X
        # t3 = flip_byte(t1, 14)

        # return (t0, t1, t2, t3)

        # t0 = ((flip_byte(byte1, 8) // 4) << 6) + flip_byte(byte2, 8)
        # t1 = ((flip_byte(byte1, 8) & 0x3f) << 8) + flip_byte(byte2, 8)

        t0 = (byte2 << 6) + (byte1 & 0x3f)

        t1 = ((byte2 & 0x3f) << 8) + byte1
        return (t0, t1, 0, 0)


    def mot_max(self):
        try:
            ret = self.dev.ctrl_transfer(0x40, self.SET_MOTOR_MAX)
        except usb.core.USBError:
            print "Could not send SET_MOTOR_MAX vendor request."

    def mot_half(self):
        try:
            ret = self.dev.ctrl_transfer(0x40, self.SET_MOTOR_HALF)
        except usb.core.USBError:
            print "Could not send SET_MOTOR_HALF vendor request."

    def mot_coast(self):
        try:
            ret = self.dev.ctrl_transfer(0x40, self.SET_MOTOR_COAST)
        except usb.core.USBError:
            print "Could not send SET_MOTOR_COAST vendor request."

    def spindown(self):
        readings = range(2000)
        self.mot_max()
        time.sleep(1)
        self.mot_coast()
        idx = 0
        for idx in range(len(readings)):
            readings[idx] = self.enc_readAng()

        return readings

def _cdf(arr):
    total = 0

    for val in arr:
        total = total + val
        yield total

def cdf(arr):
    return [v for v in _cdf(arr)]

def flip_byte(b, length):
    return int(('{:0' + str(length) + 'b}').format(b)[::-1], 2)

def run_spindown():
    from matplotlib import pyplot as plt
    ec = encodertest()

    readings = ec.spindown()
    nexts = readings[1:]
    prevs = readings[:-1]
    diff = [v[1] - v[0] for v in zip(prevs, nexts)]
    diff2 = [v if v > 0 else -1 * v for v in diff]
    diff3 = [v if v < 8000 else 0 for v in diff2]
    integral = cdf(diff3)

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

def run_calibration_curve():
    from matplotlib import pyplot as plt
    ec = encodertest()
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
            measurements[idx] = ec.enc_readAng()

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
def run_calibration_2():
    from matplotlib import pyplot as plt
    ec = encodertest()
    raw_input("Press ENTER to begin measuring")

    measurements = []
    while True:
        try:
            val = ec.angle()
            measurements.append(val)
        except:
            break

    m0s = [m[0] for m in measurements]
    m1s = [m[1] for m in measurements]
    m2s = [m[2] for m in measurements]
    m3s = [m[3] for m in measurements]
    # plt.plot(m0s, 'r')
    # plt.figure()

    plt.plot(m1s, 'b')

    # plt.plot(m2s, 'g')
    # plt.figure()

    # plt.plot(m3s, 'k')
    plt.show()


if __name__ == '__main__':
    run_calibration_curve()