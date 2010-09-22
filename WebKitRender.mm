#import <Foundation/Foundation.h>
#include <public/WebView.h>


using namespace WebKit;

int main (int argc, const char * argv[]) {
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

    WebView *webView = WebView::create(NULL, NULL);
    
    [pool drain];
    return 0;
}
