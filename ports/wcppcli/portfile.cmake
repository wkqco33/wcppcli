vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO wkqco33/wcppcli
    REF c894c4d8a4abdc91512578905fadf293cbd9397d
    SHA512 b4872b84d0d0f9f4173efd8b2414c04965b9e9ce1b48f962617806d062e037400da4caae9db020b7c8eb3e2259ff4a72b2c01e9fbed2d437b99f9bb4d7823319
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
