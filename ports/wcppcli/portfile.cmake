vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO wkqco33/wcppcli
    REF b4e608d0168a2a46170e2a7d683cb9ed1fd9f1ca
    SHA512 61dfb12aa1663656b743c531464f4e1e0de4281d521913bd68689e63ab791541bc64705c81364bd5b23dc37557271343d2315827a635490bf4b97a1edcde5bb9
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
