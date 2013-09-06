#---------------------------------------------------------------------------------
# Clear the implicit built in rules
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITXENON)),)
$(error "Please set DEVKITXENON in your environment. export DEVKITXENON=<path to>devkitPPC")
endif

include $(DEVKITXENON)/rules

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
#---------------------------------------------------------------------------------
TARGET		:=	$(notdir $(CURDIR))
BUILD		:=	build
SOURCES		:=	source source/libwiigui source/tinyxml data source/lang
DATA		:=	
INCLUDES	:=	mplayer
MPLAYER		:=	$(CURDIR)/mplayer
#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------

# lto
#OPTIFLAGS	=	-Ofast -flto -fuse-linker-plugin -mcpu=cell -mtune=cell -fno-tree-vectorize -fno-tree-slp-vectorize -ftree-vectorizer-verbose=1
OPTIFLAGS =  -O3 -mcpu=cell -mtune=cell -fno-tree-vectorize -fno-tree-slp-vectorize -ftree-vectorizer-verbose=1 

ASFLAGS		=	-Wa,-a32
CFLAGS		=	-g  -fno-pic $(OPTIFLAGS) -Wall $(MACHDEP) $(INCLUDE)
CXXFLAGS		=	$(CFLAGS)

MACHDEP_LD	=	-DXENON -m32 -maltivec -fno-pic -mhard-float -L$(DEVKITXENON)/xenon/lib/32 -u read -u _start -u exc_base
LDFLAGS		=	-g $(MACHDEP_LD) $(OPTIFLAGS) -Wl,-Map,$(notdir $@).map

#CFLAGS	= -ffunction-sections -fdata-sections -g -O2 -Wall $(MACHDEP) $(INCLUDE)
#CXXFLAGS	=	$(CFLAGS)

#LDFLAGS	=	-g $(MACHDEP) -Wl,--gc-sections -Wl,-Map,$(notdir $@).map

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS		:=	 -lmplayer  -lavformat -lavcodec -lswscale -lavutil -lpostproc -lpng -lz -lxenon -lm  -liconv -lfreetype -lfribidi -lbz2 -lfat -lext2fs -lntfs -lxtaf

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS		:=

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

#---------------------------------------------------------------------------------
# automatically build a list of object files for our project
#---------------------------------------------------------------------------------
CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
sFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))
TTFFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.ttf)))
LANGFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.lang)))
PNGFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.png)))
PCMFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.pcm)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
	export LD	:=	$(CC)
else
	export LD	:=	$(CXX)
endif

export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
			$(TTFFILES:.ttf=.ttf.o) \
			$(LANGFILES:.lang=.lang.o) \
			$(PNGFILES:.png=.png.o) \
			$(PCMFILES:.pcm=.pcm.o) \
			$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) \
			$(sFILES:.s=.o) $(SFILES:.S=.o)

#---------------------------------------------------------------------------------
# build a list of include paths
#---------------------------------------------------------------------------------
export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(LIBXENON_INC)/freetype2/ \
			-I$(CURDIR)/$(BUILD) \
			-I$(MPLAYER)/ffmpeg/ \
			-I$(LIBXENON_INC)

#---------------------------------------------------------------------------------
# build a list of library paths
#---------------------------------------------------------------------------------
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib) \
			-L$(MPLAYER)/ \
			-L$(MPLAYER)/ffmpeg/libavcodec \
			-L$(MPLAYER)/ffmpeg/libavformat \
			-L$(MPLAYER)/ffmpeg/libavutil \
			-L$(MPLAYER)/ffmpeg/libpostproc \
			-L$(MPLAYER)/ffmpeg/libswscale \
			-L$(MPLAYER)/ffmpeg/ \
			-L$(LIBXENON_LIB)

export OUTPUT	:=	$(CURDIR)/$(TARGET)
.PHONY: $(BUILD) clean

#---------------------------------------------------------------------------------
$(BUILD):
	@echo build player
	cd mplayer; $(MAKE) -f Makefile lib -j4; cd ../..
	@[ -d $@ ] || mkdir -p $@
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile -j4

#---------------------------------------------------------------------------------
clean:
	@echo clean ... do make fullclean to clean everything
	@rm -fr $(BUILD) $(OUTPUT).elf $(OUTPUT).elf32
	
fullclean: clean
	@echo clean mplayer and libav
	cd mplayer; $(MAKE) -f Makefile clean;
	
dist: fullclean
  	@rm -f $(TARGET).tar.gz
	@$(MAKE)
	@mkdir -p release/_DEBUG
	@cp $(TARGET).elf release/_DEBUG/
	@cp $(TARGET).elf32 release/xenon.elf
	@mkdir -p release/mplayer/
	@cp mplayer-files/* release/mplayer/
	@cd release; tar czvf $(TARGET).tar.gz *; mv $(TARGET).tar.gz ..
	@rm -rf release
	@$(MAKE) clean

#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(OUTPUT).elf32: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)

-include $(DEPENDS)

%.png.o : %.png
	@echo $(notdir $<)
	@$(bin2o)
	
%.ttf.o : %.ttf
	@echo $(notdir $<)
	$(bin2o)
	
%.lang.o : %.lang
	@echo $(notdir $<)
	$(bin2o)
	
%.elf:
	@echo linking ... $(notdir $@)
	$(LD)  $^ $(LDFLAGS) $(LIBPATHS) $(LIBS) -n -T $(LDSCRIPT) -o $@

%.elf32: %.elf
	@echo converting and stripping ... $(notdir $@)
	$(OBJCOPY) -O elf32-powerpc --adjust-vma 0x80000000 $< $@
	$(PREFIX)strip $@


#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

#source/ffs_content.c: genffs.py data/ps.psu data/vs.vsu
#	python genffs.py > source/ffs_content.c

run: $(BUILD) $(OUTPUT).elf32
	cp $(OUTPUT).elf32 /srv/tftp/tftpboot/xenon
	$(PREFIX)strip /srv/tftp/tftpboot/xenon
#	/home/dev360/run
