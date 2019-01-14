#import "FrontmostApplicationController.h"
#import "libkrbn.h"
#import <pqrs/weakify.h>

@interface FrontmostApplicationController ()

@property(unsafe_unretained) IBOutlet NSTextView* textView;

- (void)callback:(NSString*)bundleIdentifier
        filePath:(NSString*)filePath;

@end

static void staticCallback(const char* bundle_identifier,
                           const char* file_path,
                           void* context) {
  FrontmostApplicationController* p = (__bridge FrontmostApplicationController*)(context);
  [p callback:[NSString stringWithUTF8String:bundle_identifier]
      filePath:[NSString stringWithUTF8String:file_path]];
}

@implementation FrontmostApplicationController

- (void)setup {
  libkrbn_enable_frontmost_application_monitor(staticCallback,
                                               (__bridge void*)(self));
}

- (void)dealloc {
  libkrbn_disable_frontmost_application_monitor();
}

- (void)callback:(NSString*)bundleIdentifier filePath:(NSString*)filePath {
  if ([@"org.pqrs.Karabiner.EventViewer" isEqualToString:bundleIdentifier] ||
      [@"org.pqrs.Karabiner-EventViewer" isEqualToString:bundleIdentifier]) {
    return;
  }

  @weakify(self);
  dispatch_async(dispatch_get_main_queue(), ^{
    @strongify(self);
    if (!self) {
      return;
    }

    NSTextStorage* textStorage = self.textView.textStorage;

    [textStorage beginEditing];

    // Clear if text is huge.
    if (textStorage.length > 1024 * 1024) {
      [textStorage setAttributedString:[[NSAttributedString alloc] initWithString:@""]];
    }

    NSString* bundleIdentifierLine = [NSString stringWithFormat:@"Bundle Identifier:  %@\n", bundleIdentifier];
    NSString* filePathLine = [NSString stringWithFormat:@"File Path:          %@\n\n", filePath];
    NSFont* font = [NSFont fontWithName:@"Menlo" size:11];
    [textStorage appendAttributedString:[[NSAttributedString alloc] initWithString:bundleIdentifierLine
                                                                        attributes:@{NSFontAttributeName : font}]];
    [textStorage appendAttributedString:[[NSAttributedString alloc] initWithString:filePathLine
                                                                        attributes:@{NSFontAttributeName : font}]];

    [textStorage endEditing];
    [self.textView scrollRangeToVisible:NSMakeRange(self.textView.string.length, 0)];
  });
}

@end
