//
//  ViewController.m
//  OCExtension
//
//  Created by stduser on 15/11/23.
//  Copyright © 2015年 SVS. All rights reserved.
//

#import "ViewController.h"
#import "OCExtension.h"

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    if (is_debugging()) {
        self.view.backgroundColor = [UIColor redColor];
    }
}

@end
