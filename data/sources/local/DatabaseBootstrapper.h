#pragma once

namespace Data {
// One-shot database bootstrap to be called during app startup.
struct DatabaseBootstrapper {
    static void initialize();
};
}