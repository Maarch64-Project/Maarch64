import struct

magic = 0xfeedfacf # MH_MAGIC_64
cputype = 0x0100000c # CPU_TYPE_ARM64
cpusubtype = 0x00000000
filetype = 0x2 # MH_EXECUTE
ncmds = 2
sizeofcmds = 72 + 24 # LC_SEGMENT_64 + LC_MAIN
flags = 0x00200085
reserved = 0

header = struct.pack("<IIIIIIII", magic, cputype, cpusubtype, filetype, ncmds, sizeofcmds, flags, reserved)

# Command 1: LC_SEGMENT_64 (__TEXT)
cmd1_type = 0x19
cmd1_size = 72
segname = b"__TEXT\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
vmaddr = 0x100000000
vmsize = 0x4000
fileoff = 0
filesize = 4096
maxprot = 7
initprot = 5
nsects = 0
seg_flags = 0

segment_cmd = struct.pack("<II16sQQQQiiII", cmd1_type, cmd1_size, segname, vmaddr, vmsize, fileoff, filesize, maxprot, initprot, nsects, seg_flags)

# Command 2: LC_MAIN
cmd2_type = 0x80000028 # LC_MAIN
cmd2_size = 24
entryoff = 32 + 72 + 24
stacksize = 0

lc_main_cmd = struct.pack("<IIQQ", cmd2_type, cmd2_size, entryoff, stacksize)

# ARM64 Code: MOV X0, #42; MOV X16, #1 (sys_exit); SVC #0x80
code = struct.pack("<III", 0xd2800540, 0xd2800030, 0xd4001001)

padding = b"\x00" * (4096 - (len(header) + len(segment_cmd) + len(lc_main_cmd) + len(code)))

macho_binary = header + segment_cmd + lc_main_cmd + code + padding

with open("fixtures/macho_arm64_sample", "wb") as f:
    f.write(macho_binary)

print("Successfully created Mach-O ARM64 binary: fixtures/macho_arm64_sample")
