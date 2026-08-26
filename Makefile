TARGET = apple2enh
OUT = ED
LOADADDR = 1000
AUXTYPE = 0010

CDIR = c
ASMDIR = asm
BUILDDIR = build
BINDIR = bin

OMNIDISK = /Volumes/SSD 1TB/Developer_2022/AppleII/virtual disks/OM1
DEST = $(OMNIDISK)/$(OUT)

C_SRCS = \
	$(CDIR)/ed.c \
	$(CDIR)/edit.c \
	$(CDIR)/misc.c \
	$(CDIR)/drawscrn.c \
	$(CDIR)/fillvport.c \
	$(CDIR)/strings.c \
	$(CDIR)/waitkey.c \
	$(CDIR)/scroll.c \
	$(CDIR)/load.c \
	$(CDIR)/save.c \
	$(CDIR)/clip.c \
	$(CDIR)/cmds.c \
	$(CDIR)/search.c

ASM_SRCS = \
	$(ASMDIR)/input_c_wrapper.s \
	$(ASMDIR)/input_v2.s \
	$(ASMDIR)/ed_support.s

C_OBJS = $(C_SRCS:$(CDIR)/%.c=$(BUILDDIR)/%.o)
ASM_OBJS = $(ASM_SRCS:$(ASMDIR)/%.s=$(BUILDDIR)/%.o)

OBJS = $(C_OBJS) $(ASM_OBJS)

.PHONY: all deploy clean

all: deploy

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

$(BUILDDIR)/%.o: $(CDIR)/%.c | $(BUILDDIR)
	cc65 -t $(TARGET) -I $(CDIR) $< -o $(BUILDDIR)/$*.s
	ca65 -t $(TARGET) -I $(CDIR) $(BUILDDIR)/$*.s -o $@

$(BUILDDIR)/%.o: $(ASMDIR)/%.s | $(BUILDDIR)
	ca65 -t $(TARGET) -I $(CDIR) $< -o $@

$(BINDIR)/$(OUT): $(OBJS) | $(BINDIR)
	cl65 -t $(TARGET) \
		-Wl -S,0x$(LOADADDR) \
		-Wl -D,__EXEHDR__=0 \
		-m $(BUILDDIR)/ed.map \
		$(OBJS) \
		-o $(BINDIR)/$(OUT)

deploy: $(BINDIR)/$(OUT)
	cp -f "$(BINDIR)/$(OUT)" "$(DEST)"
	xattr -wx prodos.FileType 06 "$(DEST)"
	xattr -wx prodos.AuxType $(AUXTYPE) "$(DEST)"

clean:
	rm -f $(BUILDDIR)/*.o
	rm -f $(BUILDDIR)/*.s
	rm -f $(BUILDDIR)/ed.map
	rm -f $(BINDIR)/$(OUT)