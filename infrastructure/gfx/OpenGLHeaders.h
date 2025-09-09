#pragma once

/*
 * OpenGL Headers Include Guard
 * 
 * This header prevents common OpenGL header conflicts by ensuring proper inclusion order.
 * Include this file instead of directly including OpenGL headers to avoid conflicts.
 * 
 * Common issues this prevents:
 * - "OpenGL header already included" errors with glad
 * - Conflicting OpenGL function definitions
 * - Platform-specific OpenGL header conflicts
 */

#ifndef FANGJIA_OPENGL_HEADERS_H
#define FANGJIA_OPENGL_HEADERS_H

// Prevent Qt OpenGL headers from being included first
#ifndef QT_OPENGL_ES_2
#define QT_OPENGL_ES_2 0
#endif

// Prevent standard OpenGL headers from conflicting with Qt
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1
#endif

// Include Qt OpenGL in the correct order
#include <qopenglfunctions.h>
#include <qopenglshaderprogram.h>
#include <qopenglvertexarrayobject.h>
#include <qopenglbuffer.h>
#include <qopenglframebufferobject.h>

// Note: If using glad, include it after Qt headers:
// #include <glad/glad.h>

// Note: If using stb_image, ensure it's available in include path
// You can download it from: https://github.com/nothings/stb
// Or add it via CMake FetchContent

#endif // FANGJIA_OPENGL_HEADERS_H