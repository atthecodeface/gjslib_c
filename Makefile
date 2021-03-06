
all: prog batch


OS := $(shell uname)

LINK      = g++
LINKFLAGS = -g -lGL -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf
CPPFLAGS  = -g -Wall -I/usr/include/SDL2

ifeq ($(OS),Darwin)
GLM = ../glm
LINK      = c++
LINKFLAGS = -g -iframework /Library/Frameworks -framework SDL2 -framework SDL2_image -framework SDL2_ttf -framework OpenGL -lpng16  -ljpeg -L/usr/local/lib 
CPPFLAGS  = -std=c++11 -DGLM_FORCE_RADIANS -DGL_GLEXT_PROTOTYPES -g -Wall -I$(GLM) -iframework /Library/Frameworks -I/Library/Frameworks/SDL2.framework/Headers -I/Library/Frameworks/SDL2_image.framework/Headers -I/Library/Frameworks/SDL2_ttf.framework/Headers -I/usr/local/include
endif

PROG_OBJS = main.o key_value.o texture.o shader.o filter.o image_io.o lens_projection.o quaternion.o vector.o quaternion_image_correlator.o
BATCH_OBJS = batch.o key_value.o texture.o shader.o filter.o image_io.o lens_projection.o quaternion.o vector.o
# image_correlator.o
PY_OBJS := gjslib_c.o python_texture.o python_filter.o python_lens_projection.o python_quaternion.o python_vector.o python_image_correlator.o python_quaternion_image_correlator.o\
	filter.o shader.o key_value.o texture.o image_io.o lens_projection.o quaternion.o vector.o image_correlator.o quaternion_image_correlator.o
PYTHON := python2.6
FRAMEWORK_PATH := /Library/Frameworks

#CC := ${GCC} -dynamic -fno-strict-aliasing  -fno-common -fwrapv
#CXX := ${GCC} -dynamic -fno-strict-aliasing -fno-common -fwrapv
#LINK := ${GCC}
#LINKASOBJ := ld -r -o
#LINKASBIN := ${GPP} ${LINKFLAGS} -o
#LINKDYNLIB := ${GPP} ${LINKFLAGS} -dynamiclib -o 
#PYTHONLINKLIB := ${GPP} -L/opt/local/lib -bundle -undefined dynamic_lookup ${LOCAL_LINKFLAGS} -o
#${PYTHON_LIB}: ${C_MODEL_OBJS}  $(TARGET_DIR)/derived_model_list.o ${SUPPORT_PYTHON_OBJS} ${ENGINE_OBJECTS}
#	@echo "Building Python simulation library for GUI sims ${PYTHON_LIB}"
#	${Q}${PYTHONLINKLIB} $@ $(call os_lib_hdr,$@) ${C_MODEL_OBJS} $(TARGET_DIR)/derived_model_list.o ${SUPPORT_PYTHON_OBJS} ${MODEL_LIBS} ${LOCAL_LINKLIBS} ${CYCLICITY_LIBS}

PYTHON=python2.7
FRAMEWORK_PATH := /Library/Frameworks
PYTHON_INCLUDES := -I${FRAMEWORK_PATH}/Python.framework/Versions/Current/include/${PYTHON}

all: gjslib_c.so

python_%.o: python_%.cpp
	c++ ${PYTHON_INCLUDES} ${CPPFLAGS} -c $< -o $@

gjslib_%.o: gjslib_%.cpp
	c++ ${PYTHON_INCLUDES} ${CPPFLAGS} -c $< -o $@

gjslib_c.so: ${PY_OBJS}
	c++ -bundle -undefined dynamic_lookup -o gjslib_c.so ${PY_OBJS} -lpng16 -ljpeg -L/usr/local/lib 

test: test_quaternion test_lens_projection

test_quaternion: quaternion_test
	./quaternion_test

quaternion_test.o: quaternion.h quaternion_test.cpp test.h 

quaternion_test: quaternion_test.o quaternion.o
	$(LINK) quaternion_test.o quaternion.o $(LINKFLAGS) -o quaternion_test


test_lens_projection: lens_projection_test
	./lens_projection_test

lens_projection_test.o: lens_projection.h lens_projection_test.cpp test.h 

lens_projection_test: lens_projection_test.o lens_projection.o quaternion.o
	$(LINK) lens_projection_test.o lens_projection.o quaternion.o $(LINKFLAGS) -o lens_projection_test


prog: $(PROG_OBJS)
	$(LINK) $(PROG_OBJS) $(LINKFLAGS) -o prog

batch: $(BATCH_OBJS)
	$(LINK) $(BATCH_OBJS) $(LINKFLAGS) -o batch

batch_test: batch
	./batch -i images/IMG_1900.JPG -i images/IMG_1901.JPG --orient=0

