packages:=boost openssl qrencode protobuf

ifneq ($(host_os),ios)
ifneq ($(host_flavor),android)
packages += libevent zeromq libcryptopp
endif
endif

ios_packages = qrencode djinni libcryptoppunity libevent
ifeq ($(host_flavor),android)
packages += libcryptoppunity
endif

qt_native_packages = native_protobuf
qt_packages = qrencode protobuf zlib

qt_linux_packages:=qt expat dbus libxcb xcb_proto libXau xproto freetype fontconfig libX11 xextproto libXext xtrans

rapidcheck_packages = rapidcheck

qt_darwin_packages=qt
qt_mingw32_packages=qt

wallet_packages=bdb

upnp_packages=miniupnpc

darwin_native_packages = native_biplist native_ds_store native_mac_alias

ifneq ($(build_os),darwin)
darwin_native_packages += native_cctools native_libtapi native_cdrkit native_libdmg-hfsplus

ifeq ($(strip $(FORCE_USE_SYSTEM_CLANG)),)
darwin_native_packages+= native_clang
endif

endif
