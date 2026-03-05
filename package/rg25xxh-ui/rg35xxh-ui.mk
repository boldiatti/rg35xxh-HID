################################################################################
#
# rg35xxh-ui
#
################################################################################

RG35XXH_UI_VERSION = 1.0
RG35XXH_UI_SITE = $(BR2_EXTERNAL_RG35XXH_PATH)/package/rg35xxh-ui
RG35XXH_UI_SITE_METHOD = local
RG35XXH_UI_LICENSE = GPL-2.0
RG35XXH_UI_DEPENDENCIES = sdl2 sdl2_fbcon

define RG35XXH_UI_BUILD_CMDS
    $(MAKE) CC=$(TARGET_CC) -C $(@D) all
endef

define RG35XXH_UI_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m 0755 $(@D)/rg35xxh-ui \
        $(TARGET_DIR)/usr/bin/rg35xxh-ui
endef

$(eval $(generic-package))