batch_test2: batch
	./batch -i images/IMG_1900.JPG -i images/IMG_1902.JPG --orient=0

batch_test3: batch
	./batch -i images/IMG_1901.JPG -i images/IMG_1902.JPG --orient=0

batch_test_10: batch
	./batch -i images/IMG_1900.JPG -i images/IMG_1901_r10.JPG --orient=0

batch_test_25: batch
	./batch -i images/IMG_1900.JPG -i images/IMG_1901_r25.JPG --orient=0

batch_test_r: batch
	./batch -i images/IMG_1900.JPG -i images/IMG_1901_r90.JPG --orient=2

blah2: prog
	./prog -n 12 -i images/IMG_1854.JPG -i images/IMG_1855.JPG \
						--filter='glsl:yuv_from_rgb(0,2)&-DSELECTED_VALUE=r' \
						--filter='glsl:yuv_from_rgb(1,8)&-DSELECTED_VALUE=r' \
	                    --filter='glsl:gauss(2,3)&-DX_NOT_Y=false&-DNUM_WEIGHTS=9&-DWEIGHTS=gauss_offset_weights9' \
	                    --filter='glsl:gauss(3,4)&-DX_NOT_Y=true&-DNUM_WEIGHTS=9&-DWEIGHTS=gauss_offset_weights9' \
						--filter='save:test_a.png(4)'  \
	                    --filter='glsl:gauss(8,9)&-DX_NOT_Y=false&-DNUM_WEIGHTS=9&-DWEIGHTS=gauss_offset_weights9' \
	                    --filter='glsl:gauss(9,10)&-DX_NOT_Y=true&-DNUM_WEIGHTS=9&-DWEIGHTS=gauss_offset_weights9' \
						--filter='save:test_b.png(10)'  \
	                    --filter='glsl:harris(4,5)&-DNUM_OFFSETS=25&-DOFFSETS=offsets_2d_25' \
						--filter='save:test_h.png(5)' \
						--filter='find:a(5)' \
						--filter='corr:correlation_copy_shader(2,6)'  --filter='save:test_corr.png(6)' \
	                    --filter='glsl:sum_sq_pixel_diff(6,2,11)' --filter='save:test_ada.png(11)'  --filter='find:a(11)'  \
	                    --filter='glsl:sum_sq_pixel_diff(6,8,11)' --filter='save:test_bda.png(11)'  --filter='find:a(11)' 

blah2b: prog
	./prog -n 12 -i images/IMG_1854.JPG -i images/IMG_1855.JPG \
						--filter='glsl:yuv_from_rgb(0,4)&-DSELECTED_VALUE=r' \
						--filter='glsl:yuv_from_rgb(1,10)&-DSELECTED_VALUE=r' \
	                    --filter='glsl:harris(4,5)&-DNUM_OFFSETS=25&-DOFFSETS=offsets_2d_25' \
						--filter='save:test_h.png(5)' \
						--filter='find:a(5)' \
						--filter='corr:correlation_copy_shader(4,6)'  --filter='save:test_corr.png(6)' \
	                    --filter='glsl:sum_sq_pixel_diff(6,4,11)' --filter='save:test_ada.png(11)'  --filter='find:a(11)'  \
	                    --filter='glsl:sum_sq_pixel_diff(6,10,11)' --filter='save:test_bda.png(11)' --filter='find:a(11)' 

blah2c: prog
	./prog -n 12 -i images/IMG_1854.JPG -i images/IMG_1855.JPG \
						--filter='glsl:yuv_from_rgb(0,2)&-DSELECTED_VALUE=r' \
	                    --filter='glsl:windowed_equalization(2,4)&-DNUM_OFFSETS=81&-DOFFSETS=offsets_2d_81' \
						--filter='glsl:yuv_from_rgb(1,8)&-DSELECTED_VALUE=r' \
	                    --filter='glsl:windowed_equalization(8,10)&-DNUM_OFFSETS=81&-DOFFSETS=offsets_2d_81' \
	                    --filter='glsl:harris(4,5)&-DNUM_OFFSETS=25&-DOFFSETS=offsets_2d_25' \
						--filter='save:test_a.png(4)'  \
						--filter='save:test_b.png(10)'  \
						--filter='save:test_h.png(5)' \
						--filter='find:a(5)' \
						--filter='corr:correlation_copy_shader(4,6)'  --filter='save:test_corr.png(6)' \
	                    --filter='glsl:sum_sq_pixel_diff(6,4,11)' --filter='save:test_ada.png(11)'  --filter='find:a(11)'  \
	                    --filter='glsl:sum_sq_pixel_diff(6,10,11)' --filter='save:test_bda.png(11)' --filter='find:a(11)' 

