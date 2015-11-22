//
//  main.m
//  OCExtension
//
//  Created by stduser on 15/11/23.
//  Copyright © 2015年 SVS. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "AppDelegate.h"
#import "OCExtension.h"

int main(int argc, char * argv[]) {
    @autoreleasepool {

        int *p = (int *)0x1234;
        NSLog(@"%p is %s", p, is_memory_readable(p, sizeof(*p)) ? "readable" : "unreadable");
        NSLog(@"%p is %s", p, is_memory_writable(p, sizeof(*p)) ? "writable" : "unwritable");

        int n = 1234;
        p = &n;
        NSLog(@"%p is %s", p, is_memory_readable(p, sizeof(*p)) ? "readable" : "unreadable");
        NSLog(@"%p is %s", p, is_memory_writable(p, sizeof(*p)) ? "writable" : "unwritable");

        const int c = 1234;
        p = (int *)&c;
        NSLog(@"%p is %s", p, is_memory_readable(p, sizeof(*p)) ? "readable" : "unreadable");
        NSLog(@"%p is %s", p, is_memory_writable(p, sizeof(*p)) ? "writable" : "unwritable");

        const char *s = "1234";
        p = (int *)s;
        NSLog(@"%p is %s", p, is_memory_readable(p, sizeof(*p)) ? "readable" : "unreadable");
        NSLog(@"%p is %s", p, is_memory_writable(p, sizeof(*p)) ? "writable" : "unwritable");

        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}
