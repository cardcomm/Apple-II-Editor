TARGET = apple2enh
OUT = ED.bin
LOADADDR = 2000
AUXTYPE = 0020

OMNIDISK = /Volumes/SSD 1TB/Developer_2022/AppleII/virtual disks/OM1
DEST = $(OMNIDISK)/$(OUT)

all:
	cl65 -t $(TARGET) \
		-Wl -S,0x$(LOADADDR) \
		-Wl -D,__EXEHDR__=0 \
		-m ed.map \
		ed_support_test.c \
		ed_support.s \
		-o $(OUT)
	mv -f "$(OUT)" "$(DEST)"
	xattr -wx prodos.FileType 06 "$(DEST)"
	xattr -wx prodos.AuxType $(AUXTYPE) "$(DEST)"

clean:
	rm -f "$(OUT)" *.o ed.map