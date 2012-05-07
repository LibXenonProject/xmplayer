OBJS				+= xenon/dsputil_ppc.o             \

FFT-OBJS-$(HAVE_GNU_AS)			+= xenon/fft_altivec_s.o
XENON-OBJS-$(CONFIG_FFT)		+= xenon/fft_altivec.o             \
                                          $(FFT-OBJS-yes)
XENON-$(CONFIG_H264DSP)			+= xenon/h264_altivec.o
XENON-OBJS-$(CONFIG_MPEGAUDIODSP)	+= xenon/mpegaudiodec_altivec.o
XENON-OBJS-$(CONFIG_VC1_DECODER)	+= xenon/vc1dsp_altivec.o
XENON-OBJS-$(CONFIG_VP3_DECODER)	+= xenon/vp3dsp_altivec.o
XENON-OBJS-$(CONFIG_VP5_DECODER)	+= xenon/vp3dsp_altivec.o
XENON-OBJS-$(CONFIG_VP6_DECODER)	+= xenon/vp3dsp_altivec.o
XENON-OBJS-$(CONFIG_VP8_DECODER)	+= xenon/vp8dsp_altivec.o

OBJS-XENON				+= xenon/dsputil_altivec.o         \
                                           xenon/fdct_altivec.o            \
                                           xenon/float_altivec.o           \
                                           xenon/fmtconvert_altivec.o      \
                                           xenon/gmc_altivec.o             \
                                           xenon/idct_altivec.o            \
                                           xenon/int_altivec.o             \
                                           xenon/mpegvideo_altivec.o       \



OBJS					+= xenon/dsputil_vmx.o \
					   xenon/float_altivec.o \
					   xenon/fft_altivec.o \
					   xenon/fmtconvert_vmx.o \
					   xenon/h264_vmx.o \
					   xenon/mpegaudiodec_vmx.o 

#					   xenon/fft_altivec_s.o
	
					   

