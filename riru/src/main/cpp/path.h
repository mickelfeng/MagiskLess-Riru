#pragma once

#include <functional>

namespace path {

    const char* GetPath();

    void SetPath(const char *p);

    std::string GetPathForSelf(const char *name);

    std::string GetPathForSelfLib(const char *name);
}
