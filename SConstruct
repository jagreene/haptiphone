#Uncomment the line below to automatically load code after building
#import bootloadercmd as b
import sys

env = Environment(PIC = '24FJ128GB206',
                  CC = 'xc16-gcc',
                  PROGSUFFIX = '.elf',
                  CFLAGS = '-g -omf=elf -x c -mcpu=$PIC',
                  LINKFLAGS = '-omf=elf -mcpu=$PIC -Wl,--script="app_p24FJ128GB206.gld"',
                  CPPPATH = './lib')
#Path for OSX
if sys.platform == 'darwin':
   env.PrependENVPath('PATH', '/Applications/microchip/xc16/v1.25/bin')
#Path for Linux
elif sys.platform == 'linux2' or 'linux':
   env.PrependENVPath('PATH', '/opt/microchip/xc16/v1.25/bin')

bin2hex = Builder(action = 'xc16-bin2hex $SOURCE -omf=elf',
                  suffix = 'hex',
                  src_suffix = 'elf')
env.Append(BUILDERS = {'Hex' : bin2hex})
list = Builder(action = 'xc16-objdump -S -D $SOURCE > $TARGET',
               suffix = 'lst',
               src_suffix = 'elf')
env.Append(BUILDERS = {'List' : list})

env.Program('haptiphone', ['haptiphone.c',
            'descriptors.c',
            './lib/usb.c',
            './lib/node.c',
            './lib/ui.c',
            './lib/timer.c',
            './lib/oc.c',
            './lib/pin.c',
            './lib/spi.c',
            './lib/common.c',
            './lib/uart.c'])

#print('Creating builder to load hex file via bootloader...')
#def load_function(target, source, env):
#    bl = b.bootloadercmd()
#    bl.import_hex(source[0].rstr())
#    bl.write_device()
#    bl.bootloader.start_user()
#    bl.bootloader.close()
#    return None
#
#load = Builder(action=load_function,
#               suffix = 'none',
#               src_suffix = 'hex')
#
#env.Append(BUILDERS = {'Load' : load})

env.Hex('haptiphone')
env.List('haptiphone')
# To automatically load the hex file, you need to run scons like this:
# >scons --site-dir ../site_scons
#d
#c
#env.Load('haptiphone') #Uncomment this line to automatically load the hex file
