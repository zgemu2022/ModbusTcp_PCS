ifndef CROSS_COMPILE
#CROSS_COMPILE	:= arm-none-linux-gnueabihf-
CROSS_COMPILE	:= arm-linux-gnueabihf-
endif

CC			:=	${CROSS_COMPILE}gcc
CPP			:=	${CROSS_COMPILE}cpp
AS			:=	${CROSS_COMPILE}gcc
AR			:=	${CROSS_COMPILE}ar
LINKER			:=	${CROSS_COMPILE}ld
OC			:=	${CROSS_COMPILE}objcopy
OD			:=	${CROSS_COMPILE}objdump
NM			:=	${CROSS_COMPILE}nm
PP			:=	${CROSS_COMPILE}gcc -E

BUILD_DIR := $(PWD)/build
LIB_DIR := -L$(PWD)/src/lib
MAKE_DEP = -Wp,-MD,$(DEP) -MT $$@ -MP

INCLUDES		+=	-I./src				\
                    -I./src/include     \
	                -I./src/socket \
					-I./src/protocol \
					-I./src/importlib		

_SOURCES_ += ./src/modbus_tcp_main.c
_SOURCES_ += $(wildcard ./src/socket/*.c)
_SOURCES_ += $(wildcard ./src/protocol/*.c)
_SOURCES_ += $(wildcard ./src/importlib/*.c)
$(eval OBJS1 := $(patsubst %.c,%.o,$(notdir $(_SOURCES_)))) 
$(info OBJS1 = $(OBJS1))
$(eval OBJS = $(addprefix $(BUILD_DIR)/,$(OBJS1)))
$(info OBJS= $(OBJS))


all:$(OBJS) libmodtcp.so
	@echo "12345 ssdlh"
#编译完成后的拷贝的路径
	-cp libmodtcp.so  /mnt/hgfs/Tcu_lib/libmodtcp1.so;
#-cp libmodtcp.so  /mnt/hgfs/share_E/zgEMU/Program/libmodtcp.so;

libmodtcp.so: $(OBJS)
	@echo "54321 xxxxx"
	$(LINKER)   $(LIB_DIR) -fmessage-length=0 -shared $^ -lDllCommon -lpthread -lrt -ldl -o $@ 
define MAKE_C

$(eval OBJ = $(addprefix $(1)/,$(patsubst %.c,%.o,$(notdir $(2)))))
$(info OBJ= $(OBJ))
$(eval DEP := $(patsubst %.o,%.d,$(OBJ)))
$(OBJ): $(2)
	-mkdir -p $(BUILD_DIR)
	@echo "  CC      $$<"
	$(CC) -Wall -shared -fPIC -nostartfiles -c  $(INCLUDES)  $(MAKE_DEP)  $$< -o $$@
endef

#$(eval $(call MAKE_C,$(BUILD_DIR),$(_SOURCES_)))
#gcc -shared -fPIC -nostartfiles -o mylib.so mylib.c

$(eval $(foreach obj,$(_SOURCES_),$(call MAKE_C,$(BUILD_DIR),$(obj))))

.PHONY: clean
clean:
	@echo "clean"
	-rm *.so
	-rm -rf 	$(BUILD_DIR)
