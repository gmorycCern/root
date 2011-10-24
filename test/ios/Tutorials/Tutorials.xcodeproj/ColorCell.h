//
//  ColorCell.h
//  Tutorials
//
//  Created by Timur Pocheptsov on 8/11/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

//
//This is a small and simple custom view, used inside color picker (UIPickerView).
//

@interface ColorCell : UIView {
   float rgb[3];
}

- (id) initWithFrame : (CGRect) frame;
- (void) dealloc;

- (void) setRGB : (const double *) rgb;
- (void) drawRect : (CGRect) rect;

@end