blah3: prog
	./prog -n 12 -i images/IMG_1854.JPG -i images/IMG_1855.JPG \
						--filter='glsl:yuv_from_rgb(0,2)&-DSELECTED_VALUE=r&-DINTENSITY_XSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_YSCALE=1.0&-DINTENSITY_YOFS=0.0' \
						--filter='glsl:yuv_from_rgb(1,8)&-DSELECTED_VALUE=r&-DINTENSITY_XSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_YSCALE=1.0&-DINTENSITY_YOFS=0.0' \
	                    --filter='glsl:gauss(2,3)&-DX_NOT_Y=false&-DNUM_WEIGHTS=9&-DWEIGHTS=gauss_offset_weights9' \
	                    --filter='glsl:gauss(3,4)&-DX_NOT_Y=true&-DNUM_WEIGHTS=9&-DWEIGHTS=gauss_offset_weights9' \
						--filter='save:test_a.png(2)'  \
	                    --filter='glsl:gauss(8,9)&-DX_NOT_Y=false&-DNUM_WEIGHTS=9&-DWEIGHTS=gauss_offset_weights9' \
	                    --filter='glsl:gauss(9,10)&-DX_NOT_Y=true&-DNUM_WEIGHTS=9&-DWEIGHTS=gauss_offset_weights9' \
						--filter='save:test_b.png(8)'  \
	                    --filter='glsl:harris(4,5)&-DNUM_OFFSETS=25&-DOFFSETS=offsets_2d_25' \
						--filter='save:test_h.png(5)' \
						--filter='find:a(5)' \
						--filter='corr:correlation_copy_circle(2,6)'  --filter='save:test_corr.png(6)'

blah4: prog
	./prog -n 12 -i images/IMG_1854.JPG -i images/IMG_1855.JPG \
						--filter='glsl:yuv_from_rgb(0,2)&-DSELECTED_VALUE=r&-DINTENSITY_XSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_YSCALE=1.0&-DINTENSITY_YOFS=0.0' \
						--filter='glsl:yuv_from_rgb(1,8)&-DSELECTED_VALUE=r&-DINTENSITY_XSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_YSCALE=1.0&-DINTENSITY_YOFS=0.0' \
	                    --filter='glsl:gauss(2,3)&-DX_NOT_Y=false&-DNUM_WEIGHTS=9&-DWEIGHTS=gauss_offset_weights9' \
	                    --filter='glsl:gauss(3,4)&-DX_NOT_Y=true&-DNUM_WEIGHTS=9&-DWEIGHTS=gauss_offset_weights9' \
						--filter='save:test_a.png(2)'  \
	                    --filter='glsl:gauss(8,9)&-DX_NOT_Y=false&-DNUM_WEIGHTS=9&-DWEIGHTS=gauss_offset_weights9' \
	                    --filter='glsl:gauss(9,10)&-DX_NOT_Y=true&-DNUM_WEIGHTS=9&-DWEIGHTS=gauss_offset_weights9' \
						--filter='save:test_b.png(8)'  \
	                    --filter='glsl:harris(4,5)&-DNUM_OFFSETS=25&-DOFFSETS=offsets_2d_25' \
						--filter='save:test_h.png(5)' \
						--filter='find:a(5)' \
						--filter='corr:correlation_dft_circle(2,6)'  --filter='save:test_corr.png(6)' \
						--filter='glsl:sq_dft_diff(6,2,7)'  --filter='save:test_ada.png(7)' \
						--filter='find:a(7)' \
						--filter='glsl:sq_dft_diff(6,10,7)'  --filter='save:test_bda.png(7)' \
						--filter='find:a(7)'

blah5: prog
	./prog -n 12 -i images/IMG_1854.JPG -i images/IMG_1855.JPG \
						--filter='glsl:yuv_from_rgb(0,2)&-DSELECTED_VALUE=r&-DINTENSITY_XSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_YSCALE=1.0&-DINTENSITY_YOFS=0.0' \
						--filter='glsl:yuv_from_rgb(1,8)&-DSELECTED_VALUE=r&-DINTENSITY_XSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_YSCALE=1.0&-DINTENSITY_YOFS=0.0' \
	                    --filter='glsl:harris(2,5)&-DNUM_OFFSETS=25&-DOFFSETS=offsets_2d_25' \
						--filter='save:test_a.png(2)'  \
						--filter='save:test_b.png(8)'  \
						--filter='save:test_h.png(5)' \
						--filter='find:a(5)' \
						--filter='corr:correlation_dft_circle(2,6)&-DCORR_CIRCLE_RADIUS=8'  --filter='save:test_corr_8.png(6)' \
						--filter='glsl:sq_dft_diff(6,8,7)&-DCORR_CIRCLE_RADIUS=8'           --filter='save:test_bda_8.png(7)' \
						--filter='corr:correlation_dft_circle(2,6)&-DCORR_CIRCLE_RADIUS=6'  --filter='save:test_corr_6.png(6)' \
						--filter='glsl:sq_dft_diff(6,8,3)&-DCORR_CIRCLE_RADIUS=6'           --filter='save:test_bda_6.png(3)' \
						--filter='glsl:alu_buffers(3,7,4)&-DOP=src_b*src_a'  --filter='save:test_mult_2.png(4)' \
						--filter='find:a(3)' \
						--filter='find:a(4)'

