#import <Foundation/Foundation.h>

void* chromix_context_new() {
    return [[NSAutoreleasePool alloc] init];
}

void chromix_context_close(void* pool) {
    [(NSAutoreleasePool*)pool drain];
}
