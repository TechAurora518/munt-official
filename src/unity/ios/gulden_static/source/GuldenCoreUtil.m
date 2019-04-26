//
//  GuldenCoreUtil.m
//  GuldenCore
//
//  Created by Willem de Jonge on 03/04/2019.
//  Copyright © 2019 Gulden.com BV. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "GuldenCore/DBGuldenUnifiedBackend.h"

NSString* GuldenCoreStaticFilterPath(BOOL testnet)
{
    NSBundle* fwBundle = [NSBundle bundleForClass:[DBGuldenUnifiedBackend class]];
    NSString* filterFile = @"staticfiltercp";
    if (testnet)
        filterFile = [filterFile stringByAppendingString:@"testnet"];
    NSString* filterPath = [fwBundle pathForResource:filterFile ofType:NULL];
    return filterPath;
}
