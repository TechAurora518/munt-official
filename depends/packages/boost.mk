package=boost
$(package)_version=1.77.0
$(package)_download_path=https://boostorg.jfrog.io/artifactory/main/release/$($(package)_version)/source/
$(package)_file_name=boost_$(subst .,_,$($(package)_version)).tar.bz2
$(package)_sha256_hash=fc9f85fc030e233142908241af7a846e60630aa7388de9a5fafb1f3a26840854

define $(package)_set_vars
$(package)_config_opts_release=variant=release
$(package)_config_opts_debug=variant=debug
$(package)_config_opts=--layout=tagged --build-type=complete --user-config=user-config.jam
$(package)_config_opts+=threading=multi link=static -sNO_BZIP2=1 -sNO_ZLIB=1
$(package)_config_opts_linux=threadapi=pthread runtime-link=shared
$(package)_config_opts_darwin=--toolset=clang-darwin runtime-link=shared
$(package)_config_opts_mingw32=binary-format=pe target-os=windows threadapi=win32 runtime-link=static
$(package)_config_opts_mingw64=binary-format=pe target-os=windows threadapi=win32 runtime-link=static
$(package)_config_opts_x86_64_mingw32=address-model=64
$(package)_config_opts_x86_64_mingw64=address-model=64
$(package)_config_opts_i686_mingw32=address-model=64
$(package)_config_opts_i686_mingw64=address-model=64
$(package)_config_opts_i686_linux=address-model=32 architecture=x86
$(package)_config_opts_aarch64_ios=architecture=x86 target-os=iphone
$(package)_config_opts_android=define=BOOST_FILESYSTEM_DISABLE_STATX
$(package)_toolset_$(host_os)=gcc
$(package)_toolset_darwin=clang-darwin
$(package)_toolset_ios=darwin
$(package)_archiver_$(host_os)=$($(package)_ar)
$(package)_archiver_darwin=$($(package)_ar)
$(package)_archiver_ios=libtool
$(package)_config_libraries=chrono,filesystem,program_options,system,thread,test
$(package)_cxxflags=-std=c++17
$(package)_cxxflags_$(host_os)=-fvisibility=hidden
$(package)_cxxflags_linux+=-fPIC
$(package)_cxxflags_android+=-fPIC
$(package)_cxxflags_ios=-fvisibility=default
endef

define $(package)_preprocess_cmds
  echo "using $($(package)_toolset_$(host_os)) : : $($(package)_cxx) : <cxxflags>\"$($(package)_cxxflags) $($(package)_cppflags)\" <linkflags>\"$($(package)_ldflags)\" <archiver>\"$($(package)_archiver_$(host_os))\" <striper>\"$(host_STRIP)\"  <ranlib>\"$(host_RANLIB)\" <rc>\"$(host_WINDRES)\" : ;" > user-config.jam
endef

define $(package)_config_cmds
  ./bootstrap.sh --without-icu --with-libraries=$(boost_config_libraries)
endef

define $(package)_build_cmds
  ./b2 --ignore-site-config -d2 -j2 -d1 --prefix=$($(package)_staging_prefix_dir) $($(package)_config_opts) stage
endef

define $(package)_stage_cmds
  ./b2 --ignore-site-config -d0 -j4 --prefix=$($(package)_staging_prefix_dir) $($(package)_config_opts) install
endef
