#pragma once

#include <cstdlib>
#include <optional>
#include <string>

namespace wcppcli::detail {

    inline std::optional<std::string> read_environment_variable(const char* name) {
        #ifdef _WIN32
            char* value = nullptr;
            size_t size = 0;
            if (_dupenv_s(&value, &size, name) != 0 || value == nullptr) {
                return std::nullopt;
            }
            std::string result(value, size > 0 ? size - 1 : 0);
            std::free(value);
            return result;
        #else
            const char* value = std::getenv(name);
            return value == nullptr ? std::nullopt : std::optional<std::string>(value);
        #endif
    }

} // namespace wcppcli::detail
