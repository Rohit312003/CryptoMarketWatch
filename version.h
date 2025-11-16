/******************************************************************************
 * globals.h
 * Author: Rohit Kumar
 * Contact: rohit312003@gmail.com
 * Date: 15-11-2025
 *
 * Description:
 *   Defines compile-time configuration flags for Debug and Release modes.
 *   Use these macros to wrap logging, diagnostics, or experimental features.
 ******************************************************************************/

#pragma once

// Define one of the following macros to toggle Debug or Release behavior:
// Uncomment for Debug build (extra logging, diagnostics)
//#define __DEBUGMODE__

// Uncomment for Release build (optimized, minimal logging)
//#define __RELEASEMODE__

#ifdef __DEBUGMODE__
// Place code that should ONLY be included in Debug builds below.
// Example: qDebug() << "Debug log: some value = " << value;
#endif

#ifdef __RELEASEMODE__
// Place code that should ONLY be included in Release builds below.
// Example: Disable qDebug(), enable performance-related flags
#endif

/*
 * Usage Example:
 *      #ifdef __DEBUGMODE__
 *          qDebug() << "Debug log: variable x =" << x;
 *      #endif
 *
 *      #ifdef __RELEASEMODE__
 *          // Minimal output, optimizations, etc.
 *      #endif
 */
