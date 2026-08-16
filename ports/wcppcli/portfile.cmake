vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO wkqco33/wcppcli
    REF af36314d12483fe4f965d0e6acd85becc571c9d0
    SHA512 f3269383dde9347f640e0decdd837e4a2fa229cafc5da74e48a5120817acf1730cb6db220d10146bb5cfbb8f7602a6a3a423726c3d6e96d6e134c3b827bc683a
    HEAD_REF master
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DWCPPCLI_BUILD_EXAMPLES=OFF
        -DWCPPCLI_BUILD_TOOL=OFF
        -DWCPPCLI_BUILD_TESTS=OFF
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/wcppcli)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
