#import <Foundation/Foundation.h>

extern int chromix_main(int, const char **);

int main(int argc, const char * argv[]) {
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    int result = chromix_main(argc, argv);
    [pool drain];
    return result;
}