blah6: prog
	./prog -n 12 -i images/IMG_1900.JPG -i images/IMG_1901.JPG \
						--filter='glsl:yuv_from_rgb(0,2)&-DSELECTED_VALUE=r&-DINTENSITY_XSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_YSCALE=1.0&-DINTENSITY_YOFS=0.0' \
						--filter='glsl:yuv_from_rgb(1,8)&-DSELECTED_VALUE=r&-DINTENSITY_XSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_YSCALE=1.0&-DINTENSITY_YOFS=0.0' \
						--filter='save:test_a.png(2)'  \
						--filter='save:test_b.png(8)'  \
	                    --filter='glsl:harris(2,5)&-DNUM_OFFSETS=25&-DOFFSETS=offsets_2d_25' \
						--filter='save:test_h.png(5)' \
						--filter='find:a(5)' \
						--filter='corr:correlation_dft_circle(2,6)&-DCORR_CIRCLE_RADIUS=8'  --filter='save:test_corr_8.png(6)' \
						--filter='glsl:sq_dft_diff(6,8,7)&-DCORR_CIRCLE_RADIUS=8'           --filter='save:test_bda_8.png(7)' \
						--filter='corr:correlation_dft_circle(2,6)&-DCORR_CIRCLE_RADIUS=4'  --filter='save:test_corr_4.png(6)' \
						--filter='glsl:sq_dft_diff(6,8,3)&-DCORR_CIRCLE_RADIUS=4'           --filter='save:test_bda_4.png(3)' \
						--filter='glsl:alu_buffers(3,7,4)&-DOP=src_b*src_a'  --filter='save:test_mult_2.png(4)' \
						--filter='corr:correlation_dft_circle(2,6)&-DCORR_CIRCLE_RADIUS=6'  --filter='save:test_corr_6.png(6)' \
						--filter='glsl:sq_dft_diff(6,8,7)&-DCORR_CIRCLE_RADIUS=6'           --filter='save:test_bda_6.png(7)' \
						--filter='glsl:alu_buffers(4,7,3)&-DOP=src_b*src_a'  --filter='save:test_mult_3.png(3)' \
						--filter='corr:correlation_dft_circle(2,6)&-DCORR_CIRCLE_RADIUS=7'  --filter='save:test_corr_7.png(6)' \
						--filter='glsl:sq_dft_diff(6,8,3)&-DCORR_CIRCLE_RADIUS=7'           --filter='save:test_bda_7.png(3)' \
						--filter='glsl:alu_buffers(3,7,4)&-DOP=src_b*src_a'  --filter='save:test_mult_4.png(4)' \
						--filter='find:a(4)'

blah7: prog
	./prog -n 12 -i images/IMG_1900.JPG -i images/IMG_1901.JPG \
						--filter='glsl:yuv_from_rgb(0,2)&-DINTENSITY_XSCALE=(3456.0/5184.0/2.0)&-DINTENSITY_XOFS=0.2&-DINTENSITY_YSCALE=(1.0/2.0)&-DINTENSITY_YOFS=0.43' \
						--filter='glsl:yuv_from_rgb(1,3)&-DINTENSITY_XSCALE=(3456.0/5184.0/2.0)&-DINTENSITY_XOFS=0.2&-DINTENSITY_YSCALE=(1.0/2.0)&-DINTENSITY_YOFS=0.43' \
						--filter='save:test_a.png(2)'  \
						--filter='save:test_b.png(3)'  \
	                    --filter='glsl:harris(2,5)&-DNUM_OFFSETS=25&-DOFFSETS=offsets_2d_25' \
						--filter='save:test_h.png(5)' \
						--filter='find:a(5)' \
						--filter='corr:correlation_dft_circle(2,6)&-DCORR_CIRCLE_RADIUS=8&-Usrc_x_ofs=0.0f&-Usrc_y_ofs=-4.0f'  --filter='save:test_corr_8.png(6)' \
						--filter='glsl:sq_dft_diff(6,3,7)&-DCORR_CIRCLE_RADIUS=8'           --filter='save:test_bda_8.png(7)' \
						--filter='find:a(7)'



