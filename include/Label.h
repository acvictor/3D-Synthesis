#pragma once

#include <map>
#include <string>

#include "Colour.h"

std::map<std::string, Colour> labels = {
    {"unlabeled", Colour(0, 0, 0)},
    {"ego vehicle", Colour(0, 0, 0)},
    {"rectification border",  Colour(0, 0, 0)},
    {"out of roi",  Colour(0, 0, 0)},
    {"static", Colour(0, 0, 0)},
    {"dynamic", Colour(111, 74, 0)},
    {"ground", Colour(81, 0, 81)},
    {"road", Colour(128, 64, 128)},
    {"sidewalk", Colour(244, 35, 232)},
    {"parking", Colour(250, 170, 160)},
    {"rail track", Colour(230, 150, 140)},
    {"building", Colour(70, 70, 70)},
    {"wall", Colour(102, 102,156)},
    {"fence", Colour(190, 153, 153)},
    {"guard rail", Colour(180, 165,180)},
    {"bridge", Colour(150, 100, 100)},
    {"tunnel", Colour(150, 120, 90)},
    {"pole", Colour(153, 153, 153)},
    {"polegroup", Colour(153, 153, 153)},  
    {"traffic light", Colour(250, 170, 30)},
    {"traffic sign", Colour(220, 220, 0)},
    {"vegetation", Colour(107, 142, 35)},
    {"terrain", Colour(152, 251, 152)},
    {"sky", Colour(70, 130, 180) },
    {"person", Colour(220, 20, 60)},
    {"rider", Colour(255, 0, 0)},
    {"car", Colour(0, 0, 142)},
    {"truck", Colour(0, 0, 70)},
    {"bus", Colour(0, 60, 100) },  
    {"caravan", Colour(0, 0, 90)},
    {"trailer", Colour(0, 0, 110)},
    {"train", Colour(0, 80, 100)},
    {"motorcycle", Colour(0, 0, 230)},
    {"bicycle", Colour(119, 11, 32)},
    {"license plate", Colour(0, 0, 142)} 
};