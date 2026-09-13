vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO wkqco33/wcppcli
    REF 9394f8670229eb5cd68522d220b2790a6620ad2e
    SHA512 42f01ac4c08965ff07fd30b3aa1513416665c3da23b9e325df01df701bd6318ee7cfd849fe84573a9265d994b736f7bfa9f19208ff127ae9064005df42717d6f
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