exercise: gauss harris windowed_equalization sobel alu_constant alu_mult fft

alu_constant: prog
	./prog -n 3 \
		--filter='glsl:alu_buffers(1,1,0)&-DOP=vec4(0.5)'  --filter='save:test_constant_50.png(0)' \
		--filter='glsl:alu_buffers(0,0,1)&-DOP=vec4(0.8)'  --filter='save:test_constant_80.png(0)' \
		--filter='glsl:alu_buffers(1,0,2)&-DOP=src_a*src_b'  --filter='save:test_constant_40.png(2)'

alu_mult: prog
	./prog -n 3 \
		--filter='glsl:alu_buffers(1,1,0)&-DOP=vec4(x*y)'       --filter='save:test_xy.png(0)' \
		--filter='glsl:alu_buffers(0,0,1)&-DOP=vec4((1-x)*y)'   --filter='save:test_xy2.png(1)' \
		--filter='glsl:alu_buffers(1,0,2)&-DOP=vec4(1-4*src_a*src_b)' --filter='save:test_xymult.png(2)'

windowed_equalization: prog
	./prog -n 4 -i images/IMG_1854.JPG \
						--filter='glsl:yuv_from_rgb(0,1)' \
	                    --filter='glsl:windowed_equalization(1,2)&-DNUM_OFFSETS=81&-DOFFSETS=offsets_2d_81' \
						--filter='save:test_a.png(2)'

copy_img: prog
	./prog -n 4 -i images/IMG_1854.JPG \
						--filter='save:test_a.png(0)'

sobel: prog
	./prog -n 4 -i images/IMG_1854.JPG \
					    --filter='glsl:yuv_from_rgb(0,1)' \
	                    --filter='glsl:convolve_2d(1,3)&-DNUM_WEIGHTS=9&-DOFFSET_WEIGHTS=sobel_weights' \
						--filter='save:test_a.png(3)'


gauss: prog
	./prog -n 4 -i images/IMG_1854.JPG \
						--filter='glsl:yuv_from_rgb(0,1)&-DSELECTED_VALUE=r' \
	                    --filter='glsl:gauss(1,2)&-DX_NOT_Y=false&-DNUM_WEIGHTS=9&-DWEIGHTS=gauss_offset_weights9' \
	                    --filter='glsl:gauss(2,3)&-DX_NOT_Y=true&-DNUM_WEIGHTS=9&-DWEIGHTS=gauss_offset_weights9' \
						--filter='save:test_a.png(3)'

harris: prog
	./prog -n 4 -i images/IMG_1854.JPG \
						--filter='glsl:yuv_from_rgb(0,1)' \
	                    --filter='glsl:harris(1,2)&-DNUM_OFFSETS=25&-DOFFSETS=offsets_2d_25' \
						--filter='save:test_a.png(2)'

fft: prog
	./prog -n 12 -i images/IMG_1900.JPG \
						--filter='glsl:yuv_from_rgb(0,2)&-DSELECTED_VALUE=r' \
	                    --filter='glsl:gauss(2,3)&-DX_NOT_Y=false&-DNUM_WEIGHTS=9&-DWEIGHTS=gauss_offset_weights9' \
	                    --filter='glsl:gauss(3,2)&-DX_NOT_Y=true&-DNUM_WEIGHTS=9&-DWEIGHTS=gauss_offset_weights9' \
						--filter='save:test_a.png(2)'  \
	                    --filter='glsl:harris(2,5)&-DNUM_OFFSETS=25&-DOFFSETS=offsets_2d_25' \
						--filter='save:test_h.png(5)' \
						--filter='find:a(5)' \
						--filter='corr:correlation_dft_circle(2,6)&-DCORR_CIRCLE_RADIUS=8'  --filter='save:test_corr_8.png(6)' \
						--filter='find:a(6)'

fft2: prog
	./prog -n 12 -i images/IMG_1900.JPG -i images/IMG_1901_r90.JPG \
						--filter='glsl:yuv_from_rgb(0,2)&-DINTENSITY_XSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_YSCALE=1.0&-DINTENSITY_YOFS=0.0' \
						--filter='glsl:yuv_from_rgb(1,3)&-DINTENSITY_YSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_XSCALE=1.0&-DINTENSITY_YOFS=0.0' \
						--filter='save:test_a.png(2)'  \
						--filter='save:test_b.png(3)'  \
	                    --filter='glsl:circle_dft(2,4)&-DNUM_CIRCLE_STEPS=8&-DDFT_CIRCLE_RADIUS=6' \
	                    --filter='glsl:circle_dft(3,5)&-DNUM_CIRCLE_STEPS=8&-DDFT_CIRCLE_RADIUS=6' \
						--filter='save:test_c0.png(4)' \
						--filter='save:test_c2.png(4)&conv=fred&green=1' \
						--filter='save:test_c2.png(4)&conv=fred&green=1' \
						--filter='find:a(4)' \
	                    --filter='glsl:circle_dft_diff(4,5,6)&-Uuv_base_x=413f&-Uuv_base_y=648f' \
	                    --filter='glsl:circle_dft_diff(4,5,7)&-Uuv_base_x=409f&-Uuv_base_y=652f' \
	                    --filter='glsl:circle_dft_diff(4,5,8)&-Uuv_base_x=405f&-Uuv_base_y=648f' \
	                    --filter='glsl:circle_dft_diff(4,5,9)&-Uuv_base_x=409f&-Uuv_base_y=644f' \
						--filter='save:test_d.png(6)'

