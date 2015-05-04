//
//  Styling.cpp
//  TerHell
//
//  Created by Fabian Moron Zirfas on 03.05.15.
//
//

#include "Styling.h"

Styling::Styling(){
    fg = ofColor::fromHex(0xffd600);// yellow #ffd600
    bg = ofColor::fromHex(0x374650); //grey #374650
    logo.loadImage("2_TER-HELL-Plastic-GmbH.gif");
}

ofColor Styling::getBackgroundColor(){
    return bg;
}

ofColor Styling::getForgroundColor(){
    return fg;
}


ofImage Styling::getLogo(){
    return logo;
}