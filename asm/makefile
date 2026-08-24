TARGET = apple2enh
OUT = ED
LOADADDR = 2000
AUXTYPE = 0020

OMNIDISK = /Volumes/SSD 1TB/Developer_2022/AppleII/virtual disks/OM1
DEST = $(OMNIDISK)/$(OUT)

all:
	cl65 -t $(TARGET) \
		-Wl -S,0x$(LOADADDR) \
		-Wl -D,__EXEHDR__=0 \
		-m ed.map \
		ed.c \
		edit.c \
		misc.c \
		drawscrn.c \
		fillvport.c \
		strings.c \
		waitkey.c \
		scroll.c \
		load.c \
		save.c \
		clip.c \
		cmds.c \
		search.c \
		input_c_wrapper.s \
		input_v2.s \
		ed_support.s \
		-o $(OUT)
	mv -f "$(OUT)" "$(DEST)"
	xattr -wx prodos.FileType 06 "$(DEST)"
	xattr -wx prodos.AuxType $(AUXTYPE) "$(DEST)"

clean:
	rm -f "$(OUT)" *.o ed.map