fft3: prog
	./prog -n 12 -i images/IMG_1900.JPG -i images/IMG_1901_r90.JPG \
						--filter='glsl:yuv_from_rgb(0,2)&-DINTENSITY_XSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_YSCALE=1.0&-DINTENSITY_YOFS=0.0' \
						--filter='glsl:yuv_from_rgb(1,3)&-DINTENSITY_YSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_XSCALE=1.0&-DINTENSITY_YOFS=0.0' \
						--filter='save:test_a.png(2)'  \
						--filter='save:test_b.png(3)'  \
	                    --filter='glsl:circle_dft(2,4)&-DNUM_CIRCLE_STEPS=8&-DDFT_CIRCLE_RADIUS=6' \
	                    --filter='glsl:circle_dft(3,5)&-DNUM_CIRCLE_STEPS=8&-DDFT_CIRCLE_RADIUS=6' \
						--filter='save:test_c0.png(4)' \
						--filter='save:test_c2.png(4)&conv=fred&green=1' \
						--filter='save:test_c2.png(4)&conv=fred&green=1' \
						--filter='find:a(4)' \
	                    --filter='glsl:circle_dft_diff(4,5,6)&-Uuv_base_x=417f&-Uuv_base_y=618f' \
	                    --filter='glsl:circle_dft_diff(4,5,7)&-Uuv_base_x=413f&-Uuv_base_y=622f' \
	                    --filter='glsl:circle_dft_diff(4,5,8)&-Uuv_base_x=409f&-Uuv_base_y=618f' \
	                    --filter='glsl:circle_dft_diff(4,5,9)&-Uuv_base_x=413f&-Uuv_base_y=614f' \
						--filter='save:test_d.png(6)'

fft4: prog
	./prog -n 12 -i images/IMG_1900.JPG -i images/IMG_1902.JPG \
						--filter='glsl:yuv_from_rgb(0,2)&-DINTENSITY_XSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_YSCALE=1.0&-DINTENSITY_YOFS=0.0' \
						--filter='glsl:yuv_from_rgb(1,3)&-DINTENSITY_YSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_XSCALE=1.0&-DINTENSITY_YOFS=0.0' \
						--filter='save:test_a.png(2)'  \
						--filter='save:test_b.png(3)'  \
	                    --filter='glsl:circle_dft(2,4)&-DNUM_CIRCLE_STEPS=8&-DDFT_CIRCLE_RADIUS=6&-DCIRCLE_COMPONENT=r' \
	                    --filter='glsl:circle_dft(3,5)&-DNUM_CIRCLE_STEPS=8&-DDFT_CIRCLE_RADIUS=6&-DCIRCLE_COMPONENT=r' \
						--filter='save:test_c0.png(4)' \
						--filter='save:test_c2.png(4)&conv=fred&green=1' \
						--filter='save:test_c2.png(4)&conv=fred&green=1' \
						--filter='find:a(4)' \
	                    --filter='glsl:circle_dft_diff(4,5,6)&-Uuv_base_x=413f&-Uuv_base_y=648f' \
	                    --filter='glsl:circle_dft_diff(4,5,7)&-Uuv_base_x=409f&-Uuv_base_y=652f' \
	                    --filter='glsl:circle_dft_diff(4,5,8)&-Uuv_base_x=405f&-Uuv_base_y=648f' \
	                    --filter='glsl:circle_dft_diff(4,5,9)&-Uuv_base_x=409f&-Uuv_base_y=644f' \
						--filter='save:test_d.png(6)'


