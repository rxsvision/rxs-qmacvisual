#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(PHOTOMETRICSTERO_LIB)
#  define PHOTOMETRICSTERO_EXPORT Q_DECL_EXPORT
# else
#  define PHOTOMETRICSTERO_EXPORT Q_DECL_IMPORT
# endif
#else
# define PHOTOMETRICSTERO_EXPORT
#endif
