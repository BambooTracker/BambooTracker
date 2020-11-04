import numpy as np

cent = 32
note = 12
octave = 8

total = cent * note * octave

knums = np.arange(total)

# For SSG
#cents = 1200 + 100 * knums / 32
# For FM
cents = 100 * knums / 32

hz = 440 * 2 ** ((cents-5700)/1200)

clock = 3993600 * 2

## FM
tbl = np.empty((8, 2 ** 11))
for i in range(tbl.shape[0]):
    for j in range(tbl.shape[1]):
        tbl[i, j] = j * (2 ** (i-21)) * clock / 144

def find_nearest_index_fm(array, values):
    ret = np.empty((len(values),2), dtype=np.int)
    for i in np.arange(len(values)):
        a = np.abs(array - values[i])
        idx = np.unravel_index(a.argmin(), a.shape)
        ret[i,0] = idx[0]
        ret[i,1] = idx[1]
    return ret

ret = find_nearest_index_fm(tbl, hz)

b16 = np.empty(len(ret), dtype=np.int16)
for i in range(len(ret)):
    b16[i] = (ret[i, 0] << 11) + ret[i, 1]

with open("out.txt", mode="w") as f:
    for i in range(len(b16)):
        f.write("0x{:04x}".format(b16[i]))
        if (i + 1) % 12 == 0:
            f.write(",\n")
        else:
            f.write(", ")
"""
## SSG tone
tbl = np.empty(4096)
for i in range(len(tbl)):
    tbl[i] = 0 if i == 0 else clock / (64 * i)

def find_nearest_index_ssg(array, values):
    ret = np.empty(len(values), dtype=np.int)
    for i in np.arange(len(values)):
        ret[i] = np.abs(array - values[i]).argmin()
    return ret

ret = find_nearest_index_ssg(tbl, hz)

b16 = ret

with open("out.txt", mode="w") as f:
    for i in range(len(b16)):
        f.write("0x{:03x}".format(b16[i]))
        if (i + 1) % 12 == 0:
            f.write(",\n")
        else:
            f.write(", ")

## SSG saw
tbl = np.empty(65535)
for i in range(len(tbl)):
    tbl[i] = 0 if i == 0 else clock / (1024 * i)

ret = find_nearest_index_ssg(tbl, hz)

b16 = ret

with open("out.txt", mode="w") as f:
    for i in range(len(b16)):
        f.write("0x{:03x}".format(b16[i]))
        if (i + 1) % 12 == 0:
            f.write(",\n")
        else:
            f.write(", ")

## SSG triangle
ret = find_nearest_index_ssg(tbl/2, hz)

b16 = ret

with open("out.txt", mode="w") as f:
    for i in range(len(b16)):
        f.write("0x{:03x}".format(b16[i]))
        if (i + 1) % 12 == 0:
            f.write(",\n")
        else:
            f.write(", ")
"""