fft5: prog
	./prog -n 12 -i images/IMG_1900.JPG -i images/IMG_1901.JPG \
						--filter='glsl:yuv_from_rgb(0,2)&-DINTENSITY_XSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_YSCALE=1.0&-DINTENSITY_YOFS=0.0' \
						--filter='glsl:yuv_from_rgb(1,3)&-DINTENSITY_XSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_YSCALE=1.0&-DINTENSITY_YOFS=0.0' \
						--filter='save:test_a.png(2)'  \
						--filter='save:test_b.png(3)'  \
	                    --filter='glsl:circle_dft(2,4)&-DNUM_CIRCLE_STEPS=8&-DDFT_CIRCLE_RADIUS=6&-DCIRCLE_COMPONENT=r' \
	                    --filter='glsl:circle_dft(3,5)&-DNUM_CIRCLE_STEPS=8&-DDFT_CIRCLE_RADIUS=6&-DCIRCLE_COMPONENT=r' \
	                    --filter='glsl:circle_dft_diff(4,5,6)&-Uuv_base_x=413f&-Uuv_base_y=648f' \
	                    --filter='glsl:circle_dft_diff(4,5,7)&-Uuv_base_x=409f&-Uuv_base_y=652f' \
	                    --filter='glsl:circle_dft_diff(4,5,8)&-Uuv_base_x=405f&-Uuv_base_y=648f' \
	                    --filter='glsl:circle_dft_diff(4,5,9)&-Uuv_base_x=409f&-Uuv_base_y=644f' \
						--filter='save:test_d.png(6)'

fft5b: prog
	./prog -n 12 -i images/IMG_1900.JPG -i images/IMG_1901_r10.JPG \
						--filter='glsl:yuv_from_rgb(0,2)&-DINTENSITY_XSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_YSCALE=1.0&-DINTENSITY_YOFS=0.0' \
						--filter='glsl:yuv_from_rgb(1,3)&-DINTENSITY_XSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_YSCALE=1.0&-DINTENSITY_YOFS=0.0' \
						--filter='save:test_a.png(2)'  \
						--filter='save:test_b.png(3)'  \
	                    --filter='glsl:circle_dft(2,4)&-DNUM_CIRCLE_STEPS=8&-DDFT_CIRCLE_RADIUS=6&-DCIRCLE_COMPONENT=r' \
	                    --filter='glsl:circle_dft(3,5)&-DNUM_CIRCLE_STEPS=8&-DDFT_CIRCLE_RADIUS=6&-DCIRCLE_COMPONENT=r' \
	                    --filter='glsl:circle_dft_diff(4,5,6)&-Uuv_base_x=413f&-Uuv_base_y=648f' \
	                    --filter='glsl:circle_dft_diff(4,5,7)&-Uuv_base_x=409f&-Uuv_base_y=652f' \
	                    --filter='glsl:circle_dft_diff(4,5,8)&-Uuv_base_x=405f&-Uuv_base_y=648f' \
	                    --filter='glsl:circle_dft_diff(4,5,9)&-Uuv_base_x=409f&-Uuv_base_y=644f' \
						--filter='save:test_d10.png(6)'

fft5c: prog
	./prog -n 12 -i images/IMG_1900.JPG -i images/IMG_1901_r25.JPG \
						--filter='glsl:yuv_from_rgb(0,2)&-DINTENSITY_XSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_YSCALE=1.0&-DINTENSITY_YOFS=0.0' \
						--filter='glsl:yuv_from_rgb(1,3)&-DINTENSITY_XSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_YSCALE=1.0&-DINTENSITY_YOFS=0.0' \
						--filter='save:test_a.png(2)'  \
						--filter='save:test_b.png(3)'  \
	                    --filter='glsl:circle_dft(2,4)&-DNUM_CIRCLE_STEPS=8&-DDFT_CIRCLE_RADIUS=6&-DCIRCLE_COMPONENT=r' \
	                    --filter='glsl:circle_dft(3,5)&-DNUM_CIRCLE_STEPS=8&-DDFT_CIRCLE_RADIUS=6&-DCIRCLE_COMPONENT=r' \
	                    --filter='glsl:circle_dft_diff(4,5,6)&-Uuv_base_x=413f&-Uuv_base_y=648f' \
	                    --filter='glsl:circle_dft_diff(4,5,7)&-Uuv_base_x=409f&-Uuv_base_y=652f' \
	                    --filter='glsl:circle_dft_diff(4,5,8)&-Uuv_base_x=405f&-Uuv_base_y=648f' \
	                    --filter='glsl:circle_dft_diff(4,5,9)&-Uuv_base_x=409f&-Uuv_base_y=644f' \
						--filter='save:test_d25.png(6)'

