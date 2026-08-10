#include <stdio.h>
#include <string.h>

// Mock Android Bionic declarations
extern int __android_log_print(int prio, const char *tag, const char *fmt, ...);
extern int __system_property_get(const char *name, char *value);

int main() {
    __android_log_print(4, "Maarch64AndroidApp", "Hello from Linux-Native Android NDK Runtime!");
    
    char prop_val[128] = {0};
    __system_property_get("ro.build.version.sdk", prop_val);
    __android_log_print(4, "Maarch64AndroidApp", "Queried SDK Version: %s", prop_val);
    
    __system_property_get("ro.product.model", prop_val);
    __android_log_print(4, "Maarch64AndroidApp", "Device Model: %s", prop_val);
    
    return 0;
}
