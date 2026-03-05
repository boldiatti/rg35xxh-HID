################################################################################
#
# rg35xxh-controller
#
################################################################################

RG35XXH_CONTROLLER_VERSION = 1.0
RG35XXH_CONTROLLER_SITE = $(BR2_EXTERNAL_RG35XXH_PATH)/package/rg35xxh-controller
RG35XXH_CONTROLLER_SITE_METHOD = local
RG35XXH_CONTROLLER_LICENSE = GPL-2.0
RG35XXH_CONTROLLER_DEPENDENCIES = libevdev bluez5_utils libusb alsa-lib

define RG35XXH_CONTROLLER_BUILD_CMDS
    $(MAKE) CC=$(TARGET_CC) -C $(@D) all
endef

define RG35XXH_CONTROLLER_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m 0755 $(@D)/rg35xxh-controller \
        $(TARGET_DIR)/usr/bin/rg35xxh-controller
endef

$(eval $(generic-package))