fft5d: prog
	./prog -n 12 -i images/IMG_1900.JPG -i images/IMG_1901_r90.JPG \
						--filter='glsl:yuv_from_rgb(0,2)&-DINTENSITY_XSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_YSCALE=1.0&-DINTENSITY_YOFS=0.0' \
						--filter='glsl:yuv_from_rgb(1,3)&-DINTENSITY_YSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_XSCALE=1.0&-DINTENSITY_YOFS=0.0' \
						--filter='save:test_a.png(2)'  \
						--filter='save:test_b.png(3)'  \
	                    --filter='glsl:circle_dft(2,4)&-DNUM_CIRCLE_STEPS=8&-DDFT_CIRCLE_RADIUS=6&-DCIRCLE_COMPONENT=r' \
	                    --filter='glsl:circle_dft(3,5)&-DNUM_CIRCLE_STEPS=8&-DDFT_CIRCLE_RADIUS=6&-DCIRCLE_COMPONENT=r' \
	                    --filter='glsl:circle_dft_diff(4,5,6)&-Uuv_base_x=413f&-Uuv_base_y=648f' \
	                    --filter='glsl:circle_dft_diff(4,5,7)&-Uuv_base_x=409f&-Uuv_base_y=652f' \
	                    --filter='glsl:circle_dft_diff(4,5,8)&-Uuv_base_x=405f&-Uuv_base_y=648f' \
	                    --filter='glsl:circle_dft_diff(4,5,9)&-Uuv_base_x=409f&-Uuv_base_y=644f' \
						--filter='save:test_d90.png(6)'

fft6: prog
	./prog -n 12 -i images/IMG_1900.JPG -i images/IMG_1901_r25.JPG \
						--filter='glsl:yuv_from_rgb(0,2)&-DINTENSITY_XSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_YSCALE=1.0&-DINTENSITY_YOFS=0.0' \
						--filter='glsl:yuv_from_rgb(1,3)&-DINTENSITY_XSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_YSCALE=1.0&-DINTENSITY_YOFS=0.0' \
	                    --filter='glsl:circle_dft(2,4)&-DDFT_CIRCLE_RADIUS=8&-DCIRCLE_COMPONENT=g' \
	                    --filter='glsl:circle_dft(3,5)&-DDFT_CIRCLE_RADIUS=8&-DCIRCLE_COMPONENT=g' \
	                    --filter='glsl:circle_dft_diff(4,5,6)&-Uuv_base_x=413f&-Uuv_base_y=648f' \
	                    --filter='glsl:circle_dft_diff(4,5,7)&-Uuv_base_x=409f&-Uuv_base_y=652f' \
	                    --filter='glsl:circle_dft_diff(4,5,8)&-Uuv_base_x=405f&-Uuv_base_y=648f' \
	                    --filter='glsl:circle_dft_diff(4,5,9)&-Uuv_base_x=409f&-Uuv_base_y=644f' \
	                    --filter='glsl:circle_dft_diff_combine(6,7,8,9,10)&-DDISCRETE_CIRCLE_OFS=discrete_circle_offsets_4_16&-DNUM_OFFSETS=16' \
						--filter='find:a(10)&min_distance=2.5&max_elements=100&minimum=0.1' \

fft7: prog
	./prog -n 12 -i images/IMG_1900.JPG -i images/IMG_1901_r25.JPG \
						--filter='glsl:yuv_from_rgb(0,2)&-DINTENSITY_XSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_YSCALE=1.0&-DINTENSITY_YOFS=0.0' \
						--filter='glsl:yuv_from_rgb(1,3)&-DINTENSITY_XSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_YSCALE=1.0&-DINTENSITY_YOFS=0.0' \
	                    --filter='glsl:circle_dft(2,4)&-DNUM_CIRCLE_STEPS=8&-DDFT_CIRCLE_RADIUS=8&-DCIRCLE_COMPONENT=g' \
	                    --filter='glsl:circle_dft(3,5)&-DNUM_CIRCLE_STEPS=8&-DDFT_CIRCLE_RADIUS=8&-DCIRCLE_COMPONENT=g' \
	                    --filter='glsl:circle_dft_diff(4,5,6)&-Uuv_base_x=413f&-Uuv_base_y=648f' \
	                    --filter='glsl:circle_dft_diff(4,5,7)&-Uuv_base_x=409f&-Uuv_base_y=652f' \
	                    --filter='glsl:circle_dft_diff(4,5,8)&-Uuv_base_x=405f&-Uuv_base_y=648f' \
	                    --filter='glsl:circle_dft_diff(4,5,9)&-Uuv_base_x=409f&-Uuv_base_y=644f' \
	                    --filter='glsl:circle_dft_diff_combine(6,7,8,9,10)&-DDISCRETE_CIRCLE_OFS=discrete_circle_offsets_4_32&-DNUM_OFFSETS=32' \
						--filter='save:test_d25.png(10)' \
						--filter='find:a(10)' \



octagon1: batch
	./batch -i images/IMG_2157.JPG -i images/IMG_2158.JPG --orient=0


clean:
	rm *.o prog

