// PageCurl fragment shader is based on code from http://labs.calyptus.eu/pagecurl/
// Copyright (c) 2010 Calyptus Life AB <http://calyptus.eu>
// Licensed under The MIT License:
// http://www.opensource.org/licenses/mit-license.php

#version 440
layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;
layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    float time;  // Ranges from 0.0 to 1.0
};
layout(binding = 1) uniform sampler2D sourceTex;
layout(binding = 2) uniform sampler2D targetTex;

const float MIN_AMOUNT = -0.16;
const float MAX_AMOUNT = 1.3;
float amount = time * (MAX_AMOUNT - MIN_AMOUNT) + MIN_AMOUNT;

const float PI = 3.141592653589793;

const float scale = 512.0;
const float sharpness = 3.0;

float cylinderCenter = amount;
// 360 degrees * amount
float cylinderAngle = 2.0 * PI * amount;

const float cylinderRadius = 1.0 / PI / 2.0;

vec3 hitPoint(float hitAngle, float yc, vec3 point, mat3 rrotation) {
    float hitPoint = hitAngle / (2.0 * PI);
    point.y = hitPoint;
    return rrotation * point;
}


vec4 antiAlias(vec4 color1, vec4 color2, float distance) {
    distance *= scale;
    if (distance < 0.0) return color2;
    if (distance > 2.0) return color1;
    float dd = pow(1.0 - distance / 2.0, sharpness);
    return ((color2 - color1) * dd) + color1;
}

float distanceToEdge(vec3 point) {
    float dx = abs(point.x > 0.5 ? 1.0 - point.x : point.x);
    float dy = abs(point.y > 0.5 ? 1.0 - point.y : point.y);
    if (point.x < 0.0) dx = -point.x;
    if (point.x > 1.0) dx = point.x - 1.0;
    if (point.y < 0.0) dy = -point.y;
    if (point.y > 1.0) dy = point.y - 1.0;
    if ((point.x < 0.0 || point.x > 1.0) && (point.y < 0.0 || point.y > 1.0)) return sqrt(dx * dx + dy * dy);
    return min(dx, dy);
}

vec4 seeThrough(float yc, vec2 p, mat3 rotation, mat3 rrotation) {
    float hitAngle = PI - (acos(yc / cylinderRadius) - cylinderAngle);
    vec3 point = hitPoint(hitAngle, yc, rotation * vec3(p, 1.0), rrotation);

    if (yc <= 0.0 && (point.x < 0.0 || point.y < 0.0 || point.x > 1.0 || point.y > 1.0)) {
        return texture(targetTex, qt_TexCoord0);
    }

    if (yc > 0.0) return texture(sourceTex, p);

    vec4 color = texture(sourceTex, point.xy);
    vec4 tcolor = vec4(0.0);

    return antiAlias(color, tcolor, distanceToEdge(point));
}

vec4 seeThroughWithShadow(float yc, vec2 p, vec3 point, mat3 rotation, mat3 rrotation) {
    float shadow = distanceToEdge(point) * 30.0;
    shadow = (1.0 - shadow) / 3.0;
    if (shadow < 0.0) shadow = 0.0;
    else shadow *= amount;

    vec4 shadowColor = seeThrough(yc, p, rotation, rrotation);
    shadowColor.r -= shadow;
    shadowColor.g -= shadow;
    shadowColor.b -= shadow;
    return shadowColor;
}

vec4 backside(float yc, vec3 point) {
    vec4 color = texture(sourceTex, point.xy);
    float gray = (color.r + color.b + color.g) / 15.0;
    gray += (8.0 / 10.0) * (pow(1.0 - abs(yc / cylinderRadius), 2.0 / 10.0) / 2.0 + (5.0 / 10.0));
    color.rgb = vec3(gray);
    return color;
}

vec4 behindSurface(float yc, vec3 point, mat3 rrotation) {
    float shado = (1.0 - ((-cylinderRadius - yc) / amount * 7.0)) / 6.0;
    shado *= 1.0 - abs(point.x - 0.5);

    yc = (-cylinderRadius - cylinderRadius - yc);

    float hitAngle = (acos(yc / cylinderRadius) + cylinderAngle) - PI;
    point = hitPoint(hitAngle, yc, point, rrotation);

    if (yc < 0.0 && point.x >= 0.0 && point.y >= 0.0 && point.x <= 1.0 && point.y <= 1.0 && (hitAngle < PI || amount > 0.5)){
        shado = 1.0 - (sqrt(pow(point.x - 0.5, 2.0) + pow(point.y - 0.5, 2.0)) / (71.0 / 100.0));
        shado *= pow(-yc / cylinderRadius, 3.0);
        shado *= 0.5;
    } else
        shado = 0.0;

    return vec4(texture(targetTex, qt_TexCoord0).rgb - shado, 1.0);
}

void main(void) {
    const float angle = 30.0 * PI / 180.0;
    float c = cos(-angle);
    float s = sin(-angle);

    mat3 rotation = mat3(
        c, s, 0,
        -s, c, 0,
        0.12, 0.258, 1
    );

    c = cos(angle);
    s = sin(angle);

    mat3 rrotation = mat3(
        c, s, 0,
        -s, c, 0,
        0.15, -0.5, 1
    );

    vec3 point = rotation * vec3(qt_TexCoord0, 1.0);

    float yc = point.y - cylinderCenter;

    if (yc < -cylinderRadius) {
        // Behind surface
        fragColor = behindSurface(yc, point, rrotation);
        return;
    }

    if (yc > cylinderRadius) {
        // Flat surface
        fragColor = texture(sourceTex, qt_TexCoord0);
        return;
    }

    float hitAngle = (acos(yc / cylinderRadius) + cylinderAngle) - PI;

    float hitAngleMod = mod(hitAngle, 2.0 * PI);
    if ((hitAngleMod > PI && amount < 0.5) || (hitAngleMod > PI/2.0 && amount < 0.0)) {
        fragColor = seeThrough(yc, qt_TexCoord0, rotation, rrotation);
        return;
    }

    point = hitPoint(hitAngle, yc, point, rrotation);

    if (point.x < 0.0 || point.y < 0.0 || point.x > 1.0 || point.y > 1.0) {
        fragColor = seeThroughWithShadow(yc, qt_TexCoord0, point, rotation, rrotation);
        return;
    }

    vec4 color = backside(yc, point);

    vec4 otherColor;
    if (yc < 0.0) {
        float shado = 1.0 - (sqrt(pow(point.x - 0.5, 2.0) + pow(point.y - 0.5, 2.0)) / 0.71);
        shado *= pow(-yc / cylinderRadius, 3.0);
        shado *= 0.5;
        otherColor = vec4(0.0, 0.0, 0.0, shado);
    } else {
        otherColor = texture(sourceTex, qt_TexCoord0);
    }

    color = antiAlias(color, otherColor, cylinderRadius - abs(yc));

    vec4 cl = seeThroughWithShadow(yc, qt_TexCoord0, point, rotation, rrotation);
    float dist = distanceToEdge(point);

    fragColor = antiAlias(color, cl, dist) * qt_Opacity;
}